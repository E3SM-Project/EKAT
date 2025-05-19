#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

#include "ekat_test_utils.hpp"
#include "ekat_arch.hpp"
#include "ekat_fpe.hpp"
#include "ekat_assert.hpp"

#include "ekat_comm.hpp"
#ifdef EKAT_ENABLE_MPI
#include <mpi.h>
#endif

// These two functions MUST be defined somewhere. EKAT provides the ekat::TestSession library,
// which implements them. If users want to define their own, they can implement their library,
// and PREVENT linking of ekat::TestSession mini-lib
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

  // Split args into those recognized by catch2, and everything else (which is stored in TestSession)
  auto args = split_args (argc,argv);

  // Parse test-specific args
  auto& ts = ekat::TestSession::get();
  ts.parse_test_args(args.custom);

  // Parse catch2 args
  Catch::Session catch_session;
  auto ret = catch_session.applyCommandLine(args.catch2.size()-1,const_cast<char**>(args.catch2.data()));
  EKAT_REQUIRE_MSG (ret==0,"Error! Something went wrong while parsing command line.\n");

  bool am_i_root = true;
  ekat::Comm comm;
#ifdef EKAT_ENABLE_MPI
  comm.reset_mpi_comm(MPI_COMM_WORLD);
#endif
  am_i_root = comm.am_i_root();

  // Initialize the test session
  // Ekat provides a default impl, but the user can choose to not use it, and provide one instead.
  // The default impl initializes kokkos and print config settings.
  ekat_initialize_test_session(argc,argv,am_i_root);

  comm.barrier();
  std::cout << "Starting catch session on rank " << comm.rank() << " out of " << comm.size() << "\n";
  comm.barrier();

  // Run tests
  int num_failed = catch_session.run();

  // Finalizes test session
  // Ekat provides a defalt impl, but the user can choose to not use it, and provide one instead.
  // The default impl finalizes kokkos
  ekat_finalize_test_session ();

#ifdef EKAT_ENABLE_MPI
  // Finalize MPI
  MPI_Finalize();
#endif

  // Return test result
  return num_failed != 0 ? 1 : 0;
}
