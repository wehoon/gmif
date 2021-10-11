#include "utils.h"
#include <cctype>
#include <cmath>
#include <sstream>

namespace gmif {
namespace utils {

static const double kEpsilon = pow(10.0, -GMIF_DOUBLE_PRECISION);
static const char kSpaceWhite[] = "\n\r\t\f\v ";
static const char kQuot = '"';

bool IsDoubleZero(double d) {
  return -kEpsilon < d && d < kEpsilon;
}

bool DoubleEqual(double d1, double d2) {
  return IsDoubleZero(d1 - d2);
}

void StrLower(std::string& str) {
  for (char& c : str) {
    if (isalpha(c) && isupper(c)) {
      c = static_cast<char>(std::tolower(c));
    }
  }
}

void StrUpper(std::string& str) {
  for (char& c : str) {
    if (isalpha(c) && islower(c)) {
      c = static_cast<char>(std::toupper(c));
    }
  }
}

void StrTrimLeft(std::string& str, const std::string& sub) {
  auto pos = str.find_first_not_of(sub);
  if (pos == std::string::npos) {
    str.clear();
  } else {
    str.erase(0, pos);
  }
}

void StrTrimRight(std::string& str, const std::string& sub) {
  auto pos = str.find_last_not_of(sub);
  if (pos == std::string::npos) {
    str.clear();
  } else {
    str.erase(pos + 1);
  }
}

void StrTrim(std::string& str, const std::string& sub) {
  StrTrimLeft(str, sub);
  StrTrimRight(str, sub);
}

void StrTrimLeftSpace(std::string& str) {
  StrTrimLeft(str, kSpaceWhite);
}

void StrTrimRightSpace(std::string& str) {
  StrTrimRight(str, kSpaceWhite);
}

void StrTrimSpace(std::string& str) {
  StrTrimLeftSpace(str);
  StrTrimRightSpace(str);
}

void StrSplit(const std::string& str, const std::string& sep, std::vector<std::string>& res) {
  res.clear();
  size_t start = 0;
  size_t end = str.find(sep);
  while (end != std::string::npos) {
    auto substr = str.substr(start, end - start);
    res.push_back(std::move(substr));
    start = end + sep.size();
    end = str.find(sep, start);
  }
  if (start != str.size()) {
    auto substr = str.substr(start);
    res.push_back(std::move(substr));
  } else {
    res.emplace_back("");
  }
}

void ParseQuot(const char*& s) {
  ++s;  // skip begin quotation
  while (*s != '\0') {
    if (*s == kQuot) {
      ++s;  // skip end quotation
      break;
    }
    if (*s == '\\') ++s;  // skip ESC
    ++s;
  }
}

void ParseNormal(const char*& s, char sep) {
  while (*s != '\0' && *s != sep) {
    if (*s == '\\')
      ++s;
    ++s;
  }
}

void StrSplitKeepQuot(const std::string& str, char sep, std::vector<std::string>& res) {
  res.clear();
  const char* start = str.c_str();
  const char* s = start;
  while (*s != '\0') {
    if (*s == sep) {
      res.emplace_back(start, s - start);
      ++s;
      start = s;
    } else {
      if (*s == kQuot) {
        ParseQuot(s);
      } else {
        ParseNormal(s, sep);
      }
    }
  }
  if (*start != '\0') {
    res.emplace_back(start, s - start);
  } else {
    res.emplace_back("");
  }
}

double to_double(const std::string& s) {
  return to_double(s.c_str());
}

double to_double(const char* s) {
  return strtod(s, nullptr);
}

int64_t to_int(const std::string& s) {
  return to_int(s.c_str());
}

int64_t to_int(const char* s) {
  return strtol(s, nullptr, 10);
}

}  // namespace utils
}  // namespace gmif
