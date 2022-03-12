program test_cuda_rdc

  interface
    subroutine run_f90 () bind(c)
    end subroutine run_f90
  end interface

  call run_f90()

end program
