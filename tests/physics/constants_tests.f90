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

#define test_val(x, y, tol) if (.not. approx_eq(x,y,tol)) call log_failure(x,y, __LINE__, nerr)

program constants_test
  use constants_test_mod
  use ekat_physical_constants

  real(c_real) :: tol
  real(c_real) :: standard_temperature, &
                  standard_density
  integer :: nerr

  nerr = 0

  tol = 1.0e-12_c_real

  test_val(4*atan(1.0_c_real), pi, tol)
  test_val(avogadro, 6.022214076e23_c_real, 1.0e-7_c_real)
  test_val(boltzmann, 1.380649e-23_c_real, 1.0e-7_c_real)
  test_val(r_gas, 8.314563841815325_c_real, real(1.0e-7,c_real))
  test_val(gravity, 9.80616_c_real, 3e-9_c_real)
  test_val(earth_radius, 6.37122e6_c_real, tol)
  test_val(real(std_pressure, c_real), real(101325, c_real), tol)
  test_val(real(standard_pressure, c_real), 100000._c_real, tol)
  test_val(real(sidereal_day, c_real), 86164._c_real, tol)
  test_val(real(calendar_day, c_real), real(24*60*60, c_real), tol)
  test_val(earth_omega * sidereal_day, 2*pi, tol)
  test_val(stefan_boltzmann, 5.67e-8_c_real, 1.5e-8_c_real)
  test_val(karman, 0.4_c_real, 1.5e-8_c_real)

  standard_temperature = freezing_pt_h2o
  standard_density = standard_pressure / (rgas_dry_air * standard_temperature)
  test_val(standard_density*rgas_dry_air*melting_pt_h2o, standard_pressure, tol)

  test_val(molec_weight_c * 1000_c_real ,  12.0107_c_real, 2e-8_c_real)
  test_val(molec_weight_ch4 * 1000_c_real ,  16.04_c_real, 1.6e-4_c_real)
  test_val(molec_weight_co * 1000_c_real ,  28.01_c_real, 3.7e-6_c_real)
  test_val(molec_weight_co2 * 1000_c_real ,  44.01_c_real, 1.2e-5_c_real)
  test_val(molec_weight_dry_air * 1000_c_real ,  28.966_c_real, 1.5e-8_c_real)
  test_val(molec_weight_h2o * 1000_c_real ,  18.016_c_real, 4.2e-8_c_real)
  test_val(molec_weight_h2so4 * 1000_c_real ,  98.079_c_real, tol)
  test_val(molec_weight_n2 * 1000_c_real ,  28.0134_c_real, tol)
  test_val(molec_weight_nh3 * 1000_c_real ,  17.03052_c_real, tol)
  test_val(molec_weight_nh4 * 1000_c_real ,  18.04_c_real, 6e-5_c_real)
  test_val(molec_weight_no2 * 1000_c_real ,  44.013_c_real, 5e-6_c_real)
  test_val(molec_weight_o * 1000_c_real ,  15.9994_c_real, tol)
  test_val(molec_weight_o2 * 1000_c_real ,  31.998_c_real, tol)
  test_val(molec_weight_o3 * 1000_c_real ,  47.9982_c_real, tol)
  test_val(molec_weight_so4 * 1000_c_real ,  96.06_c_real, tol)
  test_val(carbon_13_to_12_ratio ,  0.0112372_c_real, tol)

  test_val(weight_ratio_h2o_air ,  0.62197_c_real, 1e-6_c_real)
  test_val(density_h2o_liquid ,  1000._c_real, tol)
  test_val(density_h2o_ice ,  917._c_real, tol)
  test_val(freezing_pt_h2o ,  273.15_c_real, tol)
  test_val(melting_pt_h2o ,  273.15_c_real, tol)
  test_val(latent_heat_evap ,  2.501e6_c_real, tol)
  test_val(latent_heat_fusion ,  3.337e5_c_real, tol)
  test_val(latent_heat_sublimation ,  2834700._c_real, tol)
  test_val(rgas_h2o_vapor ,  461.501_c_real, 2e-5_c_real)
  test_val(surface_tension_h2o_air_273k ,  0.07564_c_real, tol)
  test_val(cp_h2o_liq ,  4.188e3_c_real, tol)
  test_val(cp_h2o_vapor ,  1.81e3_c_real, tol)
  test_val(cp_h2o_ice ,  2.11727e3_c_real, tol)
  test_val(cp_dry_air ,  1.00464e3_c_real, tol)
  test_val(triple_point_h2o ,  273.16_c_real, tol)

  test_val(density_seawater ,  1026._c_real, tol)
  test_val(freezing_pt_seawater ,  271.35_c_real, 2e-10_c_real)
  test_val(ocean_surface_layer_depth ,  3._c_real, tol)
  test_val(ocean_ref_salinity ,  34.7_c_real, tol)
  test_val(cp_seawater ,  3996._c_real, tol)

  test_val(vmsow_ratio_o18_o16 , 2005.2e-6_c_real, tol)
  test_val(vmsow_ratio_o17_o16 , 379e-6_c_real, tol)
  test_val(vmsow_ratio_o16_total_o , 0.997628_c_real, tol)
  test_val(vmsow_ratio_2h_h , 155.76e-6_c_real, tol)
  test_val(vmsow_ratio_3h_h , 1.85e-6_c_real, tol)
  test_val(vmsow_ratio_h_total_h , 0.99984426_c_real, tol)

  test_val(ice_ref_salinity , 4._c_real, tol)
  test_val(freezing_pt_h2o_zerop , 273.2122_c_real, tol)
  test_val(dfreeze_zerop_dp , real(-7.43e-8, c_real), tol)
  test_val(dfreeze_zerop_ds , real(-5.63e-2, c_real), tol)
  test_val(dfreeze_zerop_dpds , real(-1.74e-10, c_real), tol)
  test_val(thermal_conductivity_ice , 2.1_c_real, tol)

  test_val(rgas_dry_air ,  287.0420493_c_real, 1.5e-5_c_real)
  test_val(cp_dry_air ,  1004.64_c_real, tol)
  test_val(gas_const_ratio_zvir, rgas_h2o_vapor/rgas_dry_air - 1_c_real, tol)
  test_val(specific_heat_ratio_cpvir, cp_h2o_vapor/cp_dry_air -1_c_real, tol)

  assert(nerr == 0)
  ! if we made it this far, all tests have passed
  print*, "tests pass"
end program
