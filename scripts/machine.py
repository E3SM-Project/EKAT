from tpb_utils import expect, get_available_cpu_count

###############################################################################
class Machine(object):
###############################################################################
    """
    Parent class for objects describing a machine to use for EAMxx standalone testing.
    """

    def __init__ (self,name,mach_file=None,num_bld_res=-1,num_run_res=-1):
        self.name = name
        self.mach_file = mach_file
        self.num_bld_res = num_bld_res if num_bld_res > 0 else get_available_cpu_count()
        self.num_run_res = num_run_res if num_run_res > 0 else get_available_cpu_count()

        self.env_setup = []
        self.gpu_arch = None
        self.batch = None
        self.cxx_compiler = None
        self.c_compiler   = None
        self.ftn_compiler = None
        self.baselines_dir = None
        self.valg_supp_file = None
        self.baseline_gen_label = "baseline_gen"

        # Projects can dump in this file (relative to cmake build dir) the list of
        # baselines files that need to be copied to the baseline dir. This allows
        # TPB to ensure that ALL baselines tests complete sucessfully before copying
        # any file to the baselines directory
        self.baselines_summary_file = None

    def uses_gpu (self):
        return self.gpu_arch is not None

###############################################################################
class MpiMachine(object):
###############################################################################
    """
    Parent class for objects describing a machine to use for EAMxx standalone testing.
    """

    def __init__ (self,name,mach_file=None,num_bld_res=-1,num_run_res=-1):
        super().__init__(name,mach_file,num_bld_res,num_run_res)

        self.cxx_compiler = "mpicxx"
        self.c_compiler   = "mpicc"
        self.ftn_compiler = "mpifort"
