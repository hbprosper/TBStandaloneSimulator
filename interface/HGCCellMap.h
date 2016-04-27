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
    int celltype;
    int posid;
    int u;
    int v;
    double x;
    double y;
    double z;
    double count;
  };
  HGCCellMap(std::string inputFilename="");
  virtual ~HGCCellMap(); 

  std::pair<int, int>       operator()(size_t cellid);
  std::pair<double, double> uv2xy(int u, int v);
  std::pair<int, int>       xy2uv(double x, double y);
  int  posid(int u, int v);

  std::pair<int, int>       uv2eid(int layer,
				   int sensor_u, int sensor_v,
				   int u, int v);
  int                       celltype(int layer,
				     int sensor_u, int sensor_v,
				     int u, int v);
  
  std::vector<HGCCellMap::Cell> cells(int layer=1,
				      int sensor_u=0,
				      int sensor_v=0);

private:
  std::map<size_t, std::pair<int, int> > _uvmap;
  std::map<std::pair<int, int>, int> _posid;
  std::map<std::pair<int, int>, std::pair<double, double> > _xymap;

  std::map<int, std::map<std::pair<int, int>, int> > _celltype;
  std::map<int, std::map<std::pair<int, int>, std::pair<int, int> > > _eidmap;

  HGCSSGeometryConversion _geom;
  TH2Poly* _map;
  std::map<int, std::vector<HGCCellMap::Cell> > _cells;

  inline
    int makeKey(int layer, int sensor_u, int sensor_v)
  {
    return 1000000*layer + 1000*sensor_u + sensor_v;
  }
};

#endif
