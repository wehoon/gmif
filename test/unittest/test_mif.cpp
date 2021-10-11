#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <gtest/gtest.h>
#include <chrono>
#include "gmif/gmif.h"
#include "utils.h"

using namespace std::chrono;
using namespace gmif;
using namespace geos::geom;

#define LOG_TIME(exp, msg)                                                                         \
  {                                                                                                \
    auto start = system_clock::now();                                                              \
    EXPECT_TRUE(exp);                                                                              \
    auto end = system_clock::now();                                                                \
    auto duration = duration_cast<microseconds>(end - start);                                      \
    auto etime = double(duration.count()) * microseconds::period::num / microseconds::period::den; \
    std::cout << (msg) << ", elapsed time: " << etime << "s" << std::endl;                         \
  }

class MifTest : public ::testing::Test {
 protected:
  std::string data_dir_;
  std::string point_demo_path_;
  std::string line_demo_path_;
  std::string region_demo_path_;

  void SetUp() override {
    data_dir_ = "test/data/";
    point_demo_path_ = data_dir_ + "point_demo";
    line_demo_path_ = data_dir_ + "line_demo";
    region_demo_path_ = data_dir_ + "region_demo";
  }
};

void TestHeader(std::shared_ptr<Mif>& mif_ptr) {
  EXPECT_EQ(mif_ptr->header().getVersion(), 300);
  EXPECT_EQ(mif_ptr->header().getDelimiter(), ',');
  EXPECT_EQ(mif_ptr->header().getColumnSize(), 4);
  EXPECT_EQ(mif_ptr->header().getColumnIndex("id"), 0);
  EXPECT_EQ(mif_ptr->header().getColumnIndex("kind"), 3);
  EXPECT_EQ(mif_ptr->header().getColumnIndex("no-exist"), -1);
  EXPECT_EQ(mif_ptr->header().getColumnName(2), "length");
  EXPECT_THROW(mif_ptr->header().getColumnName(5), std::out_of_range);
  EXPECT_EQ(mif_ptr->header().getColumnType(1), "char(6)");
  EXPECT_TRUE(mif_ptr->header().hasColumn("code"));
  EXPECT_FALSE(mif_ptr->header().hasColumn("no-exist"));
  EXPECT_FALSE(mif_ptr->header().addColumn("code", "char(2)"));
  EXPECT_TRUE(mif_ptr->header().addColumn("new-col", "char(2)"));
  EXPECT_EQ(mif_ptr->header().getColumnIndex("new-col"), 4);
}

void TestElement(std::shared_ptr<Mif>& mif_ptr) {
  EXPECT_EQ(mif_ptr->elements().size(), 4);

  auto& elem = mif_ptr->elements().at(3);

  EXPECT_TRUE(elem != nullptr);
  EXPECT_TRUE(elem->hasColumn("id"));
  EXPECT_EQ(elem->getAttr("id").getInt(), 1237);
  EXPECT_EQ(elem->getAttr("code").getStr(), "120100");
  EXPECT_THROW(elem->getAttr("no-exist"), std::out_of_range);

  AttrValue v;
  EXPECT_TRUE(elem->getAttr("length", v));
  EXPECT_EQ(v.getDouble(), 10.12);

  AttrValue new_val("new-value");
  EXPECT_NO_THROW(elem->addOrUpdateAttr("new-col", new_val));
  EXPECT_EQ(elem->getAttr("new-col").getStr(), "new-value");

  AttrValue update_val("update-value");
  EXPECT_NO_THROW(elem->addOrUpdateAttr("code", update_val));
  EXPECT_EQ(elem->getAttr("code").getStr(), "update-value");
}

