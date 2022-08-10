#ifndef EKAT_LOGGER_HPP
#define EKAT_LOGGER_HPP

#include "ekat/logging/ekat_log_file_policy.hpp"
#include "ekat/logging/ekat_log_mpi_policy.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_assert.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>

namespace ekat {
namespace logger {

// Some common types from spdlog
using LogLevel = spdlog::level::level_enum;

/* A Logger class customized for console and file output.

  Each log has two "sinks" for output: one for the console, and one for file(s).
  Either sink can be null (to suppress output), but they both always exist.

  The Logger class is templated on three policies, that regulate how the two
  sinks behave: LogFilePolicy, MpiOutputPolicy, and LogNamePolicy:
   - LogFilePolicy: determines what kind of file sink to create
   - MpiOutputPolicy: determines which ranks can produce output
   - LogNamePolicy: determines if MPI rank info enters the file name

  Logs have a logging level. Messages with priorities below this level will
  be ignored by all sinks. Independently, each Log's sink has its own level,
  and messages with priority below the sink's priority level will be ignored
  by that sink.

  Logging levels (increasing priority):
  - off
  - trace
  - debug
  - info
  - warn
  - err
  - critical

  For example, the log level for the LogBasicFile policy could be "debug," while the
  level of the console logger could be set to "info". In this case, no debug
  messages will show up on the console, but they will be in the log files.

  Once the logger is created, you can log information by calling the proper method:
    logger.info("something here")
    logger.debug("my debug stuff")
    logger.warn("watch out!")
  Depending on the logger level and the levels of the sinks, some info may not appear
  in the log. E.g., if logger had level "info" the second call would not produce any
  log. Furthermore, if one of the two sinks had level "warn", the first call would
  also not produce any log *in that sink*.

  In principle, separate modules, classes, etc. could have their own logger.
  These loggers can share output files; see tests/logger/logger_tests.cpp.

  The ekat::Logger class inherits from spdlog::logger, to simplify the many
  features of that library to the ones EKAT is most likely to use, while retaining all
  of its functionality.
*/

template<typename LogFilePolicy   = LogNoFile,
         typename MpiOutputPolicy = LogRootRank>
class Logger : public spdlog::logger {
private:

  using sink_t = spdlog::sinks::sink;

  std::shared_ptr<sink_t> csink;
  std::shared_ptr<sink_t> fsink;

  std::string logfile_name;

public:

  // primary constructor
  Logger (const std::string& log_name,
          const LogLevel log_level,
          const ekat::Comm& comm,
          const std::string& suffix = ".log")
   : spdlog::logger(log_name)
   , logfile_name  ("")
  {
    // make the console sink; default console level = log level
    csink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    if (MpiOutputPolicy::should_log(comm)) {
      logfile_name = (MpiOutputPolicy::get_log_name(log_name, comm) + suffix);
      fsink = LogFilePolicy::get_file_sink(logfile_name);
    } else {
      fsink = LogNoFile::get_file_sink(logfile_name);
    }
    this->sinks().push_back(csink);
    this->sinks().push_back(fsink);

    // Set the log level of logger as well as the sinks
    if (not MpiOutputPolicy::should_log(comm)) {
      this->set_level(LogLevel::off);
    } else {
      this->set_level(log_level);
      csink->set_level(log_level);
      fsink->set_level(log_level);
    }
  }

  // Shared sinks constructor
  template <typename MOP>
  Logger(const std::string& log_name,
         const LogLevel log_level,
         const ekat::Comm& comm,
         Logger<LogFilePolicy,MOP>& src) :
    spdlog::logger(log_name)
  {
    csink = src.get_console_sink();
    fsink = src.get_file_sink();
    this->sinks().push_back(csink);
    this->sinks().push_back(fsink);

    // Only set the level of the logger. Do *not* alter the sinks levels,
    // since you are "borrowing" them from another logger
    if (not MpiOutputPolicy::should_log(comm)) {
      this->set_level(LogLevel::off);
    } else {
      this->set_level(log_level);
    }
  }

  // accessors
  spdlog::sink_ptr get_console_sink() const {
    return csink;
  }
  spdlog::sink_ptr get_file_sink() const {
    return fsink;
  }

  LogLevel log_level() const {
    return this->level();
  }
  LogLevel console_level() const {
    return this->get_console_sink()->level();
  }
  LogLevel file_level() const {
    return this->get_file_sink()->level();
  }

  void set_log_level(const LogLevel lev) {
    this->set_level(lev);
  }
  void set_console_level(const LogLevel lev) {
    this->get_console_sink()->set_level(lev);
  }
  void set_file_level(const LogLevel lev) {
    this->get_file_sink()->set_level(lev);
  }

  // Use this to change the format of logged messages.
  // The default is "[date time] [log name] [log level] <msg>".
  void set_format (const std::string& s = "%v") {
    this->set_pattern(s);
  }

  // The string "%v" is corresponds to "<msg>" only.
  void set_no_format () { set_format("%v"); }


  // Get the log file name (if any)
  std::string get_logfile_name () const { return logfile_name; }
};

} // namespace logger
} // namespace ekat

#endif // EKAT_LOGGER_HPP
