#include <mpi.h>
#include <stdio.h>

int main (int, char**)
{
#if defined(OMPI_MAJOR_VERSION)
#pragma message("OpenMPI found")
#elif defined(MPICH_VERSION)
#pragma message("MPICH found")
#else
#pragma message("No MPI found")
#endif

  return 0;
}
