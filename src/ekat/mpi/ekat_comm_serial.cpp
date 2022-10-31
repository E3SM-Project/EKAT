#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_assert.hpp"

#include <cassert>

// This file is compiled when EKAT_ENABLE_MPI is not defined. It provides the
// correct functionality for a single-process configuration.

namespace ekat
{

Comm::Comm()
  : m_mpi_comm(MPI_COMM_SELF), m_size(1), m_rank(0)
{
}

Comm::Comm(MPI_Comm mpi_comm)
  : m_mpi_comm(mpi_comm), m_size(1), m_rank(0)
{
}

void Comm::reset_mpi_comm (MPI_Comm new_mpi_comm)
{
}

void Comm::barrier () const
{
}

Comm Comm::split (const int color) const
{
  return Comm(MPI_COMM_SELF);
}

void Comm::check_mpi_inited () const
{
}

template<>
MPI_Datatype get_mpi_type <char> () {
  return MPI_CHAR;
}

template<>
MPI_Datatype get_mpi_type <short> () {
  return MPI_SHORT;
}
template<>
MPI_Datatype get_mpi_type <int> () {
  return MPI_INT;
}
template<>
MPI_Datatype get_mpi_type <long> () {
  return MPI_LONG;
}
template<>
MPI_Datatype get_mpi_type <long long> () {
  return MPI_LONG_LONG;
}
template<>
MPI_Datatype get_mpi_type <float> () {
  return MPI_FLOAT;
}
template<>
MPI_Datatype get_mpi_type <double> () {
  return MPI_DOUBLE;
}
#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
template<>
MPI_Datatype get_mpi_type <bool> () {
  return MPI_CXX_BOOL;
}
#endif

} // namespace ekat
