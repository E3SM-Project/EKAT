#include <mpi.h>

#include <vector>
#include <cstdio>

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  MPI_Comm comm(MPI_COMM_WORLD);

  int me, sz, other;
  MPI_Comm_rank(comm,&me);
  MPI_Comm_size(comm,&sz);
  other = 1 - me;

  std::vector<MPI_Request> sreq(1), rreq(1);
  std::vector<MPI_Status> stat(1);

  std::vector<double> send(1), recv(1);

  recv[0] = -1;
  send[0] = me;

  MPI_Isend(&send[0],1,MPI_DOUBLE, other, 1234, comm, &sreq[0]);
  MPI_Irecv(&recv[0],1,MPI_DOUBLE, other, 1234, comm, &rreq[0]);

  MPI_Waitall (1, &sreq[0], &stat[0]);
  MPI_Waitall (1, &rreq[0], &stat[0]);

  printf("[rank=%d], %d sent me: %f\n", me, other, recv[0]);

  MPI_Finalize();

  return 0;
}
