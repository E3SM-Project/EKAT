#include <catch2/catch.hpp>

#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"

#include <fstream>
#include <iostream>

TEST_CASE("rank_in_name", "[logging]") {

  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);

  {
    Logger<LogBasicFile,LogAllRanks,NameWithRank> with_rank("with", LogLevel::info, comm);
    Logger<LogBasicFile,LogAllRanks,NameWithoutRank> without_rank("without", LogLevel::info, comm);

    // Silence console for this test
    with_rank.set_console_level(LogLevel::off);
    without_rank.set_console_level(LogLevel::off);
    with_rank.set_no_format();
    without_rank.set_no_format();

    with_rank.info("message");
    without_rank.info("message");
  }

  // Make sure ranks are all done flushing to file
  comm.barrier();

  for (int i=0; i<comm.size(); ++i) {
    const std::string logfilename = "with." + std::to_string(comm.size())
                                      + "." + std::to_string(i) + ".log";

    std::ifstream with_rank_log(logfilename);
    REQUIRE( with_rank_log.is_open() );
    std::string line, content;
    while (std::getline(with_rank_log,line)) {
      content += line;
    }
    REQUIRE (content=="message");

    std::ifstream without_rank_log(logfilename);
  }

  // NameWithoutRank should yield a single file, with comm.size() copies
  // of the same line 'message'.
  std::ifstream without_rank_log("without.log");
  REQUIRE( without_rank_log.is_open() );
  std::vector<std::string> content;
  for (int i=0; i<comm.size(); ++i) {
    std::string line;
    while (std::getline(without_rank_log,line)) {
      content.push_back(line);
    }
  }
  REQUIRE (content==std::vector<std::string> (comm.size(),"message"));
}

TEST_CASE("rank_0_console_only", "[logging]") {

  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);

  std::string msg = "rank" + std::to_string(comm.rank());
  {
    Logger<LogNoFile,LogRank0> mylog("console_rank0_only", LogLevel::debug, comm);
    mylog.set_no_format();
    mylog.warn(msg);
  }

  // Verify no file was generated
  std::ifstream lf("console_rank0_only.log");
  REQUIRE( not lf.is_open() );
}

TEST_CASE("rank_0_file_only", "[logging]") {

  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);

  std::string msg = "rank" + std::to_string(comm.size()) + "." + std::to_string(comm.rank());
  std::string msg0 = "rank" + std::to_string(comm.size()) + ".0";
  {
    Logger<LogBasicFile,LogRank0,NameWithRank> with_rank("file_rank0_only_with", LogLevel::debug, comm);
    Logger<LogBasicFile,LogRank0,NameWithoutRank> without_rank("file_rank0_only_without", LogLevel::debug, comm);
    with_rank.set_console_level(LogLevel::off);
    without_rank.set_console_level(LogLevel::off);
    with_rank.set_no_format();
    without_rank.set_no_format();
    with_rank.warn(msg);
    without_rank.warn(msg);
  }

  // Make sure rank0 is done flushing to file
  comm.barrier();

  // Verify files contents
  for (int i=0; i<comm.size(); ++i) {
    const std::string logfilename = "file_rank0_only_with." + std::to_string(comm.size())
                                      + "." + std::to_string(i) + ".log";
    std::ifstream lf(logfilename);
    if (i==0) {
      REQUIRE( lf.is_open() );
      std::string line, content;
      while (std::getline(lf,line)) {
        content += line;
      }
      REQUIRE (content==msg0);
    } else {
      REQUIRE (not lf.is_open() );
    }
  }

  const std::string logfilename = "file_rank0_only_without.log";
  std::ifstream lf(logfilename);
  REQUIRE( lf.is_open() );
  std::string line, content;
  while (std::getline(lf,line)) {
    content += line;
  }
  REQUIRE (content==msg0);
}
