#include <geos/geom/GeometryFactory.h>
#include <iomanip>
#include "gmif/gmif.h"
#include "io.h"
#include "utils.h"

#ifdef GMIF_SHOW_TIME
#include <chrono>
#endif

using namespace geos::geom;

namespace gmif {

std::unique_ptr<Mif> Mif::Load(const std::string& layer_path, bool mid_only) {
#ifdef GMIF_SHOW_TIME
  auto start = std::chrono::system_clock::now();
#endif
  std::ifstream mif_ifs;
  std::ifstream mid_ifs;
  if (!(io::TryOpenFile(layer_path, {"mif", "MIF", "Mif"}, mif_ifs) &&
        io::TryOpenFile(layer_path, {"mid", "MID", "Mid"}, mid_ifs))) {
    return nullptr;
  }

  std::unique_ptr<Mif> res = std::unique_ptr<Mif>(new Mif);
  if (io::ReadHeader(mif_ifs, res->header()) != 0) {
    LOG_ERROR << "read header failed" << std::endl;
    return nullptr;
  }

  PrecisionModel pm(GMIF_COORD_PRECISION, 0, 0);
  auto geos_factory = GeometryFactory::create(&pm, -1);

  while (mid_ifs.good() && !mid_ifs.eof()) {
    std::shared_ptr<MifElement> elem = std::make_shared<MifElement>();
    int status =
        io::ReadSingleElement(geos_factory, mif_ifs, mid_ifs, res->header(), mid_only, *elem);
    if (status == 0) {
      res->elements_.push_back(elem);
    } else if (status == 1) {
      break;  // eof
    } else {
      LOG_ERROR << "read feature failed" << std::endl;
      return nullptr;
    }
  }
#ifdef GMIF_SHOW_TIME
  auto end = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  auto etime = double(duration.count()) * std::chrono::microseconds::period::num /
               std::chrono::microseconds::period::den;
  std::cout << "load mif '" << layer_path << "' elapsed time: " << etime << "s" << std::endl;
#endif
  return res;
}

bool Mif::Dump(const std::string& out_layer_path) {
  std::string mif_file = out_layer_path + ".mif";
  std::string mid_file = out_layer_path + ".mid";

  std::ofstream mif_fout(mif_file.c_str(), std::ios_base::out | std::ios_base::trunc);
  std::ofstream mid_fout(mid_file.c_str(), std::ios_base::out | std::ios_base::trunc);

  if (mif_fout.fail() || mid_fout.fail()) {
    LOG_ERROR << "can`t open dump file: '" << out_layer_path << ".[mid/mif]'" << std::endl;
    return false;
  }

  mif_fout << std::setprecision(GMIF_COORD_PRECISION) << std::fixed;

  io::WriteHeader(mif_fout, header_);

  for (size_t i = 0; i < elements_.size(); ++i) {
    if (elements_[i] == nullptr) {
      LOG_ERROR << "dump element[" << i << "] failed." << std::endl;
      return false;
    }
    io::WriteSingleElement(mif_fout, mid_fout, header_, *(elements_[i]));
  }

  mif_fout.close();
  mid_fout.close();

  return true;
}

}  // namespace gmif