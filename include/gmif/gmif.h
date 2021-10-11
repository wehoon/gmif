#ifndef GMIF_INCLUDE_GMIF_GMIF_H_
#define GMIF_INCLUDE_GMIF_GMIF_H_

#include <geos/geom/Geometry.h>
#include <bitset>
#include <map>
#include <memory>
#include <vector>
//#include <variant>  // C++17 need

//! 坐标经度小数点有效位
#ifndef GMIF_COORD_PRECISION
#define GMIF_COORD_PRECISION (6)
#endif

namespace gmif {

//! 复合属性值类型
class AttrValue {
 public:
  AttrValue();
  AttrValue(const std::string& v);
  AttrValue(const char* v);
  AttrValue(int32_t v);
  AttrValue(double v);
  AttrValue(const AttrValue& rhs) = default;
  AttrValue(AttrValue&& rhs) noexcept;
  ~AttrValue() = default;

  AttrValue& operator=(const AttrValue& rhs) = default;
  AttrValue& operator=(AttrValue&& rhs) noexcept;
  AttrValue& operator=(const std::string& v);
  AttrValue& operator=(const char* v);
  AttrValue& operator=(int32_t v);
  AttrValue& operator=(double v);

  //! 比较运算符要求init_flag_一致
  bool operator==(const AttrValue& rhs) const;
  bool operator<(const AttrValue& rhs) const;
  bool operator>(const AttrValue& rhs) const;

  //! 判断实际值相等
  bool ValueEqual(AttrValue& rhs);

  const std::string& getStr();
  int32_t getInt();
  double getDouble();

 private:
  //  std::variant<int32_t, double, std::string> val_;  // C++17 need
  std::string str_val_;
  double num_val_;
  std::bitset<2> init_flag_;
};

//! 属性集合
typedef std::map<std::string, AttrValue> AttrMap;

//! 几何对象
typedef geos::geom::Geometry Geometry;
typedef std::shared_ptr<Geometry> GeometryPtr;

//! MIF文件头结构
class MifHeader {
 public:
  static const uint32_t kDefaultVersion = 300;

  static const std::string kCharSetNeutral;
  static const std::string kCharSetChinese;

  static const std::string kCoordSysLL;
  static const std::string kCoordSysMC;

  MifHeader()
      : version_(kDefaultVersion),
        charset_(kCharSetChinese),
        delimiter_('\t'),
        coordsys_(kCoordSysLL) {}

  uint32_t getVersion() const { return version_; }
  void setVersion(uint32_t version) { version_ = version; }

  const std::string& getCharset() const { return charset_; }
  void setCharset(const std::string& charset) { charset_ = charset; }

  char getDelimiter() const { return delimiter_; }
  void setDelimiter(char delimiter) { delimiter_ = delimiter; }

  const std::string& getCoordsys() const { return coordsys_; }
  std::string& getCoordsys() { return coordsys_; }
  void setCoordsys(const std::string& coordsys) { coordsys_ = coordsys; }

  const std::string& getTransform() const { return transform_; }
  void setTransform(const std::string& transform) { transform_ = transform; }

  const std::vector<size_t>& getUniqueVec() const { return unique_vec_; }
  std::vector<size_t>& getUniqueVec() { return unique_vec_; }
  void setUniqueVec(const std::vector<size_t>& uniqueVec) { unique_vec_ = uniqueVec; }

  const std::vector<size_t>& getIndexVec() const { return index_vec_; }
  std::vector<size_t>& getIndexVec() { return index_vec_; }
  void setIndexVec(const std::vector<size_t>& indexVec) { index_vec_ = indexVec; }

  /**
   * @brief 获取字段数量
   * @return
   */
  size_t getColumnSize() const { return col_name_vec_.size(); }

  /**
   * @brief 获取字段索引下标
   * @param lower_col_name 小写字段名称
   * @return 成功返回字段索引下标, 失败返回-1
   */
  int32_t getColumnIndex(const std::string& lower_col_name) const noexcept;

  /**
   * @brief 通过索引获取字段原始名称
   * @param index 索引下标, 需满足[0, ColumnSize), 否则抛出异常
   * @return 字段原始名称
   */
  const std::string& getColumnName(size_t index) const;

