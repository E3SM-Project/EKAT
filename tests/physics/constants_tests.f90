module constants_test_mod
  use ekat_physical_constants, only: c_real
implicit none

contains

  pure function approx_eq(x, y, tol)
    logical :: approx_eq
    real(c_real), intent(in) :: x, y
    real(c_real), intent(in) :: tol
    approx_eq = (abs(x-y)/abs(x) < tol)
  end function

  subroutine fatal_error(message, line)
    character(len=*), intent(in) :: message
    integer :: line

    print *, message, line
    stop
  end subroutine

end module

! This macro halts the program if the predicate x isn't true.
#define assert(x) if (.not. (x)) call fatal_error("Assertion failed at line", __LINE__)

program constants_test
  use constants_test_mod
  use ekat_physical_constants

  real(c_real) :: tol
  real(c_real), parameter :: standard_pressure = 100000_c_real
  real(c_real) :: standard_temperature, &
                  standard_density

  tol = real(1.0e-8, c_real)
  assert(approx_eq(4*atan(1.0_c_real), pi, tol))
  assert(approx_eq(6*pi_sixth, pi, tol))
  assert(approx_eq(gravity, real(9.80616, c_real), tol))
  assert(approx_eq(earth_radius, real(6.37122e6, c_real), tol))
  assert(approx_eq(std_pressure, real(101325, c_real), tol))
  assert(approx_eq(sidereal_day, real(86164, c_real), tol))
  assert(approx_eq(calendar_day, real(24*60*60, c_real), tol))
  assert(approx_eq(earth_omega * sidereal_day, 2*pi, tol))


  standard_temperature = freezing_pt_h2o
  standard_density = standard_pressure / (rgas_dry_air * standard_temperature)
  assert(approx_eq(standard_density*rgas_dry_air*melting_pt_h2o, standard_pressure, tol))
  tol = real(1.0e-4, c_real)
  assert(approx_eq(avogadro, real(6.022e23, c_real), tol))
  assert(approx_eq(r_gas, real(8.31446, c_real), tol))
  assert(approx_eq(molec_weight_dry_air * 1000 ,  real(28.966, c_real), tol))
  assert(approx_eq(molec_weight_h2o * 1000 ,  real(18.016, c_real), tol))
  assert(approx_eq(molec_weight_h2so4 * 1000 ,  real(98.079, c_real), tol))
  assert(approx_eq(molec_weight_nh3 * 1000 ,  real(17.031, c_real), tol))
  assert(approx_eq(molec_weight_so4 * 1000 ,  real(96.06, c_real), tol))
  assert(approx_eq(molec_weight_nh4 * 1000 ,  real(18.04, c_real), tol))
  assert(approx_eq(molec_weight_co2 * 1000 ,  real(44.01, c_real), tol))
  assert(approx_eq(molec_weight_o3 * 1000 ,  real(48, c_real), tol))
  assert(approx_eq(molec_weight_no2 * 1000 ,  real(44.013, c_real), tol))
  assert(approx_eq(molec_weight_co * 1000 ,  real(28.01, c_real), tol))
  tol = real(1.0e-3, c_real)
  assert(approx_eq(molec_weight_ch4 * 1000 ,  real(16.04, c_real), tol))
  assert(approx_eq(molec_weight_o2 * 1000 ,  real(32, c_real), tol))
  assert(approx_eq(molec_weight_n2 * 1000 ,  real(28, c_real), tol))
  assert(approx_eq(triple_point_h2o ,  real(273.16, c_real), tol))
  assert(approx_eq(density_h2o_liquid ,  real(1000, c_real), tol))
  assert(approx_eq(density_h2o_ice ,  real(917, c_real), tol))
  assert(approx_eq(cp_dry_air ,  real(1.00464e3, c_real), tol))
  assert(approx_eq(cp_h2o_vapor ,  real(1.81e3, c_real), tol))
  assert(approx_eq(cp_h2o_liq ,  real(4.188e3, c_real), tol))
  assert(approx_eq(cp_h2o_ice ,  real(2.11727e3, c_real), tol))
  assert(approx_eq(latent_heat_evap ,  real(2.501e6, c_real), tol))
  assert(approx_eq(latent_heat_fusion ,  real(3.337e5, c_real), tol))

  ! if we made it this far, all tests have passed
  print*, "tests pass"
end program
