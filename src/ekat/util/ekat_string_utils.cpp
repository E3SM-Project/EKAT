#include "ekat/util/ekat_string_utils.hpp"
#include "ekat/ekat_assert.hpp"

#include <algorithm>
#include <set>
#include <sstream>
#include <list>

namespace ekat {

void strip (std::string& str, const char c) {
  auto new_end = std::remove(str.begin(),str.end(),c);
  str.erase(new_end,str.end());
}

std::vector<std::string> split(const std::string& str, const char del) {
  std::vector<std::string> blocks;

  auto start = 0;
  auto pos = str.find(del);
  while (pos!=std::string::npos) {
    blocks.push_back(str.substr(start,pos-start));
    start = pos + 1;
    pos = str.find(del,start);
  }

  // Don't forget to add the substring from the last occurrence of 'del' (if any) to the end of str
  blocks.push_back(str.substr(start));
  return blocks;
}

std::string trim (const std::string& s, const char c) {
  if (s=="") {
    return s;
  }
  int nl = 0;
  while (s[nl] == c) {
    ++nl;
  }
  int size = s.size();
  int nt = 0;
  while (nt<size && s[size-nt-1] == c) {
    ++nt;
  }

  return s.substr(nl,size-nl-nt);
}

std::string strint (const std::string& s, const int i) {
  std::stringstream ss;
  ss << s << " " << i;
  return ss.str();
}

std::string upper_case (const std::string& s) {
  std::string s_up = s;
  std::transform(s_up.begin(), s_up.end(), s_up.begin(),
                 [](unsigned char c)->char { return std::toupper(c); }
                );
  return s_up;
}

double jaro_similarity (const std::string& s1, const std::string& s2) {
  // Two equal strings always have similarity of 1, regardless of whether they are empty or not
  if (s1==s2) {
    return 1;
  }

  const int len1 = s1.size();
  const int len2 = s2.size();
  const int max_dist = std::max(len1,len2)/2 - 1;

  double matches = 0;
  std::vector<int> s1_matches(len1,0);
  std::vector<int> s2_matches(len2,0);
  for (int i=0; i<len1; ++i) {
    for (int j=std::max(0,i-max_dist); j<std::min(len2,i+max_dist+1); ++j) {
      if (s1[i]==s2[j] && s2_matches[j]==0) {
        matches += 1.0;
        s1_matches[i] = 1;
        s2_matches[j] = 1;
        break;
      }
    }
  }

  // If no matches, we're done
  if (matches==0) {
    return 0;
  }

  // There are matches. Count transpositions
  double transp = 0;
  for (int i=0, k=0; i<len1; ++i) {
    if (s1_matches[i]==1) {
      while (s2_matches[k]==0) {
        ++k;
      }
      if (s1[i]!=s2[k]) {
        transp += 0.5;
      }
      ++k;
    }
  }

  return ( matches/len1 + matches/len2 + (matches-transp)/matches ) / 3.0;
}

double jaro_winkler_similarity (const std::string& s1, const std::string& s2,
                                const int l, const double p, const double thresh) {
  EKAT_ASSERT_MSG (l>=0 && l<=4, "Error! Jaro-Winkler similarity requries 0<=L<=4.\n");
  EKAT_ASSERT_MSG (p>=0 && p<=1.0/l, "Error! Jaro-Winkler similarity requries 0<=p<=1/L.\n");
  EKAT_ASSERT_MSG (thresh>0 && thresh<1.0, "Error! Jaro-Winkler boosh thresholt requries 0<thresh<1.\n");

  double sim_j = jaro_similarity(s1,s2);;

  if (sim_j>thresh) {
    const int end = std::min(static_cast<int>(std::min(s1.size(),s2.size())),l);
    int common_prefix_len = 0;
    while (s1[common_prefix_len]==s2[common_prefix_len] && common_prefix_len<end) {
      ++common_prefix_len;
    }
    sim_j += common_prefix_len*p*(1-sim_j);
  }
  return sim_j;
}

// This is a helper function for token-based similarity indexes. It gathers
// tokens from the given string for the given list of delimiters.
std::list<std::string> gather_tokens(const std::string& s,
                                     const std::vector<char>& delimiters,
                                     const std::string& atomic) {
  std::list<std::string> all_tokens;
  std::string delim_str;
  for (char delim: delimiters) {
    delim_str.append(1, delim);
  }

  if (atomic!="") {
    auto pos = s.find(atomic);
    if (pos==std::string::npos) {
      // The atomic string wasn't found.
      return gather_tokens(s,delimiters,"");
    }
    // The atomic string was found. Add it as a token, then remove if from
    // the input string, and run again on what's left.
    // But first, we need to check that the string is delimited left and right
    // by a delimiter (or string boundaries), otherwise it doesn't count.
    auto slen = s.size();
    auto alen = atomic.size();

    // These lambda implement exactly what you think they do,
    // but are more verbose than the corresponding checks when used.
    auto at_s_start = [&] () -> bool {
      return pos==0;
    };
    auto at_s_end = [&] () -> bool {
      return (pos+alen)==slen;
    };
    auto delim_before = [&] () -> bool {
      return s.find_first_of(delim_str,pos-1)==(pos-1);
    };
    auto delim_after = [&] () -> bool {
      return s.find_first_of(delim_str,pos+alen)==(pos+alen);
    };

    if ( !at_s_start() && !delim_before() ) {
      // The atomic string is not at the start of s, but it is
      // not preceeded by a delimiter.
      // So fall back to the case where we don't search for atomic
      return gather_tokens(s,delimiters,"");
    } else if ( !at_s_end() && !delim_after() ) {
      // The atomic string is not at the end of s, but it is
      // not followed by a delimiter.
      // So fall back to the case where we don't search for atomic
      return gather_tokens(s,delimiters,"");
    }

    // If we didn't hit any of the previous return statements, then the atomic
    // substring was found, and it is a sub-token.
    all_tokens.push_back(atomic);

    std::string s_mod(s);
    auto erase_begin = at_s_start() ? 0 : pos-1;
    auto erase_len   = delim_after() ? alen+1 : alen;
    s_mod.erase(erase_begin,erase_len);

    // Note: splice is better than manual push back, since it only moves a couple of ptrs.
    all_tokens.splice(all_tokens.end(),gather_tokens(s_mod,delimiters,atomic));
    return all_tokens;
  }

  std::size_t prev = 0, pos;
  std::stringstream sstr(s);
  while ((pos = s.find_first_of(delim_str, prev)) != std::string::npos)
  {
    if (pos > prev) {
      all_tokens.push_back(s.substr(prev, pos-prev));
    }
    prev = pos+1;
  }
  if (prev < s.length()) {
    all_tokens.push_back(s.substr(prev, std::string::npos));
  }

  return all_tokens;
}

double jaccard_similarity (const std::string& s1, const std::string& s2,
                           const std::vector<char>& delimiters,
                           const bool tokenize_s1,
                           const bool tokenize_s2)
{
  // Nobody should call this case, but just in case: not tokenizing either one,
  // is equivalent to returning s1==s2
  if (!tokenize_s1 && !tokenize_s2) {
    return static_cast<double>(s1==s2);
  }
  // Break the first and second strings up into tokens using all given
  // delimiters.
  std::list<std::string> s1_tokens, s2_tokens;
  std::set<std::string> s1_set, s2_set;

  if (tokenize_s1) {
    if (tokenize_s2) {
      s2_tokens = gather_tokens(s2, delimiters);
      s1_tokens = gather_tokens(s1, delimiters);
    } else {
      // S2 is a single token
      s2_tokens.push_back(s2);
      s1_tokens = gather_tokens(s1, delimiters, s2);
    }
  } else {
    // We already took care of the case were we don't tokenize either one,
    // so we can be sure tokenize_s2 is true.
    // S2 is a single token
      if (s2=="zenith_angle") {
        printf("zenith_angle\n");
      }
    s1_tokens.push_back(s1);
    s2_tokens = gather_tokens(s2, delimiters, s1);
  }
  s1_set = std::set<std::string>(s1_tokens.begin(), s1_tokens.end());
  s2_set = std::set<std::string>(s2_tokens.begin(), s2_tokens.end());

  // Compute the intersection of the two sets of tokens.
  std::vector<std::string> s_intersection(std::max(s1_set.size(), s2_set.size()));
  auto iter = std::set_intersection(s1_set.begin(), s1_set.end(),
                                    s2_set.begin(), s2_set.end(),
                                    s_intersection.begin());
  s_intersection.resize(iter - s_intersection.begin());

  // The Jaccard index is the ratio of the size of the intersection to that
  // of the union.
  double int_size = static_cast<double>(s_intersection.size());
  double un_size = static_cast<double>(s1_tokens.size() + s2_tokens.size() - int_size);
  return int_size / un_size;
}

// ===================== Case Insensitive String ================== //

bool caseInsensitiveEqualString (const std::string& s1, const std::string& s2) {
  auto charComp = [](const char c1, const char c2)->bool{
    return c1==c2 || std::toupper(c1)==std::toupper(c2);
  };
  return s1.size()==s2.size() &&
         std::equal(s1.begin(),s1.end(),s2.begin(),charComp);
}

bool caseInsensitiveLessString (const std::string& s1, const std::string& s2) {
  auto charCompLess = [](const char c1, const char c2)->bool{
    return std::toupper(c1)<std::toupper(c2);
  };
  auto charCompEq = [](const char c1, const char c2)->bool{
    return std::toupper(c1)==std::toupper(c2);
  };
  for (auto it1=s1.begin(),it2=s2.begin(); it1!=s1.end() && it2!=s2.end(); ++it1,++it2) {
    if (charCompLess(*it1,*it2)) {
      return true;
    } else if (!charCompEq(*it1,*it2)) {
      return false;
    }
  }
  return s1.size()<s2.size();
}

bool caseInsensitiveLessEqualString (const std::string& s1, const std::string& s2) {
  auto charCompLess = [](const char c1, const char c2)->bool{
    return std::toupper(c1)<std::toupper(c2);
  };
  auto charCompEq = [](const char c1, const char c2)->bool{
    return std::toupper(c1)==std::toupper(c2);
  };
  for (auto it1=s1.begin(),it2=s2.begin(); it1!=s1.end() && it2!=s2.end(); ++it1,++it2) {
    if (charCompLess(*it1,*it2)) {
      return true;
    } else if (!charCompEq(*it1,*it2)) {
      return false;
    }
  }
  return s1.size()<=s2.size();
}

} // namespace ekat
