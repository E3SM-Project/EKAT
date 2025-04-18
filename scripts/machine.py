from tpb_utils import expect, run_cmd_no_fail, ensure_psutil

import os, sys, pathlib
ensure_psutil()
import psutil

EAMXX_DIR = pathlib.Path(__file__).parent.parent
CIMEROOT = os.path.join(EAMXX_DIR,"..","..","cime")

###############################################################################
def logical_cores_per_physical_core():
###############################################################################
    return psutil.cpu_count() // psutil.cpu_count(logical=False)

###############################################################################
def get_cpu_ids_from_slurm_env_var():
###############################################################################
    """
    Parse the SLURM_CPU_BIND_LIST, and use the hexadecimal value to determine
    which CPUs on this node are assigned to the job
    NOTE: user should check that the var is set BEFORE calling this function
    """

    cpu_bind_list = os.getenv('SLURM_CPU_BIND_LIST')

    expect (cpu_bind_list is not None,
            "SLURM_CPU_BIND_LIST environment variable is not set. Check, before calling this function")

    # Remove the '0x' prefix and convert to an integer
    mask_int = int(cpu_bind_list, 16)

    # Generate the list of CPU IDs
    cpu_ids = []
    for i in range(mask_int.bit_length()):  # Check each bit position
        if mask_int & (1 << i):  # Check if the i-th bit is set
            cpu_ids.append(i)

    return cpu_ids

###############################################################################
def get_available_cpu_count(logical=True):
###############################################################################
    """
    Get number of CPUs available to this process and its children. logical=True
    will include hyperthreads, logical=False will return only physical cores
    """
    if 'SLURM_CPU_BIND_LIST' in os.environ:
        cpu_count = len(get_cpu_ids_from_slurm_env_var())
    else:
        cpu_count = len(psutil.Process().cpu_affinity())

    if not logical:
        hyperthread_ratio = logical_cores_per_physical_core()
        return int(cpu_count / hyperthread_ratio)
    else:
        return cpu_count

###############################################################################
class Machine(object):
###############################################################################
    """
    Parent class for objects describing a machine to use for EAMxx standalone testing.
    """

    def __init__ (self,name,mach_file,num_bld_res=-1,num_run_res=-1,concrete=True):
        self.name = name
        self.mach_file = mach_file
        self.num_bld_res = num_bld_res if num_bld_res > 0 else get_available_cpu_count()
        self.num_run_res = num_run_res if num_run_res > 0 else get_available_cpu_count()
        self.concrete = True

        self.env_setup = []
        self.gpu_arch = "none"
        self.batch = ""
        self.cxx_compiler = "mpicxx"
        self.c_compiler   = "mpicc"
        self.ftn_compiler = "mpifort"
        self.baselines_dir = ""
        self.valg_supp_file = None

    def uses_gpu (self):
        return self.gpu_arch!="none"

###############################################################################
class Generic(Machine):
###############################################################################
    concrete = True

    @classmethod
    def setup(cls):
        super().setup_base("linux-generic")
        # Set baselines_dir to PWD/../ctest-build/baselines
        cls.baselines_dir = os.path.join(os.path.dirname(__file__), '..', 'ctest-build', 'baselines')

###############################################################################
def get_all_machines (base=Machine):
###############################################################################
    # If the user has the file ~/.cime/scream_mach_specs.py, import the machine type Local from it
    if pathlib.Path("~/.cime/scream_mach_specs.py").expanduser().is_file(): # pylint: disable=no-member
        sys.path.append(str(pathlib.Path("~/.cime").expanduser()))
        # Add the directory of this file to sys.path, so that ~/.cime/scream_mach_specs.py,
        # if present, can simply do "from machine_specs import Machine" to define machine Local
        sys.path.append(os.path.dirname(__file__))

        # Pylint does not see that this import adds one subclass to Machine
        # Also, its static analysis runs on a static sys.path, without ~/.cime,
        # so it doesn't find the module
        from scream_mach_specs import Local# pylint: disable=unused-import, import-error

    machines = []
    for m in base.__subclasses__():
        if m.concrete:
            m.setup() # Init the class static data
            machines.append(m)

        machines.extend(get_all_machines(m))

    return machines

###############################################################################
def get_machine (name):
###############################################################################

    all_machines = get_all_machines()

    for m in all_machines:
        if m.name==name:
            return m

    raise ValueError(f"Machine with name '{name}' not found.")

###############################################################################
def get_all_supported_machines ():
###############################################################################
    return [m.name for m in get_all_machines()]

###############################################################################
def is_machine_supported(machine):
###############################################################################
    return machine in get_all_supported_machines()

###############################################################################
def assert_machine_supported(machine):
###############################################################################
    expect(is_machine_supported(machine),
           "Machine {} is not currently supported by scream testing system.\n"
          f" Currently supported machines: {','.join(get_all_supported_machines())}\n"
           " Note: you can also create a file `~/.cime/scream_mach_specs.py` with your local machine specs.".format(machine))

###############################################################################
def get_mach_env_setup_command(machine, ctest_j=None):
###############################################################################
    """
    ctest_j=None -> probe for hardware for testing resources
    ctest_j=-1   -> Skip CTEST_PARALLEL_LEVEL
    """

    mach = get_machine(machine)
    mach_custom_env = mach.env_setup
    if ctest_j != -1:
        ctest_j = mach.num_run_res if ctest_j is None else ctest_j
        mach_custom_env.append("export CTEST_PARALLEL_LEVEL={}".format(ctest_j))

    if not mach.uses_gpu():
        mach_custom_env.append("export OMP_PROC_BIND=spread")

    return mach_custom_env

###############################################################################
def setup_mach_env(machine, ctest_j=None):
###############################################################################
    assert_machine_supported(machine)

    env_setup = get_mach_env_setup_command(machine, ctest_j=ctest_j)

    # Do something only if this machine has env specs
    if env_setup != []:
        # Running the env command only modifies the env in the subprocess
        # But we can return the resulting PATH, and update current env with that

        # Get the whole env string after running the env_setup command
        curr_env = run_cmd_no_fail("{{ {};  }} > /dev/null && env | sort".format(" && ".join(env_setup)),verbose=True)

        # Split by line. We are assuming that each env variable is *exactly* on one line
        curr_env_list = curr_env.split("\n")

        # For each line, split the string at the 1st '='.
        # The resulting length-2 stirng is (ENV_VAR_NAME, ENV_VAR_VALUE);
        # use it to update the os environment
        for item in curr_env_list:
            # On fedora systems, the environment contains the annoying entry (on 2 lines)
            #
            # BASH_FUNC_module()=() {  eval `/usr/bin/modulecmd bash $*`
            # }
            # Which breaks the assumption that each env var is on one line.
            # On some systems, this variable seems to have a different name,
            # and there can potentially be other BASH_FUNC_blah variables.
            # To get around this, discard lines that either do not contain '=',
            # or that start with BASH_FUNC_.
            if item.find("BASH_FUNC_") != -1 or item.find("=") == -1:
                continue

            # 2 means only 1st occurence will cause a split.
            # Just in case some env var value contains '='
            item_list = item.split("=",2)
            os.environ.update( dict( { item_list[0] : item_list[1] } ) )
