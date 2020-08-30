#include "ekat/ekat_assert.hpp"

#include <Kokkos_Core.hpp>

int main(int argc, char** argv) {
  Kokkos::initialize(argc, argv);
  Kokkos::parallel_for(Kokkos::RangePolicy<>(0,1),
                       KOKKOS_LAMBDA(int) {
    const char* test = "KERNEL_ASSERT_FAIL_TEST";
    EKAT_KERNEL_ASSERT_MSG (1<0,test);
  });
  Kokkos::finalize();
  return 0;
}
