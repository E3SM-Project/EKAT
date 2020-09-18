#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/util/ekat_string_utils.hpp"

#include <Kokkos_Core.hpp>

#include <cstdlib>

namespace ekat {

bool argv_matches(const std::string& s, const std::string& short_opt, const std::string& long_opt) {
  return (s == short_opt) || (s == long_opt) || s == ("-" + short_opt);
}

int get_test_device (const int mpi_rank)
{
  // Set to -1 by default, which leaves kokkos in full control
  int dev_id = -1;

#ifdef KOKKOS_ENABLE_CUDA
  auto count_str = getenv("CTEST_RESOURCE_GROUP_COUNT");
  if (count_str!=nullptr) {
    // If CTest is setting the CTEST_RESOURCE_GROUP_COUNT variable,
    // it means that it is potentially scheduling multiple tests
    // at the same time, exploiting the resources avaialble on the node.
    // Note: this logic should only be enabled on gpu builds

    // Note: use std::stoi with a std::string rather than std::atoi on a c-string,
    //       since the latter fails silently, while the former throws an exception
    //       if the conversion fails (either invalid input or integer out of range).

    int res_group_count = std::stoi(std::string(count_str));

    // Pick a resource group based on mpi rank (round robin);
    int my_res_group = mpi_rank % res_group_count;

    // Read the resources in this group
    auto key = "CTEST_RESOURCE_GROUP_" + std::to_string(my_res_group);
    auto res_type = getenv(key.c_str());
    EKAT_REQUIRE_MSG (res_type!=nullptr,
                        "Error! Missing '" + key + "' env var. Something might be off with res group detection,\n"
                        "       or with the properties set for the test.\n"
                        "       CTEST_RESOURCE_COUNT: " + std::to_string(res_group_count) + "\n"
                        "       Res group id for this rank: " + std::to_string(my_res_group) + "\n");

    key += "_" + upper_case(std::string(res_type));
    EKAT_REQUIRE_MSG(getenv(key.c_str())!=nullptr,
                        "Error! Missing '" + key + "' env var. Something might be off with res group detection,\n"
                        "       or with the properties set for the test.\n"
                        "       CTEST_RESOURCE_COUNT: " + std::to_string(res_group_count) + "\n"
                        "       Res group id for this rank: " + std::to_string(my_res_group) + "\n"
                        "       Res group type for this rank: " + std::string(res_type) + "\n");

    auto res = std::string(getenv(key.c_str()));

    // res should look like 'id:N,slots:M'. If multiple resources are assigned to a group,
    // there would be multiple strings like that, separated by ';'. We don't support that,
    // so we check that there's no ';' in res.
    // Note: We should always have M=1, since ekat only asks for 1 slot per resource
    EKAT_REQUIRE_MSG(res.find(';')==std::string::npos, "Error! Multiple resources specified for group " + std::to_string(my_res_group) + "\n");

    auto id_N_slots_M = split(res,',');
    EKAT_REQUIRE_MSG(id_N_slots_M.size()==2, "Error! Something seems wrong with resource spec '" + res + "'\n");

    auto slots_M = split(id_N_slots_M[1],':');
    EKAT_REQUIRE_MSG(slots_M.size()==2, "Error! Something seems wrong with resource spec '" + res + "'\n");
    EKAT_REQUIRE_MSG(slots_M[0]=="slots", "Error! Something seems wrong with resource spec '" + res + "'\n");
    EKAT_REQUIRE_MSG(slots_M[1]=="1", "Error! Something seems wrong with resource spec '" + res + "'\n");

    auto id_N = split(id_N_slots_M[0],':');
    EKAT_REQUIRE_MSG(id_N.size()==2, "Error! Something seems wrong with resource spec '" + res + "'\n");
    EKAT_REQUIRE_MSG(id_N[0]=="id", "Error! Something seems wrong with resource spec '" + res + "'\n");

    dev_id = std::stoi(id_N[1]);
  }
#else
  (void) mpi_rank;
#endif

  return dev_id;
}

} // namespace ekat
