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

/* Suppress console output from ranks > 0.  All ranks still write to files, if
  file logging is enabled.
*/
struct LogOnlyRank0 {
  // nullify the console sink for all ranks != 0
  static void update_sinks(std::vector<spdlog::sink_ptr>& sinks, const ekat::Comm& mpicomm=ekat::Comm()) {
    if (!mpicomm.am_i_root()) {
      //sinks.erase(sinks.begin()); // this is a hack, but it works
      sinks[0] = std::make_shared<spdlog::sinks::null_sink_mt>();
    }
  }

  // append rank id to the log name
  static std::string name_with_rank(const std::string& name, const ekat::Comm& mpicomm=ekat::Comm()) {
    return name + "_rank" + std::to_string(mpicomm.rank());
  }
};

/* All ranks write to the console with rank ids.
*/
struct LogAllRanks {
  // leave all sinks alone, including the console
  static void update_sinks(std::vector<spdlog::sink_ptr>& sinks, const ekat::Comm& mpicomm=ekat::Comm()) {}

  // append rank id to the log name
  static std::string name_with_rank(const std::string& name, const ekat::Comm& mpicomm=ekat::Comm()) {
    return name + "_rank" + std::to_string(mpicomm.rank());
  }
};

}

#endif
