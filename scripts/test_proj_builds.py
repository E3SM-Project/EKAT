import os

from pathlib    import Path
import concurrent.futures as threading

from machine    import Machine
from build_type import BuildType
from tpb_utils  import module_from_path, objects_from_module, expect
from git_utils  import get_current_ref, get_current_sha

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
        self._baseline_dir  = baseline_dir
        self._cmake_args    = cmake_args
        self._work_dir      = Path(work_dir or os.getcwd()+"/ctest-build").absolute()
        self._verbose       = verbose
        self._no_build      = no_build,
        self._no_run        = no_run,
        self._test_regex    = test_regex
        self._test_labels   = test_labels
        self._root_dir      = Path(root_dir or os.getcwd()).absolute()
        self._config_dir    = config_folder or self._root_dir / "scripts"
        self._machine       = None
        self._builds        = []

        # We print some git sha info (as well as store it in baselines) so make sure we are in a git repo
        expect(get_current_sha() is not None,
               f"Root dir: {self._root_dir}, does not appear to be a git repo")

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
                expect (bt in [b.name for b in avail_builds],
                        f"Could not locate build '{bt}' in {self._config_dir}/project_build_types.py")

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

        if self._baseline_dir:

            if self._baseline_dir == "AUTO":
                self._baseline_dir = Path(self._machine.baselines_dir).expanduser().absolute()
            else:
                self._baseline_dir = Path(self._baseline_dir).expanduser().absolute()

            expect (self._work_dir != self._baseline_dir,
                    f"For your safety, do NOT use the work dir to store baselines. Use a different one (a subdirectory works too).")


        # Make the baseline dir, if not already existing.
        if self._generate:
            expect(self._baseline_dir is not None, "Cannot generate without -b/--baseline-dir")

        ###################################
        #    Set computational resources  #
        ###################################

        if self._parallel:
            # NOTE: we ASSUME that num_run_res>=num_bld_res, which is virtually always true


            # Our way of partitioning the compute node among the different builds only
            # works if the number of bld/run resources is no-less than the number of builds
            expect (self._machine.num_run_res>=len(builds),
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
        git_sha = get_current_sha ()

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

        num_workers = len(self._builds) if self._parallel else 1
        with threading.ProcessPoolExecutor(max_workers=num_workers) as executor:
            fcn = self.generate_baselines if self._generate else self.run_tests
            future_to_build = {
                executor.submit(fcn,build) : build
                for build in self._builds}

            for future in threading.as_completed(future_to_build):
                build = future_to_build[future]
                builds_success[build] = future.result()
                success &= builds_success[build]

        for b,s in builds_success.items():
            if not s:
                last_test   = self.get_last_cbuild_file(t,"TestsFailed")
                last_build  = self.get_last_cbuild_file(t,"Build")
                last_config = self.get_last_cbuild_file(t,"Configure")
                if last_test is not None:
                    print(f"Build type {b} failed at building time. Here's a list of failed builds:")
                    print (last_test.read_text())
                elif last_build is not None:
                    print(f"Build type {b} failed at build time. Here's the build log:")
                    print (last_build.read_text())
                elif last_config is not None:
                    print(f"Build type {b} failed at config time. Here's the config log:\n\n")
                    print (last_config.read_text())
                else:
                    print(f"Build type {b} failed before configure step.")

        return success

    ###############################################################################
    def generate_baselines(self,build):
    ###############################################################################

        expect(build.uses_baselines,
               f"Something is off. generate_baseline should have not be called for build {build}")

        baseline_dir = self._baseline_dir / build.longname
        test_dir     = self._work_dir     / build.longname

        if test_dir.exists():
            shutil.rmtree(test_dir)
        test_dir.mkdir()

        num_test_res = self.create_ctest_resource_file(test,test_dir)
        cmake_config = self.generate_cmake_config(test)
        cmake_config +=  " -DSCREAM_ONLY_GENERATE_BASELINES=ON"
        cmake_config += f" -DSCREAM_BASELINES_DIR={baseline_dir}"
        cmake_config += f" -DSCREAM_TEST_MAX_TOTAL_THREADS={num_test_res}"

        print("===============================================================================")
        print(f"Generating baseline for build {build}, with cmake config '{cmake_config}'")
        print("===============================================================================")

        # We cannot just crash if we fail to generate baselines, since we would
        # not get a dashboard report if we did that. Instead, just ensure there is
        # no baseline file to compare against if there's a problem.
        stat, _, err = run_cmd(f"{cmake_config} {self._root_dir}",
                               from_dir=test_dir, verbose=True)
        if stat != 0:
            print (f"WARNING: Failed to create baselines (config phase):\n{err}")
            return False

        cmd = f"make -j{build.compile_res_count}"
        if self._parallel:
            resources = self.get_taskset_resources(build, for_compile)
            cmd = f"taskset -c {','.join([str(r) for r in resources])} sh -c '{cmd}'"

        stat, _, err = run_cmd(cmd, from_dir=test_dir, verbose=True)

        if stat != 0:
            print (f"WARNING: Failed to create baselines (build phase):\n{err}")
            return False

        cmd  = f"ctest -j{build.testing_res_count}"
        cmd +=  " -L baseline_gen"
        cmd += f" --resource-spec-file {test_dir}/ctest_resource_file.json"
        stat, _, err = run_cmd(cmd, from_dir=test_dir, verbose=True)

        if stat != 0:
            print (f"WARNING: Failed to create baselines (run phase):\n{err}")
            return False

        # Read list of nc files to copy to baseline dir
        with open(test_dir/"data/baseline_list","r",encoding="utf-8") as fd:
            files = fd.read().splitlines()

            with SharedArea():
                for fn in files:
                    # In case appending to the file leaves an empty line at the end
                    if fn != "":
                        src = Path(fn)
                        dst = baseline_dir / "data" / src.name
                        shutil.copyfile(src, dst)

        # Store the sha used for baselines generation. This is only for record
        # keeping.
        self.set_baseline_file_sha(build)
        build.baselines_missing = False

        # Clean up the directory by removing everything
        shutil.rmtree(test_dir)

        return True

    ###############################################################################
    def run_tests(self, build):
    ###############################################################################

        test_dir = self.get_test_dir(self._work_dir,build)
        cmake_config = self.generate_cmake_config(build, for_ctest=True)
        ctest_config = self.generate_ctest_config(cmake_config, [], build)

        print("===============================================================================")
        print(f"Running tests for build {build}, with cmake config '{cmake_config}'")
        print("===============================================================================")

        if self._config_only:
            ctest_config += "-DCONFIG_ONLY=TRUE"

        if self._quick_rerun and (test_dir/"CMakeCache.txt").is_file():
            # Do not purge bld dir, and do not rerun config step.
            # Note: make will still rerun cmake if some cmake file has changed
            if self._quick_rerun_failed:
                ctest_config += "--rerun-failed "
        else:
            # This directory might have been used before during another test-all-eamxx run.
            # Although it's ok to build in the same dir, we MUST make sure to erase cmake's cache
            # and internal files from the previous build (CMakeCache.txt and CMakeFiles folder),
            # Otherwise, we may not pick up changes in certain cmake vars that are already cached.
            run_cmd_no_fail("rm -rf CMake*", from_dir=test_dir)

        success = run_cmd(ctest_config, from_dir=test_dir, arg_stdout=None, arg_stderr=None, verbose=True)[0] == 0

        return success

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

        resources = self.get_taskset_resources(test, for_compile=False)

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

