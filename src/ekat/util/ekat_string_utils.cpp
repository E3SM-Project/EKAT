#include "ekat/util/ekat_string_utils.hpp"
#include "ekat/ekat_assert.hpp"

#include <algorithm>
#include <sstream>

namespace ekat {
namespace util {

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

std::string trim (const std::string& s) {
  if (s=="") {
    return s;
  }
  int nl = 0;
  while (s[nl] == ' ') {
    ++nl;
  }
  int size = s.size();
  int nt = 0;
  while (nt<size && s[size-nt-1] == ' ') {
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

} // namespace util
} // namespace ekat
