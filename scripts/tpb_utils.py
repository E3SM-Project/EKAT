"""
Utilities
"""

import os
import sys
import re
import signal
import subprocess
import psutil
import site
import argparse
import importlib
import importlib.util
import inspect

###############################################################################
def module_from_path(module_filepath,module_name):
###############################################################################
    """
    Imports and returns a module from a path and filename.
    This leaves sys.path intact
    """

    # Create a module spec
    spec = importlib.util.spec_from_file_location(module_name, module_filepath)

    # Create a new module based on the spec
    module = importlib.util.module_from_spec(spec)

    # Execute the module in its own namespace
    spec.loader.exec_module(module)

    return module

###########################################################################
def objects_from_module(module,base_type,*args):
###########################################################################
    """
    From a given module, create an instance of all types inheriting from base_type
    The derived types must ALL be constructible from the input args
    """

    objects = []

    for cls_name, cls in inspect.getmembers(module, inspect.isclass):
        # Check if the class is a subclass of BuildType
        if issubclass(cls, base_type) and cls is not base_type:
            # Create an instance of the class
            objects.append(cls(*args))

    return objects

###############################################################################
def expect(condition, error_msg, exc_type=SystemExit, error_prefix="ERROR:"):
###############################################################################
    """
    Similar to assert except doesn't generate an ugly stacktrace. Useful for
    checking user error, not programming error.

    >>> expect(True, "error1")
    >>> expect(False, "error2")
    Traceback (most recent call last):
        ...
    SystemExit: ERROR: error2
    """
    if not condition:
        msg = error_prefix + " " + error_msg
        raise exc_type(msg)

###############################################################################
def run_cmd(cmd, from_dir=None, verbose=None, dry_run=False, env_setup=None,
            output_file=None,error_file=None,
            output_to_screen=False,error_to_screen=False,
            combine_output=False):
###############################################################################
    """
    Wrapper around subprocess to make it much more convenient to run shell commands

    >>> run_cmd('ls file_i_hope_doesnt_exist')[0] != 0
    True
    """

    expect (not (output_to_screen and output_file),
            "Cannot redirect output both to screen and file.\n")
    expect (not (error_to_screen and error_file),
            "Cannot redirect errors both to screen and file.\n")
    expect (not (combine_output and (error_file or error_to_screen)),
            "Makes no sense to request combined output, and then provide a special handle for stderr.\n")

    # If the cmd needs some env setup, the user can pass the setup string, which will be
    # executed right before the cmd
    if env_setup:
        cmd = f"{env_setup} && {cmd}"

    if output_to_screen:
        arg_stdout = None
    else:
        arg_stdout = subprocess.PIPE
    if error_to_screen:
        arg_stderr = None
    else:
        arg_stderr = subprocess.PIPE

    from_dir = str(from_dir) if from_dir else from_dir

    if verbose:
        print("RUN: {}\nFROM: {}".format(cmd, os.getcwd() if from_dir is None else from_dir))

    if dry_run:
        return 0, "", ""

    proc = subprocess.Popen(cmd,
                            shell=True,
                            stdout=arg_stdout,
                            stderr=arg_stderr,
                            stdin=None,
                            cwd=from_dir)

    output, errput = proc.communicate()

    if output is not None:
        try:
            output = output.decode('utf-8', errors='ignore')
            output = output.strip()
        except AttributeError:
            pass

        if output_file is not None:
            with open(output_file,'w') as fd:
                fd.write(output)
    if errput is not None:
        try:
            errput = errput.decode('utf-8', errors='ignore')
            errput = errput.strip()
        except AttributeError:
            pass


        if error_file is not None:
            with open(error_file,'w') as fd:
                fd.write(error)

    return proc.returncode, output, errput

###############################################################################
def run_cmd_no_fail(cmd, from_dir=None, verbose=None, dry_run=False, env_setup=None,
                    output_file=None,error_file=None,
                    output_to_screen=False,error_to_screen=False,
                    combine_output=False):
###############################################################################
    """
    Wrapper around subprocess to make it much more convenient to run shell commands.
    Expects command to work. Just returns output string.
    """
    stat, output, errput = run_cmd(cmd, from_dir=from_dir,verbose=verbose,dry_run=dry_run,env_setup=env_setup,
                                   output_file=output_file,error_file=error_file,
                                   output_to_screen=output_to_screen,error_to_screen=error_to_screen,
                                   combine_output=combine_output)
    expect (stat==0,
            "Command failed unexpectedly"
            f"  - command: {cmd}"
            f"  - error: {errput if errput else output}"
            f"  - from dir: {from_dir or os.getcwd()}")

    return output

