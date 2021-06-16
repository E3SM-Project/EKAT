#ifndef EKAT_LOG_FILE_HPP
#define EKAT_LOG_FILE_HPP

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <string>

// This file contains policy classes for choosing the behavior of the ekat
// logger with respect to file output.
// Each policy must implement the
//
// static spdlog::sink_ptr get_file_sink(std::string logfilename)
//
// method.

namespace ekat {
namespace logger {

// To support large logging tasks, you can optionally distribute output across a range
// of files using the "rotating_file_sink".
// These two #defines are used only by the LogBigFiles policy.
// EKAT_LOG_N_FILES sets the number of rotating files
#define EKAT_LOG_N_FILES 5
// EKAT_LOG_MAX_FILE_SIZE_MB sets the max size of each file
#define EKAT_LOG_MAX_FILE_SIZE_MB 8

// No file output; only console logging.
struct LogNoFile {
  static constexpr bool has_filename = false;
  static std::shared_ptr<spdlog::sinks::null_sink_mt>
    get_file_sink(const std::string& logfilename="") {
    return std::make_shared<spdlog::sinks::null_sink_mt>();}
};

// Basic file output.  Each Logger (usually, 1 per MPI rank) writes to its own
// file.  No file size limit.
template <spdlog::level::level_enum FileLogLevel = spdlog::level::debug>
struct LogBasicFile {
  static constexpr bool has_filename = true;
  static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> get_file_sink(
    const std::string& logfilename) {
    auto result = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilename, true);
    result->set_level(FileLogLevel);
    return result;
  }
};

// Large file output.  Each Logger (usually, 1 per MPI rank) writes to its own
// unique set of log files.  The number of log files is determined at compile
// time by EKAT_N_LOG_FILES, and each file is limited to a maximum of
// EKAT_MAX_LOG_FILE_SIZE_MB megabytes.  When the first file is full, it is
// closed and the next file is started.   If the nth file gets full, the first
// file is overwritten, then the second file is overwritten if that file gets
// full, etc.
template <spdlog::level::level_enum FileLogLevel = spdlog::level::debug>
struct LogBigFiles {
  static constexpr bool has_filename = true;
  static constexpr int one_mb = 1048576;
  static constexpr int n_files = EKAT_LOG_N_FILES;
  static constexpr int mb_per_file = EKAT_LOG_MAX_FILE_SIZE_MB;
  static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> get_file_sink(
    const std::string& logfilename) {
    auto result = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      logfilename, mb_per_file * one_mb, n_files);
    result->set_level(FileLogLevel);
    return result;
  }
};

}// namespace logger
}//  namespace ekat
#endif
