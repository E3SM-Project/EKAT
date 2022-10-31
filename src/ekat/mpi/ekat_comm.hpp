#ifndef EKAT_COMM_HPP
#define EKAT_COMM_HPP

#include <ekat/ekat_config.h>

#include <type_traits>

#ifdef EKAT_ENABLE_MPI
#include <mpi.h>
#else
#include <algorithm> // for std::copy
// These are stand-ins for the MPI data types that appear in Comm's interface.
enum MPI_Comm {
  MPI_COMM_NULL,
  MPI_COMM_WORLD,
  MPI_COMM_SELF
};
enum MPI_Datatype {
  MPI_CHAR,
  MPI_SHORT,
  MPI_INT,
  MPI_LONG,
  MPI_LONG_LONG,
  MPI_FLOAT,
  MPI_DOUBLE,
#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
  MPI_CXX_BOOL,
#endif
};
enum MPI_Op {
  MPI_OP_NULL,
  MPI_MAX,
  MPI_MIN,
  MPI_SUM,
  MPI_PROD,
  MPI_LAND,
  MPI_BAND,
  MPI_LOR,
  MPI_BOR,
  MPI_LXOR,
  MPI_BXOR,
  MPI_MINLOC,
  MPI_MAXLOC,
  MPI_REPLACE,
};
#endif

namespace ekat
{

// A small wrapper around an MPI_Comm, together with its rank/size

// NOTE: this class checks that MPI is already init-ed, and errors out
//       if it is not. It is YOUR responsibility to make sure MPI is
//       init-ed before you create a ekat::Comm

class Comm
{
public:

  // The default comm creates a wrapper to MPI_COMM_SELF, rather than MPI_COMM_WORLD,
  // because it is safer to assume I'm the only proc in the comm rather than assuming
  // that the whole world is in my group.
  Comm ();

  // This constructor wraps the given MPI_Comm
  explicit Comm (MPI_Comm mpi_comm);

  // This method resets the stored MPI_Comm to the given one,
  // updating m_size and m_rank accordingly.
  // WARNING: it is YOUR responsibility to ensure that
  //   1) MPI is already inited (call check_mpi_init first, if you are not sure)
  //   2) the call is collective on both the stored and input comm's
  void reset_mpi_comm (MPI_Comm new_mpi_comm);

  // Expose root rank, to avoid having users "assuming" root=0,
  // even though it's probably a safe assumption.
  int root_rank () const { return 0; }
  bool am_i_root () const { return m_rank==root_rank(); }

  int  rank () const { return m_rank; }
  int  size () const { return m_size; }
  MPI_Comm mpi_comm () const { return m_mpi_comm; }

  // Convenience functions wrapping MPI analogues.
  // NOTE: the methods are templated on the values types, but so far the only
  // supporterd types are: int, float, double

  template<typename T>
  void broadcast (T* vals, const int count, const int root) const;

  template<typename T>
  void scan (const T* my_vals, T* result, const int count, const MPI_Op op) const;

  template<typename T>
  void all_reduce (const T* my_vals, T* result, const int count, const MPI_Op op) const;

  template<typename T>
  void all_gather (const T* my_vals, T* all_vals, const int count) const;

  // In place version of the above
  template<typename T>
  void scan (T* result, const int count, const MPI_Op op) const;

  template<typename T>
  void all_reduce (T* inout_vals, const int count, const MPI_Op op) const;

  template<typename T>
  void all_gather (T* all_vals, const int count) const;

  void barrier () const;

  Comm split (const int color) const;
private:

  // Checks (with an assert) that MPI is already init-ed.
  void check_mpi_inited () const;

  MPI_Comm  m_mpi_comm;

  int       m_size;
  int       m_rank;
};

template<typename T>
MPI_Datatype get_mpi_type ();

// ========================= IMPLEMENTATION =========================== //

template<typename T>
void Comm::broadcast (T* vals, const int count, const int root) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  MPI_Bcast(vals,count,get_mpi_type<T>(),root,m_mpi_comm);
#endif
}

template<typename T>
void Comm::scan (const T* my_vals, T* result, const int count, const MPI_Op op) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  MPI_Scan(my_vals,result,count,get_mpi_type<T>(),op,m_mpi_comm);
#else
  std::copy(my_vals, my_vals + count, result);
#endif
}

template<typename T>
void Comm::all_reduce (const T* my_vals, T* result, const int count, const MPI_Op op) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  MPI_Allreduce(my_vals,result,count,get_mpi_type<T>(),op,m_mpi_comm);
#else
  std::copy(my_vals, my_vals + count, result);
#endif
}

template<typename T>
void Comm::all_gather (const T* my_vals, T* all_vals, const int count) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  auto mpi_type = get_mpi_type<T>();
  MPI_Allgather(my_vals, count,mpi_type,
                all_vals,count,mpi_type,
                m_mpi_comm);
#else
  std::copy(my_vals, my_vals + count, all_vals);
#endif
}

template<typename T>
void Comm::scan (T* inout_vals, const int count, const MPI_Op op) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  MPI_Scan(MPI_IN_PLACE,inout_vals,count,get_mpi_type<T>(),op,m_mpi_comm);
#endif
}

template<typename T>
void Comm::all_reduce (T* inout_vals, const int count, const MPI_Op op) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  MPI_Allreduce(MPI_IN_PLACE,inout_vals,count,get_mpi_type<T>(),op,m_mpi_comm);
#endif
}

template<typename T>
void Comm::all_gather (T* inout_vals, const int count) const
{
#ifdef EKAT_ENABLE_MPI
  check_mpi_inited();
  auto mpi_type = get_mpi_type<T>();
  MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
                inout_vals,count,mpi_type,
                m_mpi_comm);
#endif
}

} // namespace ekat

#endif // EKAT_COMM_HPP
