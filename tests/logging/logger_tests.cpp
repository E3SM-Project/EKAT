#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
// #include "ekat/logging/ekat_log_output.hpp"
#include "ekat/logging/ekat_logger.hpp"

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

    }

    SECTION("console only, no mpi") {

      Logger<> mylog("ekat_log_test_console_only", "debug");

      mylog.info("This is a console-only message, with level = info");

    }

}
