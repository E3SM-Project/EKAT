#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

#include "ekat/ekat_config.h"

#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/ekat_session.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/mpi/ekat_comm.hpp"

#ifdef EKAT_ENABLE_MPI
#include <mpi.h>
#endif

void ekat_initialize_test_session (int argc, char** argv, const bool print_config);
void ekat_finalize_test_session ();

struct Args {
  std::vector<char*> catch2;
  std::vector<char*> custom;
};

Args split_args (int argc, char** argv)
{
  Args args;

  args.catch2.push_back(argv[0]); // the program name

  // Parse the command line arguments
  bool foundArgsFlag = false;
  for (int i = 1; i < argc; ++i) { // Start from 1 to skip the program name
    if (foundArgsFlag) {
      // If we've already found --args, add the rest to customArgs
      args.custom.push_back(argv[i]);
    } else if (std::string(argv[i]) == "--args") {
        foundArgsFlag = true;
    } else {
      args.catch2.push_back(argv[i]);
    }
  }

  args.catch2.push_back(nullptr); // Catch2 expects a null-terminated array

  return args;
}

int main (int argc, char **argv) {

#ifdef EKAT_ENABLE_MPI
  // Initialize MPI
  MPI_Init(&argc,&argv);
#endif

  auto& ts = ekat::TestSession::get();
  Catch::Session catch_session;
  auto cli = catch_session.cli();
  int device = -1;
  // If we are on a gpu build, we might have a test device id to use
  cli |= Catch::clara::Opt(device, "device")["--kokkos-device-id"]
             ("The device to be used (for GPU runs only");
  catch_session.cli(cli);

  // Split args into those recognized by catch2, and everything else (which is stored in TestSession)
  auto args = split_args (argc,argv);

  // Parse test-specific args
  ts.parse_test_args(args.custom);

  // Parse catch2 args
  EKAT_REQUIRE_MSG(catch_session.applyCommandLine(args.catch2.size()-1,
                                                  const_cast<char**>(args.catch2.data()))==0,
                     "Error! Something went wrong while parsing command line.\n");

  ekat::Comm comm(MPI_COMM_WORLD);
  bool am_i_root = comm.am_i_root();

  // Initialize test session (initializes kokkos and print config settings).
  // Ekat provides a default impl, but the user can choose
  // to not use it, and provide one instead.
  ekat_initialize_test_session(argc,argv,am_i_root);


#ifndef NDEBUG
  comm.barrier();
  std::cout << "Starting catch session on rank " << comm.rank() << " out of " << comm.size() << "\n";
  comm.barrier();
#endif

  // Run tests
  int num_failed = catch_session.run();

  // Finalizes test session (finalizes kokkos).
  // Ekat provides a defalt impl, but the user can choose
  // to not use it, and provide one instead.
  ekat_finalize_test_session ();

#ifdef EKAT_ENABLE_MPI
  // Finalize MPI
  MPI_Finalize();
#endif

  // Return test result
  return num_failed != 0 ? 1 : 0;
}
