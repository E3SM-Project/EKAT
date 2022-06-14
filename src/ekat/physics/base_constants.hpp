#ifndef BASE_PHYSICAL_CONSTANTS_HPP
#define BASE_PHYSICAL_CONSTANTS_HPP

#include <type_traits>

/*
 DO NOT EDIT
 This file was automatically generated from ekat/src/ekat/physics/constants.yaml

ALL CONSTANTS ARE DEFINED IN SI UNITS

	time:                seconds   [s]
	length:              meters    [m]
	mass:                kilograms [kg]
	temperature          Kelvin    [K]
	pressure             Pascals   [Pa]
	force                Newtons   [N]
	amount of substance  Moles     [mol]
	work                 Joules    [J]
*/

namespace ekat {
namespace physics {

template <typename Scalar>
struct BaseConstants {
	static_assert(std::is_floating_point<Scalar>::value, "floating point type required.");

	/*
	Ammonia
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_nh3 = 0.0170305;

	/*
	Ammonium
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_nh4 = 0.018039;

	/*
	Avogadro
	units: #/mol
	source info:
		name: NIST CODATA 2018
		url: https://physics.nist.gov/cuu/Constants/index.html
		citation: E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.
	*/
	static constexpr Scalar avogadro = 6.02214076e+23;

	/*
	Boltzmann
	units: J/K
	source info:
		name: NIST CODATA 2018
		url: https://physics.nist.gov/cuu/Constants/index.html
		citation: E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.
	*/
	static constexpr Scalar boltzmann = 1.380649e-23;

	/*
	Calendar_day
	units: s
	source info:
		name: 24 * 60 * 60
	*/
	static constexpr Scalar calendar_day = 86400;

	/*
	Carbon_atom
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_c = 0.0120107;

	/*
	Carbon_dioxide
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_co2 = 0.0440095;

	/*
	Carbon_monoxide
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_co = 0.0280101;

	/*
	Delta_C13
	units: n/a
	source info:
		doi: https://doi.org/10.25844/h9q1-0p82
		citation: Z. Sharp, 2017, Principles of Stable Isotope Geochemistry, University of New Mexico.
	*/
	static constexpr Scalar carbon_13_to_12_ratio = 0.011179;

	/*
	Dry_air
	units: kg/mol
	source info:
		url: https://nvlpubs.nist.gov/nistpubs/jres/83/jresv83n5p419_a1b.pdf
		citation: F. E. Jones, 1978, The air density equation and the transfer of the mass unit, J. Res. National Bur. Standards 83(5).
	*/
	static constexpr Scalar molec_weight_dry_air = 0.028963;

	/*
	Dry_air_gas
	units: J/kg/K
	source info:
		definition: r_gas / molec_weight_dry_air
	*/
	static constexpr Scalar rgas_dry_air = 287.072;

	/*
	Earth_mean_radius
	units: m
	source info:
		name: NASA Space Science Data Coordinated Archive
		url: https://nssdc.gsfc.nasa.gov/planetary/planets/earthpage.html
	*/
	static constexpr Scalar earth_radius = 6371000;

	/*
	Earth_rotation
	units: 1/s
	source info:
		definition: 2*pi/sidereal_day
	*/
	static constexpr Scalar earth_omega = 7.29211e-05;

	/*
	Ice_density
	units: kg/mol
	source info:
		url: https://tsapps.nist.gov/publication/get_pdf.cfm?pub_id=926353
		citation: Harvey, A. (2019), Properties of Ice and Supercooled Water, CRC Handbook of Chemistry and Physics, CRC Press, Boca Raton, FL, [online] (Accessed May 31, 2022)
	*/
	static constexpr Scalar density_h2o_ice_0c = 916.7;

	/*
	Karman
	units: n/a
	source info:
		name: Journal of Fluid Mechanics
		citation: S. C. C. Bailey, M. Vallikivi, M. Hultmark, and A. J. Smits, 2014, Estimating the value of von Karman's constant in turbulent pipe flow, J. Fluid Mech. 749.
	*/
	static constexpr Scalar karman = 0.4;

	/*
	Latent_heat_water_evaporation
	units: J/kg
	source info:
		citation: R. R. Rogers and M. K. Yau, 1989, A Short Course in Cloud Physics, 3rd ed., Butterworth-Heinemann.
		note: Table 2.1
	*/
	static constexpr Scalar latent_heat_evap_0c = 2501000;

