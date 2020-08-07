#include "ekat/ekat_types.hpp"
#include "ekat/util/ekat_utils.hpp"
#include "ekat/util/file_utils.hpp"
#include "ekat/ekat_assert.hpp"

#include <sys/stat.h>

#include <iostream>

namespace ekat {
namespace util {
template <typename Scalar>
void write (const char* filename, Scalar* a, const int n) {
  FILEPtr fid(fopen(filename, "w"));
  ekat_require_msg( fid, "Could not open " << filename << " for writing.");
  write<int>(&n, 1, fid);
  write<Scalar>(a, n, fid);
}

template <typename Scalar>
void read (const char* filename, Scalar* a, const int n) {
  FILEPtr fid(fopen(filename, "r"));
  ekat_require_msg( fid, "Could not open " << filename << " for reading.");
  int n_file;
  read<int>(&n_file, 1, fid);
  ekat_require_msg(n_file == n, "Expected " << n << " but got " << n_file);
  read<Scalar>(a, n, fid);
}
} // namespace util
} // namespace ekat

extern "C" {
bool array_io_file_exists (const char* filename) {
  struct stat s;
  const bool exists = stat(filename, &s) == 0;
  return exists;
}

bool array_io_write (const char* filename, ekat::Real** a, const int n) {
  try {
    ekat::util::write(filename, *a, n);
    return true;
  } catch (std::exception& e) {
    std::cerr << "array_io_write failed with: " << e.what() << "\n";
    return false;
  }
}

bool array_io_read (const char* filename, ekat::Real** a, const int n) {
  try {
    ekat::util::read(filename, *a, n);
    return true;
  } catch (std::exception& e) {
    std::cerr << "array_io_read failed with: " << e.what() << "\n";
    return false;
  }
}
} // extern "C"
