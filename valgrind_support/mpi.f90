program test
  use mpi

  integer, parameter :: single_kind = selected_real_kind( p=7 )
  integer, parameter :: double_kind = selected_real_kind( p=14 )

  integer :: ierr, comm, me, other

  integer, allocatable :: sreq(:), rreq(:)
  integer, allocatable :: stat(:,:)
  real(kind=single_kind), allocatable :: send_s(:), recv_s(:)
  real(kind=double_kind), allocatable :: send_d(:), recv_d(:)
  integer, allocatable :: send_i(:), recv_i(:)
  character, allocatable :: send_c(:), recv_c(:)
  logical, allocatable :: send_l(:), recv_l(:)

  call mpi_init(ierr)

  comm = MPI_COMM_WORLD

  call mpi_comm_rank(comm,me,ierr)
  other = 1 - me

  allocate(sreq(1))
  allocate(rreq(1))
  allocate(stat(MPI_STATUS_SIZE,1))

  allocate(send_s(1))
  allocate(send_d(1))
  allocate(send_i(1))
  allocate(send_c(1))
  allocate(send_l(1))
  allocate(recv_s(1))
  allocate(recv_d(1))
  allocate(recv_i(1))
  allocate(recv_c(1))
  allocate(recv_l(1))

  send_s(1) = 1
  send_d(1) = 1
  send_i(1) = 1
  send_c(1) = 'c'
  send_l(1) = .true.

  ! Single
  call mpi_isend(send_s, 1, MPI_FLOAT, other, 1234, comm, sreq(1), ierr)
  call mpi_irecv(recv_s, 1, MPI_FLOAT, other, 1234, comm, rreq(1), ierr)
  call mpi_waitall (1, sreq, stat, ierr)
  call mpi_waitall (1, rreq, stat, ierr)

  ! Double
  call mpi_isend(send_d, 1, MPI_DOUBLE, other, 1234, comm, sreq(1), ierr)
  call mpi_irecv(recv_d, 1, MPI_DOUBLE, other, 1234, comm, rreq(1), ierr)
  call mpi_waitall (1, sreq, stat, ierr)
  call mpi_waitall (1, rreq, stat, ierr)

  ! Int
  call mpi_isend(send_i, 1, MPI_INTEGER, other, 1234, comm, sreq(1), ierr)
  call mpi_irecv(recv_i, 1, MPI_INTEGER, other, 1234, comm, rreq(1), ierr)
  call mpi_waitall (1, sreq, stat, ierr)
  call mpi_waitall (1, rreq, stat, ierr)

  ! Char
  call mpi_isend(send_c, 1, MPI_CHARACTER, other, 1234, comm, sreq(1), ierr)
  call mpi_irecv(recv_c, 1, MPI_CHARACTER, other, 1234, comm, rreq(1), ierr)
  call mpi_waitall (1, sreq, stat, ierr)
  call mpi_waitall (1, rreq, stat, ierr)

  ! Logical
  call mpi_isend(send_l, 1, MPI_LOGICAL, other, 1234, comm, sreq(1), ierr)
  call mpi_irecv(recv_l, 1, MPI_LOGICAL, other, 1234, comm, rreq(1), ierr)
  call mpi_waitall (1, sreq, stat, ierr)
  call mpi_waitall (1, rreq, stat, ierr)

  call mpi_finalize(ierr)

end program test
