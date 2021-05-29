#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_pack.hpp"

using namespace ekat;
using namespace ekat::logger;

bool trace_msg_in_file(Logger<LogBasicFile<Log::level::trace>, LogAllRanks>& logger) {
  return (logger.should_log(Log::level::trace)) and (logger.get_file_sink()->should_log(Log::level::trace));
}

TEST_CASE("log_mpi", "[logging]") {

  Comm comm(MPI_COMM_WORLD);

  SECTION("all ranks log") {
    Logger<LogBasicFile<Log::level::trace>, LogAllRanks> mylog("mpilog", "trace");

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

  SECTION("only rank 0") {

    Logger<LogBasicFile<Log::level::debug>, LogOnlyRank0> mylog("rank0_only", "debug");

    mylog.warn("if you see this in the console from any rank except 0, something is wrong.\n It will show up in every file.");
    mylog.info("that previous message covered multiple lines.");

    ekat::Pack<double,4> apack;
    for (int i=0; i<4; ++i) {
      apack[i] = i + i/10.0;
    }
    mylog.debug("This debug message has a Pack<double,4> with data {}", apack);

    // check that ranks > 0 deleted their console sink
    const Comm mpicomm(MPI_COMM_WORLD);
    if (mpicomm.rank() > 0) {
      REQUIRE( mylog.sinks().size() == 1);
    }

  }
}
