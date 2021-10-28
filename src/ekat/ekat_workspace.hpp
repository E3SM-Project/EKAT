#ifndef EKAT_WSM_HPP
#define EKAT_WSM_HPP

#include "ekat/util/ekat_arch.hpp"
#include "ekat/kokkos/ekat_kokkos_utils.hpp"
#include "ekat/kokkos/ekat_kokkos_types.hpp"

namespace unit_test {
struct UnitWrap;
}

namespace ekat {

/*
 * WorkspaceManager is a utility for requesting workspaces
 * (temporary memory blocks) from within a Kokkos kernel. Workspaces
 * are shared within thread teams.
 *
 * The WorkspaceManager should be initialized before Kokkos kernels
 * are run.  Users need to specify the size of each individual
 * sub-block (in terms of number of T's), the maximum number of
 * sub-blocks (AKA "spaces") they intend to use, and the team policy
 * that they intend to use for their Kokkos kernels.
 *
 * Once inside a Kokkos kernel, the user will call get_workspace on
 * their WorkspaceManager. This will return a Workspace object which
 * is the user's iterface to the entire memory block, which mostly
 * involves taking/releasing memory sub-blocks (represented by kokkos
 * views). The API for taking/releasing sub-blocks has a number of
 * variants in order to offer both simplicity/clarity and high
 * performance. The simplest way to use your Workspace is to manage
 * your sub-blocks individually with the "take" and "release"
 * methods. You'll get better performance with the methods that deal
 * with the sub-blocks in bulk.  We've seen the best performance by
 * making a single take_many_and_reset call at the beginning of the
 * Kokkos kernel, though this may not be practical for large
 * kernels. High-granularity take/release calls can allow you to use a
 * smaller Workspace (by allowing you to use a smaller max_used).
 *
 * You can fine-tune the max_used by making a large overestimate for
 * this value, running your kernel, and then calling report, which
 * will tell you the actual maximum number of sub-blocks that you
 * used. Note that all sub-blocks have a name.
 */

template <typename T, typename DeviceT=DefaultDevice>
class WorkspaceManager
{
 public:

  //
  // ---------- Types ---------
  //

  using Device = DeviceT;

  using TeamPolicy = typename KokkosTypes<Device>::TeamPolicy;
  using MemberType = typename KokkosTypes<Device>::MemberType;
  using ExeSpace   = typename KokkosTypes<Device>::ExeSpace;

  template <typename S>
  using view_1d = typename KokkosTypes<Device>::template view_1d<S>;
  template <typename S>
  using view_2d = typename KokkosTypes<Device>::template view_2d<S>;
  template <typename S>
  using view_3d = typename KokkosTypes<Device>::template view_3d<S>;

  template <typename S, int N>
  using view_1d_ptr_array = typename KokkosTypes<Device>::template view_1d_ptr_array<S, N>;

  //
  // -------- Contants --------
  //

  // Default overprov factor for large GPU problems, testing has shown 1.25 is optimal
  static constexpr double GPU_DEFAULT_OVERPROVISION_FACTOR = 1.25;

  //
  // ------- public API ---------
  //

  // Constructor, call from host
  //   size: The number of T's per sub-block
  //   max_used: The maximum number of active sub-blocks
  //   policy: The team policy for Kokkos kernels using this WorkspaceManager
  //   overprov_factor: How many workspace slots to overprovision (only applies to GPU for large problems)
  WorkspaceManager(int size, int max_used, TeamPolicy policy,
                   const double& overprov_factor=GPU_DEFAULT_OVERPROVISION_FACTOR);

  // Constructor, call from host
  //   Same as above, but here the user initializes the data.
  //   This is useful when the user wants to pre-reserve the data
  //   for the WorkspaceManager. The user is responsible for
  //   ensuring that m_max_ws_idx*m_total*m_max_used contiguous
  //   data is available, for which the get_total_slots_to_be_used()
  //   function can be helpful.
  WorkspaceManager(T* data, int size, int max_used, TeamPolicy policy,
                   const double& overprov_factor=GPU_DEFAULT_OVERPROVISION_FACTOR);

