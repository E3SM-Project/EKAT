#ifndef EKAT_LOG_MPI_HPP
#define EKAT_LOG_MPI_HPP

#include "ekat/mpi/ekat_comm.hpp"
#include "spdlog/sinks/null_sink.h"
#include <mpi.h>

// This file contains policy classes for choosing the behavior of the ekat
// logger with respect to mpi ranks.
// Each policy must implement the
//
// static void update_sinks(std::vector<spdlog::sink_ptr>& sinks)
// static std::string name_with_rank(const std::string& name)
//
// methods.
namespace ekat {

/* Console log ignores MPI Rank; all ranks will write to the console without rank ids.
  File logs (if used) will still have their rank id in their filenames.
*/
struct LogIgnoreRank {
  // leave sinks alone
  static void update_sinks(std::vector<spdlog::sink_ptr>& sinks) {}

  // leave name alone
  static std::string name_with_rank(const std::string& name) {
    return name;
  }
};


/* Suppress console output from ranks > 0.  All ranks still write to files, if
  file logging is enabled.
*/
struct LogOnlyRank0 {
  // erase the console sink for all ranks != 0
  static void update_sinks(std::vector<spdlog::sink_ptr>& sinks) {
    const Comm mpicomm(MPI_COMM_WORLD);
    if (!mpicomm.am_i_root()) {
      sinks.erase(sinks.begin());
    }
  }

  // append rank id to the log name
  static std::string name_with_rank(const std::string& name) {
    const Comm mpicomm(MPI_COMM_WORLD);
    return name + "_rank" + std::to_string(mpicomm.rank());
  }
};

/* All ranks write to the console with rank ids.
*/
struct LogAllRanks {
  // leave all sinks alone, including the console
  static void update_sinks(std::vector<spdlog::sink_ptr>& sinks) {}

  // append rank id to the log name
  static std::string name_with_rank(const std::string& name) {
    const Comm mpicomm(MPI_COMM_WORLD);
    return name + "_rank" + std::to_string(mpicomm.rank());
  }
};

}

#endif
