#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

#include "ekat_test_utils.hpp"
#include "ekat_assert.hpp"

#ifdef EKAT_ENABLE_MPI_COMM
#include "ekat_comm.hpp"
#ifdef EKAT_ENABLE_MPI
#include <mpi.h>
#endif
#endif

#ifndef USER_DEFINED_TEST_SESSION
#include "ekat_session.hpp"
#endif

// If USER_DEFINED_TEST_SESSION is defined, the user MUST provide implementations
// for the following two routines
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
#ifdef EKAT_ENABLE_MPI_COMM
  ekat::Comm comm;
#ifdef EKAT_ENABLE_MPI
  comm.reset_mpi_comm(MPI_COMM_WORLD);
#endif
  am_i_root = comm.am_i_root();
#endif

  // Initialize test session (initializes kokkos and print config settings).
  // Ekat provides a default impl, but the user can choose
  // to not use it, and provide one instead.
  ekat_initialize_test_session(argc,argv,am_i_root);

#ifdef EKAT_ENABLE_MPI_COMM
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

#ifndef USER_DEFINED_TEST_SESSION
void ekat_initialize_test_session (int argc, char** argv, const bool print_config) {
  ekat::initialize_ekat_session (argc,argv,print_config);

#ifdef EKAT_ENABLE_FPE_DEFAULT_MASK
  int mask = FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW;
  ekat::enable_fpes(mask);
#endif

  if (print_config) {
    printf(" active avx set: %s\n", active_avx_string().c_str());
    printf(" compiler id: %s\n", compiler_id_string().c_str());
    printf(" current FPE: mask=%d, %s\n", get_enabled_fpes(), fpe_config_string().c_str());
  }
}

void ekat_finalize_test_session () {
  ekat::finalize_ekat_session ();
}
#endif
