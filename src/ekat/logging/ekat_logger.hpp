#ifndef EKAT_LOGGER_HPP
#define EKAT_LOGGER_HPP

#include "spdlog/spdlog.h"
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ekat {
namespace logger {

// expose the default logger
namespace Log = spdlog;

/** Logging levels (increasing priority):
  - off
  - trace
  - debug
  - info
  - warn
  - err
  - critical
*/


/** @brief Logger for console and file output.

*/
template <typename LogFilePolicy = LogNoFile,
   typename LogMpiPolicy = LogIgnoreRank>
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
    fsink->set_level(clevel);

    this->sinks().push_back(csink);
    this->sinks().push_back(fsink);

    LogMpiPolicy::update_sinks(this->sinks());

    this->set_level(clevel);
  }


  ~Logger() {
    spdlog::drop(this->name());
  }
};




} // namespace logger
} // namespace ekat
#endif
