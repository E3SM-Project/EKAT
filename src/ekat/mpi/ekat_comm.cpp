#include "ekat/mpi/ekat_comm.hpp"
#include "ekat/ekat_assert.hpp"

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
  EKAT_REQUIRE_MSG (new_mpi_comm!=MPI_COMM_NULL,
      "Error! ekat::Comm requires non-null MPI comm.");

  m_mpi_comm = new_mpi_comm;

  MPI_Comm_size(m_mpi_comm,&m_size);
  MPI_Comm_rank(m_mpi_comm,&m_rank);

#ifdef EKAT_MPI_ERRORS_ARE_FATAL
  MPI_Comm_set_errhandler(m_mpi_comm,MPI_ERRORS_ARE_FATAL);
#else
  MPI_Comm_set_errhandler(m_mpi_comm,MPI_ERRORS_RETURN);
#endif
}

void Comm::barrier () const
{
  check_mpi_inited();
  MPI_Barrier(m_mpi_comm);
}

Comm Comm::split (const int color) const
{
  check_mpi_inited ();

  MPI_Comm new_comm;
  MPI_Comm_split(m_mpi_comm,color,m_rank,&new_comm);

  return Comm(new_comm);
}

void Comm::check_mpi_inited () const
{
  int flag;
  MPI_Initialized (&flag);
  assert (flag!=0);
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