  /**
   * @brief 通过索引获取字段类型
   * @param index 索引下标, 需满足[0, ColumnSize), 否则抛出异常
   * @return 字段类型
   */
  const std::string& getColumnType(size_t index) const;

  /**
   * @brief 判断是否存在字段
   * @param col_lower_name 字段小写名称
   * @return 存在返回true, 不存在返回false
   */
  bool hasColumn(const std::string& col_lower_name) const noexcept;

  /**
   * @brief 添加字段
   * @param col_name 字段名称
   * @param col_type 字段类型
   * @return 若已存在字段添加失败并返回false, 否则添加成功并返回true
   */
  bool addColumn(const std::string& col_name, const std::string& col_type) noexcept;

  /**
   * @brief 通过字段名称删除字段
   * @param col_name 字段名称
   * @return 存在字段删除成功并返回true, 不存在字段返回false
   */
  bool deleteColumnByName(const std::string& col_name) noexcept;

  /**
   * @brief 通过索引下标删除字段
   * @param index 字段下标索引
   * @return 存在字段删除成功并返回true, 不存在字段返回false
   */
  bool deleteColumnByIndex(size_t index) noexcept;

 private:
  uint32_t version_;       // MIF规格版本
  std::string charset_;    // 字符集
  char delimiter_;         // 分隔符
  std::string coordsys_;   // 坐标系
  std::string transform_;  // 转换

  std::vector<size_t> unique_vec_;
  std::vector<size_t> index_vec_;

  std::vector<std::string> col_name_vec_;     // 字段名列表(转为全小写)
  std::vector<std::string> col_type_vec_;     // 字段类型列表(转为全小写)
  std::map<std::string, int32_t> col_index_;  // 字段下标索引(转为全小写)
};

//! MIF元素结构
class MifElement {
 public:
  MifElement() : geo_(nullptr) {}

  const GeometryPtr& getGeo() const { return geo_; }
  void setGeo(const GeometryPtr& geo) { geo_ = geo; }

  const AttrMap& getAttrsMap() const { return attrs_map_; }
  void setAttrsMap(const AttrMap& attrs_map) { attrs_map_ = attrs_map; }
  void setAttrsMap(AttrMap&& attrs_map) { attrs_map_ = std::move(attrs_map); }

  /**
   * @brief 是否包含字段
   * @param col_lower_name 字段小写名称
   * @return 包含返回true, 不包含返回false
   */
  bool hasColumn(const std::string& col_lower_name) noexcept;

  /**
   * @brief 获取属性值
   * @param col_lower_name 字段小写名称
   * @param res_val 若成功返回的属性值
   * @return 成功返回true, 失败返回false
   */
  bool getAttr(const std::string& col_lower_name, AttrValue& res_val) noexcept;

  /**
   * @brief 获取属性值, 失败抛出异常
   * @param col_lower_name 字段小写名称
   * @return 成功返回属性值
   */
  AttrValue& getAttr(const std::string& col_lower_name);

  /**
   * @brief 新增或更新属性值
   * @param col_lower_name 字段小写名称
   * @param val 属性值
   */
  void addOrUpdateAttr(const std::string& col_lower_name, const AttrValue& val) noexcept;

 private:
  GeometryPtr geo_;
  AttrMap attrs_map_;
};

//! Mif结构
class Mif {
 public:
  /**
   * @brief 加载数据
   * @param layer_path 图层路径, 不带MID/MIF后缀
   * @param mid_only 是否只加载MID信息
   * @return 成功返回Mif对象指针, 失败返回nullptr
   */
  static std::unique_ptr<Mif> Load(const std::string& layer_path, bool mid_only = false);

  /**
   * @brief 保存数据
   * @param out_layer_path 图层路径, 不带MIF/MID后缀
   * @return 成功返回true, 失败返回false
   */
  bool Dump(const std::string& out_layer_path);

  //! 获取MIF头
  MifHeader& header() { return header_; }

  //! 获取元素列表
  std::vector<std::shared_ptr<MifElement>>& elements() { return elements_; }

 private:
  MifHeader header_;
  std::vector<std::shared_ptr<MifElement>> elements_;
};

//! MIF读文件流
class MifIStream {
  // TODO: MifIStream
};

//! MIF写文件流
class MifOStream {
  // TODO: MifOStream
};

}  // namespace gmif

#endif  // GMIF_INCLUDE_GMIF_GMIF_H_
