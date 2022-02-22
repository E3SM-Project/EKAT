module ekat_array_io_mod
  interface
     function array_io_file_exists(filename) result(exists) bind(c)
       use iso_c_binding
       character(kind=c_char), intent(in) :: filename(*)
       logical(kind=c_bool) :: exists
     end function array_io_file_exists

     function array_io_write_double(filename, a, n) result(ok) bind(c)
       use iso_c_binding
       character(kind=c_char), intent(in) :: filename(*)
       type(c_ptr), intent(in) :: a
       integer(kind=c_int), intent(in), value :: n
       logical(kind=c_bool) :: ok
     end function array_io_write_double
     function array_io_read_double(filename, a, n) result(ok) bind(c)
       use iso_c_binding
       character(kind=c_char), intent(in) :: filename(*)
       type(c_ptr) :: a
       integer(kind=c_int), intent(in), value :: n
       logical(kind=c_bool) :: ok
     end function array_io_read_double

     function array_io_write_float(filename, a, n) result(ok) bind(c)
       use iso_c_binding
       character(kind=c_char), intent(in) :: filename(*)
       type(c_ptr), intent(in) :: a
       integer(kind=c_int), intent(in), value :: n
       logical(kind=c_bool) :: ok
     end function array_io_write_float
     function array_io_read_float(filename, a, n) result(ok) bind(c)
       use iso_c_binding
       character(kind=c_char), intent(in) :: filename(*)
       type(c_ptr) :: a
       integer(kind=c_int), intent(in), value :: n
       logical(kind=c_bool) :: ok
     end function array_io_read_float
  end interface
end module ekat_array_io_mod
