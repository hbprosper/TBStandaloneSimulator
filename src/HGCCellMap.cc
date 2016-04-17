// -------------------------------------------------------------------------
// Description: map sim cell id to (u, v) coordinates and (x, y) to (u, v)
// Created: 06-Apr-2016 Harrison B. Prosper
// -------------------------------------------------------------------------
#include <fstream>
#include <iostream>
#include "TSystem.h"
#include "HGCal/Geometry/interface/HGCalTBCellVertices.h"
#include "HGCal/Geometry/interface/HGCalTBTopology.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellMap.h"
// -------------------------------------------------------------------------
using namespace std;
namespace {
  int MODEL=5;                   // TB2016 model
  int CELL_SIZE_X=6.496345;      // side length of one pixel (cell)
  int NCELL=11;
  double SIDE=NCELL*CELL_SIZE_X; // side length of sensor
  double WIDTH=2*SIDE;           // width of sensor corner to corner
};

HGCCellMap::HGCCellMap(string inputFilename)
  : _uvmap(map<size_t, pair<int, int> >()),
    _type(map<pair<int, int>, int>()),
    _xymap(map<pair<int, int>, pair<double, double> >()),
    _geom(HGCSSGeometryConversion(MODEL, CELL_SIZE_X))
{
  if ( inputFilename == "" )
    inputFilename=string("$CMSSW_BASE/src/HGCal/TBStandaloneSimulator/data/"
			 "sensor_cellid_uv_map.txt");

  char inpfile[1024];
  sprintf(inpfile, "%s", gSystem->ExpandPathName(inputFilename.c_str()));
  ifstream fin(inpfile);
  if ( ! fin.good() )
    {
      cout << "** HGCCellMap - unable to open file " 
	   << inpfile << endl;
      exit(0);
    }
  // initialize hexagonal map
  _geom.initialiseHoneyComb(WIDTH, CELL_SIZE_X);
  _map = _geom.hexagonMap();

  string line;
  getline(fin, line);

  int posid, cellid, u, v;
  double x, y;  
  while (fin >> posid >> cellid >> u >> v >> x >> y)
    {
      pair<int, int> uv(u, v);
      _uvmap[cellid] = uv;
      _type[uv] = posid;

      pair<double, double> xy(x, y);
      _xymap[uv] = xy;

      pair<pair<int, int>, int> cell(uv, posid);
      _cells.push_back(cell);
    }
  fin.close();
}

HGCCellMap::~HGCCellMap() 
{
}

std::vector<pair<pair<int, int>, int> >
HGCCellMap::cells() { return _cells; }


pair<int, int>
HGCCellMap::operator()(size_t cellid)
{
  if ( _uvmap.find(cellid) != _uvmap.end() )
    return _uvmap[cellid];
  else
    return pair<int, int>(-123456, -123456);
}

pair<double, double>
HGCCellMap::operator()(int u, int v)
{
  pair<int, int> key(u, v);
  if ( _xymap.find(key) != _xymap.end() )
    return _xymap[key];
  else
    return pair<double, double>(-123456, -123456);
}

pair<int, int>
HGCCellMap::xy2uv(double x, double y)
{
  int cellid = _map->FindBin(x, y);
  return (*this)(cellid);
}


bool
HGCCellMap::valid(int u, int v)
{
  pair<int, int> key(u, v);
  return _xymap.find(key) != _xymap.end();
}


int
HGCCellMap::type(int u, int v)
{
  pair<int, int> key(u, v);
  if ( _type.find(key) != _type.end() )
    return _type[key];
  else
    return -1;
}
