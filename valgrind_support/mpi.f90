program test
  use mpi

  integer :: ierr, comm, me, other, sz

  integer, allocatable :: sreq(:), rreq(:)
  integer, allocatable :: stat(:,:)
  real, allocatable :: send(:), recv(:)

  call mpi_init(ierr)

  comm = MPI_COMM_WORLD

  call mpi_comm_rank(comm,me,ierr)
  other = 1 - me

  allocate(sreq(1))
  allocate(rreq(1))
  allocate(stat(MPI_STATUS_SIZE,1))

  allocate(send(1))
  allocate(recv(1))

  send(1) = me

  call mpi_isend(send, 1, MPI_REAL, other, 1234, comm, sreq(1), ierr)
  call mpi_irecv(recv, 1, MPI_REAL, other, 1234, comm, rreq(1), ierr)

  call mpi_waitall (1, sreq, stat, ierr)
  call mpi_waitall (1, rreq, stat, ierr)

  print *, "other sent:", recv(1)

  call mpi_finalize(ierr)

end program test
