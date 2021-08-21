#ifndef EKAT_COMM_HPP
#define EKAT_COMM_HPP

#include <type_traits>

#include <mpi.h>

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

  bool am_i_root () const { return m_rank==0; }
  int  rank () const { return m_rank; }
  int  size () const { return m_size; }
  MPI_Comm mpi_comm () const { return m_mpi_comm; }

  // Convenience functions wrapping MPI analogues.
  // NOTE: the methods are templated on the values types, but so far the only
  // supporterd types are: int, float, double
  template<typename T>
  void scan (const T* my_vals, T* result, const int count, const MPI_Op op) const;

  template<typename T>
  void all_reduce (const T* my_vals, T* result, const int count, const MPI_Op op) const;

  template<typename T>
  void broadcast (T* vals, const int count, const int root) const;

  void barrier () const;
private:

  template<typename T>
  static MPI_Datatype get_mpi_type () {
    // Sanity check
    static_assert (
        std::is_same<T,int>::value ||
        std::is_same<T,float>::value ||
        std::is_same<T,double>::value,
        "Error! Type not supported for MPI operations.\n");

    return std::is_same<T,int>::value ? MPI_INT :
          (std::is_same<T,float>::value ? MPI_FLOAT : MPI_DOUBLE);
  }

  // Checks (with an assert) that MPI is already init-ed.
  void check_mpi_inited () const;

  MPI_Comm  m_mpi_comm;

  int       m_size;
  int       m_rank;
};

// ========================= IMPLEMENTATION =========================== //

template<typename T>
void Comm::scan (const T* my_vals, T* result, const int count, const MPI_Op op) const
{
  check_mpi_inited();
  MPI_Scan(my_vals,result,count,get_mpi_type<T>(),op,m_mpi_comm);
}

template<typename T>
void Comm::all_reduce (const T* my_vals, T* result, const int count, const MPI_Op op) const
{
  check_mpi_inited();
  MPI_Allreduce(my_vals,result,count,get_mpi_type<T>(),op,m_mpi_comm);
}

template<typename T>
void Comm::broadcast (T* vals, const int count, const int root) const
{
  check_mpi_inited();
  MPI_Bcast(vals,count,get_mpi_type<T>(),root,m_mpi_comm);
}

} // namespace ekat

#endif // EKAT_COMM_HPP
