#include "ekat/util/ekat_utils.hpp"

namespace ekat {
namespace util {

bool eq (const std::string& a, const char* const b1, const char* const b2) {
  return (a == std::string(b1) || (b2 && a == std::string(b2)) ||
          a == std::string("-") + std::string(b1));

}

} // namespace util
} // namespace ekat