	/*
	Latent_heat_water_fusion
	units: J/kg
	source info:
		doi: https://doi.org/10.1007/1-4020-3266-8_124
		citation: D. R. Legates, 2005, Latent Heat, in J. E. Oliver, Encyclopedia of World Climatology, Springer.
	*/
	static constexpr Scalar latent_heat_fusion = 333700;

	/*
	Latent_heat_water_sublimation
	units: J/kg
	source info:
		citation: R. R. Rogers and M. K. Yau, 1989, A Short Course in Cloud Physics, 3rd ed., Butterworth-Heinemann.
		note: Table 2.1
	*/
	static constexpr Scalar latent_heat_sublimation = 2834700;

	/*
	Methane
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_ch4 = 0.0160425;

	/*
	Nitrogen
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_n2 = 0.0280134;

	/*
	Nitrogen_dioxide
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_no2 = 0.0460055;

	/*
	Oxygen
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_o2 = 0.0319988;

	/*
	Oxygen_atom
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_o = 0.0159994;

	/*
	Ozone
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_o3 = 0.0479982;

	/*
	Pi
	units: n/a
	source info:
		name: Mathematica 12.0.0
		command: N[Pi, 64]
	*/
	static constexpr Scalar pi = 3.141592653589793;

	/*
	Sidereal_day
	units: s
	source info:
		name: NASA Space Science Data Coordinated Archive
		url: https://nssdc.gsfc.nasa.gov/planetary/factsheet/earthfact.html
	*/
	static constexpr Scalar sidereal_day = 86164.2;

	/*
	Specific_heat_dry_air_const_pressure
	units: J/kg/K
	source info:
		citation: C. A. Riegel and A. F. C. Bridger, 1992, Fundamentals of Atmospheric Dynamics and Thermodynamics, World Scientific.
	*/
	static constexpr Scalar cp_dry_air = 1004.64;

	/*
	Specific_heat_dry_air_const_volume
	units: J/kg/K
	source info:
		citation: C. A. Riegel and A. F. C. Bridger, 1992, Fundamentals of Atmospheric Dynamics and Thermodynamics, World Scientific.
	*/
	static constexpr Scalar cv_dry_air = 717.6;

	/*
	Specific_heat_ice
	units: J/kg/K
	source info:
		url: https://tsapps.nist.gov/publication/get_pdf.cfm?pub_id=926353
		citation: Harvey, A. (2019), Properties of Ice and Supercooled Water, CRC Handbook of Chemistry and Physics, CRC Press, Boca Raton, FL, [online] (Accessed May 31, 2022)
	*/
	static constexpr Scalar cp_h2o_ice = 2110;

	/*
	Specific_heat_water
	units: J/kg/K
	source info:
		name: NIST Chemistry WebBook SRD 69
		doi: https://doi.org/10.18434/T4D303
		citation: E. W. Lemmon, I. H. Bell, M. L. Huber, M. O. McLinden, Thermophysical properties of fluid systems in NIST Chemistry WebBook NIST Standard Database Number 69, P. J. Linstrom and W. G. Mallard, eds.
	*/
	static constexpr Scalar cp_h2o_liq_0c = 4219.4;

	/*
	Specific_heat_water_vapor_const_pressure
	units: J/kg/K
	source info:
		citation: C. A. Riegel and A. F. C. Bridger, 1992, Fundamentals of Atmospheric Dynamics and Thermodynamics, World Scientific.
	*/
	static constexpr Scalar cp_h2o_vapor = 1846;

	/*
	Standard_acceleration_gravity
	units: m/s2
	source info:
		name: NIST CODATA 2018
		url: https://physics.nist.gov/cuu/Constants/index.html
		citation: E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.
	*/
	static constexpr Scalar gravity = 9.806649999999999;

	/*
	Standard_atmospheric_pressure
	units: Pa
	source info:
		name: NIST CODATA 2018
		url: https://physics.nist.gov/cuu/Constants/index.html
		citation: E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.
	*/
	static constexpr Scalar standard_pressure = 100000;

	/*
	Stefan_Boltzmann
	units: W/m2/K4
	source info:
		name: NIST CODATA 2018
		url: https://physics.nist.gov/cuu/Constants/index.html
		citation: E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.
	*/
	static constexpr Scalar stefan_boltzmann = 5.670374419e-08;

	/*
	Sulfate
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_so4 = 0.096064;

	/*
	Sulfuric_acid
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_h2so4 = 0.098078;

	/*
	Surface_tension_h2o_air
	units: N/m
	source info:
		doi: https://doi.org/10.1063/1.4768782
		citation: A. Mulero, I. Cachadi\~na and M. I. Parra, 2012, Recommended correlations for the surface tension of common fluids, Journal of Chemical Reference Data, 41, 043105
	*/
	static constexpr Scalar surface_tension_h2o_air_0c = 0.07570689080553497;

