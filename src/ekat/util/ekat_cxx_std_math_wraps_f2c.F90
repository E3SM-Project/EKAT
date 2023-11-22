module cxx_std_math_wraps_f2c
  use iso_c_binding

  implicit none

  public :: &
    cxx_pow,      &
    cxx_sqrt,     &
    cxx_cbrt,     &
    cxx_gamma,    &
    cxx_log,      &
    cxx_log10,    &
    cxx_exp,      &
    cxx_expm1,    &
    cxx_erf

  !
  ! These are some routine math operations that are not BFB between
  ! fortran and C++ on all platforms. In particular, on GPU platforms,
  ! the (CPU) fortran and (GPU) C++ may differ. Hence, we make fortran
  ! call the C++ impl, to ensure bfb-ness.
  !

  interface cxx_pow
    module procedure cxx_pow_impl_d cxx_pow_impl_f
  end interface
  interface cxx_sqrt
    module procedure cxx_sqrt_impl_d cxx_sqrt_impl_f
  end interface
  interface cxx_cbrt
    module procedure cxx_cbrt_impl_d cxx_cbrt_impl_f
  end interface
  interface cxx_gamma
    module procedure cxx_gamma_impl_d cxx_gamma_impl_f
  end interface
  interface cxx_log
    module procedure cxx_log_impl_d cxx_log_impl_f
  end interface
  interface cxx_log10
    module procedure cxx_log10_impl_d cxx_log10_impl_f
  end interface
  interface cxx_exp
    module procedure cxx_exp_impl_d cxx_exp_impl_f
  end interface
  interface cxx_expm1
    module procedure cxx_expm1_impl_d cxx_expm1_impl_f
  end interface
  interface cxx_tanh
    module procedure cxx_tanh_impl_d cxx_tanh_impl_f
  end interface
  interface cxx_erf
    module procedure cxx_erf_impl_d cxx_erf_impl_f
  end interface

