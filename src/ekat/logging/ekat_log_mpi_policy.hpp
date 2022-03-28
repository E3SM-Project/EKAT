#ifndef EKAT_LOG_MPI_POLICY_HPP
#define EKAT_LOG_MPI_POLICY_HPP

#include "ekat/mpi/ekat_comm.hpp"

// This file contains policy classes for choosing the behavior of the ekat
// logger with respect to MPI ranks.

namespace ekat {
namespace logger {

// =========== Log Name Policies =========== //

// Append ".$Size.$Rank" to input string
struct NameWithRank {
  static std::string get_name (const std::string& prefix, const Comm& comm) {
    return prefix + "." + std::to_string(comm.size())
                  + "." + std::to_string(comm.rank());
  }
};

// Returns input string
struct NameWithoutRank {
  static std::string get_name (const std::string& prefix, const Comm& /* comm */) {
    return prefix;
  }
};

// =========== Output Policies =========== //

// All ranks have output
struct LogAllRanks {
  static bool should_log (const Comm& /* comm */) {
    return true;
  }
};

// Only root rank has output
struct LogRank0 {
  static bool should_log (const Comm& comm) {
    return comm.am_i_root();
  }
};

}// namespace logger
}//  namespace ekat

#endif // EKAT_LOG_MPI_POLICY_HPP
