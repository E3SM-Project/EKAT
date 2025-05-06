#ifndef EKAT_TEST_UTILS_HPP
#define EKAT_TEST_UTILS_HPP

#include <map>
#include <vector>
#include <string>

namespace ekat {

struct TestSession {
  static TestSession& get () {
    static TestSession s;
    return s;
  }

  void parse_test_args (const std::vector<char*>& args);

  // These three store test args, but in different ways
  //  - flags contains args of the form "-s" or "--p" (not followed by another string) as a map string->bool
  //  - params contains args of the form "-p value"/"--param value"
  //  - vec_params contains args of the form "-p val1 val2 val3"/"--param val1 val2 val3"
  // Notice that, as such, params entries will ALWAYS be in vec_params as well (with legnth 1)
  std::map<std::string,bool>                      flags;
  std::map<std::string,std::string>               params;
  std::map<std::string,std::vector<std::string>>  vec_params;
private:
  TestSession() = default;
};


template <typename RealType, typename rngAlg, typename PDF>
void genRandArray(RealType *const x, int length, rngAlg &engine, PDF &&pdf) {
  for (int i = 0; i < length; ++i) {
    x[i] = pdf(engine);
  }
}

// Check whether a string (from argv) matches a predefined option name,
// either in its short or long form. It is meant to check something like this:
//
//   bool is_num_problems = argv_matches(argv[1], "-np", "--num_problems");
//
// Additionally, we also check if the input matches the short option name with an extra '-',
// so in the above example we would also check "--np".
bool argv_matches(const std::string& str, const std::string& short_opt, const std::string& long_opt);

} // namespace ekat

#endif // EKAT_TEST_UTILS_HPP
