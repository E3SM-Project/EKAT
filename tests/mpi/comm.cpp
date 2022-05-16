#include <catch2/catch.hpp>
#include "ekat/mpi/ekat_comm.hpp"

// Instantiate get_mpi_type for a user defined type
// to check that the user can extend comm functionalities
// to new types
struct TwoInts {
  int i;
  int j;
  TwoInts () = default;
  TwoInts (const int val) {
    i = val;
    j = -val;
  }
  TwoInts& operator= (const int val) {
    i = val;
    j = -val;
    return *this;
  }
};

bool operator== (const TwoInts& lhs, const TwoInts& rhs) {
  return lhs.i==rhs.i && lhs.j==rhs.j;
}

namespace ekat {
  template<>
  MPI_Datatype get_mpi_type<TwoInts> () {
    return MPI_2INT;
  }
}

namespace {

template<typename T>
void test_broadcast (const ekat::Comm& comm) {
  const int rank = comm.rank();
  const int size = comm.size();
  T* vals = new T[size];
  vals[rank] = -rank;

  for (int i=0; i<size; ++i) {
    comm.broadcast(&vals[i],1,i);

    REQUIRE (vals[i]==T(-i));
  }

  delete[] vals;
}

template<typename T>
void test_scan (const ekat::Comm& comm) {
  const int rank = comm.rank();
  T val = rank;
  T sum;

  comm.scan(&val,&sum,1,MPI_SUM);

  const T sum_gauss = T(rank*(rank+1))/2;

  REQUIRE(sum==sum_gauss);
}

template<typename T>
void test_scan_in_place (const ekat::Comm& comm) {
  const int rank = comm.rank();
  T sum = rank;

  comm.scan(&sum,1,MPI_SUM);

  const T sum_gauss = T(rank*(rank+1))/2;

  REQUIRE(sum==sum_gauss);
}

template<typename T>
void test_reduce (const ekat::Comm& comm, T val, const T tgt, const MPI_Op op) {
  T reduced_val;
  comm.all_reduce(&val,&reduced_val,1,op);
  REQUIRE (reduced_val==tgt);
}

template<typename T>
void test_reduce_in_place (const ekat::Comm& comm, T val, const T tgt, const MPI_Op op) {
  comm.all_reduce(&val,1,op);
  REQUIRE (val==tgt);
}

template<typename T>
void test_gather (const ekat::Comm& comm) {
  const T rank = comm.rank();
  const int size = comm.size();
  T* ranks = new T [size];

  comm.all_gather(&rank, ranks, 1);

  for (int i=0; i<size; ++i) {
    REQUIRE (ranks [i]==T(i));
  }
  delete[] ranks;
}

template<typename T>
void test_gather_in_place (const ekat::Comm& comm) {
  const T rank = comm.rank();
  const int size = comm.size();
  T* ranks = new T [size];
  ranks[comm.rank()] = rank;

  comm.all_gather(ranks, 1);

  for (int i=0; i<size; ++i) {
    REQUIRE (ranks [i]==T(i));
  }
  delete[] ranks;
}

TEST_CASE ("ekat_comm","") {
  using namespace ekat;

  Comm comm(MPI_COMM_WORLD);
  const int rank = comm.rank();
  const int size = comm.size();

  REQUIRE (comm.root_rank()==0);
  REQUIRE (comm.am_i_root() == (comm.root_rank()==comm.rank()));

  SECTION ("null") {
    REQUIRE_THROWS (Comm(MPI_COMM_NULL));
  }

  SECTION ("broadcast") {
    test_broadcast<char>(comm);
#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
    test_broadcast<bool>(comm);
#endif
    test_broadcast<int>(comm);
    test_broadcast<float>(comm);
    test_broadcast<double>(comm);
  }

  SECTION ("scan") {
    test_scan<int>(comm);
    test_scan<float>(comm);
    test_scan<double>(comm);
  }

  SECTION ("scan_in_place") {
    test_scan_in_place<int>(comm);
    test_scan_in_place<float>(comm);
    test_scan_in_place<double>(comm);
  }

  SECTION ("reduce") {
    const int sum_gauss = (size-1)*size/2;
    test_reduce<int>(comm,rank,sum_gauss,MPI_SUM);
    test_reduce<float>(comm,rank,sum_gauss,MPI_SUM);
    test_reduce<double>(comm,rank,sum_gauss,MPI_SUM);

#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
    test_reduce<bool>(comm,comm.am_i_root(),size==1,MPI_LAND);
    test_reduce<bool>(comm,comm.am_i_root(),true,MPI_LOR);
#endif
  }

  SECTION ("reduce_in_place") {
    const int sum_gauss = (size-1)*size/2;
    test_reduce_in_place<int>(comm,rank,sum_gauss,MPI_SUM);
    test_reduce_in_place<float>(comm,rank,sum_gauss,MPI_SUM);
    test_reduce_in_place<double>(comm,rank,sum_gauss,MPI_SUM);

#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
    test_reduce_in_place<bool>(comm,comm.am_i_root(),size==1,MPI_LAND);
    test_reduce_in_place<bool>(comm,comm.am_i_root(),true,MPI_LOR);
#endif
  }

  SECTION ("gather") {
    test_gather<char>(comm);
#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
    test_gather<bool>(comm);
#endif
    test_gather<short>(comm);
    test_gather<int>(comm);
    test_gather<long>(comm);
    test_gather<long long>(comm);
    test_gather<float>(comm);
    test_gather<double>(comm);
    test_gather<TwoInts>(comm);
  }

  SECTION ("gather_in_place") {
    test_gather_in_place<char>(comm);
#if MPI_VERSION>3 || (MPI_VERSION==3 && MPI_SUBVERSION>=1)
    test_gather_in_place<bool>(comm);
#endif
    test_gather_in_place<short>(comm);
    test_gather_in_place<int>(comm);
    test_gather_in_place<long>(comm);
    test_gather_in_place<long long>(comm);
    test_gather_in_place<float>(comm);
    test_gather_in_place<double>(comm);
    test_gather_in_place<TwoInts>(comm);
  }

  SECTION ("split") {
    auto new_comm = comm.split(rank % 2);
    
    const bool am_even = rank%2 == 0;
    const int num_odd  = size/2;
    const int num_even = size%2 + num_odd;

    // Check the new comm has the right size
    REQUIRE ( ((am_even && new_comm.size()==num_even) ||
               (not am_even && new_comm.size()==num_odd)) );

    // Check that the right world ranks ended up in the new comm
    int* ranks = new int[new_comm.size()];

    new_comm.all_gather(&rank,ranks,1);

    for (int i=0; i<new_comm.size(); ++i) {
      if (am_even) {
        REQUIRE ( ranks[i] == 2*i );
      } else {
        REQUIRE ( ranks[i] == (2*i+1) );
      }
    }

    delete[] ranks;
  }
}

} // anonymous namespace
