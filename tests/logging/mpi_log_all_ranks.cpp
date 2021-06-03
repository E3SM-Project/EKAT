#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_pack.hpp"

using namespace ekat;
using namespace ekat::logger;

template <typename FP>
bool trace_msg_in_file(Logger<FP>& logger) {
  return (logger.should_log(Log::level::trace)) and (logger.get_file_sink()->should_log(Log::level::trace));
}

TEST_CASE("mpi log all ranks", "[logging]") {

  Comm comm(MPI_COMM_WORLD);

  Logger<LogBasicFile<Log::level::trace>> mylog("mpilog", Log::level::trace, comm);

  // set the console log level higher than the log's
  mylog.get_console_sink()->set_level(Log::level::debug);

  mylog.debug("here is a debug message that will also show up in this rank's log file.");
  mylog.trace("this trace message will only show up in the file");

  // verify that the log will write trace-level messages
  REQUIRE( mylog.level() == Log::level::trace );
  REQUIRE( mylog.should_log(Log::level::trace) );
  // verify that trace level messages will not show up on the console
  REQUIRE( !mylog.get_console_sink()->should_log(Log::level::trace) );
  // verify that the file sink will write trace messages
  REQUIRE( mylog.get_file_sink()->level() == Log::level::trace );
  REQUIRE( mylog.get_file_sink()->should_log(Log::level::trace) );
  // verify that trace messages will actually show up in the file
  REQUIRE(trace_msg_in_file(mylog));

  // increase the log priority level, but keep the file priority level at trace
  mylog.set_level(Log::level::info);
  // verify that trace messages will not show up in the file or the console
  mylog.trace("This message will not show up anywhere.");
  REQUIRE( !trace_msg_in_file(mylog) );
  REQUIRE( !mylog.should_log(Log::level::trace) );
}
