#ifndef EKAT_ZIP_HPP
#define EKAT_ZIP_HPP

#include <ekat_assert.hpp>

/*
 * Utilities to zip iterators
 *
 * This file defines utilities needed to "zip" iterators, backporting C++23 zip fcn
 * from the std::views and std::ranges namespaces. This utility allows to do
 *
 *   for (auto& [a.b,c] : ekat::zip(c1,c2,c3)) {
 *     // Use a,b,c
 *   }
 *
 * where c1, c2, and c3 are objects for which std::begin/end/cbegin/cend is well defined
 * (e.g., any std container, as well as static arrays)
 */

namespace ekat {

template <typename IteratorsTuple>
class ZipIterator;

template <typename... Iterators>
class ZipIterator<std::tuple<Iterators...>> {
public:
  static constexpr int N = sizeof...(Iterators);

  ZipIterator () = default;
  ZipIterator (std::tuple<Iterators...> iters) : iterators(iters) {}

  // Dereference operator
  auto operator*() {
    return std::apply([](Iterators... its){
      return std::make_tuple(*its...);
    },iterators);
  }

  // Increment operator
  ZipIterator& operator++() {
    auto inc = [](auto&... it) { (++it, ...); };
    std::apply(inc,iterators);
    return *this;
  }

  // Comparison operator
  bool operator!=(const ZipIterator& other) const {
    return !equal(std::make_index_sequence<N>{},other);
  }

private:
  template <std::size_t... Indices>
  bool equal (std::index_sequence<Indices...>, const ZipIterator& other) const {
    return (... && (std::get<Indices>(iterators) == std::get<Indices>(other.iterators)));
  }

  std::tuple<Iterators...> iterators;
};

template <typename... Iterators>
ZipIterator<Iterators...> zip_iterator (std::tuple<Iterators...> iters)
{
  return ZipIterator<Iterators...> (iters);
}

template <typename... Containers>
class Zip {
public:
  static constexpr int N = sizeof...(Containers);
  static_assert (N>0, "Zip<> is ill-formed.");

  Zip (const std::tuple<Containers&...>& containers)
   : _begin (std::apply([](auto&...args){return std::make_tuple(std::begin (args)...);},containers))
   , _end   (std::apply([](auto&...args){return std::make_tuple(std::end   (args)...);},containers))
   , _cbegin(std::apply([](auto&...args){return std::make_tuple(std::cbegin(args)...);},containers))
   , _cend  (std::apply([](auto&...args){return std::make_tuple(std::cend  (args)...);},containers))
  {
    check_sizes(containers);
  }

  auto begin  () const { return _begin; }
  auto end    () const { return _end;   }
  auto cbegin () const { return _cbegin; }
  auto cend   () const { return _cend;   }

private:
  void check_sizes (const std::tuple<Containers&...>& containers) {
    const auto sz = std::size(std::get<0>(containers));

    bool same = std::apply([&](const auto&...c) { return ((std::size(c)==sz) && ...); }, containers);
    EKAT_REQUIRE_MSG (same,
       "[Zip] Error! All containers must have the same size.");
  }

  ZipIterator<std::tuple<decltype(std::begin (std::declval<Containers>()))...>> _begin;
  ZipIterator<std::tuple<decltype(std::end   (std::declval<Containers>()))...>> _end;
  ZipIterator<std::tuple<decltype(std::cbegin(std::declval<Containers>()))...>> _cbegin;
  ZipIterator<std::tuple<decltype(std::cend  (std::declval<Containers>()))...>> _cend;
};


// Helper function to create a ZipIterator
template <typename... Containers>
auto zip(Containers&&... containers) {
  return Zip<Containers...>(std::forward_as_tuple(containers...));
}

} // namespace ekat

#endif // EKAT_ZIP_HPP
