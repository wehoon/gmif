#ifndef GMIF_SRC_UTILS_H_
#define GMIF_SRC_UTILS_H_

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

#ifndef GMIF_DOUBLE_PRECISION
#define GMIF_DOUBLE_PRECISION (8)
#endif

namespace gmif {
namespace utils {

void StrLower(std::string& str);
void StrUpper(std::string& str);

void StrTrimLeft(std::string& str, const std::string& sub);
void StrTrimRight(std::string& str, const std::string& sub);
void StrTrim(std::string& str, const std::string& sub);

void StrTrimLeftSpace(std::string& str);
void StrTrimRightSpace(std::string& str);
void StrTrimSpace(std::string& str);

void StrSplit(const std::string& str, const std::string& sep, std::vector<std::string>& res);
void StrSplitKeepQuot(const std::string& str, char sep, std::vector<std::string>& res);

bool IsDoubleZero(double d);
bool DoubleEqual(double d1, double d2);

template <typename T>
std::string to_string(const T& a_value, const int n = GMIF_DOUBLE_PRECISION) {
  std::ostringstream ss;
  ss.precision(n);
  ss << std::fixed << a_value;
  // TODO: trailing zeros
  return ss.str();
}

double to_double(const std::string& s);
double to_double(const char* s);
int64_t to_int(const std::string& s);
int64_t to_int(const char* s);

}  // namespace utils
}  // namespace gmif

#if (_WIN32 || WIN64)
#define FILENAME_ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILENAME_ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LOG_ERROR \
  (std::cerr << "[ERROR]" << FILENAME_ << ":" << __LINE__ << "(" << __FUNCTION__ << "): ")

namespace std {

template <typename T>
ostream& operator<<(ostream& os, const std::vector<T>& v) {
  os << "[";
  for (size_t i = 0; i < v.size(); ++i) {
    if (i != 0)
      os << ", ";
    os << v.at(i);
  }
  os << "]";
  return os;
}

template <typename T1, typename T2>
ostream& operator<<(ostream& os, const std::map<T1, T2>& m) {
  os << "{";
  for (auto it = m.begin(); it != m.end(); ++it) {
    if (it != m.begin())
      os << ", ";
    os << it->first << ": " << it->second;
  }
  os << "}";
  return os;
}

template <typename T>
ostream& operator<<(ostream& os, const std::set<T>& s) {
  os << "{";
  for (auto it = s.begin(); it != s.end(); ++it) {
    if (it != s.begin())
      os << ", ";
    os << *it;
  }
  os << "}";
  return os;
}

}  // namespace std

#endif  // GMIF_SRC_UTILS_H_
