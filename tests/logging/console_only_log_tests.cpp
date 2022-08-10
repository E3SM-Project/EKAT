#include <catch2/catch.hpp>

#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"

#include <fstream>

TEST_CASE("console_only", "[logging]") {
  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);
  std::string log_name = "console_only";

  // We cannot be sure when loggers flush to file. The only sure thing
  // is that when they go out of scope (or anyway get destroyed)
  // the flushing is completed. So wrap Loggers in a scope
  SECTION("all_ranks")
  {
    {
      // Exploit LogAllRanks to decorate message with ".$size.$rank".
      std::string msg = LogAllRanks::get_log_name("all_ranks",comm);


      Logger<LogNoFile,LogAllRanks> mylog("all_ranks_console_only", LogLevel::debug, comm);
      mylog.set_no_format();
      mylog.warn(msg);
      REQUIRE (mylog.get_logfile_name()=="");
    }

    // Verify no file was generated.
    auto fname = LogAllRanks::get_log_name("all_ranks_console_only",comm) + ".log";
    std::ifstream log_file (fname);
    REQUIRE (not log_file.is_open());
  }

  SECTION("root_rank") {
    {
      // Exploit LogAllRanks to decorate message with ".$size.$rank".
      std::string msg = LogAllRanks::get_log_name("root_rank",comm);

      Logger<LogNoFile,LogRootRank> mylog("root_rank_console_only", LogLevel::debug, comm);
      mylog.set_no_format();
      mylog.warn(msg);
    }

    // Verify no file was generated.
    auto fname = LogRootRank::get_log_name("all_ranks_console_only",comm) + ".log";
    std::ifstream log_file (fname);
    REQUIRE (not log_file.is_open());
  }
}
