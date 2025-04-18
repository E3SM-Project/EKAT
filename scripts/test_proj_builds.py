import os
import pathlib
import threading
import shutil
import psutil
import json
import re
import itertools

from machine    import Machine
from build_type import BuildType
from tpb_utils  import module_from_path, objects_from_module, expect, run_cmd
from git_utils  import get_current_ref, get_current_sha, is_git_repo

###############################################################################
class TestProjBuilds(object):
###############################################################################

    ###########################################################################
    def __init__(self, config_folder=None, submit=False, parallel=False, generate=False,
                 baseline_dir=None, machine_name=None,
                 cmake_args=None, build_types=None,
                 work_dir=None, root_dir=None, no_build=False, no_run=False,
                 verbose=False, test_regex=None, test_labels=None):
    ###########################################################################

        self._submit        = submit
        self._parallel      = parallel
        self._generate      = generate
        self._baselines_dir  = baseline_dir
        self._cmake_args    = cmake_args
        self._work_dir      = pathlib.Path(work_dir or os.getcwd()+"/ctest-build").absolute()
        self._verbose       = verbose
        self._no_build      = no_build
        self._no_run        = no_run
        self._test_regex    = test_regex
        self._test_labels   = test_labels
        self._root_dir      = pathlib.Path(root_dir or os.getcwd()).absolute()
        self._config_dir    = config_folder or self._root_dir / "scripts"
        self._machine       = None
        self._builds        = []

        if not self._work_dir.exists():
            self._work_dir.mkdir()

        # We print some git sha info (as well as store it in baselines) so make sure we are in a git repo
        expect(is_git_repo(self._root_dir),
               f"Root dir: {self._root_dir}, does not appear to be a git repo")

        # If we submit, we must a) not be generating, and b) be able to find the CTestConfig.cmake script in the root dir
        if self._submit:
            expect (not self._generate,
                    "Cannot submit to cdash when generating baselines. Re-run without -g.")

            ctest_config = self._root_dir / "CTestConfig.cmake"
            expect (ctest_config.exists(),
                    f"Cannot submit to cdash. CTestConfig.cmake was not found in root_dir={self._root_dir}")

        #########################
        #  Set the machine obj  #
        #########################

        mach_module    = module_from_path(self._config_dir / "project_machines.py","project_machines")
        avail_machines = objects_from_module(mach_module,Machine)

        for m in avail_machines:
            if m.name == machine_name:
                self._machine = m

        expect (self._machine is not None,
                f"Could not locate machine '{machine_name}' in {self._config_dir}/project_machines.py")

        #########################
        #  Set the build types  #
        #########################

        builds_module = module_from_path(self._config_dir / "project_build_types.py","project_build_types")
        avail_builds  = objects_from_module(builds_module,BuildType,self._machine)

        if build_types:
            for bt in build_types:
                expect (bt in [b.shortname for b in avail_builds],
                        "Could not locate build\n"
                        f"  - requested build: '{bt}'\n"
                        f"  - build types module: {self._config_dir}/project_build_types.py\n"
                        f"  - available builds: '{','.join(b.shortname for b in avail_builds)}'")

            for b in avail_builds:
                if b.shortname in build_types:
                    if not self._generate or b.uses_baselines:
                        self._builds.append(b)
                    else:
                        print(f"Discarding build '{b.longname}, since it does not use baselines and -g was used")
        else:
            # Build all builds that are "on by default"
            for b in avail_builds:
                if b.on_by_default:
                    # When generating, don't consider builds that don't use baselines
                    if not self._generate or b.uses_baselines:
                        self._builds.append(b)

        ###################################
        #      Compute baseline info      #
        ###################################

        if self._baselines_dir:
            if self._baselines_dir == "AUTO":
                self._baselines_dir = pathlib.Path(self._machine.baselines_dir).expanduser().absolute()
            else:
                self._baselines_dir = pathlib.Path(self._baselines_dir).expanduser().absolute()

            expect (self._work_dir != self._baselines_dir,
                    f"For your safety, do NOT use the work dir to store baselines. Use a different one (a subdirectory works too).")

            if not self._generate:
                self.check_baselines_are_present()

        # Make the baseline dir, if not already existing.
        if self._generate:
            expect(self._baselines_dir is not None, "Cannot generate without -b/--baseline-dir")

        ###################################
        #    Set computational resources  #
        ###################################

        if self._parallel:
            # NOTE: we ASSUME that num_run_res>=num_bld_res, which is virtually always true


            # Our way of partitioning the compute node among the different builds only
            # works if the number of bld/run resources is no-less than the number of builds
            expect (self._machine.num_run_res>=len(self._builds),
                    "Cannot process build types in parallel, since we don't have enough resources.\n"
                    f" - build types: {','.join(b.shortname for b in self._builds)}\n"
                    f" - num run res: {self._machine.num_run_res}")

            num_bld_res_left = self._machine.num_bld_res
            num_run_res_left = self._machine.num_run_res

            for i,test in enumerate(self._builds):
                num_left = len(self._builds)-i
                test.testing_res_count = num_bld_res_left // num_left
                test.compile_res_count = num_run_res_left // num_left

                num_bld_res_left -= test.compile_res_count;
                num_run_res_left -= test.testing_res_count;
        else:
            # We can use all the res on the node
            for test in self._builds:
                test.testing_res_count = self._machine.num_run_res
                test.compile_res_count = self._machine.num_bld_res

    ###############################################################################
    def run(self):
    ###############################################################################

        git_ref = get_current_ref ()
        git_sha = get_current_sha (short=True)

        print("###############################################################################")
        if self._generate:
            print(f"Generating baselines from git ref '{git_ref}' (sha={git_sha})")
        else:
            print(f"Running tests for git ref '{git_ref}' (sha={git_sha})")
        print("###############################################################################")

        success = True
        builds_success = {
            build : False
            for build in self._builds}

        mach_module   = module_from_path(self._config_dir / "project_machines.py","project_machines")
        builds_module = module_from_path(self._config_dir / "project_build_types.py","project_build_types")

        num_workers = len(self._builds) if self._parallel else 1

        build_success = {}
        def run_thread (build):
            if self._generate:
                result = self.generate_baselines(build)
            else:
                result = self.run_tests(build)
            builds_success[build] = result

        threads = []
        for build in self._builds:
            thread = threading.Thread(target=run_thread,args=[build])
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

        success = True
        for b,s in builds_success.items():
            if not s:
                success = False
                last_test   = self.get_phase_log(b,"TestsFailed")
                last_build  = self.get_phase_log(b,"Build")
                last_config = self.get_phase_log(b,"Configure")
                if last_test is not None:
                    print(f"Build type {b} failed at testing time. Here's the list of failed tests:")
                    print (last_test.read_text())
                elif last_build is not None:
                    print(f"Build type {b} failed at build time. Here's the build log:")
                    print (last_build.read_text())
                elif last_config is not None:
                    print(f"Build type {b} failed at config time. Here's the config log:\n\n")
                    print (last_config.read_text())
                else:
                    print(f"Build type {b} failed at an unknown stage (likely before configure step).")

        return success

    ###############################################################################
    def generate_baselines(self,build):
    ###############################################################################

        expect(build.uses_baselines,
               f"Something is off. generate_baseline should have not be called for build {build}")

        baseline_dir = self._baselines_dir / build.longname
        build_dir    = self._work_dir     / build.longname

        if build_dir.exists():
            shutil.rmtree(build_dir)
        build_dir.mkdir(parents=True)

        num_test_res = self.create_ctest_resource_file(build,build_dir)
        cmake_config = self.generate_cmake_config(build)

        print("===============================================================================")
        print(f"Generating baseline for build {build.longname}")
        print(f"  cmake config: {cmake_config}")
        print("===============================================================================")

        # Create the Testing/Temporary folder
        logs_dir = build_dir / "Testing/Temporary"
        logs_dir.mkdir(parents=True)

        # If non-empty, run these env setup cmds BEFORE running any command
        env_setup = " && ".join(self._machine.env_setup)

        # We cannot just crash if we fail to generate baselines, since we would
        # not get a dashboard report if we did that. Instead, just ensure there is
        # no baseline file to compare against if there's a problem.
        log_file = f"{logs_dir}/LastConfig.log"
        cmd = f"{cmake_config}"
        stat, _, err = run_cmd(cmd,output_file=log_file,combine_output=True,
                               env_setup=env_setup,from_dir=build_dir, verbose=True)
        if stat != 0:
            print (f"WARNING: Failed to create baselines (config phase):\n{err}")
            return False

        if self._no_build:
            print("  - Skipping build/test phase, since --no-build was used")
            return True

        cmd = f"make -j{build.compile_res_count}"
        if self._parallel:
            resources = self.get_taskset_resources(build, for_compile)
            cmd = f"taskset -c {','.join([str(r) for r in resources])} sh -c '{cmd}'"

        log_file = f"{logs_dir}/LastBuild.log"
        stat, _, err = run_cmd(cmd, output_file=log_file,combine_output=True,
                               env_setup=env_setup, from_dir=build_dir, verbose=True)

        if stat != 0:
            print (f"WARNING: Failed to create baselines (build phase):\n{err}")
            return False

        if self._no_run:
            print("  - Skipping test phase, since --no-build was used")
            return True

        cmd  = f"ctest -j{build.testing_res_count}"
        cmd += f" -L {self._machine.baseline_gen_label}"
        cmd += f" --resource-spec-file {build_dir}/ctest_resource_file.json"
        stat, _, err = run_cmd(cmd, output_to_screen=True,combine_output=True,
                               env_setup=env_setup, from_dir=build_dir, verbose=True)

        if stat != 0:
            print (f"WARNING: Failed to create baselines (run phase):\n{err}")
            return False

        # Read list of nc files to copy to baseline dir
        if self._machine.baselines_summary_file is not None:
            with open(build_dir/self._machine.baselines_summary_file,"r",encoding="utf-8") as fd:
                files = fd.read().splitlines()

                with SharedArea():
                    for fn in files:
                        # In case appending to the file leaves an empty line at the end
                        if fn != "":
                            src = pathlib.Path(fn)
                            dst = baseline_dir / "data" / src.name
                            shutil.copyfile(src, dst)

        # Store the sha used for baselines generation. This is only for record keeping.
        baseline_file = baseline_dir / "baseline_git_sha"
        with baseline_file.open("w", encoding="utf-8") as fd:
            sha = get_current_commit()
            return fd.write(sha)
        build.baselines_missing = False

        return True

    ###############################################################################
    def run_tests(self, build):
    ###############################################################################

        build_dir = self.get_test_dir(self._work_dir,build)
        cmake_config = self.generate_cmake_config(build)

        print("===============================================================================")
        print(f"Running tests for build {build.longname}")
        print(f"  cmake config: {cmake_config}")
        print("===============================================================================")

        # If non-empty, run these env setup cmds BEFORE running any command
        env_setup = " && ".join(self._machine.env_setup)

        stat, _, err = run_cmd(f"{cmake_config}",env_setup=env_setup,
                               from_dir=build_dir,verbose=self._verbose)
        if stat != 0:
            print (f"WARNING: Failed to run tests (config phase):\n{err}")
            return False

        if self._no_build:
            print("  - Skipping build/test phase, since --no-build was used")
            return True

        cmd = f"make -j{build.compile_res_count}"
        if self._parallel:
            resources = self.get_taskset_resources(build, for_compile)
            cmd = f"taskset -c {','.join([str(r) for r in resources])} sh -c '{cmd}'"

        stat, _, err = run_cmd(cmd, env_setup=env_setup, from_dir=build_dir, verbose=True)

        if stat != 0:
            print (f"WARNING: Failed to run tests (build phase):\n{err}")
            return False

        if self._no_run:
            print("  - Skipping test phase, since --no-build was used")
            return True

        cmd  = f"ctest -j{build.testing_res_count}"
        cmd += f" --resource-spec-file {build_dir}/ctest_resource_file.json"
        if self._test_regex:
            cmd += f" -R {self._test_regex}"
        if self._test_labels:
            cmd += f" -L {self._test_labels}"

        stat, _, err = run_cmd(cmd, env_setup=env_setup, from_dir=build_dir, verbose=True)

        if self._submit:
            run_cmd(f"ctest -S {self._root_dir}/CTestConfig.cmake --submit",
                    from_dir=self._root_dir,verbose=True)

        if stat != 0:
            print (f"WARNING: Failed to run tests (run phase):\n{err}")
            return False

        return True

    ###############################################################################
    def create_ctest_resource_file(self, build, build_dir):
    ###############################################################################
        # Create a json file in the build dir, which ctest will then use
        # to schedule tests in parallel.
        # In the resource file, we have N res groups with 1 slot, with N being
        # what's in build.testing_res_count. On CPU machines, res groups
        # are cores, on GPU machines, res groups are GPUs. In other words, a
        # res group is where we usually bind an individual MPI rank.
        # The id of the res groups is offset-ed so that it is unique across all builds

        resources = self.get_taskset_resources(build, for_compile=False)

        data = {}

        # This is the only version numbering supported by ctest, so far
        data["version"] = {"major":1,"minor":0}

        # We add leading zeroes to ensure that ids will sort correctly
        # both alphabetically and numerically
        devices = []
        for res_id in resources:
            devices.append({"id":f"{res_id:05d}"})

        # Add resource groups
        data["local"] = [{"devices":devices}]

        with (build_dir/"ctest_resource_file.json").open("w", encoding="utf-8") as outfile:
            json.dump(data,outfile,indent=2)

        return len(resources)

    ###############################################################################
    def get_taskset_resources(self, build, for_compile):
    ###############################################################################
        res_name = "compile_res_count" if for_compile else "testing_res_count"

        if not for_compile and self._machine.uses_gpu():
            # For GPUs, the cpu affinity is irrelevant. Just assume all GPUS are open
            affinity_cp = list(range(self._machine.num_run_res))
        elif "SLURM_CPU_BIND_LIST" in os.environ:
            affinity_cp = get_cpu_ids_from_slurm_env_var()
        else:
            this_process = psutil.Process()
            affinity_cp = list(this_process.cpu_affinity())

        affinity_cp.sort()

        if self._parallel:
            it = itertools.takewhile(lambda item: item != build, self._builds)
            offset = sum(getattr(prevs, res_name) for prevs in it)
        else:
            offset = 0

        expect(offset < len(affinity_cp),
               f"Offset {offset} out of bounds (max={len(affinity_cp)}) for build {build}\naffinity_cp: {affinity_cp}")
        resources = []
        for i in range(0, getattr(build, res_name)):
            resources.append(affinity_cp[offset+i])

        return resources

    ###############################################################################
    def get_phase_log(self,build,phase):
    ###############################################################################
        build_dir = self._work_dir / build.longname
        ctest_results_dir = pathlib.Path(build_dir,"Testing","Temporary")
        log_filename = f"Last{phase}.log"
        files = list(ctest_results_dir.glob(log_filename))
        expect(len(files)==1,
                 "Found zero or multiple log files:\n"
                f"  - build: {build.longname}\n"
                f"  - build dir: {build_dir}\n"
                f"  - log file name: {log_filename}\n"
                f"  - files found: [{','.join(f.name for f in files)}]")

        return files[0]

    ###############################################################################
    def generate_cmake_config(self, build):
    ###############################################################################

        # Ctest only needs config options, and doesn't need the leading 'cmake '
        result  = "cmake"
        if self._machine.mach_file is not None:
            result += f" -C {self._machine.mach_file}"

        # Build-specific cmake options
        for key, value in build.cmake_args:
            result += f" -D{key}={value}"

        # Compilers
        if self._machine.cxx_compiler is not None:
            result += f" -DCMAKE_CXX_COMPILER={self._machine.cxx_compiler}"
        if self._machine.c_compiler is not None:
            result += f" -DCMAKE_C_COMPILER={self._machine.c_compiler}"
        if self._machine.ftn_compiler is not None:
            result += f" -DCMAKE_Fortran_COMPILER={self._machine.ftn_compiler}"

        # User-requested config options
        for arg in self._cmake_args:
            expect ("=" in arg,
                    f"Invalid value for -c/--cmake-args: {arg}. Should be `VAR_NAME=VALUE`.")

            name, value = arg.split("=", 1)
            # Some effort is needed to ensure quotes are perserved
            result += f" -D{name}='{value}'"

        result += f" -S {self._root_dir}"

        return result

    ###############################################################################
    def check_baselines_are_present(self):
    ###############################################################################
        """
        Check that all baselines are present for the build types that use baselines
        """

        # Sanity check (should check this before calling this fcn)
        expect(self._baselines_dir is not None,
                "Error! Baseline directory not correctly set.")

        print (f"Checking baselines directory: {self._baselines_dir}")
        missing = []
        for build in self._builds:
            if build.uses_baselines:
                data_dir = self._baselines_dir / build.longname / "data"
                if not data_dir.is_dir():
                    build.baselines_missing = True
                    missing.append(build.longname)
                    print(f" -> Build {build.longname} is missing baselines")
                else:
                    print(f" -> Build {build.longname} appears to have baselines")
            else:
                print(f" -> Build {build.longname} does not use baselines")

        expect (len(missing)==0,
                f"Re-run with -g to generate missing baselines for builds {missing}")
