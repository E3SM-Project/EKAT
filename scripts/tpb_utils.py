"""
Utilities
"""

import os
import sys
import re
import signal
import subprocess
import site
import argparse
import importlib
import importlib.util
import inspect
from pathlib import Path

###############################################################################
def module_from_path(module_filepath,module_name):
###############################################################################
    """
    Imports and returns a module from a path and filename.
    This leaves sys.path intact
    """

    print (f"mod path: {module_filepath}")
    print (f"mod name: {module_name}")

    # Create a module spec
    spec = importlib.util.spec_from_file_location(module_name, module_filepath)

    print (f"spec: {spec}")

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
def run_cmd(cmd, input_str=None, from_dir=None, verbose=None, dry_run=False,
            arg_stdout=subprocess.PIPE, arg_stderr=subprocess.PIPE, env=None, combine_output=False):
###############################################################################
    """
    Wrapper around subprocess to make it much more convenient to run shell commands

    >>> run_cmd('ls file_i_hope_doesnt_exist')[0] != 0
    True
    """
    arg_stderr = subprocess.STDOUT if combine_output else arg_stderr
    from_dir = str(from_dir) if from_dir else from_dir

    if verbose:
        print("RUN: {}\nFROM: {}".format(cmd, os.getcwd() if from_dir is None else from_dir))

    if dry_run:
        return 0, "", ""

    if input_str is not None:
        stdin = subprocess.PIPE
        input_str = input_str.encode('utf-8')
    else:
        stdin = None

    proc = subprocess.Popen(cmd,
                            shell=True,
                            stdout=arg_stdout,
                            stderr=arg_stderr,
                            stdin=stdin,
                            cwd=from_dir,
                            env=env)

    output, errput = proc.communicate(input_str)
    if output is not None:
        try:
            output = output.decode('utf-8', errors='ignore')
            output = output.strip()
        except AttributeError:
            pass
    if errput is not None:
        try:
            errput = errput.decode('utf-8', errors='ignore')
            errput = errput.strip()
        except AttributeError:
            pass

    stat = proc.wait()

    return stat, output, errput

###############################################################################
def run_cmd_no_fail(cmd, input_str=None, from_dir=None, verbose=None, dry_run=False,
                    arg_stdout=subprocess.PIPE, arg_stderr=subprocess.PIPE, env=None, combine_output=False, exc_type=SystemExit):
###############################################################################
    """
    Wrapper around subprocess to make it much more convenient to run shell commands.
    Expects command to work. Just returns output string.

    >>> run_cmd_no_fail('echo foo') == 'foo'
    True
    >>> run_cmd_no_fail('echo THE ERROR >&2; false') # doctest:+ELLIPSIS
    Traceback (most recent call last):
        ...
    SystemExit: ERROR: Command: 'echo THE ERROR >&2; false' failed with error ...

    >>> run_cmd_no_fail('grep foo', input_str='foo') == 'foo'
    True
    >>> run_cmd_no_fail('echo THE ERROR >&2', combine_output=True) == 'THE ERROR'
    True
    """
    stat, output, errput = run_cmd(cmd, input_str=input_str, from_dir=from_dir, verbose=verbose, dry_run=dry_run,
                                   arg_stdout=arg_stdout, arg_stderr=arg_stderr, env=env, combine_output=combine_output)
    if stat != 0:
        # If command produced no errput, put output in the exception since we
        # have nothing else to go on.
        errput = output if not errput else errput
        if errput is None:
            errput = ""

        expect(False, "Command: '{}' failed with error '{}' from dir '{}'".format(cmd, errput, os.getcwd() if from_dir is None else from_dir), exc_type=exc_type)

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
