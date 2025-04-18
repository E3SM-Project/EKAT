"""
Git-related Utilities
"""

from tpb_utils import expect, run_cmd

import os

###############################################################################
def get_current_ref(repo=None):
###############################################################################
    """
    Return the name of the current branch for a repository
    If in detached HEAD state, returns None
    """

    stat, output, err = run_cmd("git rev-parse --abbrev-ref HEAD",from_dir=repo)
    expect (stat==0, f"Error! The command 'git rev-parse --abbrev-ref HEAD' failed with error: {err}")

    return output

###############################################################################
def get_current_sha(short=False,repo=None):
###############################################################################
    """
    Return the sha1 of the current HEAD commit

    >>> get_current_commit() is not None
    True
    """

    rc, output, err = run_cmd(f"git rev-parse {'--short' if short else ''} HEAD",from_dir=repo)

    if rc != 0:
        print(f"Warning: getting current commit failed with error: {err}")

    return output if rc == 0 else None