  // Helper functions which return the number of bytes that will be reserved for a given
  // set of constructor inputs. Note, this does not actually create an instance of the WSM,
  // but is useful for when memory needs to be reserved in a different scope than the
  // WSM is created.
  static int get_total_bytes_needed(int size, int max_used, TeamPolicy policy,
                                    const double& overprov_factor=GPU_DEFAULT_OVERPROVISION_FACTOR);

  // call from host.
  //
  // Will report usage statistics for your workspaces. These statistics will
  // have much more detail for debug builds.
  void report() const;

  class Workspace;

  // call from device
  //
  // Returns a Workspace object which provides access to sub-blocks.
  KOKKOS_INLINE_FUNCTION
  Workspace get_workspace(const MemberType& team, const char* name = "") const;

  // call from device
  //
  // Releases a Workspace object, should normally be called via the
  // Workspace destructor.
  KOKKOS_INLINE_FUNCTION
  void release_workspace(const MemberType& team, const Workspace& ws) const;

  class Workspace {
   public:

    // Take an individual sub-block
    template <typename S=T>
    KOKKOS_INLINE_FUNCTION
    Unmanaged<view_1d<S> > take(const char* name) const;

    // Take several sub-blocks. The user gets pointers to their sub-blocks
    // via the ptrs argument.
    template <size_t N, typename S=T>
    KOKKOS_INLINE_FUNCTION
    void take_many(const Kokkos::Array<const char*, N>& names,
                   const view_1d_ptr_array<S, N>& ptrs) const;

    // Similar to take_many except assumes that there is enough contiguous
    // memory avaiable in the Workspace for N sub-blocks. This method is higher-performing
    // than take_many.
    template <size_t N, typename S=T>
    KOKKOS_INLINE_FUNCTION
    void take_many_contiguous_unsafe(const Kokkos::Array<const char*, N>& names,
                                     const view_1d_ptr_array<S, N>& ptrs) const;

    // Take an individual sub-block while telling the WorkSpaceManager to skip over the
    // next n_sub_blocks-1 sub-blocks. This allows the user to safely access the memory
    // of these sub-block, which is useful for creating local 2d views through the
    // WorkspaceManager.
    //
    // Example: Local 2d view of size (n, m_size).
    // Code:
    //   const auto local_slot = workspace.take_n_size_block("local_slot",n);
    //   const auto local_var  = Unmanaged2dViewT<ScalarT>(reinterpret_cast<ScalarT*>(local_slot.data()),
    //                                                     n, m_sizes);
    template <typename S=T>
    KOKKOS_INLINE_FUNCTION
    Unmanaged<view_1d<S> > take_macro_block(const char* name, const int n_sub_blocks) const;

    // Combines reset and take_many_contiguous_unsafe. This is the most-performant
    // option for a kernel to use N sub-blocks that are needed for the duration of the
    // kernel.
    template <size_t N, typename S=T>
    KOKKOS_INLINE_FUNCTION
    void take_many_and_reset(const Kokkos::Array<const char*, N>& names,
                             const view_1d_ptr_array<S, N>& ptrs) const;

    // Release an individual sub-block.
    template <typename View>
    KOKKOS_FORCEINLINE_FUNCTION
    void release(const View& space, std::enable_if<View::rank == 1>* = 0) const
    { release_impl<typename View::value_type>(space); }

    // Release several contiguous sub-blocks.
    template <size_t N, typename S=T>
    KOKKOS_INLINE_FUNCTION
    void release_many_contiguous(const view_1d_ptr_array<S, N>& ptrs) const;

    // Release block of size n*m_size.
    template <typename S=T>
    KOKKOS_INLINE_FUNCTION
    void release_macro_block(const Unmanaged<view_1d<S> >& space, const int n_sub_blocks) const;

#ifndef NDEBUG
    // Get the name of a sub-block
    template <typename View>
    KOKKOS_INLINE_FUNCTION
    const char* get_name(const View& space, std::enable_if<View::rank == 1>* = 0) const
    { return get_name_impl<typename View::value_type>(space); }
#endif

    // Reset back to initial state. All sub-blocks will be considered inactive.
    KOKKOS_INLINE_FUNCTION
    void reset() const;

    // Print the linked list. Obviously not a device function.
    void print() const;

    //
    // ---------- Private --------------
    //

    // Not technically private, but not part of the API since the user won't call it directly
    KOKKOS_INLINE_FUNCTION
    ~Workspace();

#ifndef KOKKOS_ENABLE_CUDA
   private:
#endif

