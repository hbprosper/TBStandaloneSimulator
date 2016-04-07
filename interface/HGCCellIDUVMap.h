#ifndef HGCCELLIDUVMAP_H
#define HGCCELLIDUVMAP_H
/** \class

	\author Harrison B. Prosper 
	map from sim cell id to (u,v) coordinates
	and from (u,v) to (x, y) coordinates assuming
	cell is centered at (0, 0).
 */
#include <map>
#include <string>

class HGCCellIDUVMap
{
 public:
  HGCCellIDUVMap(std::string inputFilename="");

  virtual ~HGCCellIDUVMap(); 

  std::pair<int, int> operator()(size_t cellid);
  std::pair<double, double> operator()(int u, int v);

private:
  std::map<size_t, std::pair<int, int> > _uvmap;
  std::map<std::pair<int, int>, std::pair<double, double> > _xymap;
};


#endif
