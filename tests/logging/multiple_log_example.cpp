#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_pack.hpp"
#include <fstream>
#include <sstream>

using namespace ekat;
using namespace ekat::logger;

TEST_CASE("multiple logs, same file", "[logging]") {

  Comm comm(MPI_COMM_WORLD);

  using file_policy = LogBasicFile<Log::level::debug>;
  // first, we create a main log.
  // this log will determine the name of the shared log file.
  Logger<file_policy> main_log("main_log", Log::level::info, comm);
  main_log.console_output_rank0_only(comm);
  const std::string logfilename = "main_log_rank" + std::to_string(comm.rank()) + "_logfile.txt";
  REQUIRE( main_log.logfile_name() == logfilename);

  // now we can create a sub-log that share's the main log's file.
  // this log will have a different name and, therefore, different message tags,
  // in both the console and the log files.
  // It can also have a different logging level.
  Logger<file_policy> component_log("component_log", Log::level::debug, main_log, comm);
  component_log.console_output_rank0_only(comm);

  main_log.info("The main log sets up the logfile  {}", main_log.logfile_name());
  component_log.debug("The component log shares the file; its messages are tagged with its name.");

  REQUIRE(main_log.logfile_name() == component_log.logfile_name());

  // check that the main log *will not* output debug messages
  main_log.debug("this will not be logged from main_log.");
  REQUIRE( !main_log.should_log(Log::level::debug) );

  // check that the component log *will* output debug messages
  component_log.debug("this will be logged from component_log.");
  REQUIRE( component_log.should_log(Log::level::debug) );

  // verify that all ranks produce a log file
  std::ifstream lf(logfilename);
  REQUIRE( lf.is_open() );
}
