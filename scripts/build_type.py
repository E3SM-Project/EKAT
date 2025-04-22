import re

from tpb_utils import expect

###############################################################################
class BuildType(object):
###############################################################################
    """
    Class of predefined build types for the project.
    The script 'test-proj-build' will query this object for runtime info on the build
    """

    def __init__(self, shortname, project, machine, builds_specs):
        # Check inputs
        expect (isinstance(builds_specs,dict),
                f"BuildType constructor expects a dict object for 'builds_specs' (got {type(builds_specs)} instead).\n")
        expect (shortname in builds_specs.keys(),
                f"BuildType '{shortname}' not found in the 'build_types' section of the config file.\n"
                f" - available build types: {','.join(b for b in builds_specs.keys() if b!="default")}\n")

        # Get props for this build type and for a default build
        props   = builds_specs[shortname]
        default = builds_specs.get('default',{})

        # Set build props
        self.shortname   = shortname
        self.longname    = props.get('longname',shortname)
        self.description = props.get('description',None)
        self.uses_baselines = props.get('uses_baselines',None) or default.get('uses_baselines',True)
        self.on_by_default  = props.get('on_by_default',None) or default.get('on_by_default',True)

        cmake_args         = props.get('cmake_args',[])   # Store in temps, since we need
        default_cmake_args = default.get('cmake_args',[]) # to combine these two after validating

        # Validate props
        expect (isinstance(self.uses_baselines,bool),
                "Invalid value for uses_baselines.\n"
                f"  - build name: {shortname}\n"
                f"  - input value: {self.uses_baselines}\n"
                f"  - input type: {type(self.uses_baselines)}\n"
                 "  - expected type: bool\n")
        expect (isinstance(self.on_by_default,bool),
                "Invalid value for on_by_default.\n"
                f"  - build name: {shortname}\n"
                f"  - input value: {self.on_by_default}\n"
                f"  - input type: {type(self.on_by_default)}\n"
                 "  - expected type: bool\n")
        expect (isinstance(cmake_args,dict),
                "Invalid value for cmake_args.\n"
                f"  - build name: {shortname}\n"
                f"  - input value: {cmake_args}\n"
                f"  - input type: {type(cmake_args)}\n"
                 "  - expected type: list\n")
        expect (isinstance(default_cmake_args,dict),
                "Invalid value for cmake_args.\n"
                f"  - build name: default\n"
                f"  - input value: {default_cmake_args}\n"
                f"  - input type: {type(default_cmake_args)}\n"
                 "  - expected type: list\n")

        print(f"default: {default_cmake_args}")
        print(f"type: {type(default_cmake_args)}")
        self.cmake_args = {}
        for k,v in default_cmake_args.items():
            self.cmake_args[k] = v
        for k,v in cmake_args.items():
            self.cmake_args[k] = v

        # Perform substitution of ${..} strings
        pattern = r'\$\{(\w+)\.(\w+)\}'
        objects = {
            'project' : project,
            'machine' : machine,
            'build'   : self
        }
        for k,v in self.cmake_args.items():
            matches = re.findall(pattern,str(v))
            for obj_name, att_name in matches:
                expect (att_name!=k,
                        f"Cannot use the value of {obj_name}.{k} to change its own value (no recursion allowed).\n")

                expect (obj_name in objects.keys(),
                        f"Invalid configuration ${{{obj_name}.{att_name}}}. Must be ${{obj.attr}}, with obj='project', 'machine', or 'build'")
                obj = objects[obj_name]

                try:

                    value = getattr(obj,att_name)
                    expect (not value is None,
                            f"Cannot use attribute {obj_name}.{att_name} in configuration, since it is None.\n")
                    value_str = str(value)
                    v = v.replace(f"${{{obj_name}.{att_name}}}",value)
                except AttributeError:
                    print (f"{obj_name} has no attribute '{att_name}'\n")
                    print (f"  - existing attributes: {dir(obj)}\n")
                    raise

            self.cmake_args[k] = v

        #  print("after substitution:")
        #  for k,v in self.cmake_args.items():
        #      print(f" {k}: {v}")
        # Properties set at runtime by the TestProjBuild
        self.compile_res_count = None
        self.testing_res_count = None
        self.baselines_missing = False
