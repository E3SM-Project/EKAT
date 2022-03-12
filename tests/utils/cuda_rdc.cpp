#include "cuda_rdc_kernel.hpp"
#include <ekat/ekat_session.hpp>

#include <iostream>

extern "C"
{

void run_f90 () {

  using namespace ekat;
  using KT = KokkosTypes<DefaultDevice>;

  initialize_ekat_session();
  KT::RangePolicy policy(0,10);
  Kokkos::parallel_for (policy,KOKKOS_LAMBDA(int i) {
    my_kernel(i);
  });
  std::cout << "Hello world!\n";
  finalize_ekat_session();
}

}
