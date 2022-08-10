#include "ekat/logging/ekat_log_file_policy.hpp"
#include "ekat/logging/ekat_log_mpi_policy.hpp"
#include "ekat/logging/ekat_logger.hpp"
#include "ekat/mpi/ekat_comm.hpp"

#include <catch2/catch.hpp>

#include <fstream>

TEST_CASE("log tests", "[logging]") {
  using namespace ekat;
  using namespace ekat::logger;

  ekat::Comm comm;

  SECTION("log_to_file") {

    std::string dbg_line = "here is a debug message that will also show up in this rank's log file.";
    {
      Logger<LogBasicFile,LogRootRank>
        mylog("log_two", LogLevel::debug, comm);
      mylog.set_file_level(LogLevel::trace);
      mylog.set_no_format();

      // Although the file level is trace, the logger level is debug
      REQUIRE( !mylog.should_log(LogLevel::trace) );

      REQUIRE (mylog.get_logfile_name()=="log_two.log");

      mylog.debug(dbg_line);

      // the file level is trace, but the log level is debug (debug > trace),
      // so the trace messages will be skipped.
      mylog.trace("this message won't show up anywhere.");
    }

    // The log file is not necessarily written as you write log messages.
    // For performance reasons, log messages may first be collated into a buffer; that
    // buffer is then written according to spdlog's internal logic.
    // This means that in order to check the previous section's log file, we have to
    // wait for its log to go out of scope (which guarantees that the file will be written).
    // The content of the log file should just be the debug line.
    const std::string logfilename = "log_two.log";
    std::ifstream lf(logfilename);
    REQUIRE( lf.is_open() );
    std::string line, content;
    while (std::getline(lf,line)) {
      content += line;
    }
    REQUIRE (content==dbg_line);
  }

  SECTION("two_loggers_sinks_sharing") {
    using logger_t = Logger<LogBasicFile,LogRootRank>;
    std::string info_line_1 = "info line 1";
    std::string info_line_2 = "info line 2";
    std::string warn_line = "warn line";
    {
      logger_t src("log_three",LogLevel::warn,comm);
      logger_t dst("log_four", LogLevel::info, comm, src);
      src.set_no_format();
      dst.set_no_format();

      // Check we inherit sinks
      REQUIRE (dst.file_level()==LogLevel::warn);
      REQUIRE (dst.console_level()==LogLevel::warn);

      src.warn(warn_line);
      dst.info(info_line_1);

      dst.set_file_level(LogLevel::trace);
      dst.info(info_line_2);
    }

    // Check log content only after they go out of scope, since that's the only
    // way to guarantee that buffers have been flushed by spdlog.
    const std::string logfilename = "log_three.log";
    std::ifstream lf(logfilename);
    REQUIRE( lf.is_open() );
    std::string line, content;
    while (std::getline(lf,line)) {
      content += line + "\n";
    }
    std::string expected = warn_line + "\n" + info_line_2 + "\n";
    REQUIRE (content==expected);
  }
}
