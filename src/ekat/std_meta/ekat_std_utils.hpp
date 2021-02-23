#ifndef EKAT_STD_UTILS_HPP
#define EKAT_STD_UTILS_HPP

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <set>

namespace ekat {

// This function returns an iterator which is of the same type of c.begin()
template<typename ContainerType, typename T>
auto find (ContainerType& c, T&& value) -> decltype(c.begin()) {
  return std::find(c.begin(),c.end(),value);
}

// Note: in C++20, both std::set and std::map will have the 'contains' method.
template<typename ContainerType, typename T>
bool contains (const ContainerType& c, T&& value) {
  return std::find(c.begin(),c.end(),value) != c.end();
}

template<typename ContainerType, typename T>
bool erase (ContainerType& c, const T& value) {
  auto it = std::find(c.begin(),c.end(),value);
  if (it!=c.end()) {
    c.erase(it);
    return true;
  }
  return false;
}

// Shortcuts to avoid using long iterators syntax (like v.begin() and v.end())
template<typename ContainerType, typename T>
int count (const ContainerType& c, const T& value) {
  return std::count(c.begin(), c.end(), value);
}

// A set of weak_ptr would not compile, due to the lack of operator<.
// To overcome this, one could add a Compare type to the set template
// arguments.

template<typename T>
struct WeakPtrLess {
  inline bool operator () (const std::weak_ptr<T>& p, const std::weak_ptr<T>& q) const {
    return p.owner_before(q);
  }
};

template<typename T>
using WeakPtrSet = std::set<std::weak_ptr<T>,WeakPtrLess<T>>;

} // namespace ekat

// Note: this *must* be in the std namespace if you want ADL lookup to work
//       when doing something like
//          os << my_vec
//       with os of type std::ostream, and my_vec a std::vector<T>.

namespace std {

template<typename T>
ostream& operator<< (ostream& out, const vector<T>& v) {
  const int n = v.size();
  if (n==0) {
    return out;
  }

  for (int i=0; i<n-1; ++i) {
    out << v[i] << std::string(" ");
  }
  out << v.back();

  return out;
}

} // namespace std

#endif // EKAT_STD_UTILS_HPP