###############################################################################
def check_minimum_python_version(major, minor):
###############################################################################
    """
    Check your python version.

    >>> check_minimum_python_version(sys.version_info[0], sys.version_info[1])
    >>>
    """
    msg = "Python " + str(major) + ", minor version " + str(minor) + " is required, you have " + str(sys.version_info[0]) + "." + str(sys.version_info[1])
    expect(sys.version_info[0] > major or
           (sys.version_info[0] == major and sys.version_info[1] >= minor), msg)


###############################################################################
def ensure_pip():
###############################################################################
    """
    Ensures that pip is available. Notice that we cannot use the _ensure_pylib_impl
    function below, since it would cause circular dependencies. This one has to
    be done by hand.
    """
    try:
        import pip # pylint: disable=unused-import

    except ModuleNotFoundError:
        # Use ensurepip for installing pip
        import ensurepip
        ensurepip.bootstrap(user=True)

        # needed to "rehash" available libs
        site.main() # pylint: disable=no-member

        import pip # pylint: disable=unused-import

###############################################################################
def pip_install_lib(pip_libname):
###############################################################################
    """
    Ask pip to install a version of a package which is >= min_version
    """
    # Installs will use pip, so we need to ensure it is available
    ensure_pip()

    # Note: --trusted-host may not work for ancient versions of python
    #       --upgrade makes sure we get the latest version, even if one is already installed
    stat, _, err = run_cmd("{} -m pip install --upgrade {} --trusted-host files.pythonhosted.org --user".format(sys.executable, pip_libname))
    expect(stat == 0, "Failed to install {}, cannot continue:\n{}".format(pip_libname, err))

    # needed to "rehash" available libs
    site.main() # pylint: disable=no-member

###############################################################################
def package_version_ok(pkg, min_version=None):
###############################################################################
    """
    Checks that the loaded package's version is >= that the minimum required one.
    If no minimum version is passed, then return True
    """
    if min_version is not None:
        try:
            from pkg_resources import parse_version

            return parse_version(pkg.__version__) >= parse_version(min_version)
        except ImportError:
            # Newer versions of python cannot use pkg_resources
            ensure_packaging()
            from packaging.version import parse

            return parse(pkg.__version__) >= parse(min_version)

    else:
        return True

###############################################################################
def _ensure_pylib_impl(libname, min_version=None, pip_libname=None):
###############################################################################
    """
    Internal method, clients should not call this directly; please use of the
    public ensure_XXX methods. If one does not exist, we will need to evaluate
    if we want to add a new outside dependency.
    """

    install = False
    try:
        pkg = importlib.import_module(libname)

        if not package_version_ok(pkg,min_version):
            print("Detected version for package {} is too old: detected {}, required >= {}. Will attempt to upgrade the package locally".format(libname, pkg.__version__,min_version))
            install = True

    except ImportError:
        print("Detected missing package {}. Will attempt to install locally".format(libname))
        pip_libname = pip_libname if pip_libname else libname

        install = True

    if install:
        pip_install_lib(pip_libname)
        pkg = importlib.import_module(libname)

    expect(package_version_ok(pkg,min_version),
           "Error! Could not find version {} for package {}.".format(min_version,libname))

# We've accepted these outside dependencies
def ensure_yaml():      _ensure_pylib_impl("yaml", pip_libname="pyyaml",min_version='5.1')
def ensure_pylint():    _ensure_pylib_impl("pylint")
def ensure_psutil():    _ensure_pylib_impl("psutil")
def ensure_netcdf4():   _ensure_pylib_impl("netCDF4")
def ensure_packaging(): _ensure_pylib_impl("packaging")

###############################################################################
class GoodFormatter(
    argparse.ArgumentDefaultsHelpFormatter,
    argparse.RawDescriptionHelpFormatter
):
###############################################################################
    """
    We want argument default info to be added but we also want to
    preserve formatting in the description string.
    """

###############################################################################
###############################################################################
###############################################################################
###############################################################################
###############################################################################
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
class SharedArea(object):
###############################################################################
    """
    Enable 0002 umask within this manager
    """

    def __init__(self, new_perms=0o002):
        self._orig_umask = None
        self._new_perms  = new_perms

    def __enter__(self):
        self._orig_umask = os.umask(self._new_perms)

    def __exit__(self, *_):
        os.umask(self._orig_umask)

