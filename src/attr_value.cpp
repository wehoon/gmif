#include "gmif/gmif.h"
#include "utils.h"

namespace gmif {

AttrValue::AttrValue() : num_val_(0), init_flag_(0) {}

AttrValue::AttrValue(const std::string& v) : str_val_(v), num_val_(0), init_flag_(1) {}

AttrValue::AttrValue(const char* v) : str_val_(v), num_val_(0), init_flag_(1) {}

AttrValue::AttrValue(int32_t v) : num_val_(v), init_flag_(2) {}

AttrValue::AttrValue(double v) : num_val_(v), init_flag_(2) {}

AttrValue::AttrValue(AttrValue&& rhs) noexcept : num_val_(rhs.num_val_), init_flag_(rhs.init_flag_) {
  str_val_ = std::move(rhs.str_val_);
}

AttrValue& AttrValue::operator=(AttrValue&& rhs) noexcept {
  if (this != &rhs) {
    str_val_ = std::move(rhs.str_val_);
    num_val_ = rhs.num_val_;
    init_flag_ = rhs.init_flag_;
  }
  return *this;
}

AttrValue& AttrValue::operator=(const std::string& v) {
  str_val_.assign(v);
  init_flag_ = 1;
  return *this;
}

AttrValue& AttrValue::operator=(const char* v) {
  str_val_.assign(v);
  init_flag_ = 1;
  return *this;
}

AttrValue& AttrValue::operator=(int32_t v) {
  num_val_ = v;
  init_flag_ = 2;
  return *this;
}

AttrValue& AttrValue::operator=(double v) {
  num_val_ = v;
  init_flag_ = 2;
  return *this;
}

bool AttrValue::operator==(const AttrValue& rhs) const {
  if (init_flag_.test(0) && rhs.init_flag_.test(0)) {
    return str_val_ == rhs.str_val_;
  }
  if (init_flag_.test(1) && rhs.init_flag_.test(1)) {
    return utils::DoubleEqual(num_val_, rhs.num_val_);
  }
  return true;  // non initialization
}

bool AttrValue::operator<(const AttrValue& rhs) const {
  if (init_flag_.test(0) && rhs.init_flag_.test(0)) {
    return str_val_ < rhs.str_val_;
  }
  if (init_flag_.test(1) && rhs.init_flag_.test(1)) {
    return num_val_ < rhs.num_val_;
  }
  return false;
}

bool AttrValue::operator>(const AttrValue& rhs) const {
  if (init_flag_.test(0) && rhs.init_flag_.test(0)) {
    return str_val_ > rhs.str_val_;
  }
  if (init_flag_.test(1) && rhs.init_flag_.test(1)) {
    return num_val_ > rhs.num_val_;
  }
  return false;
}

const std::string& AttrValue::getStr() {
  if (!init_flag_.test(0) && init_flag_.test(1)) {
    str_val_ = utils::to_string(num_val_);
    init_flag_.set(0, true);
  }
  return str_val_;
}

int32_t AttrValue::getInt() {
  return static_cast<int32_t>(getDouble());
}

double AttrValue::getDouble() {
  if (!init_flag_.test(1) && init_flag_.test(0)) {
    num_val_ = strtod(str_val_.c_str(), nullptr);
    init_flag_.set(1, true);
  }
  return num_val_;
}

bool AttrValue::ValueEqual(AttrValue& rhs) {
  if (*this == rhs) return true;
  return getStr() == rhs.getStr();
}

}  // namespace gmif