#ifndef EKAT_LOGGER_HPP
#define EKAT_LOGGER_HPP

#include "spdlog/spdlog.h"
#include "ekat/ekat_assert.hpp"
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/mpi/ekat_comm.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/null_sink.h"
#include <iostream>
#include <cstdio>

namespace ekat {
namespace logger {

// expose the default logger; this is the easiest one to use
namespace Log = spdlog; // capitalized to avoid conflict with logarithm

/* A Logger class customized for console and file output.

  Each log has two "sinks" for output, the first is for the console, the second
  is for files.  Either sink can be null (to suppress output), but they both always exist.

  The default Logger<> sets up colored output to the console only (no log files),
  for all mpi ranks. The LogFilePolicy template parameter determines whether or not the
  log will also write output to a file. Files and console can have different logging levels.

  Logs have a logging level.  Messages with priorities below this level will
  be ignored by all sinks.  Independently, each Log's sink has its own level, and messages
  with priority below the sink's priority level will be ignored by that sink.

  Logging levels (increasing priority):
  - off
  - trace
  - debug
  - info
  - warn
  - err
  - critical

  For example, the log level for the LogBasicFile policy could be "debug," while the
  level of the console logger could be set to "info".  In this case, no debug
  messages will show up on the console, but they will be in the log files.

  The default priority for the spdlog library is "info."  For EKAT, we've set the
  default to "debug."  Both are easy to change.

  In principle, separate modules, classes, etc. could have their own logger.
  These loggers can share output files; see multiple_log_example.cpp.

  The ekat::Logger class is a subclass of spdlog::logger class, to simplify the many
  features of that library to the ones EKAT is most likely to use, while retaining all
  of its functionality.
*/
template <typename LogFilePolicy = LogNoFile> // see ekat_log_file.hpp
class Logger : public spdlog::logger {
  private:
    std::string logfile_name_;

    std::string name_with_rank(const std::string& name, const ekat::Comm& comm) const {
      return name + "_rank" + std::to_string(comm.rank());
    }

    Logger() = default;

  public:

  // primary constructor
  Logger(const std::string& log_name, const Log::level::level_enum lev,
    const ekat::Comm& comm) :
    spdlog::logger(name_with_rank(log_name, comm)),
    logfile_name_( (LogFilePolicy::has_filename ?
      name_with_rank(log_name, comm) + "_logfile.txt" : "null") )
  {
    // make the console sink; default console level = log level
    auto csink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    csink->set_level(lev);
    // make the file sink; its level is set by the file policy
    auto fsink = LogFilePolicy::get_file_sink(logfile_name_);
    // the console sink is always sink [0]
    this->sinks().push_back(csink);
    // the file sink is always sink [1]
    this->sinks().push_back(fsink);
    // set the log level
    this->set_level(lev);
  }

  // shared file sink constructor
  template <typename FP>
  Logger(const std::string& log_name, const Log::level::level_enum lev,
    Logger<FP>& other_log, const ekat::Comm& comm) :
    spdlog::logger(name_with_rank(log_name, comm)),
    logfile_name_( (LogFilePolicy::has_filename ?
      name_with_rank(log_name, comm) + "_logfile.txt" : "null") )
  {
    // make a new console sink for this log; default console level = log level
    auto csink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    csink->set_level(lev);
    this->sinks().push_back(csink);
    // this log shares the other's file sink
    this->sinks().push_back(other_log.get_file_sink());
    // it needs to share its logfile name, too
    logfile_name_ = other_log.logfile_name_;
    // set the log level
    this->set_level(lev);
  }

  // Constructor for externally created sinks
  Logger(const std::string& log_name, const Log::level::level_enum lev,
    spdlog::sink_ptr csink, spdlog::sink_ptr fsink, const ekat::Comm& comm) :
    spdlog::logger(name_with_rank(log_name, comm), {csink, fsink}),
    logfile_name_( (LogFilePolicy::has_filename ?
      name_with_rank(log_name, comm) + "_logfile.txt" : "null") ) {}

  // accessors
  spdlog::sink_ptr get_console_sink() const {
    return this->sinks()[0];
  }

  spdlog::sink_ptr get_file_sink() const {
    return this->sinks()[1];
  }

  std::string logfile_name() const {return logfile_name_;}

  Log::level::level_enum log_level() const {return this->level();}

  Log::level::level_enum console_level() const {return this->get_console_sink()->level();}

  Log::level::level_enum file_level() const {return this->get_file_sink()->level();}

  void set_log_level(const Log::level::level_enum lev) {
    this->set_level(lev);
  }

  void set_console_level(const Log::level::level_enum lev) {
    this->get_console_sink()->set_level(lev);
  }

  void set_file_level(const Log::level::level_enum lev) {
    this->get_file_sink()->set_level(lev);
  }

  // nullify console output from ranks != 0
  void console_output_rank0_only(const ekat::Comm& comm) {
    if (!comm.am_i_root()) {
      this->sinks()[0] = std::make_shared<spdlog::sinks::null_sink_mt>();
      EKAT_ASSERT( dynamic_cast<spdlog::sinks::null_sink_mt*>(this->get_console_sink().get()) );
    }
  }

  // nullify file output from ranks != 0
  void file_output_rank0_only(const ekat::Comm& comm) {
    if (!comm.am_i_root()) {
      this->sinks()[1] = std::make_shared<spdlog::sinks::null_sink_mt>();
      std::remove(logfile_name_.c_str());
      this->logfile_name_ = "null";
      EKAT_ASSERT( dynamic_cast<spdlog::sinks::null_sink_mt*>(this->get_file_sink().get()));
    }
  }
};



} // namespace logger
} // namespace ekat
#endif
