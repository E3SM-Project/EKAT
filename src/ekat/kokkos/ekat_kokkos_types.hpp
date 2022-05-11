#ifndef EKAT_KOKKOS_TYPES_HPP
#define EKAT_KOKKOS_TYPES_HPP

#include "ekat/ekat_config.h"
#include "ekat/kokkos/ekat_kokkos_meta.hpp"
#include "ekat/std_meta/ekat_std_type_traits.hpp"
#include <Kokkos_MemoryTraits.hpp>

/*
 * Header contains globally useful kokkos-related types for ekat.
 */

namespace ekat {

#if defined(KOKKOS_ENABLE_CUDA) && defined(KOKKOS_COMPILER_GNU)
// See https://github.com/kokkos/kokkos-kernels/issues/129
# define ConstExceptGnu
#else
# define ConstExceptGnu const
#endif

// The default device we use.
using DefaultDevice = Kokkos::Device<Kokkos::DefaultExecutionSpace, Kokkos::DefaultExecutionSpace::memory_space>;

// A device type to force host execution
using HostDevice = Kokkos::Device<Kokkos::DefaultHostExecutionSpace, Kokkos::DefaultHostExecutionSpace::memory_space>;

template<typename DT, typename... Props>
using ViewLR = Kokkos::View<DT,Kokkos::LayoutRight,Props...>;

#ifdef EKAT_ENABLE_DEPRECATED_CODE
// Struct for getting useful Kokkos types based on the device
template <typename DeviceType>
struct KokkosTypes
{
  using Device = DeviceType;
  using Layout = Kokkos::LayoutRight;
  using MemSpace = typename Device::memory_space;
  using ExeSpace = typename Device::execution_space;
  using TeamPolicy = Kokkos::TeamPolicy<ExeSpace>;
  using MemberType = typename TeamPolicy::member_type;
  using RangePolicy = Kokkos::RangePolicy<ExeSpace>;
  template<typename TagType>
  using RangeTagPolicy = Kokkos::RangePolicy<ExeSpace,TagType>;
  template<typename TagType>
  using TeamTagPolicy = Kokkos::TeamPolicy<ExeSpace,TagType>;

  template <typename DataType, typename MemoryTraits = Kokkos::MemoryManaged>
  using view = Kokkos::View<DataType, Layout, Device, MemoryTraits>;

  // left-layout views, may be useful for interacting with fortran
  template <typename DataType, typename MemoryTraits = Kokkos::MemoryManaged>
  using lview = Kokkos::View<DataType, Kokkos::LayoutLeft, Device, MemoryTraits>;

  // A N-dim view given scalar type and N
  template<typename Scalar, int N, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_ND = view<typename DataND<Scalar,N>::type,MemoryTraits>;

  // More verbose cases for N=1,2,3
  template <typename Scalar, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d = view<Scalar*,MemoryTraits>;

  template <typename Scalar, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_2d = view<Scalar**,MemoryTraits>;

  template <typename Scalar, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_3d = view<Scalar***,MemoryTraits>;

  template <typename Scalar, int X, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d_table = view<const Scalar[X],MemoryTraits>;

  template <typename Scalar, int X, int Y, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_2d_table = view<const Scalar[X][Y],MemoryTraits>;

  // Our workspace implementation makes this a useful type
  template <typename Scalar, int N, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d_ptr_array = Kokkos::Array<Unmanaged<view_1d<Scalar,MemoryTraits> >*, N>;

  template <typename Scalar, int N, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d_ptr_carray = Kokkos::Array<const Unmanaged<view_1d<Scalar,MemoryTraits> >*, N>;
};
#endif

template <typename MemorySpace>
struct ViewTypes
{
  using is_mem = Kokkos::is_space<MemorySpace>;
  static_assert (is_mem::value &&
                 std::is_same<MemorySpace,typename is_mem::memory_space>::value,
                 "Error! Input MemorySpace is not a valid memory space.\n");

  using Layout = Kokkos::LayoutRight;
  using MemSpace = MemorySpace;

  template <typename DataType, typename MemoryTraits = Kokkos::MemoryManaged>
  using view = Kokkos::View<DataType, Layout, MemSpace, MemoryTraits>;

  // left-layout views, may be useful for interacting with fortran
  template <typename DataType, typename MemoryTraits = Kokkos::MemoryManaged>
  using lview = Kokkos::View<DataType, Kokkos::LayoutLeft, MemSpace, MemoryTraits>;

  // A N-dim view given scalar type and N
  template<typename Scalar, int N, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_ND = view<typename DataND<Scalar,N>::type,MemoryTraits>;

  // More verbose cases for N=1,2,3
  template <typename Scalar, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d = view<Scalar*,MemoryTraits>;

  template <typename Scalar, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_2d = view<Scalar**,MemoryTraits>;

  template <typename Scalar, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_3d = view<Scalar***,MemoryTraits>;

  template <typename Scalar, int X, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d_table = view<const Scalar[X],MemoryTraits>;

  template <typename Scalar, int X, int Y, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_2d_table = view<const Scalar[X][Y],MemoryTraits>;

  // Our workspace implementation makes this a useful type
  template <typename Scalar, int N, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d_ptr_array = Kokkos::Array<Unmanaged<view_1d<Scalar,MemoryTraits> >*, N>;

  template <typename Scalar, int N, typename MemoryTraits = Kokkos::MemoryManaged>
  using view_1d_ptr_carray = Kokkos::Array<const Unmanaged<view_1d<Scalar,MemoryTraits> >*, N>;
};

template <typename ExecutionSpace>
struct PolicyTypes
{
  using is_exe = Kokkos::is_space<ExecutionSpace>;
  static_assert (is_exe::value &&
                 std::is_same<ExecutionSpace,typename is_exe::execution_space>::value,
                 "Error! Input ExecutionSpace is not a valid execution space.\n");

  using ExeSpace = ExecutionSpace;
  using TeamPolicy = Kokkos::TeamPolicy<ExeSpace>;
  using MemberType = typename TeamPolicy::member_type;
  using RangePolicy = Kokkos::RangePolicy<ExeSpace>;
  template<typename TagType>
  using RangeTagPolicy = Kokkos::RangePolicy<ExeSpace,TagType>;
  template<typename TagType>
  using TeamTagPolicy = Kokkos::TeamPolicy<ExeSpace,TagType>;
};

} // namespace ekat

#endif // EKAT_KOKKOS_TYPES_HPP
