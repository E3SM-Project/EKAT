#ifndef EKAT_LOG_MPI_HPP
#define EKAT_LOG_MPI_HPP

#include "ekat/mpi/ekat_comm.hpp"
#include "spdlog/sinks/null_sink.h"
#include <mpi.h>

// This file contains policy classes for choosing the behavior of the ekat
// logger with respect to mpi ranks.
// Each policy must implement the
//
// static void update_console_sink(spdlog::sink_ptr& csink)
//
// method.
namespace ekat {

/** Console log ignores MPI Rank.
*/
struct LogIgnoreRank {
  static void update_console_sink(spdlog::sink_ptr& csink) {}
  static std::string name_append_rank(const std::string& name) {
    return name;
  }
};


/** Suppress output from ranks > 0.
*/
struct LogOnlyRank0 {
  static void update_console_sink(spdlog::sink_ptr& csink) {
    const Comm mpicomm(MPI_COMM_WORLD);
    if (!mpicomm.am_i_root()) {
      csink = std::make_shared<spdlog::sinks::null_sink_mt>();
    }
  }

  static std::string name_append_rank(const std::string& name) {
    return name + "_rank0";
  }
};

/** Suppress output from ranks > 0.
*/
struct LogAllRanks {
  static void update_console_sink(spdlog::sink_ptr& csink) {}

  static std::string name_append_rank(const std::string& name) {
    const Comm mpicomm(MPI_COMM_WORLD);
    return name + "_rank" + std::to_string(mpicomm.rank());
  }
};

}

#endif
