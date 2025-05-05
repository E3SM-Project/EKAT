#ifndef EKAT_LOG_FILE_POLICY_HPP
#define EKAT_LOG_FILE_POLICY_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <string>

// This file contains policy classes for choosing the behavior of the ekat
// logger with respect to file output.
// Each policy must implement get_file_sink(std::string logfilename),
// returning a shared_ptr<spdlog::sinks::sink>

namespace ekat {
namespace logger {

// No file output; only console logging.
struct LogNoFile {
  using sink_t = spdlog::sinks::sink;
  using file_sink_t = spdlog::sinks::null_sink_mt;

  static std::shared_ptr<sink_t> get_file_sink(const std::string& /* file_name */) {
    return std::make_shared<file_sink_t>();
  }
};

// Basic file output. Each Logger writes to its own file. No file size limit.
struct LogBasicFile {
  using sink_t = spdlog::sinks::sink;
  using file_sink_t = spdlog::sinks::basic_file_sink_mt;

  static std::shared_ptr<sink_t> get_file_sink(const std::string& file_name) {
    return std::make_shared<file_sink_t>(file_name, true);
  }
};

// Large file output.
// Each Logger (usually, 1 per MPI rank) writes to its own
// unique set of log files.  The number of log files is determined
// by LogBigFiles::num_files, and each file is limited to a maximum of
// LogBigFiles::mb_per_file megabytes. When the first file is full, it is
// closed and the next file is started. If the nth file gets full, the first
// file is overwritten, then the second file is overwritten if that file gets
// full, etc.
struct LogBigFiles {
  static constexpr bool has_filename  = true;
  static constexpr int one_mb         = 1024*1024;
  static constexpr int num_files      = 5;
  static constexpr int mb_per_file    = 8;

  using sink_t = spdlog::sinks::sink;
  using file_sink_t = spdlog::sinks::rotating_file_sink_mt;

  static std::shared_ptr<sink_t> get_file_sink(const std::string& file_name) {
    constexpr auto file_size = mb_per_file * one_mb;
    return std::make_shared<file_sink_t>(file_name, file_size, num_files);
  }
};

}// namespace logger
}//  namespace ekat

#endif // EKAT_LOG_FILE_POLICY_HPP
