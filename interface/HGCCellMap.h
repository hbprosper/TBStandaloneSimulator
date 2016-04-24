#ifndef HGCCELLMAP_H
#define HGCCELLMAP_H
/** \class

	\author Harrison B. Prosper 
	map from sim cell id to (u,v) coordinates
	and from (u,v) to (x, y) coordinates assuming
	cell is centered at (0, 0).
 */
#include <map>
#include <string>
#include <vector>
#include "TH2Poly.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSGeometryConversion.hh"

class HGCCellMap
{
 public:
  struct Cell
  {
    int skiroc;
    int channel;
    int u;
    int v;
    int type;
    double x;
    double y;
    double z;
    double count;
  };
  HGCCellMap(std::string inputFilename="");
  virtual ~HGCCellMap(); 

  std::pair<int, int>       operator()(size_t cellid);
  std::pair<double, double> uv2xy(int u, int v);
  std::pair<int, int>       uv2eid(int u, int v);
  std::pair<int, int>       xy2uv(double x, double y);
  int uv2cellid(int u, int v);

  bool valid(int u, int v);
  int   type(int u, int v);
  std::vector<HGCCellMap::Cell> cells();

private:
  std::map<size_t, std::pair<int, int> > _uvmap;
  std::map<std::pair<int, int>, int> _type;
  std::map<std::pair<int, int>, std::pair<double, double> > _xymap;
  std::map<std::pair<int, int>, std::pair<int, int> > _eidmap;
  std::map<std::pair<int, int>, int> _cidmap;

  HGCSSGeometryConversion _geom;
  TH2Poly* _map;
  std::vector<HGCCellMap::Cell> _cells;
};

#endif
