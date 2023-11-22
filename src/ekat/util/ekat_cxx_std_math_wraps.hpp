#ifndef EKAT_CXX_STD_MATH_WRAPS_HPP
#define EKAT_CXX_STD_MATH_WRAPS_HPP

namespace ekat {

template<typename T>
T cxx_pow (T base, T exp);
template<typename T>
T cxx_sqrt(T base);
template<typename T>
T cxx_cbrt(T base);
template<typename T>
T cxx_gamma(T input);
template<typename T>
T cxx_log(T input);
template<typename T>
T cxx_log10(T input);
template<typename T>
T cxx_exp(T input);
template<typename T>
T cxx_expm1(T input);
template<typename T>
T cxx_tanh(T input);
template<typename T>
T cxx_erf(T input);

} // namespace ekat

#endif // SCREAM_PHYSICS_SHARE_HPP
