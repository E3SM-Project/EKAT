#include <iostream>
#include "mpi.h"

using namespace std;

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  cout << "Hello ";
  for (int i = 1; i < argc; ++i) {
    cout << argv[i] << " ";
  }
  cout << endl;

  MPI_Finalize();

  return 0;
}