	/*
	Thermal_conductivity_ice
	units: W/m/K
	source info:
		url: https://tsapps.nist.gov/publication/get_pdf.cfm?pub_id=926353
		citation: Harvey, A. (2019), Properties of Ice and Supercooled Water, CRC Handbook of Chemistry and Physics, CRC Press, Boca Raton, FL, [online] (Accessed May 31, 2022)
	*/
	static constexpr Scalar thermal_conductivity_h2o_ice_0c = 2.16;

	/*
	US_standard_atmospheric_pressure
	units: Pa
	source info:
		name: CRC Handbook of Physics and Chemistry Online
		url: https://hbcp.chemnetbase.com/faces/documents/14_12/14_12_0001.xhtml
		note: 1976 US Standard Atmosphere
	*/
	static constexpr Scalar std_pressure = 101325;

	/*
	Universal_gas_constant
	units: J/mol/K
	source info:
		name: NIST CODATA 2018
		url: https://physics.nist.gov/cuu/Constants/index.html
		citation: E. Tiesinga, P. J. Mohr, D. B. Newell, and B. N. Taylor, 2021, CODATA recommended values of the fundamental physical constants: 2018, Rev. Modern Phys. 93.
	*/
	static constexpr Scalar r_gas = 8.314462618;

	/*
	VSMOW_Deuterium_ratio
	units: #2h/#h
	source info:
		url: https://www-s.nist.gov/m-srmors/certificates/8535.pdf
		citation: R. Hagemann and G. Nief and E. Roth, 1970, Absolute isotopic scale for Deuterium analysis of natural waters. Absolute D/H ratio for SMOW, Tellus 22, 712--715.
	*/
	static constexpr Scalar vsmow_ratio_2h_h = 0.015576;

	/*
	VSMOW_Oxygen17_ratio
	units: #17o/#16o
	source info:
		name: NIST Reference Material 8535, 2022, VSMOW Vienna Standard Mean Ocean Water
		url: https://www-s.nist.gov/m-srmors/certificates/8535.pdf
	*/
	static constexpr Scalar vsmow_ratio_17o_16o = 0.03769;

	/*
	VSMOW_Oxygen18_ratio
	units: #18o/#16o
	source info:
		name: NIST Reference Material 8535, 2022, VSMOW Vienna Standard Mean Ocean Water
		url: https://www-s.nist.gov/m-srmors/certificates/8535.pdf
	*/
	static constexpr Scalar vsmow_ratio_18o_16o = 0.20052;

	/*
	Water_critical_pt_temperature
	units: K
	source info:
		doi: https://doi.org/10.1063/1.4768782
		citation: A. Mulero, I. Cachadi\~na and M. I. Parra, 2012, Recommended correlations for the surface tension of common fluids, Journal of Chemical Reference Data, 41, 043105
	*/
	static constexpr Scalar critical_temp_h2o = 647.096;

	/*
	Water_density
	units: kg/m3
	source info:
		doi: https://doi.org/10.1088/0026-1394/38/4/3
		citation: M. Tanaka, G. Girard, R. Davis, A. Peuto, and N. Bignell, 2001, Recommended table for the density of water between OC and 40C based on recent experimental reports, Metrologia 38, 301.
	*/
	static constexpr Scalar density_h2o_liq_0c = 999.8428;

	/*
	Water_freeze_temperature
	units: K
	source info:
		url: https://doi.org/10.1063/1.555894
		citation: Journal of Physical and Chemical Reference Data 20, 1023 (1991).
	*/
	static constexpr Scalar freezing_pt_h2o = 273.15;

	/*
	Water_molecule
	units: kg/mol
	source info:
		name: NIST Chemistry WebBook SRD 69
		url: https://webbook.nist.gov/chemistry/
		doi: https://doi.org/10.18434/T4D303
	*/
	static constexpr Scalar molec_weight_h2o = 0.0180153;

	/*
	Water_triple_pt_temperature
	units: K
	source info:
		url: https://doi.org/10.1063/1.555894
		citation: Journal of Physical and Chemical Reference Data 20, 1023 (1991).
	*/
	static constexpr Scalar triple_point_h2o = 273.16;

	/*
	Water_vapor_gas
	units: J/kg/K
	source info:
		definition: r_gas / molec_weight_h2o
	*/
	static constexpr Scalar rgas_h2o_vapor = 461.522;

}; // end struct
} // namespace physics
} // namespace ekat
#endif
