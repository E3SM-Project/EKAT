#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/ekat_pack.hpp"
#include <fstream>

using namespace ekat;
using namespace ekat::logger;

TEST_CASE("log tests", "[logging]") {

    SECTION("default logger") {

      Log::debug("here is a debug message");
      Log::info("You won't see the debug message because the default log level is '{0}', and {0} > debug = {1}", Log::level::to_string_view(Log::get_level()), (Log::get_level() > Log::level::debug) );
      Log::set_level(Log::level::debug);
      Log::debug("now you can see debug messages because we reset the log level to debug.");
      Log::info("this is an info message with a number ({}).", 42);
      Log::critical("This is a test. Here is a critical message. This is only a test.");

      ekat::Pack<double,4> apack;
      for (int i=0; i<4; ++i) {
        apack[i] = i + i/10.0;
      }

      Log::info("Packs fit into the formatting tool naturally; here's a Pack<double,4>: {}", apack);

    }

    SECTION("console only, no mpi") {

      Logger<> mylog("ekat_log_test_console_only", "debug");

      mylog.info("This is a console-only message, with level = info");
      mylog.error("Here is an error message.");

      // check that this log did not produce a file
      const std::string logfilename = "ekat_log_test_console_only_logfile.txt";
      std::ifstream lf(logfilename);
      REQUIRE( !lf.is_open() );

    }

    SECTION("console and file logging, with mpi rank info") {

      Logger<LogBasicFile<Log::level::trace>, LogAllRanks> mylog("combined_console_file_mpi", "debug");

      mylog.debug("here is a debug message that will also show up in this rank's log file.");

      // the file level is trace, but the log level is debug (debug > trace); trace messages will be skipped.
      mylog.trace("this message won't show up anywhere.");
      REQUIRE( !mylog.should_log(Log::level::trace) );

      // verify that this log did produce a file
      const std::string logfilename = "combined_console_file_mpi_rank0_logfile.txt";
      std::ifstream lf(logfilename);
      REQUIRE( lf.is_open() );

    }

}
