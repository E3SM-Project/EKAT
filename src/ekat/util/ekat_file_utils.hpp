#ifndef EKAT_FILE_UTILS_HPP
#define EKAT_FILE_UTILS_HPP

#include <cstdio>
#include <sstream>
#include <memory>

#include "ekat/ekat_assert.hpp"

namespace ekat {

struct FILECloser { void operator() (FILE* fh) { fclose(fh); } };
using FILEPtr = std::unique_ptr<FILE, FILECloser>;

template<typename T>
void write (const T* v, size_t sz, const FILEPtr& fid) {
  size_t nwrite = fwrite(v, sizeof(T), sz, fid.get());
  EKAT_REQUIRE_MSG(nwrite == sz, "write: nwrite = " << nwrite << " sz = " << sz);
}

template<typename T>
void read (T* v, size_t sz, const FILEPtr& fid) {
  size_t nread = fread(v, sizeof(T), sz, fid.get());
  EKAT_REQUIRE_MSG(nread == sz, "read: nread = " << nread << " sz = " << sz);
}

} // namespace ekat

#endif // EKAT_FILE_UTILS_HPP
