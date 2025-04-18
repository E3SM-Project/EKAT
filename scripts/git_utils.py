"""
Git-related Utilities
"""

from tpb_utils import run_cmd, run_cmd_no_fail

###############################################################################
def is_git_repo(repo=None):
###############################################################################
    """
    Check that the folder is indeed a git repo
    """

    stat, _, _ = run_cmd("git rev-parse --is-inside-work-tree",from_dir=repo)

    return stat==0

###############################################################################
def get_current_ref(repo=None):
###############################################################################
    """
    Return the name of the current branch for a repository
    If in detached HEAD state, returns None
    """

    return run_cmd_no_fail("git rev-parse --abbrev-ref HEAD",from_dir=repo)

###############################################################################
def get_current_sha(short=False,repo=None):
###############################################################################
    """
    Return the sha1 of the current HEAD commit

    >>> get_current_commit() is not None
    True
    """

    return run_cmd_no_fail(f"git rev-parse {'--short' if short else ''} HEAD",from_dir=repo)
