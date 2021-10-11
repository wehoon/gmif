#include "io.h"
#include <geos/algorithm/Orientation.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <string>
#include <vector>
#include "check.h"
#include "utils.h"

using namespace geos::geom;
using namespace geos::algorithm;

#define CHECK_ITEMS_SIZE(items, exp_size)                                                     \
  {                                                                                           \
    if (items.size() != (exp_size)) {                                                         \
      LOG_ERROR << "Parse items failed: " << items << " size != " << (exp_size) << std::endl; \
      return -1;                                                                              \
    }                                                                                         \
  }

namespace gmif {
namespace io {

enum class ColType { kStr, kDouble, kInt };

ColType GetColType(const std::string& lower_col_type_str) {
  if (lower_col_type_str.compare(0, 7, "integer") == 0 ||
      lower_col_type_str.compare(0, 8, "smallint") == 0) {
    return ColType::kInt;
  } else if (lower_col_type_str.compare(0, 7, "decimal") == 0 ||
             lower_col_type_str.compare(0, 5, "float") == 0) {
    return ColType::kDouble;
  }
  return ColType::kStr;
};

template <typename T>
void out_vec(std::ofstream& of, const std::vector<T>& v) {
  for (size_t i = 0; i < v.size(); ++i) {
    if (i != 0)
      of << ",";
    of << v[i];
  }
}

bool TryOpenFile(const std::string& base_name,
                 const std::vector<std::string>& ext_names,
                 std::ifstream& ifs) {
  std::string fpath;
  for (const auto& ext : ext_names) {
    fpath = base_name + "." + ext;
    ifs.open(fpath.c_str());
    if (ifs.fail()) {
      ifs.close();
      continue;
    }
    return true;
  }

  std::cerr << "can`t open file \"" << base_name << ".[";
  for (auto e = ext_names.begin(); e != ext_names.end(); ++e) {
    if (e != ext_names.begin())
      std::cerr << "|";
    std::cerr << *e;
  }
  std::cerr << "]\"" << std::endl;
  return false;
}

int ReadHeader(std::ifstream& mif_ifs, MifHeader& header) {
  std::string line;
  std::vector<std::string> items;
  int col_num(-1);
  while (mif_ifs.good()) {
    getline(mif_ifs, line);
    utils::StrTrimRightSpace(line);
    if (line.empty())
      continue;
    if (col_num < 0) {
      utils::StrSplit(line, " ", items);
      utils::StrLower(items[0]);

      if (items[0] == "version") {
        CHECK_ITEMS_SIZE(items, 2);
        header.setVersion(atoi(items[1].c_str()));
      }
      if (items[0] == "charset") {
        CHECK_ITEMS_SIZE(items, 2);
        utils::StrTrim(items[1], "\"");
        header.setCharset(items[1]);
      }
      if (items[0] == "delimiter") {
        CHECK_ITEMS_SIZE(items, 2);
        if (items[1].size() != 3) {  // ex. ","
          LOG_ERROR << "ParseMifHeader failed: delimiter char format error" << std::endl;
          return false;
        }
        header.setDelimiter(items[1][1]);
      }
      if (items[0] == "unique") {
        for (size_t i = 1; i < items.size(); ++i) {
          utils::StrTrim(items[i], ",");
          header.getUniqueVec().push_back(atoi(items[i].c_str()));
        }
      }
      if (items[0] == "index") {
        for (size_t i = 1; i < items.size(); ++i) {
          utils::StrTrim(items[i], ",");
          header.getIndexVec().push_back(atoi(items[i].c_str()));
        }
      }
      if (items[0] == "coordsys") {
        header.setCoordsys(line);
      }
      if (items[0] == "projection") {
        header.getCoordsys() += " " + line;
      }
      if (items[0] == "transform") {
        header.setTransform(line);
      }
      if (items[0] == "columns") {
        CHECK_ITEMS_SIZE(items, 2);
        col_num = atoi(items[1].c_str());
      }
    } else if (col_num > 0) {
      utils::StrTrimLeftSpace(line);
      utils::StrLower(line);
      utils::StrSplit(line, " ", items);
      CHECK_ITEMS_SIZE(items, 2);
      header.addColumn(items[0], items[1]);
      --col_num;
    } else {
      utils::StrLower(line);
      if (line == "data" || line == "none") {
        break;
      } else {
        std::string msg("can`t support parse mif header: ");
        msg.append(line);
        throw std::runtime_error(msg);
      }
    }
  }
  if (col_num != 0) {
    LOG_ERROR << "ParseMifHeader failed: column num not match [column:"
              << col_num + header.getColumnSize() << ", real:" << header.getColumnSize() << "]"
              << std::endl;
    return 1;
  }

  return 0;
}

/**
 * 读取单行属性
 * @param mid_ifs
 * @param header
 * @param res
 * @return 成功返回0, 失败返回-1, 文件结束返回1
 */
int ReadSingleAttr(std::ifstream& mid_ifs, const MifHeader& header, AttrMap& res) {
  std::string line;
  while (line.empty() && mid_ifs.good()) {
    getline(mid_ifs, line);
    utils::StrTrimSpace(line);
  }
  if (line.empty() && mid_ifs.eof()) {
    return 1;
  }

  std::vector<std::string> items;
  utils::StrSplitKeepQuot(line, header.getDelimiter(), items);
  if (header.getColumnSize() != items.size()) {
    LOG_ERROR << "mif header column-num(" << header.getColumnSize() << ") != mid items-size("
              << items.size() << "), items:" << items << std::endl;
    return -1;
  }
  res.clear();
  for (size_t i = 0; i < items.size(); ++i) {
    const std::string& col_name = header.getColumnName(i);
    ColType col_type = GetColType(header.getColumnType(i));
    utils::StrTrim(items[i], "\"");
    if (col_type == ColType::kInt) {
      AttrValue val = items[i].empty() ? 0 : atoi(items[i].c_str());
      res.insert(std::make_pair(col_name, std::move(val)));
    } else if (col_type == ColType::kDouble) {
      AttrValue val = items[i].empty() ? 0.0 : strtod(items[i].c_str(), nullptr);
      res.insert(std::make_pair(col_name, std::move(val)));
    } else {
      res.insert(std::make_pair(col_name, items[i]));
    }
  }
  return 0;
}

//! 判断样式关键字
bool IsStyleKeyWord(const std::string& lower_word) {
  if (lower_word.compare(0, 3, "pen") == 0)
    return true;
  if (lower_word.compare(0, 5, "brush") == 0)
    return true;
  if (lower_word.compare(0, 6, "symbol") == 0)
    return true;
  if (lower_word.compare(0, 4, "font") == 0)
    return true;
  if (lower_word.compare(0, 6, "center") == 0)
    return true;
  return false;
}

std::unique_ptr<CoordinateArraySequence> ReadCoordSeq(std::ifstream& mif_ifs, int num_pts = -1) {
  if (num_pts < 0) {
    mif_ifs >> num_pts;
  }
  if (num_pts < 0) {
    LOG_ERROR << "read coordinate sequence failed, illegal num_pts: " << num_pts << std::endl;
    return nullptr;
  }
  double x(0), y(0);
  auto coords = std::unique_ptr<CoordinateArraySequence>(new CoordinateArraySequence(num_pts));
  for (int i = 0; i < num_pts; ++i) {
    mif_ifs >> x >> y;
    coords->setAt(Coordinate(x, y), i);
  }
  return coords;
}

std::unique_ptr<LineString> ReadLineString(const GeometryFactory::Ptr& geos_factory,
                                           std::ifstream& mif_ifs,
                                           int num_pts = -1) {
  auto coords = ReadCoordSeq(mif_ifs, num_pts);
  if (coords == nullptr) {
    return nullptr;
  }
  if (coords->getSize() < 2) {
    LOG_ERROR << "read LineString coordinate size illegal: " << coords->getSize() << std::endl;
    return nullptr;
  }
  return geos_factory->createLineString(std::move(coords));
}

std::unique_ptr<Polygon> ReadPolygon(const GeometryFactory::Ptr& geos_factory,
                                     std::ifstream& mif_ifs,
                                     int num_pts = -1) {
  auto coords = ReadCoordSeq(mif_ifs, num_pts);
  if (coords == nullptr) {
    return nullptr;
  }
  if (coords->getSize() < 3) {
    LOG_ERROR << "read Polygon coordinate size illegal: " << coords->getSize() << std::endl;
    return nullptr;
  }

  // 修正闭环
  if (coords->front() != coords->back()) {
    coords->add(coords->front());
  }

  // 修正逆时针方向
  if (Orientation::isCCW(coords.get())) {
    CoordinateSequence::reverse(coords.get());
  }

  auto ring = geos_factory->createLinearRing(std::move(coords));
  return geos_factory->createPolygon(std::move(ring));
}

/**
 * 读取单个几何对象
 * @param geos_factory GEOS工厂对象
 * @param mif_ifs MIF输入流
 * @param res 返回的几何对象指针
 * @return 成功返回0, 失败返回-1, 文件结束返回1
 */
int ReadSingleGeo(const GeometryFactory::Ptr& geos_factory,
                  std::ifstream& mif_ifs,
                  GeometryPtr& res) {
  // https://baike.baidu.com/item/MIF/1416600

  std::string line;
  std::vector<std::string> items;

  while (mif_ifs.good() && !mif_ifs.eof()) {
    getline(mif_ifs, line);
    utils::StrTrimSpace(line);
    if (line.empty()) {
      continue;
    }
    utils::StrSplit(line, " ", items);
    utils::StrLower(items[0]);

    if (items[0] == "none") {
      res = nullptr;
      return 0;
    } else if (items[0] == "point") {
      CHECK_ITEMS_SIZE(items, 3);
      Coordinate p(utils::to_double(items[1]), utils::to_double(items[2]));
      res = GeometryPtr(geos_factory->createPoint(p));
      return 0;
    } else if (items[0] == "line") {
      CHECK_ITEMS_SIZE(items, 5);
      auto coords = std::unique_ptr<CoordinateArraySequence>(new CoordinateArraySequence(2));
      coords->setAt(Coordinate(utils::to_double(items[1]), utils::to_double(items[2])), 0);
      coords->setAt(Coordinate(utils::to_double(items[3]), utils::to_double(items[4])), 1);
      res = geos_factory->createLineString(std::move(coords));
      return 0;
    } else if (items[0] == "pline") {
      if (items.size() == 1) {
        res = ReadLineString(geos_factory, mif_ifs);
        return (res == nullptr) ? -1 : 0;
      } else if (items.size() == 2) {  // PLINE <coords_num>
        res = ReadLineString(geos_factory, mif_ifs, utils::to_int(items[1]));
        return (res == nullptr) ? -1 : 0;
      } else if (items.size() == 3) {  // PLINE MULTIPLE <geo_num>
        int geo_num = utils::to_int(items[2]);
        auto lines = std::vector<std::unique_ptr<Geometry>>(geo_num);
        for (int i = 0; i < geo_num; ++i) {
          lines[i] = ReadLineString(geos_factory, mif_ifs);
          if (lines[i] == nullptr) {
            return -1;
          }
        }
        res = geos_factory->createMultiLineString(std::move(lines));
        return 0;
      } else {
        LOG_ERROR << "PLINE format illegal: " << items << std::endl;
        return -1;
      }
    } else if (items[0] == "region") {
      CHECK_ITEMS_SIZE(items, 2);
      int geo_num = utils::to_int(items[1]);
      if (geo_num == 1) {
        res = ReadPolygon(geos_factory, mif_ifs);
        return (res == nullptr) ? -1 : 0;
      } else if (geo_num > 1) {
        auto regions = std::vector<std::unique_ptr<Geometry>>(geo_num);
        for (int i = 0; i < geo_num; ++i) {
          regions[i] = ReadPolygon(geos_factory, mif_ifs);
          if (regions[i] == nullptr) {
            return -1;
          }
        }
        res = geos_factory->createMultiPolygon(std::move(regions));
        return 0;
      } else {
        LOG_ERROR << "REGION format illegal: " << items << std::endl;
        return -1;
      }
    } else if (items[0] == "rect") {
      CHECK_ITEMS_SIZE(items, 5);
      double x1(utils::to_double(items[1]));
      double y1(utils::to_double(items[2]));
      double x2(utils::to_double(items[3]));
      double y2(utils::to_double(items[4]));
      auto coords = std::unique_ptr<CoordinateArraySequence>(new CoordinateArraySequence(5));
      coords->setAt(Coordinate(x1, y1), 0);
      coords->setAt(Coordinate(x2, y1), 1);
      coords->setAt(Coordinate(x2, y2), 2);
      coords->setAt(Coordinate(x1, y2), 3);
      coords->setAt(Coordinate(x1, y1), 4);
      auto ring = geos_factory->createLinearRing(std::move(coords));
      res = geos_factory->createPolygon(std::move(ring));
      return (res == nullptr) ? -1 : 0;
    } else if (IsStyleKeyWord(items[0])) {
      // ignore and skip style line
    } else {
      LOG_ERROR << "can`t support mif keyword: '" << items[0] << "'" << std::endl;
      return -1;
    }
  }
  return 1;
}

int ReadSingleElement(const GeometryFactory::Ptr& geos_factory,
                      std::ifstream& mif_ifs,
                      std::ifstream& mid_ifs,
                      const MifHeader& header,
                      bool mid_only,
                      MifElement& elem) {
  AttrMap attr_map;
  int status = ReadSingleAttr(mid_ifs, header, attr_map);
  if (status == 0) {
    elem.setAttrsMap(std::move(attr_map));
  } else {
    return status;
  }

  if (mid_only) {  // 仅读取MID属性, 几何对象置空
    elem.setGeo(nullptr);
    return 0;
  }

  GeometryPtr geo;
  status = ReadSingleGeo(geos_factory, mif_ifs, geo);
  if (status == 0) {
    elem.setGeo(geo);
  } else {
    return -1;  // 属性读取成功但几何读取失败, 整体失败
  }

  return 0;
}

int WriteHeader(std::ofstream& mif_ofs, const MifHeader& header) {
  if (!check::CheckMifHeaderValid(header)) {
    return -1;
  }

  mif_ofs << "Version " << header.getVersion() << "\n";
  mif_ofs << "Charset \"" << header.getCharset() << "\"\n";
  mif_ofs << "Delimiter \"" << header.getDelimiter() << "\"\n";
  if (!header.getUniqueVec().empty()) {
    mif_ofs << "Unique ";
    out_vec(mif_ofs, header.getUniqueVec());
    mif_ofs << "\n";
  }
  if (!header.getIndexVec().empty()) {
    mif_ofs << "Index ";
    out_vec(mif_ofs, header.getIndexVec());
    mif_ofs << "\n";
  }
  mif_ofs << header.getCoordsys() << "\n";
  if (!header.getTransform().empty()) {
    mif_ofs << header.getTransform() << "\n";
  }
  mif_ofs << "Columns " << header.getColumnSize() << "\n";
  for (size_t i = 0; i < header.getColumnSize(); ++i) {
    mif_ofs << "    " << header.getColumnName(i) << " " << header.getColumnType(i) << "\n";
  }
  mif_ofs << "Data\n";

  return 0;
}

bool WriteElementAttr(std::ofstream& mid_ofs, const MifHeader& header, MifElement& elem) {
  char delimiter = header.getDelimiter();
  size_t col_size = header.getColumnSize();
  for (size_t i = 0; i < col_size; ++i) {
    if (i != 0) {
      mid_ofs << delimiter;
    }

    AttrValue val;
    bool has_col = elem.getAttr(header.getColumnName(i), val);

    ColType col_type = GetColType(header.getColumnType(i));
    if (col_type == ColType::kInt) {
      mid_ofs << (has_col ? val.getInt() : 0);
    } else if (col_type == ColType::kDouble) {
      mid_ofs << std::fixed << (has_col ? val.getDouble() : 0.0);
    } else {
      mid_ofs << "\"" << (has_col ? val.getStr() : "") << "\"";
    }
  }
  mid_ofs << "\n";
  return true;
}

bool WriteElementGeo(std::ofstream& mif_ofs, const GeometryPtr& geo) {
  if (geo == nullptr) {
    mif_ofs << "NONE\n";
    return true;
  }
  GeometryTypeId geo_type = geo->getGeometryTypeId();
  if (geo_type == GEOS_POINT) {
    auto point = std::dynamic_pointer_cast<Point>(geo);
    mif_ofs << "POINT " << point->getX() << " " << point->getY() << "\n";
  } else if (geo_type == GEOS_LINESTRING) {
    auto line = std::dynamic_pointer_cast<LineString>(geo);
    auto coords = line->getCoordinates();
    size_t coords_size = coords->size();
    if (coords_size == 2) {
      mif_ofs << "LINE ";
      mif_ofs << coords->getX(0) << " " << coords->getY(0) << " ";
      mif_ofs << coords->getX(1) << " " << coords->getY(1) << "\n";
    } else if (coords_size > 2) {
      mif_ofs << "PLINE " << coords_size << "\n";
      for (size_t i = 0; i < coords_size; ++i) {
        mif_ofs << coords->getX(i) << " " << coords->getY(i) << "\n";
      }
    } else {
      LOG_ERROR << "LineString size is illegal: " << coords_size << std::endl;
      return false;
    }
  } else if (geo_type == GEOS_MULTILINESTRING) {
    auto multi_line = std::dynamic_pointer_cast<MultiLineString>(geo);
    size_t geo_num = multi_line->getNumGeometries();
    mif_ofs << "PLINE MULTIPLE " << geo_num << "\n";
    for (size_t i = 0; i < geo_num; ++i) {
      auto coords = multi_line->getGeometryN(i)->getCoordinates();
      mif_ofs << "  " << coords->size() << "\n";
      for (size_t j = 0; j < coords->size(); ++j) {
        mif_ofs << coords->getX(j) << " " << coords->getY(j) << "\n";
      }
    }
  } else if (geo_type == GEOS_POLYGON) {
    auto polygon = std::dynamic_pointer_cast<Polygon>(geo);
    auto coords = polygon->getCoordinates();
    size_t coords_size = coords->size();
    mif_ofs << "REGION 1\n";
    mif_ofs << "  " << coords_size << "\n";
    for (size_t i = 0; i < coords_size; ++i) {
      mif_ofs << coords->getX(i) << " " << coords->getY(i) << "\n";
    }
  } else if (geo_type == GEOS_MULTIPOLYGON) {
    auto multi_polygon = std::dynamic_pointer_cast<MultiPolygon>(geo);
    size_t geo_num = multi_polygon->getNumGeometries();
    mif_ofs << "REGION " << geo_num << "\n";
    for (size_t i = 0; i < geo_num; ++i) {
      auto coords = multi_polygon->getGeometryN(i)->getCoordinates();
      mif_ofs << "  " << coords->size() << "\n";
      for (size_t j = 0; j < coords->size(); ++j) {
        mif_ofs << coords->getX(j) << " " << coords->getY(j) << "\n";
      }
    }
  } else {
    LOG_ERROR << "can`t support dump GeometryType: '" << geo->getGeometryType() << "'" << std::endl;
    return false;
  }
  return true;
}

int WriteSingleElement(std::ofstream& mif_ofs,
                       std::ofstream& mid_ofs,
                       const MifHeader& header,
                       MifElement& elem) {
  if (WriteElementAttr(mid_ofs, header, elem) && WriteElementGeo(mif_ofs, elem.getGeo())) {
    return 0;
  }
  return -1;
}

}  // namespace io
}  // namespace gmif