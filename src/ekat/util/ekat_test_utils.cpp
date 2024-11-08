#include "ekat/ekat.hpp"
#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/ekat_assert.hpp"
#include "ekat/util/ekat_string_utils.hpp"

#include <cstdlib>

namespace ekat {

void TestSession::
parse_test_args (const std::vector<char*>& args)
{
  // We want to recognize these (AND ONLY THESE) ways to provide args:
  //  1. "-b" or "--blah": toggles an option
  //  2. "-p=foo" or "--param=foo": sets a string->string mapping
  //  3. "-p foo" or "--param foo": like the above
  //  4. "-p foo bar" or "--param foo bar": sets a string->list(string) mapping
  if (args.size()==0) {
    return;
  }

  auto is_key = [](const std::string& s) {
    return ekat::starts_with(s,"--") or ekat::starts_with(s,"-");
  };
  auto get_key = [](const std::string& s) {
    if (ekat::starts_with(s,"--"))
      return s.substr(2);
    else
      return s.substr(1);
  };

  // Convert to vector of tokens, and do some pre-processing:
  //  - if one entry is A=B split it in two (B cannot be a key)
  //  - if one entry is A,B,C split in three
  // Each token is stored as a string plus a bool that says if it's a key,
  // which is a token starting with - or --

  struct Token {
    std::string s;
    bool        key = false;

    Token (const std::string& str) : s(str) {}
    Token (const std::string& str, bool k) : s(str), key(k) {}
  };
  std::vector<Token> argsTokens;
  for (auto arg : args) {
    auto tokens = ekat::split(arg,'=');
    EKAT_REQUIRE_MSG (tokens.size()<=2,
        "Error! Badly formatted arg: '" + std::string(arg) + "'\n"
        " Cannot contain two '=' chars in a single arg.\n");

    if (tokens.size()>1) {
      EKAT_REQUIRE_MSG (is_key(tokens[0]),
        "Error! Badly formatted arg: '" + std::string(arg) + "'\n"
        " When using a=b syntax, 'a' must be a key (i.e., start with - or --)\n");
      argsTokens.emplace_back(get_key(tokens[0]),true);

      // Parse the part after '=', which can contain commas
      tokens = ekat::split(tokens.back(),",");
      for (auto t : tokens) {
        EKAT_REQUIRE_MSG (not is_key(t),
            "Error! Badly formatted arg: '" + std::string(arg) + "'\n"
            " When using --key=val1,val2 format, val1/val2 cannot be keys (i.e., start with '-').\n");
        argsTokens.emplace_back(t);
      }
    } else {
      tokens = ekat::split(arg,',');
      if (tokens.size()>1) {
        for (auto t : tokens) {
          EKAT_REQUIRE_MSG (not is_key(t),
              "Error! Badly formatted arg: '" + std::string(arg) + "'\n"
              " When using val1,val2 format, val1/val2 cannot be keys (i.e., start with '-').\n");
          argsTokens.emplace_back(t);
        }
      } else {
        // A simple arg
        if (is_key(arg)) {
          argsTokens.emplace_back(get_key(arg),true);
        } else {
          argsTokens.emplace_back(arg);
        }
      }
    }
  }

  EKAT_REQUIRE_MSG (argsTokens[0].key,
      "Error! Badly formatted --args. The first string should be of the form -f/--flag.\n"
      " - first args: '" + std::string(args[0]) + "'\n");

  auto key = argsTokens[0];
  argsTokens.erase(argsTokens.begin());

  int last_key_args = 0;
  for (auto t : argsTokens) {
    if (t.key) {
      if (last_key_args==0) {
        // We found a new key right after a key. That means the prev key was an option
        flags[key.s] = true;
      }
      key = t;
      last_key_args = 0;
      continue;
    }

    vec_params[key.s].push_back(t.s);
    ++last_key_args;
  }

  // Handle case where we end with a flag
  if (last_key_args==0) {
    flags[key.s] = true;
  }

  // Now go through the vec_params entries. If any has length 1, we also expose it in "params"
  for (const auto& it : vec_params) {
    if (it.second.size()==1) {
      params[it.first] = it.second[0];
    }
  }
}

bool argv_matches(const std::string& s, const std::string& short_opt, const std::string& long_opt) {
  return (s == short_opt) || (s == long_opt) || s == ("-" + short_opt);
}

int get_test_device (const int mpi_rank)
{
  // Set to -1 by default, which leaves kokkos in full control
  int dev_id = -1;

#ifdef EKAT_ENABLE_GPU
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
