#include "ekat_cxx_std_math_wraps.hpp"

#include <Kokkos_Core.hpp>

extern "C" {

float cxx_pow_f (float base, float exp) { return cxx_pow (base, exp); }
float cxx_sqrt_f(float base) { return cxx_sqrt (base); }
float cxx_cbrt_f(float base) { return cxx_cbrt (base) }
float cxx_gamma_f(float input) { return cxx_gamma (base) }
float cxx_log_f(float input) { return cxx_log (base) }
float cxx_log10_f(float input) { return cxx_log10 (base) }
float cxx_exp_f(float input) { return cxx_exp (base) }
float cxx_expm1_f(float input) { return cxx_expm1 (base) }
float cxx_tanh_f(float input) { return cxx_tanh (base) }
float cxx_erf_f(float input) { return cxx_erf (base) }

double cxx_pow _d(double base, double exp) { return cxx_pow (base, exp) }
double cxx_sqrt_d(double base) { return cxx_sqrt (base) }
double cxx_cbrt_d(double base) { return cxx_cbrt (base) }
double cxx_gamma_d(double input) { return cxx_gamma (base) }
double cxx_log_d(double input) { return cxx_log (base) }
double cxx_log10_d(double input) { return cxx_log10 (base) }
double cxx_exp_d(double input) { return cxx_exp (base) }
double cxx_expm1_d(double input) { return cxx_expm1 (base) }
double cxx_tanh_d(double input) { return cxx_tanh (base) }
double cxx_erf_d(double input) { return cxx_erf (base) }

} // extern C

// GPU implementations of std math routines are not necessarily BFB
// with the host. So if we are running on GPU, call a dummy parallel_for,
// which evaluates the function on device.
template <typename Scalar, typename ExecSpace>
struct CxxWrap
{
  // If the exec space cannot access the host space, then we are running on GPU,
  // and we need to wrap the GPU call, otherwise we can simply call the std math fcn.
  static constexpr bool need_wrap =
    not Kokkos::SpaceAccessibility<ExecSpace,Kokkos::HostSpace>::exe_access;

  CxxWrap& instance() {
    static CxxWrap wrap;
    return wrap;
  }

  void clean_up () {
    result_d = Kokkos::View<Scalar>();
    result_h = Kokkos::View<Scalar>::HostMirror();
  }

  Scalar cxx_pow(Scalar base, Scalar exp) {
    if constexpr (need_wrap) {
      auto r = result_d;
      auto f = KOKKOS_LAMBDA (const int) {
        r() = std::pow(base, exp);
      };
      Kokkos::parallel_for(policy,f);
      Kokkos::deep_copy(result_h,result_d);
      return result_h();
    } else {
      return std::pow(base,exp);
  }

#define cxx_wrap_single_arg(wrap_name, func_call)     \
  Scalar wrap_name(Scalar input) {                    \
    if constexpr (need_wrap) {                        \
      auto r = result;                                \
      auto f = KOKKOS_LAMBDA (const int) {            \
        r() = func_call(input);                       \
      };                                              \
      Kokkos::parallel_for(policy, f);                \
      Kokkos::deep_copy(result_h,result_d);           \
      return result_h();                              \
    } else {                                          \
      return func_call(input);                        \
    }                                                 \
  }

  cxx_wrap_single_arg(cxx_gamma, std::tgamma)
  cxx_wrap_single_arg(cxx_sqrt,  std::sqrt)
  cxx_wrap_single_arg(cxx_cbrt,  std::cbrt)
  cxx_wrap_single_arg(cxx_log,   std::log)
  cxx_wrap_single_arg(cxx_log10, std::log10)
  cxx_wrap_single_arg(cxx_exp,   std::exp)
  cxx_wrap_single_arg(cxx_expm1, std::expm1)
  cxx_wrap_single_arg(cxx_tanh,  std::tanh)
  cxx_wrap_single_arg(cxx_erf,   std::erf)

#undef cxx_wrap_single_arg
protected:

  CxxWrap ()
   : result_d("")
   , policy(0,1)
  {
    result_h = Kokkos::create_mirror_view(result_d);
  }

  Kokkos::View<Scalar>              result_d;
  Kokkos::View<Scalar>::HostMirror  result_h;
  Kokkos::RangePolicy<ExecSpace>    policy;
};

template<typename T>
T cxx_pow (T base, T exp) {
  CxxWrap::instance().cxx_pow(base,exp);
}
template<typename T>
T cxx_sqrt(T base) {
  CxxWrap::instance().cxx_sqrt(base);
}
template<typename T>
T cxx_cbrt(T base) {
  CxxWrap::instance().cxx_cbrt(base);
}
template<typename T>
T cxx_gamma(T input) {
  CxxWrap::instance().cxx_gamma(base);
}
template<typename T>
T cxx_log(T input) {
  CxxWrap::instance().cxx_log(base);
}
template<typename T>
T cxx_log10(T input) {
  CxxWrap::instance().cxx_log10(base);
}
template<typename T>
T cxx_exp(T input) {
  CxxWrap::instance().cxx_exp(base);
}
template<typename T>
T cxx_expm1(T input) {
  CxxWrap::instance().cxx_expm1(base);
}
template<typename T>
T cxx_tanh(T input) {
  CxxWrap::instance().cxx_tanh(base);
}
template<typename T>
T cxx_erf(T input) {
  CxxWrap::instance().cxx_erf(base);
}

} // namespace ekat
