#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"

#include "ekat/ekat_session.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/util/ekat_test_utils.hpp"

#include <mpi.h>

void ekat_initialize_test_session (int argc, char** argv);
void ekat_finalize_test_session ();

struct TestSession {
  static TestSession& get () {
    static TestSession s;
    return s;
  }

  std::map<std::string,std::string> params;
private:
  TestSession() = default;
};

int main (int argc, char **argv) {

  // Initialize MPI
  MPI_Init(&argc,&argv);

  // Read possible ekat-specific arguments
  auto const readCommaSeparaterParams = [] (const std::string& cmd_line_arg) {
    if (cmd_line_arg=="") {
      return;
    }
    auto& ts = TestSession::get();

    std::stringstream input(cmd_line_arg);
    std::string option;
    while (getline(input,option,',')) {
      // Split option according to key=val
      auto pos = option.find('=');
      ekat_require_msg(pos!=std::string::npos, "Error! Badly formatted command line options.\n");
      std::string key = option.substr(0,pos);
      std::string val = option.substr(pos+1);
      ekat_require_msg(key!="", "Error! Empty key found in the list of command line options.\n");
      ekat_require_msg(val!="", "Error! Empty value for key '" + key + "'.\n");
      ts.params[key] = val;
    }
  };
  Catch::Session catch_session;
  auto cli = catch_session.cli();
  cli |= Catch::clara::Opt(readCommaSeparaterParams, "key1=val1[,key2=val2[,...]]")
             ["--ekat-test-params"]
             ("list of parameters to forward to the test");
  catch_session.cli(cli);

  ekat_require_msg(catch_session.applyCommandLine(argc,argv)==0,
                     "Error! Something went wrong while parsing command line.\n");

  // If we are on a gpu build, we might have a test device id to use
  // so start by creating a copy of argv that we can extend
  std::vector<char*> args;
  for (int i=0; i<argc; ++i) {
    args.push_back(argv[i]);
  }

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int dev_id = ekat::util::get_test_device(rank);
  // Create it outside the if, so its c_str pointer survives
  std::string new_arg;
  if (dev_id>=0) {
    new_arg = "--kokkos-device=" + std::to_string(dev_id);
    args.push_back(const_cast<char*>(new_arg.c_str()));
  }

  // Initialize ekat
  ekat::initialize_ekat_session(args.size(),args.data());

  // Possibly calling user-defined initialization routines
  ekat_initialize_test_session(args.size(),args.data());

  // Run tests
  int num_failed = catch_session.run(argc, argv);

  // Possibly calling user-defined finalization routines
  ekat_finalize_test_session ();

  // Finalize ekat
  ekat::finalize_ekat_session();

  // Finalize MPI
  MPI_Finalize();

  // Return test result
  return num_failed != 0 ? 1 : 0;
}
