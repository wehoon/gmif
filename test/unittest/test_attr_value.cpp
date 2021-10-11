#include <gtest/gtest.h>
#include "gmif/gmif.h"
#include "utils.h"

using namespace gmif;

class AttrValueTest : public ::testing::Test {};

void TestAttrValue(AttrValue v, const char* exp_str, int32_t exp_int, double exp_dbl) {
  EXPECT_STREQ(v.getStr().c_str(), exp_str);
  EXPECT_EQ(v.getInt(), exp_int);
  EXPECT_TRUE(utils::DoubleEqual(v.getDouble(), exp_dbl));
}

TEST_F(AttrValueTest, TestValue) {
  TestAttrValue(AttrValue(), "", 0, 0.0);
  TestAttrValue(AttrValue("abc"), "abc", 0, 0.0);
  TestAttrValue(AttrValue("123"), "123", 123, 123.0);
  TestAttrValue(AttrValue(123), "123.00000000", 123, 123.0);
  TestAttrValue(AttrValue(123.123), "123.12300000", 123, 123.123);
  TestAttrValue(AttrValue(123.123456789), "123.12345679", 123, 123.12345679);
}

TEST_F(AttrValueTest, TestEqual) {
  EXPECT_EQ(AttrValue("abc"), AttrValue("abc"));
  EXPECT_EQ(AttrValue(123), AttrValue(123));
  EXPECT_EQ(AttrValue(123.123), AttrValue(123.123));
  EXPECT_EQ(AttrValue("123"), AttrValue(123));

  AttrValue v2(123);
  EXPECT_TRUE(AttrValue("123").ValueEqual(v2));

  v2 = 123.123456;
  EXPECT_TRUE(AttrValue("123.123456").ValueEqual(v2));
}

TEST_F(AttrValueTest, TestLess) {
  EXPECT_TRUE(AttrValue(123) < AttrValue(124));
  EXPECT_TRUE(AttrValue("123") < AttrValue("124"));
  EXPECT_TRUE(AttrValue("abc") < AttrValue("abd"));
  EXPECT_TRUE(AttrValue(123.123) < AttrValue(123.124));
  EXPECT_TRUE(AttrValue(-123.123) < AttrValue(-123.122));
}

TEST_F(AttrValueTest, TestGreater) {
  EXPECT_TRUE(AttrValue(123) > AttrValue(122));
  EXPECT_TRUE(AttrValue("123") > AttrValue("122"));
  EXPECT_TRUE(AttrValue("abc") > AttrValue("abb"));
  EXPECT_TRUE(AttrValue(123.123) > AttrValue(123.122));
  EXPECT_TRUE(AttrValue(-123.123) > AttrValue(-123.124));
}