    template <typename S>
    KOKKOS_INLINE_FUNCTION
    void release_impl(const Unmanaged<view_1d<S> >& space) const;

#ifndef NDEBUG
    template <typename S>
    KOKKOS_INLINE_FUNCTION
    const char* get_name_impl(const Unmanaged<view_1d<S> >& space) const;

    KOKKOS_INLINE_FUNCTION
    void change_num_used(int change_by) const;

    template <typename S>
    KOKKOS_INLINE_FUNCTION
    void change_indv_meta(const Unmanaged<view_1d<S> >& space, const char* name, bool release=false) const;

    KOKKOS_INLINE_FUNCTION
    int get_name_idx(const char* name, bool add) const;

    KOKKOS_INLINE_FUNCTION
    int get_alloc_count(const char* name) const
    { return m_parent.m_counts(m_ws_idx, get_name_idx(name), 0); }

    KOKKOS_INLINE_FUNCTION
    int get_release_count(const char* name) const
    { return m_parent.m_counts(m_ws_idx, get_name_idx(name), 1); }

    KOKKOS_INLINE_FUNCTION
    int get_num_used() const
    { return m_parent.m_num_used(m_ws_idx); }

    template <typename S>
    KOKKOS_INLINE_FUNCTION
    bool is_active(const Unmanaged<view_1d<S> >& space) const
    { return m_parent.m_active(m_ws_idx, m_parent.template get_index<S>(space));}
#endif

    KOKKOS_INLINE_FUNCTION
    Workspace(const WorkspaceManager& parent, int ws_idx, const MemberType& team, const char* ws_name);

    friend struct unit_test::UnitWrap;
    friend class WorkspaceManager;

    const WorkspaceManager& m_parent;
    const MemberType& m_team;
    const int m_ws_idx; // Workspace idx for m_team
    int& m_next_slot; // the next free ws slot to allocate
    const char* m_ws_name;
  }; // class Workspace

#ifndef KOKKOS_ENABLE_CUDA
 private:
#endif

  friend struct unit_test::UnitWrap;

  template <typename S=T>
  KOKKOS_FORCEINLINE_FUNCTION
  int get_index(const Unmanaged<view_1d<S> >& space) const
  { return reinterpret_cast<const int*>(reinterpret_cast<const T*>(space.data()) - m_reserve)[0]; }

  template <typename S=T>
  KOKKOS_FORCEINLINE_FUNCTION
  int get_next(const Unmanaged<view_1d<S> >& space) const
  { return reinterpret_cast<const int*>(reinterpret_cast<const T*>(space.data()) - m_reserve)[1]; }

  template <typename S=T>
  KOKKOS_FORCEINLINE_FUNCTION
  int set_next_and_get_index(const Unmanaged<view_1d<S> >& space, int next) const;

  template <typename S=T>
  KOKKOS_FORCEINLINE_FUNCTION
  Unmanaged<view_1d<S> > get_space_in_slot(const int team_idx, const int slot) const;

  KOKKOS_INLINE_FUNCTION
  void init_slot_metadata(const int ws_idx, const int slot) const;

  void init_all_metadata(const int max_ws_idx, const int max_used);

  void compute_internals(const int size, const int max_used);

  //
  // data
  //

  enum { m_pad_factor   = OnGpu<ExeSpace>::value ? 1 : 32,
         m_max_name_len = 128,
         m_max_names    = 256
  };

  TeamUtils<T,ExeSpace> m_tu;
  int m_max_ws_idx, m_reserve, m_size, m_total, m_max_used;
#ifndef NDEBUG
  view_1d<int> m_num_used;
  view_1d<int> m_high_water;
  view_2d<bool> m_active;
  view_3d<char> m_curr_names;
  view_3d<char> m_all_names;
  view_3d<int> m_counts;
#endif
  view_1d<int> m_next_slot;
  view_2d<T> m_data;

// operator() needs to be public
public:
  KOKKOS_INLINE_FUNCTION
  void operator() (const MemberType& team) const;
}; // class WorkspaceManager

template <typename T, typename D>
constexpr double WorkspaceManager<T, D>::GPU_DEFAULT_OVERPROVISION_FACTOR;

} // namespace ekat

#include "ekat_workspace_impl.hpp"

#endif // EKAT_WSM_HPP