TEST_F(MifTest, TestPointDemo) {
  std::shared_ptr<Mif> mif_ptr;
  LOG_TIME(mif_ptr = Mif::Load(point_demo_path_), "Load point-demo");

  ASSERT_TRUE(mif_ptr != nullptr);

  TestHeader(mif_ptr);
  TestElement(mif_ptr);

  const GeometryPtr& geo0 = mif_ptr->elements().at(0)->getGeo();
  ASSERT_TRUE(geo0 != nullptr);
  EXPECT_EQ(geo0->getGeometryTypeId(), GEOS_POINT);
  EXPECT_EQ(geo0->getNumPoints(), 1);
  auto point0_ptr = std::dynamic_pointer_cast<Point>(geo0);
  EXPECT_EQ(point0_ptr->getX(), 118.539272792);
  EXPECT_EQ(point0_ptr->getY(), 37.7776621352);

  const GeometryPtr& geo3 = mif_ptr->elements().at(3)->getGeo();
  ASSERT_TRUE(geo3 != nullptr);
  EXPECT_EQ(geo3->getGeometryTypeId(), GEOS_POINT);
  EXPECT_EQ(geo3->getNumPoints(), 1);
  auto point3_ptr = std::dynamic_pointer_cast<Point>(geo3);
  EXPECT_EQ(point3_ptr->getX(), 118.547544479);
  EXPECT_EQ(point3_ptr->getY(), 37.7993319101);

  EXPECT_TRUE(mif_ptr->Dump(point_demo_path_ + "_dump"));
}

TEST_F(MifTest, TestLineDemo) {
  std::shared_ptr<Mif> mif_ptr;
  LOG_TIME(mif_ptr = Mif::Load(line_demo_path_), "Load line-demo");

  ASSERT_TRUE(mif_ptr != nullptr);

  TestHeader(mif_ptr);
  TestElement(mif_ptr);

  const GeometryPtr& geo0 = mif_ptr->elements().at(0)->getGeo();
  ASSERT_TRUE(geo0 != nullptr);
  EXPECT_EQ(geo0->getGeometryTypeId(), GEOS_MULTILINESTRING);
  EXPECT_EQ(geo0->getNumPoints(), 22);
  auto line0_ptr = std::dynamic_pointer_cast<MultiLineString>(geo0);
  auto coords0_ptr = line0_ptr->getCoordinates();
  const auto& coords0 = dynamic_cast<CoordinateArraySequence&>(*coords0_ptr);
  EXPECT_EQ(coords0.size(), 22);
  EXPECT_EQ(coords0.getAt(0).x, 118.7574625);
  EXPECT_EQ(coords0.getAt(21).y, 37.733215);

  const GeometryPtr& geo1 = mif_ptr->elements().at(1)->getGeo();
  ASSERT_TRUE(geo1 != nullptr);
  EXPECT_EQ(geo1->getGeometryTypeId(), GEOS_LINESTRING);
  EXPECT_EQ(geo1->getNumPoints(), 20);
  auto line_ptr1 = std::dynamic_pointer_cast<LineString>(geo1);
  auto coords1_ptr = line_ptr1->getCoordinates();
  const auto& coords1 = dynamic_cast<CoordinateArraySequence&>(*coords1_ptr);
  EXPECT_EQ(coords1.size(), 20);
  EXPECT_EQ(coords1.getAt(0).x, 118.760968);
  EXPECT_EQ(coords1.getAt(19).y, 37.7389595);

  const GeometryPtr& geo2 = mif_ptr->elements().at(2)->getGeo();
  ASSERT_TRUE(geo2 != nullptr);
  EXPECT_EQ(geo2->getGeometryTypeId(), GEOS_LINESTRING);
  EXPECT_EQ(geo2->getNumPoints(), 2);
  auto line_ptr2 = std::dynamic_pointer_cast<LineString>(geo2);
  auto coords2_ptr = line_ptr2->getCoordinates();
  const auto& coords2 = dynamic_cast<CoordinateArraySequence&>(*coords2_ptr);
  EXPECT_EQ(coords2.size(), 2);
  EXPECT_EQ(coords2.getAt(0).x, 118.7614015);
  EXPECT_EQ(coords2.getAt(1).y, 37.738903);

  const GeometryPtr& geo3 = mif_ptr->elements().at(3)->getGeo();
  ASSERT_TRUE(geo3 != nullptr);
  EXPECT_EQ(geo3->getGeometryTypeId(), GEOS_MULTILINESTRING);
  EXPECT_EQ(geo3->getNumPoints(), 36);
  auto line_ptr3 = std::dynamic_pointer_cast<MultiLineString>(geo3);
  auto coords3_ptr = line_ptr3->getCoordinates();
  const auto& coords3 = dynamic_cast<CoordinateArraySequence&>(*coords3_ptr);
  EXPECT_EQ(coords3.size(), 36);
  EXPECT_EQ(coords3.getAt(0).x, 118.7406065);
  EXPECT_EQ(coords3.getAt(35).y, 37.7342243333);

  EXPECT_TRUE(mif_ptr->Dump(line_demo_path_ + "_dump"));
}

