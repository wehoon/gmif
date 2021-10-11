#include <cassert>
#include "gmif/gmif.h"
#include "utils.h"

namespace gmif {

const std::string MifHeader::kCharSetNeutral = "Neutral";

const std::string MifHeader::kCharSetChinese = "WindowsSimChinese";

const std::string MifHeader::kCoordSysLL = "CoordSys Earth Projection 1, 0";

const std::string MifHeader::kCoordSysMC =
    "CoordSys NonEarth Units \"m\" "
    "Bounds (-40075452.7386, -19928981.8896) (40075452.7386, 19928981.8896)";

int32_t MifHeader::getColumnIndex(const std::string& lower_col_name) const noexcept {
  if (col_index_.count(lower_col_name) > 0) {
    return col_index_.at(lower_col_name);
  }
  return -1;
}

const std::string& MifHeader::getColumnName(size_t index) const {
  return col_name_vec_.at(index);
}

const std::string& MifHeader::getColumnType(size_t index) const {
  return col_type_vec_.at(index);
}

bool MifHeader::addColumn(const std::string& col_name, const std::string& col_type) noexcept {
  std::string col_lower_name(col_name);
  utils::StrLower(col_lower_name);

  if (hasColumn(col_lower_name)) {
    return false;
  }

  col_name_vec_.push_back(col_name);
  col_type_vec_.push_back(col_type);
  col_index_.insert(std::make_pair(col_lower_name, col_name_vec_.size() - 1));
  return true;
}

bool MifHeader::deleteColumnByName(const std::string& col_name) noexcept {
  std::string col_lower_name(col_name);
  utils::StrLower(col_lower_name);

  if (!hasColumn(col_lower_name)) {
    return false;
  }

  int32_t index = getColumnIndex(col_lower_name);
  col_name_vec_.erase(col_name_vec_.begin() + index);
  col_type_vec_.erase(col_type_vec_.begin() + index);
  col_index_.erase(col_lower_name);
  return true;
}

bool MifHeader::deleteColumnByIndex(size_t index) noexcept {
  if (index >= col_name_vec_.size()) {
    return false;
  }

  std::string col_lower_name(col_name_vec_.at(index));
  utils::StrLower(col_lower_name);
  col_name_vec_.erase(col_name_vec_.begin() + index);
  col_type_vec_.erase(col_type_vec_.begin() + index);
  col_index_.erase(col_lower_name);
  return true;
}

bool MifHeader::hasColumn(const std::string& col_lower_name) const noexcept {
  return col_index_.count(col_lower_name) > 0;
}

}  // namespace gmif
