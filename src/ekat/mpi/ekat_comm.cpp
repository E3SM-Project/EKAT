#include "ekat_comm.hpp"

#include <cassert>

namespace ekat
{

Comm::Comm()
{
  check_mpi_inited();
  reset_mpi_comm (MPI_COMM_SELF);
}

Comm::Comm(MPI_Comm mpi_comm)
{
  check_mpi_inited();
  reset_mpi_comm (mpi_comm);
}

void Comm::reset_mpi_comm (MPI_Comm new_mpi_comm)
{
  m_mpi_comm = new_mpi_comm;

  MPI_Comm_size(m_mpi_comm,&m_size);
  MPI_Comm_rank(m_mpi_comm,&m_rank);

#ifdef EKAT_MPI_ERRORS_ARE_FATAL
  MPI_Comm_set_errhandler(m_mpi_comm,MPI_ERRORS_ARE_FATAL);
#else
  MPI_Comm_set_errhandler(m_mpi_comm,MPI_ERRORS_RETURN);
#endif
}

template<>
void Comm::scan_sum<int>(const int* my_vals, int* my_sums, const int count) const {
  check_mpi_inited();
  MPI_Scan(my_vals,my_sums,count,MPI_INT,MPI_SUM,m_mpi_comm);
}

template<>
void Comm::scan_sum<float>(const float* my_vals, float* my_sums, const int count) const {
  check_mpi_inited();
  MPI_Scan(my_vals,my_sums,count,MPI_FLOAT,MPI_SUM,m_mpi_comm);
}

template<>
void Comm::scan_sum<double>(const double* my_vals, double* my_sums, const int count) const {
  check_mpi_inited();
  MPI_Scan(my_vals,my_sums,count,MPI_DOUBLE,MPI_SUM,m_mpi_comm);
}

void Comm::check_mpi_inited () const
{
  int flag;
  MPI_Initialized (&flag);
  assert (flag!=0);
}


} // namespace ekat