TEST_F(MifTest, TestRegionDemo) {
  std::shared_ptr<Mif> mif_ptr;
  LOG_TIME(mif_ptr = Mif::Load(region_demo_path_), "Load region-demo");

  ASSERT_TRUE(mif_ptr != nullptr);

  TestHeader(mif_ptr);
  TestElement(mif_ptr);

  const GeometryPtr& geo0 = mif_ptr->elements().at(0)->getGeo();
  ASSERT_TRUE(geo0 != nullptr);
  EXPECT_EQ(geo0->getGeometryTypeId(), GEOS_POLYGON);
  EXPECT_EQ(geo0->getNumPoints(), 44);
  auto polygon0_ptr = std::dynamic_pointer_cast<Polygon>(geo0);
  auto coords0_ptr = polygon0_ptr->getCoordinates();
  const auto& coords0 = dynamic_cast<CoordinateArraySequence&>(*coords0_ptr);
  EXPECT_EQ(coords0.size(), 44);
  EXPECT_EQ(coords0.getAt(0).x, 118.55544);
  EXPECT_EQ(coords0.getAt(43).y, 37.879157);
  EXPECT_EQ(coords0.getAt(0), coords0.getAt(43));

  const GeometryPtr& geo1 = mif_ptr->elements().at(1)->getGeo();
  ASSERT_TRUE(geo1 != nullptr);
  EXPECT_EQ(geo1->getGeometryTypeId(), GEOS_MULTIPOLYGON);
  EXPECT_EQ(geo1->getNumPoints(), 65 + 52);
  auto polygon1_ptr = std::dynamic_pointer_cast<MultiPolygon>(geo1);
  ASSERT_EQ(polygon1_ptr->getNumGeometries(), 2);

  auto coords1_0_ptr = polygon1_ptr->getGeometryN(0)->getCoordinates();
  const auto& coords1_0 = dynamic_cast<CoordinateArraySequence&>(*coords1_0_ptr);
  EXPECT_EQ(coords1_0.size(), 65);
  EXPECT_EQ(coords1_0.getAt(0).x, 118.810769);
  EXPECT_EQ(coords1_0.getAt(64).y, 37.866871);
  EXPECT_EQ(coords1_0.getAt(0), coords1_0.getAt(64));

  auto coords1_1_ptr = polygon1_ptr->getGeometryN(1)->getCoordinates();
  const auto& coords1_1 = dynamic_cast<CoordinateArraySequence&>(*coords1_1_ptr);
  EXPECT_EQ(coords1_1.size(), 52);
  EXPECT_EQ(coords1_1.getAt(0).x, 118.734481);
  EXPECT_EQ(coords1_1.getAt(51).y, 37.799084);
  EXPECT_EQ(coords1_1.getAt(0), coords1_1.getAt(51));

  const GeometryPtr& geo2 = mif_ptr->elements().at(2)->getGeo();
  ASSERT_TRUE(geo2 != nullptr);
  EXPECT_EQ(geo2->getGeometryTypeId(), GEOS_POLYGON);
  EXPECT_EQ(geo2->getNumPoints(), 6); // correct to closed ring
  auto polygon2_ptr = std::dynamic_pointer_cast<Polygon>(geo2);
  auto coords2_ptr = polygon2_ptr->getCoordinates();
  const auto& coords2 = dynamic_cast<CoordinateArraySequence&>(*coords2_ptr);
  EXPECT_EQ(coords2.size(), 6);
  EXPECT_EQ(coords2.getAt(0).x, 118.743614);
  EXPECT_EQ(coords2.getAt(4).y, 37.801476);
  EXPECT_EQ(coords2.getAt(0), coords2.getAt(5));

  EXPECT_TRUE(mif_ptr->Dump(region_demo_path_ + "_dump"));
}