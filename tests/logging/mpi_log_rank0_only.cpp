#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_pack.hpp"
#include <fstream>

using namespace ekat;
using namespace ekat::logger;

TEST_CASE("log_mpi", "[logging]") {

  Comm comm(MPI_COMM_WORLD);

  SECTION("only rank 0") {

    Logger<LogBasicFile<Log::level::debug>, LogOnlyRank0> mylog("rank0_only", Log::level::debug, comm);
    const std::string logfilename = LogOnlyRank0::name_with_rank("rank0_only", comm) + "_logfile.txt";
    mylog.warn("if you see this in the console from any rank except 0, something is wrong.\n It will show up in every file.");
    mylog.info("that previous message covered multiple lines.");

    ekat::Pack<double,4> apack;
    for (int i=0; i<4; ++i) {
      apack[i] = i + i/10.0;
    }
    mylog.debug("This debug message has a Pack<double,4> with data {}", apack);

    // verify that all ranks produce a file
    std::ifstream lf(logfilename);
    REQUIRE( lf.is_open() );

    REQUIRE( mylog.get_logfile_name() == logfilename);
  }
}
