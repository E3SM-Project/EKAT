#include "ekat/ekat_assert.hpp"

#include <Kokkos_Core.hpp>

int main(int,char**) {
  Kokkos::parallel_for(Kokkos::RangePolicy<>(0,1),
                       KOKKOS_LAMBDA(int) {
    const char* test = "KERNEL_ASSERT_FAIL_TEST";
    EKAT_KERNEL_ASSERT_MSG (1<0,test);
  });
  return 0;
}
