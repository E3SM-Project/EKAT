#include <catch2/catch.hpp>

#include "ekat/util/ekat_factory.hpp"

namespace {

struct Base {
  virtual int foo () const = 0;
};

template<typename T>
std::shared_ptr<Base> createBase () {
  return std::make_shared<T>();
}

struct Derived1 : Base {
  int foo () const { return 1; }
};

struct Derived2 : Base {
  int foo () const { return 2; }
};

TEST_CASE("factory") {
  using namespace ekat;

  using factory_t = Factory<Base,std::string,std::shared_ptr<Base>>;

  {
    auto& factory = factory_t::instance();
    factory.register_product("one",&createBase<Derived1>);
    factory.register_product("two",&createBase<Derived2>);
  }

  auto& factory = factory_t::instance();
  REQUIRE (factory.register_size()==2);
  REQUIRE (factory.has_product("one"));
  REQUIRE (factory.has_product("two"));
  REQUIRE (not factory.has_product("three"));

  auto one = factory.create("one");
  REQUIRE (one->foo()==1);

  REQUIRE_THROWS (factory.create("three"));
}

} // empty namespace
