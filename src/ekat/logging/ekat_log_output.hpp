#ifndef EKAT_LOG_OUTPUT_HPP
#define EKAT_LOG_OUTPUT_HPP

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "ekat/logging/ekat_log_file.hpp"
#include "ekat/logging/ekat_log_mpi.hpp"

namespace ekat {
namespace logger {



template <typename LogFilePolicy = LogNoFile,
          typename LogMpiPolicy = LogIgnoreRank,
          spdlog::level::level_enum ConsoleLogLevel = spdlog::level::debug>
struct LogOutputPolicy {
  static std::initializer_list<spdlog::sink_ptr> get_sinks(const std::string& log_name) {
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console->set_level(ConsoleLogLevel);
    const std::string fname = LogMpiPolicy::name_append_rank(log_name) + "_logfile.txt";
    auto file = LogFilePolicy::get_file_sink(fname);
    std::initializer_list<spdlog::sink_ptr> result({std::move(console), std::move(file)});
    return result;
  }
};

} // namespace logger
} // namespace ekat
#endif
