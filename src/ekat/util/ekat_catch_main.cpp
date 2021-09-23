#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/ekat_session.hpp"
#include "ekat/ekat_assert.hpp"

#include <mpi.h>

void ekat_initialize_test_session (int argc, char** argv, const bool print_config);
void ekat_finalize_test_session ();

int main (int argc, char **argv) {

  // Initialize MPI
  MPI_Init(&argc,&argv);

  auto is_int = [] (const std::string& s)->bool {
    std::istringstream is(s);
    int d;
    is >> d;
    return !is.fail() && is.eof();
  };

  // Read possible ekat-specific arguments
  auto const readCommaSeparaterParams = [] (const std::string& cmd_line_arg) {
    if (cmd_line_arg=="") {
      return;
    }
    auto& ts = ekat::TestSession::get();

    std::stringstream input(cmd_line_arg);
    std::string option;
    while (getline(input,option,',')) {
      // Split option according to key=val
      auto pos = option.find('=');
      EKAT_REQUIRE_MSG(pos!=std::string::npos, "Error! Badly formatted command line options.\n");
      std::string key = option.substr(0,pos);
      std::string val = option.substr(pos+1);
      EKAT_REQUIRE_MSG(key!="", "Error! Empty key found in the list of command line options.\n");
      EKAT_REQUIRE_MSG(val!="", "Error! Empty value for key '" + key + "'.\n");
      ts.params[key] = val;
    }
  };
  Catch::Session catch_session;
  auto cli = catch_session.cli();
  auto& ts = ekat::TestSession::get();
  auto& device = ts.params["kokkos-device-id"];
  device = "-1";
  cli |= Catch::clara::Opt(readCommaSeparaterParams, "key1=val1[,key2=val2[,...]]")
             ["--ekat-test-params"]
             ("list of parameters to forward to the test");
  cli |= Catch::clara::Opt(device, "device")["--ekat-kokkos-device"]
             ("The device to be used (for GPU runs only");
  catch_session.cli(cli);

  EKAT_REQUIRE_MSG(catch_session.applyCommandLine(argc,argv)==0,
                     "Error! Something went wrong while parsing command line.\n");

  EKAT_REQUIRE_MSG (is_int(device), "Error! Please, specify an integer value for the device id.\n");

  // If we are on a gpu build, we might have a test device id to use
  // so start by creating a copy of argv that we can extend
  std::vector<char*> args;
  for (int i=0; i<argc; ++i) {
    args.push_back(argv[i]);
  }

  ekat::Comm comm(MPI_COMM_WORLD);
  //int dev_id = ekat::get_test_device(comm.rank());
  // Create it outside the if, so its c_str pointer survives
  std::string new_arg;
  if (std::stoi(device) != -1) {
    new_arg = "--kokkos-device-id=" + device;
    args.push_back(const_cast<char*>(new_arg.c_str()));
  }

  // Initialize test session (initializes kokkos and print config settings).
  // Ekat provides a default impl, but the user can choose
  // to not use it, and provide one instead.
  ekat_initialize_test_session(args.size(),args.data(),comm.am_i_root());

#ifndef NDEBUG
  MPI_Barrier(comm.mpi_comm());
  std::cout << "Starting catch session on rank " << comm.rank() << " out of " << comm.size() << "\n";
  MPI_Barrier(comm.mpi_comm());
#endif

  // Run tests
  int num_failed = catch_session.run();

  // Finalizes test session (finalizes kokkos).
  // Ekat provides a defalt impl, but the user can choose
  // to not use it, and provide one instead.
  ekat_finalize_test_session ();

  // Finalize MPI
  MPI_Finalize();

  // Return test result
  return num_failed != 0 ? 1 : 0;
}
