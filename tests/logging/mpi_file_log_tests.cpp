#include <catch2/catch.hpp>

#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"

#include <fstream>

// When multiple copies of the test are run (one per MPI_COMM_WORLD size),
// we need to make sure each test uses different names for the log files,
// even if the Logger does not append comm rank/size information.
std::string add_comm_size (const std::string& s, const ekat::Comm& comm) {
  return s + std::to_string(comm.size());
}

TEST_CASE("mpi_policies") {
  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);

  std::string prefix = "foo";
  auto all_ranks_log_name = LogAllRanks::get_log_name(prefix,comm);
  auto root_rank_log_name = LogRootRank::get_log_name(prefix,comm);

  REQUIRE (root_rank_log_name==prefix);
  REQUIRE (all_ranks_log_name==(prefix + "." + std::to_string(comm.size())
                                       + "." + std::to_string(comm.rank())));

  REQUIRE (LogAllRanks::should_log(comm)==true);
  REQUIRE (LogRootRank::should_log(comm)==comm.am_i_root());
}

TEST_CASE("log_all_ranks", "[logging]") {

  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);

  {
    Logger<LogBasicFile,LogAllRanks> with_rank("with", LogLevel::info, comm);

    // Silence console for this test
    with_rank.set_console_level(LogLevel::off);
    with_rank.set_no_format();

    with_rank.info("message");
  }

  // Make sure ranks are all done flushing to file
  comm.barrier();

  for (int i=0; i<comm.size(); ++i) {
    const std::string logfilename = LogAllRanks::get_log_name("with",comm);

    std::ifstream with_rank_log(logfilename);
    // REQUIRE( with_rank_log.is_open() );
    std::string line, content;
    while (std::getline(with_rank_log,line)) {
      content += line;
    }
    // REQUIRE (content=="message");

    std::ifstream without_rank_log(logfilename);
  }
}

TEST_CASE("root_rank_console_only", "[logging]") {

  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);
  std::string log_name = "root_rank_console_only";

  std::string msg = "hello from rank" + std::to_string(comm.rank());
  {
    Logger<LogNoFile,LogRootRank> mylog(add_comm_size("console_root_rank_only",comm), LogLevel::debug, comm);
    mylog.set_no_format();
    mylog.warn(msg);
  }
}

TEST_CASE("root_rank_file_only", "[logging]") {

  using namespace ekat;
  using namespace ekat::logger;

  Comm comm(MPI_COMM_WORLD);
  std::string log_name = add_comm_size("root_rank_file_only",comm);

  std::string msg = "rank" + std::to_string(comm.size()) + "." + std::to_string(comm.rank());
  std::string msg0 = "rank" + std::to_string(comm.size()) + ".0";
  {
    Logger<LogBasicFile,LogRootRank> log(log_name, LogLevel::debug, comm);
    log.set_console_level(LogLevel::off);
    log.set_no_format();
    log.warn(msg);
  }

  // Make sure root_rank is done flushing to file
  comm.barrier();

  // Verify rank-specific files were not generated
  for (int i=0; i<comm.size(); ++i) {
    const auto logfilename = LogAllRanks::get_log_name(log_name,comm);
    std::ifstream lf(logfilename);
    REQUIRE (not lf.is_open() );
  }

  // Verify content of root rank log
  std::ifstream lf(log_name+".log");
  REQUIRE( lf.is_open() );
  std::string line, content;
  while (std::getline(lf,line)) {
    content += line;
  }
  REQUIRE (content==msg0);
}
