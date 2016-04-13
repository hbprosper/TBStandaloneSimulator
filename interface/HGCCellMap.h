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

class HGCCellMap
{
 public:
  HGCCellMap(std::string inputFilename="");

  virtual ~HGCCellMap(); 

  std::pair<int, int> operator()(size_t cellid);
  std::pair<double, double> operator()(int u, int v);

private:
  std::map<size_t, std::pair<int, int> > _uvmap;
  std::map<std::pair<int, int>, std::pair<double, double> > _xymap;
};


#endif
