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

class HGCCellMap
{
 public:
  HGCCellMap(std::string inputFilename="");
  virtual ~HGCCellMap(); 

  std::pair<int, int> operator()(size_t cellid);
  std::pair<double, double> operator()(int u, int v);
  std::pair<int, int> operator()(double x, double y);
  bool valid(int u, int v);
  std::vector<std::pair<std::pair<int, int>, int> > cells();
  int type(int u, int v);

private:
  std::map<size_t, std::pair<int, int> > _uvmap;
  std::map<std::pair<int, int>, int> _type;
  std::map<std::pair<int, int>, std::pair<double, double> > _xymap;
  std::map<std::pair<int, int>, std::pair<int, int> > _xy2uv;
  std::vector<std::pair<std::pair<int, int>, int> > _cells;
};

#endif
