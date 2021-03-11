#ifndef EKAT_STRING_UTILS_HPP
#define EKAT_STRING_UTILS_HPP

#include <string>
#include <vector>
#include <list>

/*
 * A set of utilities for string manipulation
 *
 * This header (and its corresponding cpp file) contain two sets of things:
 *  - A class for case insensitive string. This class inherits from std::string,
 *    so *all* the functionalities of std::string are available. Additionally,
 *    when comparing two strings (with ==, !=, <, or <=), if one of the two
 *    operands is a CaseInsensitiveString object, then we perform a case
 *    insensitive comparison.
 *    Use this class if you want to allow all possible case styles for some inputs.
 *  - Some utility functions to manipulate std::string objects, such as
 *    removing leading/trailing whitespaces, split a string into substrings
 *    at every occurrence of a given char, and more.
 */

namespace ekat {

// Strip character c from input string
void strip (std::string& str, const char c);

// Split a string at every occurrence of given delimiter char
std::vector<std::string> split(const std::string& str, const char del);

// Trim leading/trailing characters matching given one (default: whitespace).
std::string trim (const std::string& s, const char c = ' ');

// Small utility that cats a space and an integer to an input string.
std::string strint (const std::string& s, const int i);

// Conver the string to all upper case
std::string upper_case (const std::string& s);

// Split a string into tokens, according to any of the provided delimiters.
// If atomic!="", substrings matching atomic will not be split with
// any of the delimiters.
std::list<std::string> gather_tokens(const std::string& s,
                                     const std::vector<char>& delimiters,
                                     const std::string& atomic = "");

// Computing similarity index between s1 and s2 using Jaro algorithm
// For a quick description of the Jaro similarity index, see, e.g.,
//   https://en.wikipedia.org/wiki/Jaro-Winkler_distance#Jaro_Similarity
// This utility can be used to provide feedback to the user, when
// a wrong string keyword is used, but the code has a pool of valid
// strings to check the input against, and provide potential matches,
// like with "string 'balh' not found; did you mean 'blah'?"
double jaro_similarity (const std::string& s1, const std::string& s2);

// Computing similarity index between s1 and s2 using Jaro-Winkler algorithm,
// which is an "adjusted" version of the Jaro one.
// For meaning and bounds for the optional arguments l, p, see e.g.
//      https://en.wikipedia.org/wiki/Jaro-Winkler_distance
// This routine computes jaro similarity (sj), and if sj>thresholds,
// it performs the winkler adjustment, otherwise returns sj.
double jaro_winkler_similarity (const std::string& s1, const std::string& s2,
                                const int l = 4,
                                const double p = 0.1,
                                const double threshold = 0.7);

// Computes the similarity index between s1 and s2 using the Jaccard algorithm
// (https://en.wikipedia.org/wiki/Jaccard_index). Unlike the Jaro and
// Jaro-Winkler indices, the Jaccard index is token-based, and useful for
// identifying similarity between phrases in which the same words appear in
// different orders. The string s1 and s2 are broken up into tokens using
// the given deliminator characters. The optional arguments split_s1 and
// split_s2 are to allow searching for an exact string. E.g., if one
// wants to find only strings containing s1="air_pressure", but allow
// s2 to be split with '_' characters, he/she can pass split_s1=false.
double jaccard_similarity (const std::string& s1, const std::string& s2,
                           const std::vector<char>& delimiters,
                           const bool tokenize_s1 = true,
                           const bool tokenize_s2 = true);

// ==================== Case Insensitive string =================== //

// A no-overhead class that inherits from std::string, which we only
// use to trigger different behavior in the ==,!=,<,<= operators.
class CaseInsensitiveString final : public std::string
{
public:
  template<typename... Args>
  CaseInsensitiveString (Args... args)
   : std::string(args...)
  {}

  virtual ~CaseInsensitiveString () = default;
};

// Case-insensitive comparison functions
bool caseInsensitiveEqualString (const std::string& s1, const std::string& s2);
bool caseInsensitiveLessString (const std::string& s1, const std::string& s2);
bool caseInsensitiveLessEqualString (const std::string& s1, const std::string& s2);

// Overloads of comparison operators, which use the routines above if at least one
// of the two inputs is indeed a CaseInsensitiveString
template<typename S1, typename S2>
typename std::enable_if<
      std::is_same<S1,CaseInsensitiveString>::value ||
      std::is_same<S2,CaseInsensitiveString>::value,
      bool>::type
operator== (const S1& s1, const S2& s2) {
  return caseInsensitiveEqualString(s1,s2);
}

template<typename S1, typename S2>
typename std::enable_if<
      std::is_same<S1,CaseInsensitiveString>::value ||
      std::is_same<S2,CaseInsensitiveString>::value,
      bool>::type
operator!= (const S1& s1, const S2& s2) {
  return ! (s1==s2);
}

template<typename S1, typename S2>
typename std::enable_if<
      std::is_same<S1,CaseInsensitiveString>::value ||
      std::is_same<S2,CaseInsensitiveString>::value,
      bool>::type
operator< (const S1& s1, const S2& s2) {
  return caseInsensitiveLessString(s1,s2);
}

template<typename S1, typename S2>
typename std::enable_if<
      std::is_same<S1,CaseInsensitiveString>::value ||
      std::is_same<S2,CaseInsensitiveString>::value,
      bool>::type
operator<= (const S1& s1, const S2& s2) {
  return caseInsensitiveLessEqualString(s1,s2);
}

} // namespace ekat

#endif // EKAT_STRING_UTILS_HPP
