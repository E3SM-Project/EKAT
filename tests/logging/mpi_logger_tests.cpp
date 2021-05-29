#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"

using namespace ekat;
using namespace ekat::logger;

TEST_CASE("log_mpi", "[logging]") {

  Comm comm(MPI_COMM_WORLD);

  SECTION("all ranks log") {
    Logger<LogBasicFile<Log::level::trace>, LogAllRanks> mylog("mpilog", "debug");

      mylog.debug("here is a debug message that will also show up in this rank's log file.");
      mylog.trace("this trace message will only show up in the file");
  }

  SECTION("only rank 0") {

    Logger<LogBasicFile<Log::level::debug>, LogOnlyRank0> mylog("rank0_only", "debug");

    mylog.warn("if you see this in the console from any rank except 0, something is wrong.\n It will show up in every file.");
    mylog.info("that previous message covered multiple lines.");
  }
}
