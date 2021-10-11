#include <gtest/gtest.h>
#include <functional>
#include "utils.h"

using namespace gmif::utils;

class UtilsTest : public ::testing::Test {};

void Test1ArgsFunc(const std::function<void(std::string&)>& func,
                   const std::string& raw_str,
                   const std::string& exp_str) {
  std::string s(raw_str);
  func(s);
  EXPECT_EQ(s, exp_str);
}

TEST_F(UtilsTest, TestStrLower) {
  Test1ArgsFunc(StrLower, "ABCD", "abcd");
  Test1ArgsFunc(StrLower, "abcAbc", "abcabc");
  Test1ArgsFunc(StrLower, "abc", "abc");
  Test1ArgsFunc(StrLower, "测试ABC", "测试abc");
  Test1ArgsFunc(StrLower, "测试Abc", "测试abc");
  Test1ArgsFunc(StrLower, "测试abc", "测试abc");
  Test1ArgsFunc(StrLower, "ABC测试abc", "abc测试abc");
  Test1ArgsFunc(StrLower, "123Abc", "123abc");
}

TEST_F(UtilsTest, TestStrUpper) {
  Test1ArgsFunc(StrUpper, "abc", "ABC");
  Test1ArgsFunc(StrUpper, "abC", "ABC");
  Test1ArgsFunc(StrUpper, "ABC", "ABC");
  Test1ArgsFunc(StrUpper, "测试abc", "测试ABC");
  Test1ArgsFunc(StrUpper, "测试abC", "测试ABC");
  Test1ArgsFunc(StrUpper, "测试ABC", "测试ABC");
  Test1ArgsFunc(StrUpper, "Abc测试ABC", "ABC测试ABC");
}

void TestSplit(const std::string& str,
               const std::string& sep,
               const std::vector<std::string>& exp_v) {
  std::vector<std::string> v;
  StrSplit(str, sep, v);
  EXPECT_EQ(v, exp_v);
}

TEST_F(UtilsTest, TestStrSplit) {
  TestSplit("abc,bcd", ",", {"abc", "bcd"});
  TestSplit("abc,bcd,", ",", {"abc", "bcd", ""});
  TestSplit(",abc,bcd", ",", {"", "abc", "bcd"});
  TestSplit(",abc,bcd,", ",", {"", "abc", "bcd", ""});
  TestSplit("abc,bcd", "#", {"abc,bcd"});
  TestSplit("abc#*bcd", "#*", {"abc", "bcd"});
  TestSplit("abc*bcd#cde", "#*", {"abc*bcd#cde"});
}

void TestSplitQuot(const std::string& str, char sep, const std::vector<std::string>& exp_v) {
  std::vector<std::string> v;
  StrSplitKeepQuot(str, sep, v);
  EXPECT_EQ(v, exp_v);
}

TEST_F(UtilsTest, TestStrKeepQuot) {
  TestSplitQuot("abc,bcd", ',', {"abc", "bcd"});
  TestSplitQuot(",abc,bcd", ',', {"", "abc", "bcd"});
  TestSplitQuot(",abc,bcd,", ',', {"", "abc", "bcd", ""});
  TestSplitQuot(",\"abc\",bcd", ',', {"", "\"abc\"", "bcd"});
  TestSplitQuot("\"a,bc\",bcd", ',', {"\"a,bc\"", "bcd"});
  TestSplitQuot(R"("a\"bc",bcd)", ',', {R"("a\"bc")", "bcd"});
  TestSplitQuot(R"("a\",bc",bcd)", ',', {R"("a\",bc")", "bcd"});
  TestSplitQuot(R"("","abc")", ',', {R"("")", R"("abc")"});
  TestSplitQuot(R"("","")", ',', {R"("")", R"("")"});
}