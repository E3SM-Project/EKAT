#include <catch2/catch.hpp>
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_pack.hpp"
#include <fstream>

using namespace ekat;
using namespace ekat::logger;

TEST_CASE("mpi log console: rank 0 only; file: all ranks", "[logging]") {

  Comm comm(MPI_COMM_WORLD);
  Logger<LogBasicFile<Log::level::debug>> mylog("console_rank0_only", Log::level::debug, comm);
  mylog.console_output_rank0_only(comm);
  const std::string logfilename = "console_rank0_only_rank" + std::to_string(comm.rank())
     + "_logfile.txt";
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

  REQUIRE( mylog.logfile_name() == logfilename);
}

TEST_CASE("mpi log console: rank 0 only; file: rank 0 only", "[logging]") {
  Comm comm(MPI_COMM_WORLD);

  Logger<LogBasicFile<Log::level::info>> mylog("all_output_rank0_only", Log::level::info, comm);
  mylog.console_output_rank0_only(comm);
  mylog.file_output_rank0_only(comm);

  mylog.warn("if you see this in the console or a file from any rank except 0, something is wrong.");

  const std::string logfilename = "all_output_rank0_only_rank" + std::to_string(comm.rank())
     + "_logfile.txt";
  std::ifstream lf(logfilename);
  if (comm.am_i_root()) {
    REQUIRE(mylog.logfile_name() == logfilename);
    REQUIRE(lf.good());
  }
  else {
    REQUIRE(mylog.logfile_name() == "null");
    REQUIRE(!lf.good());
  }
}
