#include "ekat/util/ekat_tridiag.hpp"
#include "ekat_test_config.h"

extern "C" {

void tridiag_diagdom_bfb_a1x1 (int n, Real* dl, Real* d, Real* du, Real* x) {
  Kokkos::View<Real*, Kokkos::DefaultHostExecutionSpace>
    dlv(dl, n), dv(d, n), duv(du, n), xv(x, n);
  ekat::tridiag::impl::bfb_thomas_factorize(dlv, dv, duv);
  ekat::tridiag::impl::bfb_thomas_solve(dlv, dv, duv, xv);
}

void tridiag_diagdom_bfb_a1xm (int n, int nrhs, Real* dl, Real* d, Real* du, Real* x) {
  Kokkos::View<Real*, Kokkos::DefaultHostExecutionSpace>
    dlv(dl, n), dv(d, n), duv(du, n);
  Kokkos::View<Real**, Kokkos::LayoutRight, Kokkos::DefaultHostExecutionSpace>
    xv(x, n, nrhs);
  ekat::tridiag::impl::bfb_thomas_factorize(dlv, dv, duv);
  for (int j = 0; j < nrhs; ++j) {
    ekat::tridiag::impl::bfb_thomas_solve(dlv, dv, duv,
                                 Kokkos::subview(xv, Kokkos::ALL(), j));
  }
}

} // extern "C"
