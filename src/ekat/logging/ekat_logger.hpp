#ifndef EKAT_LOGGER_HPP
#define EKAT_LOGGER_HPP

#include "spdlog/spdlog.h"
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"

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
template <typename OutputPolicy>
class Logger : public spdlog::logger {
  public:
  Logger(const std::string& log_name, const std::string& level_str) :
  spdlog::logger(log_name, OutputPolicy::get_sinks(log_name))
  {
    this->set_level(spdlog::level::from_str(level_str));
  }

};


} // namespace logger
} // namespace ekat
#endif
