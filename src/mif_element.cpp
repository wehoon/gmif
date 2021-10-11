#include "gmif/gmif.h"

namespace gmif {

bool MifElement::hasColumn(const std::string& col_lower_name) noexcept {
  return attrs_map_.count(col_lower_name) > 0;
}

bool MifElement::getAttr(const std::string& col_lower_name, AttrValue& res_val) noexcept {
  if (!hasColumn(col_lower_name)) {
    return false;
  }
  res_val = attrs_map_.at(col_lower_name);
  return true;
}

AttrValue& MifElement::getAttr(const std::string& col_lower_name) {
  return attrs_map_.at(col_lower_name);
}

void MifElement::addOrUpdateAttr(const std::string& col_lower_name, const AttrValue& val) noexcept {
  attrs_map_[col_lower_name] = val;
}

}  // namespace gmif