#ifndef GMIF_SRC_IO_H_
#define GMIF_SRC_IO_H_

#include "gmif/gmif.h"
#include <fstream>
#include <geos/geom/GeometryFactory.h>

namespace gmif {
namespace io {

/**
 * @brief 尝试打开文件, 匹配可能的扩展名
 * @param base_name 文件基础名
 * @param ext_names 备选文件扩展名几何
 * @param fin 打开的文件输入流
 * @return 成功返回true, 失败返回false
 */
bool TryOpenFile(const std::string& base_name,
                 const std::vector<std::string>& ext_names,
                 std::ifstream& ifs);

/**
 * @brief 读取MIF头信息
 * @param mif_ifs MIF输入流
 * @param header MIF头对象
 * @return 成功返回0, 失败返回-1, 文件结束返回1
 */
int ReadHeader(std::ifstream& mif_ifs, MifHeader& header);

/**
 * @brief 读取单个元素
 * @param geos_factory GEOS工厂对象
 * @param mif_ifs MIF输入流
 * @param mid_ifs MID输入流
 * @param header MIF头对象
 * @param mid_only 是否只解析MID数据
 * @param elem 返回的元素对象
 * @return 成功返回0, 失败返回-1, 文件结束返回1
 */
int ReadSingleElement(const geos::geom::GeometryFactory::Ptr& geos_factory,
                      std::ifstream& mif_ifs,
                      std::ifstream& mid_ifs,
                      const MifHeader& header,
                      bool mid_only,
                      MifElement& elem);

/**
 * @brief 写入MIF头信息
 * @param mif_ofs MIF输出流
 * @param header MIF头对象
 * @return 成功返回0, 失败返回-1
 */
int WriteHeader(std::ofstream& mif_ofs, const MifHeader& header);

/**
 * @brief 写入MIF元素信息
 * @param mif_ofs MIF输出流
 * @param mid_ofs MID输出流
 * @param header MIF头对象
 * @param elem MIF元素对象
 * @return 成功返回0, 失败返回-1
 */
int WriteSingleElement(std::ofstream& mif_ofs,
                       std::ofstream& mid_ofs,
                       const MifHeader& header,
                       MifElement& elem);
}  // namespace io
}  // namespace gmif

#endif  // GMIF_SRC_IO_H_
