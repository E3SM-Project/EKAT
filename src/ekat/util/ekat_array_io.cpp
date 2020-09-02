#include "ekat/util/ekat_file_utils.hpp"

#include <sys/stat.h>

#include <iostream>

namespace ekat {
template <typename Scalar>
void write (const char* filename, Scalar* a, const int n) {
  FILEPtr fid(fopen(filename, "w"));
  EKAT_REQUIRE_MSG( fid, "Could not open " << filename << " for writing.");
  write<int>(&n, 1, fid);
  write<Scalar>(a, n, fid);
}

template <typename Scalar>
void read (const char* filename, Scalar* a, const int n) {
  FILEPtr fid(fopen(filename, "r"));
  EKAT_REQUIRE_MSG( fid, "Could not open " << filename << " for reading.");
  int n_file;
  read<int>(&n_file, 1, fid);
  EKAT_REQUIRE_MSG(n_file == n, "Expected " << n << " but got " << n_file);
  read<Scalar>(a, n, fid);
}
} // namespace ekat

extern "C" {
bool array_io_file_exists (const char* filename) {
  struct stat s;
  const bool exists = stat(filename, &s) == 0;
  return exists;
}

// F90 has C linking, so we need to duplicate the function for single/double precision
bool array_io_write_double (const char* filename, double** a, const int n) {
  try {
    ekat::write(filename, *a, n);
    return true;
  } catch (std::exception& e) {
    std::cerr << "array_io_write failed with: " << e.what() << "\n";
    return false;
  }
}
bool array_io_write_float (const char* filename, float** a, const int n) {
  try {
    ekat::write(filename, *a, n);
    return true;
  } catch (std::exception& e) {
    std::cerr << "array_io_write failed with: " << e.what() << "\n";
    return false;
  }
}

bool array_io_read_double (const char* filename, double** a, const int n) {
  try {
    ekat::read(filename, *a, n);
    return true;
  } catch (std::exception& e) {
    std::cerr << "array_io_read failed with: " << e.what() << "\n";
    return false;
  }
}
bool array_io_read_float (const char* filename, float** a, const int n) {
  try {
    ekat::read(filename, *a, n);
    return true;
  } catch (std::exception& e) {
    std::cerr << "array_io_read failed with: " << e.what() << "\n";
    return false;
  }
}
} // extern "C"
