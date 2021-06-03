#ifndef EKAT_LOGGER_HPP
#define EKAT_LOGGER_HPP

#include "spdlog/spdlog.h"
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/null_sink.h"

namespace ekat {
namespace logger {

// expose the default logger; this is the easiest one to use
namespace Log = spdlog;

/* A Logger customized with policies for console and file output.

  The default Logger<> sets up colored output to the console only (no log files), for all mpi ranks.

  Logs have a priority logging level.  Messages with priorities below this level will
  be ignored.

  Logging levels (increasing priority):
  - off
  - trace
  - debug
  - info
  - warn
  - err
  - critical

  The default priority for the spdlog library is "info."  For EKAT, we've set the
  default to "debug."  Both are easy to change.

  The LogMpiPolicy can restrict console output to only rank 0, and it will append
  rank ids to log files and log names.

  The LogFilePolicy determines whether or not the log will also write output to a file.
  Files and console can have different logging levels.  For example, the
  log level for the LogBasicFile policy could be "debug," while the level of the console
  logger could be set to "info".  In this case, no debug messages will show up on the
  console, but they will be in the log files.

  In principle, separate modules, classes, etc. could have their own logger.
  These loggers can share output files, too; see the spdlog wiki
(https://github.com/gabime/spdlog/wiki/2.-Creating-loggers#creating-multiple-file-loggers-with-the-same-output-file) for an example.

  The ekat::Logger class is a subclass of spdlog::logger class, to simplify the many
  features of that library to the ones EKAT is most likely to use, while retaining all
  of its functionality.
*/
template <typename LogFilePolicy = LogNoFile,    // see ekat_log_file.hpp
          typename LogMpiPolicy = LogOnlyRank0> // see ekat_log_mpi.hpp
class Logger : public spdlog::logger {
  public:

  // primary constructor
  Logger(const std::string& log_name, const Log::level::level_enum lev=Log::level::debug,
    const ekat::Comm& mpicomm=ekat::Comm()) :
    spdlog::logger(LogMpiPolicy::name_with_rank(log_name, mpicomm))
  {
    auto csink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    csink->set_level(lev);

    std::string fname = LogMpiPolicy::name_with_rank(log_name, mpicomm) + "_logfile.txt";
    auto fsink = LogFilePolicy::get_file_sink(fname);

    this->sinks().push_back(csink);
    this->sinks().push_back(fsink);

    LogMpiPolicy::update_sinks(this->sinks(), mpicomm);

    this->set_level(lev);
  }

  // shared file sink constructor
  template <typename FP, typename MP>
  Logger(const std::string& log_name, const Log::level::level_enum lev,
    Logger<FP, MP>& other_log, const ekat::Comm& mpicomm=ekat::Comm()) :
    spdlog::logger(LogMpiPolicy::name_with_rank(log_name, mpicomm))
  {
    auto csink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    csink->set_level(lev);
    this->sinks().push_back(csink);
    this->sinks().push_back(other_log.get_file_sink());
    this->set_level(lev);

    LogMpiPolicy::update_sinks(this->sinks(), mpicomm);
  }

  // Constructor for externally created sinks
  Logger(const std::string& log_name, const Log::level::level_enum lev,
    spdlog::sink_ptr csink, spdlog::sink_ptr fsink, const ekat::Comm& mpicomm=ekat::Comm()) :
    spdlog::logger(LogMpiPolicy::name_with_rank(log_name, mpicomm), {csink, fsink}) {}

  spdlog::sink_ptr get_console_sink() {
    //return (this->sinks().size() > 1 ? this->sinks()[0] : spdlog::sink_ptr(nullptr));
    return this->sinks()[0];
  }

  spdlog::sink_ptr get_file_sink() {
    //return (this->sinks().size() > 1 ? this->sinks()[1] : this->sinks()[0]);
    return this->sinks()[1];
  }

  template <typename FP = LogFilePolicy>
  typename std::enable_if<FP::has_filename, std::string>::type
  get_logfile_name() {
    std::string result;
    auto fsink = this->get_file_sink();
    spdlog::sinks::basic_file_sink_mt* bptr =
      dynamic_cast<spdlog::sinks::basic_file_sink_mt*>(fsink.get());
    spdlog::sinks::rotating_file_sink_mt* rptr =
      dynamic_cast<spdlog::sinks::rotating_file_sink_mt*>(fsink.get());
    if (bptr) {
      result = bptr->filename();
    }
    else if (rptr) {
      result = rptr->filename();
    }
    return result;
  }

  template <typename FP = LogFilePolicy>
  typename std::enable_if<!FP::has_filename, std::string>::type
  get_logfile_name() {return "null";}

  private:
    Logger() = default;

};



} // namespace logger
} // namespace ekat
#endif
