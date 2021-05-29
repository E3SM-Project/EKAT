#ifndef EKAT_LOGGER_HPP
#define EKAT_LOGGER_HPP

#include "spdlog/spdlog.h"
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

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
          typename LogMpiPolicy = LogIgnoreRank> // see ekat_log_mpi.hpp
class Logger : public spdlog::logger {
  public:
    Logger(const std::string& log_name, const std::string& level_str="debug") :
     spdlog::logger(LogMpiPolicy::name_append_rank(log_name))
  {

    const auto clevel = Log::level::from_str(level_str);
    auto csink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    csink->set_level(clevel);

    std::string fname = LogMpiPolicy::name_append_rank(log_name) + "_logfile.txt";
    auto fsink = LogFilePolicy::get_file_sink(fname);

    this->sinks().push_back(csink);
    this->sinks().push_back(fsink);

    LogMpiPolicy::update_sinks(this->sinks());

    this->set_level(clevel);
  }

  spdlog::sink_ptr get_console_sink() {return this->sinks()[0];}

  spdlog::sink_ptr get_file_sink() {
    return (this->sinks().size() > 1 ? this->sinks()[1] : spdlog::sink_ptr(nullptr));
  }

};




} // namespace logger
} // namespace ekat
#endif
