module unit_test_mod
  implicit none
contains

  function test_array_io() result(nerr) bind(c)
    ! Precision independent use clauses
    use iso_c_binding
    use ekat_array_io_mod, only: array_io_file_exists

#ifdef EKAT_TEST_DOUBLE_PRECISION
    use iso_c_binding, only: c_real=>c_double
    use ekat_array_io_mod, only: array_io_write=>array_io_write_double, array_io_read=>array_io_read_double
#elif defined(EKAT_TEST_SINGLE_PRECISION)
    use iso_c_binding, only: c_real=>c_float
    use ekat_array_io_mod, only: array_io_write=>array_io_write_float, array_io_read=>array_io_read_float
#endif

    integer(kind=c_int) :: nerr
    integer ::  i, j
    real(kind=c_real), target :: a(10,3), b(10,3)
    logical :: ok

    character(kind=c_char, len=128), parameter :: &
#ifdef EKAT_TEST_DOUBLE_PRECISION
         filename = c_char_"unit_test_f90_array_io_dp.dat"//C_NULL_CHAR
#else
         filename = c_char_"unit_test_f90_array_io_sp.dat"//C_NULL_CHAR
#endif

    do j = 1,3
       do i = 1,10
          a(i,j) = 100*j + i
       end do
    end do
    nerr = 0
    ok = array_io_write(filename, c_loc(a), size(a))
    if (.not. ok) nerr = nerr + 1
    ok = array_io_file_exists(filename)
    if (.not. ok) nerr = nerr + 1
    ok = array_io_read(filename, c_loc(b), size(b))
    if (.not. ok) nerr = nerr + 1
    do j = 1,3
       do i = 1,10
          if (a(i,j) .ne. b(i,j)) nerr = nerr + 1
       end do
    end do
  end function test_array_io

end module unit_test_mod
