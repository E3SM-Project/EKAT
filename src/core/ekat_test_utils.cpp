#include "ekat_test_utils.hpp"
#include "ekat_assert.hpp"
#include "ekat_string_utils.hpp"

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

} // namespace ekat
