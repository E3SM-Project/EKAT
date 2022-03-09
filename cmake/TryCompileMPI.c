#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
#if defined(OMPI_MAJOR_VERSION)
#pragma message("OpenMPI found")
#elif defined(MPICH_VERSION)
#pragma message("MPICH found")
#else
#pragma message("Unrecognized MPI distribution")
#endif

  return 0;
}
