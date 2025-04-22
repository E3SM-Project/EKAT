import pathlib

from tpb_utils import expect, get_available_cpu_count

###############################################################################
class Machine(object):
###############################################################################
    """
    Parent class for objects describing a machine to use for EAMxx standalone testing.
    """

    def __init__ (self,name,machines_specs):
        # Check inputs
        expect (isinstance(machines_specs,dict),
                f"Machine constructor expects a dict object for 'machines_specs' (got {type(machines_specs)} instead).\n")
        expect (name in machines_specs.keys(),
                f"Machine '{name}' not found in the 'machines' section of the config file.\n"
                f" - available machines: {','.join(m for m in machines_specs.keys() if m!="default")}\n")

        # Get props for this machine and for a default machine
        props   = machines_specs[name]
        default = machines_specs.get('default',{})

        # Set mach props
        self.name = name
        self.mach_file = props.get('mach_file',None) or default.get('mach_file',None)
        self.num_bld_res = props.get('num_bld_res',None) or default.get('num_bld_res',None) or get_available_cpu_count()
        self.num_run_res = props.get('num_run_res',None) or default.get('num_run_res',None) or get_available_cpu_count()
        self.env_setup = props.get("env_setup",[])
        self.gpu_arch = props.get("gpu_arch",None)
        self.batch = props.get("batch",None)
        self.cxx_compiler = props.get("cxx_compiler",None) or default.get("cxx_compiler",None)
        self.c_compiler   = props.get("c_compiler",None) or default.get("c_compiler",None)
        self.ftn_compiler = props.get("ftn_compiler",None) or default.get("ftn_compiler",None)
        self.baselines_dir = props.get("baselines_dir",None)
        self.valg_supp_file = props.get("valg_supp_file",None)

        # Check props are valid
        print(f"mach file: {self.mach_file}")
        print(f"mach file exists: {pathlib.Path(self.mach_file).expanduser().exists()}")
        expect (self.mach_file is None or pathlib.Path(self.mach_file).expanduser().exists(),
                f"Invalid/non-existent machine file '{self.mach_file}'")
        expect (isinstance(self.env_setup,list),
                f"machine->env_setup should be a list of strings (got {type(self.env_setup)} instead).\n")

        try:
            self.num_bld_res = int(props.get('num_bld_res',None) or get_available_cpu_count())
        except ValueError as e:
            print(f"Cannot convert 'num_bld_res' entry to an integer. Please, fix the config file.\n")
            raise
        try:
            self.num_run_res = int(props.get('num_run_res',None) or get_available_cpu_count())
        except ValueError as e:
            print(f"Cannot convert 'num_run_res' entry to an integer. Please, fix the config file.\n")
            raise

    def uses_gpu (self):
        return self.gpu_arch is not None