contains
  function cxx_pow_impl_f(base, exp) bind(C)
    interface
      function cxx_pow_f(base, exp) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in)  :: base
        real(kind=c_float), value, intent(in)  :: exp

        ! return
        real(kind=c_float)               :: cxx_pow
      end function cxx_pow_f
    end interface

    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in)  :: base
    real(kind=c_float), value, intent(in)  :: exp

    ! return
    real(kind=c_float)               :: cxx_pow

    cxx_pow = cxx_pow_f(base,exp)
  end function cxx_pow_impl_f

  function cxx_sqrt_impl_f(base) bind(C)
    interface
      function cxx_sqrt_f(base) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in)  :: base

        ! return
        real(kind=c_float)               :: cxx_sqrt
      end function cxx_sqrt_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in)  :: base

    ! return
    real(kind=c_float)               :: cxx_sqrt

    cxx_sqrt = cxx_sqrt_f(base)
  end function cxx_sqrt_impl_f

  function cxx_cbrt_impl_f(base) bind(C)
    interface
      function cxx_cbrt_f(base) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in)  :: base

        ! return
        real(kind=c_float)               :: cxx_cbrt
      end function cxx_cbrt_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in)  :: base

    ! return
    real(kind=c_float)               :: cxx_cbrt
    cxx_cbrt = cxx_cbrt_f(base)
  end function cxx_cbrt_impl_f

  function cxx_gamma_impl_f(input) bind(C)
    interface
      function cxx_gamma_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in) :: input

        ! return
        real(kind=c_float)            :: cxx_gamma
      end function cxx_gamma_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in) :: input

    ! return
    real(kind=c_float)            :: cxx_gamma

    cxx_gamma = cxx_gamma_f(input)
  end function cxx_gamma_impl_f

  function cxx_log_impl_f(input) bind(C)
    interface
      function cxx_log_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in) :: input

        ! return
        real(kind=c_float)            :: cxx_log
      end function cxx_log_f
    end interface

    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in) :: input

    ! return
    real(kind=c_float)            :: cxx_log
    
    cxx_log = cxx_log_f(input)
  end function cxx_log_impl_f

  function cxx_log10_impl_f(input) bind(C)
    interface
      function cxx_log10_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in) :: input

        ! return
        real(kind=c_float)            :: cxx_log10
      end function cxx_log10_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in) :: input

    ! return
    real(kind=c_float)            :: cxx_log10

    cxx_log10 = cxx_log10_f(input)
  end function cxx_log10_impl_f

  function cxx_exp_impl_f(input) bind(C)
    interface
      function cxx_exp_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in) :: input

        ! return
        real(kind=c_float)            :: cxx_exp
      end function cxx_exp_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in) :: input

    ! return
    real(kind=c_float)            :: cxx_exp

    cxx_exp = cxx_exp_f(input)
  end function cxx_exp_impl_f

  function cxx_expm1_impl_f(input) bind(C)
    interface
      function cxx_expm1_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in) :: input

        ! return
        real(kind=c_float)            :: cxx_expm1
      end function cxx_expm1_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in) :: input

    ! return
    real(kind=c_float)            :: cxx_expm1

    cxx_expm1 = cxx_expm1_f(input)
  end function cxx_expm1_impl_f
  
  function cxx_tanh_impl_f(input) bind(C)
    interface
      function cxx_tanh_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in) :: input

        ! return
        real(kind=c_float)            :: cxx_tanh
      end function cxx_tanh_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in) :: input

    ! return
    real(kind=c_float)            :: cxx_tanh

    cxx_tanh = cxx_tanh_f(input)
  end function cxx_tanh_impl_f

  function cxx_erf_impl_f(input) bind(C)
    interface
      function cxx_erf_f(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_float), value, intent(in)  :: input

        ! return
        real(kind=c_float) :: cxx_erf
      end function cxx_erf_f
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_float), value, intent(in)  :: input

    ! return
    real(kind=c_float) :: cxx_erf

    cxx_erf = cxx_erf_f(input)
  end function cxx_erf_impl_f

  ! Double version
  function cxx_pow_impl_d(base, exp) bind(C)
    interface
      function cxx_pow_d(base, exp) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in)  :: base
        real(kind=c_double), value, intent(in)  :: exp

        ! return
        real(kind=c_double)               :: cxx_pow
      end function cxx_pow_d
    end interface

    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in)  :: base
    real(kind=c_double), value, intent(in)  :: exp

    ! return
    real(kind=c_double)               :: cxx_pow

    cxx_pow = cxx_pow_d(base,exp)
  end function cxx_pow_impl_d

  function cxx_sqrt_impl_d(base) bind(C)
    interface
      function cxx_sqrt_d(base) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in)  :: base

        ! return
        real(kind=c_double)               :: cxx_sqrt
      end function cxx_sqrt_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in)  :: base

    ! return
    real(kind=c_double)               :: cxx_sqrt

    cxx_sqrt = cxx_sqrt_d(base)
  end function cxx_sqrt_impl_d

  function cxx_cbrt_impl_d(base) bind(C)
    interface
      function cxx_cbrt_d(base) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in)  :: base

        ! return
        real(kind=c_double)               :: cxx_cbrt
      end function cxx_cbrt_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in)  :: base

    ! return
    real(kind=c_double)               :: cxx_cbrt
    cxx_cbrt = cxx_cbrt_d(base)
  end function cxx_cbrt_impl_d

  function cxx_gamma_impl_d(input) bind(C)
    interface
      function cxx_gamma_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in) :: input

        ! return
        real(kind=c_double)            :: cxx_gamma
      end function cxx_gamma_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in) :: input

    ! return
    real(kind=c_double)            :: cxx_gamma

    cxx_gamma = cxx_gamma_d(input)
  end function cxx_gamma_impl_d

  function cxx_log_impl_d(input) bind(C)
    interface
      function cxx_log_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in) :: input

        ! return
        real(kind=c_double)            :: cxx_log
      end function cxx_log_d
    end interface

    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in) :: input

    ! return
    real(kind=c_double)            :: cxx_log
    
    cxx_log = cxx_log_d(input)
  end function cxx_log_impl_d

  function cxx_log10_impl_d(input) bind(C)
    interface
      function cxx_log10_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in) :: input

        ! return
        real(kind=c_double)            :: cxx_log10
      end function cxx_log10_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in) :: input

    ! return
    real(kind=c_double)            :: cxx_log10

    cxx_log10 = cxx_log10_d(input)
  end function cxx_log10_impl_d

  function cxx_exp_impl_d(input) bind(C)
    interface
      function cxx_exp_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in) :: input

        ! return
        real(kind=c_double)            :: cxx_exp
      end function cxx_exp_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in) :: input

    ! return
    real(kind=c_double)            :: cxx_exp

    cxx_exp = cxx_exp_d(input)
  end function cxx_exp_impl_d

  function cxx_expm1_impl_d(input) bind(C)
    interface
      function cxx_expm1_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in) :: input

        ! return
        real(kind=c_double)            :: cxx_expm1
      end function cxx_expm1_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in) :: input

    ! return
    real(kind=c_double)            :: cxx_expm1

    cxx_expm1 = cxx_expm1_d(input)
  end function cxx_expm1_impl_d
  
  function cxx_tanh_impl_d(input) bind(C)
    interface
      function cxx_tanh_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in) :: input

        ! return
        real(kind=c_double)            :: cxx_tanh
      end function cxx_tanh_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in) :: input

    ! return
    real(kind=c_double)            :: cxx_tanh

    cxx_tanh = cxx_tanh_d(input)
  end function cxx_tanh_impl_d

  function cxx_erf_impl_d(input) bind(C)
    interface
      function cxx_erf_d(input) bind(C)
        use iso_c_binding

        !arguments:
        real(kind=c_double), value, intent(in)  :: input

        ! return
        real(kind=c_double) :: cxx_erf
      end function cxx_erf_d
    end interface
    use iso_c_binding

    !arguments:
    real(kind=c_double), value, intent(in)  :: input

    ! return
    real(kind=c_double) :: cxx_erf

    cxx_erf = cxx_erf_d(input)
  end function cxx_erf_impl_d

end module cxx_std_math_wraps_f2c
