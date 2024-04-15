#!/usr/bin/env python3

import argparse, sys, pathlib

###############################################################################
def parse_command_line(args, description):
###############################################################################
    parser = argparse.ArgumentParser(
        usage="""\n{0} <ARGS> [--verbose]
OR
{0} --help
""".format(pathlib.Path(args[0]).name),
        description=description,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument("-s","--meta_src", required=True,
                        help="Source file contianing EKAT_TEST_ASSERT macros used to generate source files to test individual asserts.")
    parser.add_argument("-o","--output_dir", required=True,
                        help="Directory where the generated source files should be put")

    return parser.parse_args(args[1:])

###############################################################################
def create_sources (meta_src, output_dir):
###############################################################################

    key = "EKAT_TEST_ASSERT"

    f_in = pathlib.Path(meta_src)
    with open(f_in,"r") as f:
        lines = [line.rstrip() for line in f]

    num_asserts = len([l for l in lines if key in l])
    if num_asserts==0:
        return 0

    # First, a version of the test without *any* EKAT_TEST_ASSERT line
    f_out = pathlib.Path(output_dir) / f"{f_in.stem}.cpp"
    with open (f_out,"w") as fout:
        for line in lines:
            if key not in line:
                fout.write(line + "\n")

    for n in range(1,num_asserts+1):
        f_out = pathlib.Path(output_dir) / f"{f_in.stem}_{n}.cpp"
        with open (f_out,"w") as fout:
            iassert = 1
            for line in lines:
                if key in line:
                    if iassert == n:
                        line = line.replace(key,"")
                    else:
                        # We either already generated a src file to
                        # test this assert, or we are generating for
                        # a previous assert. Either way, just rm this line
                        line = ""
                    iassert = iassert + 1
                    
                fout.write(line + "\n")

    print (num_asserts)

###############################################################################
def _main_func(description):
###############################################################################
    create_sources(**vars(parse_command_line(sys.argv, description)))

    sys.exit(0)

###############################################################################

if (__name__ == "__main__"):
    _main_func(__doc__)
