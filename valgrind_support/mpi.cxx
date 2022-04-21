#include <mpi.h>

#include <vector>
#include <iostream>

template<typename T>
MPI_Datatype mpi_dtype ();

template<>
MPI_Datatype mpi_dtype<int> () { return MPI_INT; }
template<>
MPI_Datatype mpi_dtype<float> () { return MPI_FLOAT; }
template<>
MPI_Datatype mpi_dtype<double> () { return MPI_DOUBLE; }
template<>
MPI_Datatype mpi_dtype<char> () { return MPI_INT; }
template<>
MPI_Datatype mpi_dtype<bool> () { return MPI_CXX_BOOL; }

template<typename T>
void run (MPI_Comm comm) {
  int me, sz, other;
  MPI_Comm_rank(comm,&me);
  MPI_Comm_size(comm,&sz);
  other = 1 - me;

  MPI_Request sreq[1];
  MPI_Request rreq[1];
  MPI_Status stat[1];

  T send[1];
  T recv[1];

  recv[0] = -1;
  send[0] = me;

  MPI_Isend(&send[0],1,mpi_dtype<T>(), other, 1234, comm, &sreq[0]);
  MPI_Irecv(&recv[0],1,mpi_dtype<T>(), other, 1234, comm, &rreq[0]);

  MPI_Waitall (1, &sreq[0], &stat[0]);
  MPI_Waitall (1, &rreq[0], &stat[0]);

  std::cout << "rank " << me << " received " << recv[0] << " from " << other << "\n";
}

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  MPI_Comm comm(MPI_COMM_WORLD);

  run<int>(comm);
  run<float>(comm);
  run<char>(comm);
  run<double>(comm);
  run<bool>(comm);

  MPI_Finalize();

  return 0;
}
