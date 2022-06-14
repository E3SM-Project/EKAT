module constants_test_mod
  use ekat_base_constants, only: c_real
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

  subroutine log_failure(x, y, line, nerr)
    real(c_real), intent(in) :: x, y
    integer, intent(in) :: line
    integer, intent(inout) :: nerr
    real(c_real) :: reldiff
    reldiff = abs(x-y)/abs(x)
    print *, "test failed at line ", line, ": reldiff = ", reldiff
    nerr = nerr + 1
  end subroutine

end module

! This macro halts the program if the predicate x isn't true.
#define assert(x) if (.not. (x)) call fatal_error("Assertion failed at line", __LINE__)

! Type-saving test macro
#define test_val(x, y, tol) if (.not. approx_eq(x,y,tol)) call log_failure(x,y, __LINE__, nerr)

program constants_test
  use constants_test_mod
  use ekat_base_constants

  real(c_real) :: tol
  real(c_real) :: standard_temperature, &
                  rho, &
                  P
  integer :: nerr

  nerr = 0

  tol = 1.0e-12_c_real

  test_val(4*atan(1.0_c_real), pi, tol)
  test_val(avogadro, 6.02214076e23_c_real, tol)
  test_val(boltzmann, 1.380649e-23_c_real, tol)
  test_val(r_gas, 8.314462618_c_real, tol)
  test_val(gravity, 9.80665_c_real, tol)
  test_val(earth_radius, 6371000.0_c_real, tol)
  test_val(std_pressure, 101325._c_real, tol)
  test_val(standard_pressure, 100000.0_c_real, tol)
  test_val(sidereal_day, 86164.2_c_real, tol)
  test_val(calendar_day, real(24*60*60, c_real), tol)
  test_val(earth_omega, 0.0000729211_c_real, tol)
  test_val(stefan_boltzmann, 5.670374419e-8_c_real, tol)
  test_val(karman, 0.4_c_real, tol)

  standard_temperature = freezing_pt_h2o
  rho = standard_pressure / (rgas_dry_air * standard_temperature)
  P = rho*rgas_dry_air*freezing_pt_h2o

  test_val( P, standard_pressure, tol)

  test_val(molec_weight_c, 0.0120107_c_real, tol)
  test_val(molec_weight_ch4,  0.0160425_c_real , tol)
  test_val(molec_weight_co,  0.0280101_c_real, tol)
  test_val(molec_weight_co2,  0.0440095_c_real , tol)
  test_val(molec_weight_dry_air,  0.028963_c_real , tol)
  test_val(molec_weight_h2o,  0.0180153_c_real , tol)
  test_val(molec_weight_h2so4,  0.098078_c_real , tol)
  test_val(molec_weight_n2,  0.0280134_c_real , tol)
  test_val(molec_weight_nh3,  0.0170305_c_real, tol)
  test_val(molec_weight_nh4,  0.0180390_c_real, tol)
  test_val(molec_weight_no2,  0.0460055_c_real , tol)
  test_val(molec_weight_o,  0.0159994_c_real , tol)
  test_val(molec_weight_o2,  0.0319988_c_real , tol)
  test_val(molec_weight_o3,  0.0479982_c_real , tol)
  test_val(molec_weight_so4,  0.096064_c_real , tol)
  test_val(carbon_13_to_12_ratio, 0.011179_c_real, tol)
  test_val(density_h2o_liq_0c,  999.8428_c_real , tol)
  test_val(density_h2o_ice_0c,  916.7_c_real , tol)
  test_val(freezing_pt_h2o,  273.15_c_real, tol)
  test_val(critical_temp_h2o,  647.096_c_real , tol)
  test_val(latent_heat_evap_0c, 2501000.0_c_real, tol )
  test_val(latent_heat_fusion, 333700.0_c_real, tol )
  test_val(latent_heat_sublimation, 2834700.0_c_real, tol )
  test_val(rgas_h2o_vapor, 461.522_c_real, tol)
  test_val(surface_tension_h2o_air_0c,  0.07570689080553497_c_real , tol)
  test_val(cp_h2o_liq_0c,  4219.4_c_real, tol)
  test_val(cp_h2o_vapor, 1846.0_c_real, tol)
  test_val(cp_h2o_ice, 2110.0_c_real, tol )
  test_val(cp_dry_air,  1004.64_c_real, tol)
  test_val(triple_point_h2o,  273.16_c_real, tol)
  test_val(cv_dry_air,  717.6_c_real, tol)
  test_val(thermal_conductivity_h2o_ice_0c, 2.1_c_real, tol)
  test_val(rgas_dry_air, 287.072_c_real, tol)
  test_val(cp_dry_air, 1004.64_c_real, tol)
  test_val(vsmow_ratio_18o_16o, 0.200520_c_real, tol)
  test_val(vsmow_ratio_17o_16o, 0.03769_c_real, tol)
  test_val(vsmow_ratio_2h_h, 0.015576_c_real, tol)

  assert(nerr == 0)
  ! if we made it this far, all tests have passed
  print*, "tests pass"
end program
