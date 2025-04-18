from tpb_utils import expect

###############################################################################
class BuildType(object):
###############################################################################
    """
    Parent class of predefined build types for the project.
    The script 'test-proj-build' will look at this file content to get specs
    for build-types that need to be tested
    IMPORTANT: Derived classes should be constructible from a Machine obj
    """

    def __init__(self, longname, description, cmake_args, cmake_vars_prefix=None,
                 uses_baselines=True, on_by_default=True, default_test_len=None, concrete=True):
        # What the user uses to select build types via test-proj-builds CLI.
        # Should also match the class name when converted to caps
        self.shortname      = type(self).__name__.lower()

        # A longer name used to name baseline and test directories for a build type.
        # Also used in output/error messages to refer to the test
        self.longname       = longname

        # A longer decription of the build type
        self.description    = description

        # Cmake config args for this build types. Check that quoting is done with single quotes.
        self.cmake_args     = cmake_args
        for name, arg in self.cmake_args:
            expect('"' not in arg,
                   f"In test definition for {longname}, found cmake args with double quotes {name}='{arg}'"
                   "Please use single quotes if quotes are needed.")

        # Does the build types do baseline testing
        self.uses_baselines = uses_baselines

        # Should this build types be run if the user did not specify build types at all?
        self.on_by_default  = on_by_default

        #  # TPB will allow instantiating only if concrete=True. You can define some "generic"
        #  # base classes (to avoid code duplication) and set concrete=False to prevent TPB from
        #  # considering them valid build types. E.g., you could have "MyProjBase", which defines
        #  # basic cmake options
        #  self.concrete = concrete

        #
        # Properties not set by constructor (set at runtime by the TestProjBuild)
        #

        # Resources used by this build types.
        self.compile_res_count = None
        self.testing_res_count = None

        # Does this build types need baselines
        self.baselines_missing = False
        self.baselines_expired = False
