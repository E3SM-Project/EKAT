# Wrapper around find_package for the TPLs that EKAT can build on its own.
#
# We only want to pick up an existing installation of a TPL if the user explicitly
# points to it, that is, via <Pkg>_ROOT (cmake var or env var) or <Pkg>_DIR.
# We do NOT want to accidentally pick up something that happens to be in
# CMAKE_PREFIX_PATH, PATH, the system prefixes, or the package registries.
#
# Note: we cannot use NO_DEFAULT_PATH, since that is shorthand for all the NO_*
#       options, including NO_PACKAGE_ROOT_PATH, which would make find_package
#       ignore <Pkg>_ROOT. So we disable all the other search locations explicitly.
#       Unlike HINTS/PATHS, <Pkg>_ROOT entries are expanded to the standard
#       <root>/lib*/cmake/<Pkg>*/ locations, so users only need to provide the
#       installation root.
#
# Usage: ekat_find_tpl(<Pkg> [<find_package args>...])
#
# This is a macro (not a function), so that the variables set by find_package
# (<Pkg>_FOUND, <Pkg>_DIR, ...) are visible in the caller's scope.
macro (ekat_find_tpl pkg)
  # Make sure <Pkg>_ROOT is honored (it's the default with cmake >= 3.27)
  cmake_policy (SET CMP0074 NEW)
  find_package (${pkg} ${ARGN}
    NO_CMAKE_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_PACKAGE_REGISTRY
    NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
endmacro()
