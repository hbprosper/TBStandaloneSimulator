// -------------------------------------------------------------------------
// Description: map sim cell id to (u, v) coordinates and (x, y) to (u, v)
// Created: 06-Apr-2016 Harrison B. Prosper
// -------------------------------------------------------------------------
#include <fstream>
#include <iostream>
#include "TSystem.h"
#include "HGCal/Geometry/interface/HGCalTBCellParameters.h"
#include "HGCal/Geometry/interface/HGCalTBCellVertices.h"
#include "HGCal/Geometry/interface/HGCalTBTopology.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellMap.h"
// -------------------------------------------------------------------------
using namespace std;
namespace {
  int MODEL=5;                 // TB2016 model
  int NCELL=11;                // number of pixels from side to side in sensor
  // side length of one pixel (cell) in mm
  double CELL_SIDE=10*HGCAL_TB_CELL::FULL_CELL_SIDE; 
  // side length of sensor     
  double SENSOR_SIDE=NCELL*CELL_SIDE; 
  double WIDTH=2*SENSOR_SIDE;  // width of sensor corner to corner
};

HGCCellMap::HGCCellMap(string inputFilename)
  : _uvmap(map<size_t, pair<int, int> >()),
    _type(map<pair<int, int>, int>()),
    _xymap(map<pair<int, int>, pair<double, double> >()),
    _eidmap(map<pair<int, int>, pair<int, int> >()),
    _geom(HGCSSGeometryConversion(MODEL, CELL_SIDE))
{
  if ( inputFilename == "" )
    inputFilename=string("$CMSSW_BASE/src/HGCal/TBStandaloneSimulator/data/"
			 "sensor_map.txt");

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
  _geom.initialiseHoneyComb(WIDTH, CELL_SIDE);
  _map = _geom.hexagonMap();

  string line;
  getline(fin, line);

  int posid, cellid, u, v, skiroc, channel;
  double x, y;  
  while (fin >> posid >> cellid >> u >> v >> x >> y >> skiroc >> channel)
    {
      pair<int, int> uv(u, v);
      _uvmap[cellid] = uv;
      _type[uv] = posid;

      pair<int, int> sc(skiroc, channel);
      _eidmap[uv] = sc;

      pair<double, double> xy(x, y);
      _xymap[uv] = xy;

      HGCCellMap::Cell cell;
      cell.skiroc  = skiroc;
      cell.channel = channel;
      cell.u = u;
      cell.v = v;
      cell.x = x;
      cell.y = y;
      cell.z = 0;
      cell.count = 0;
      cell.type = posid;
      _cells.push_back(cell);

      _cidmap[uv] = cellid;
    }
  fin.close();
}

HGCCellMap::~HGCCellMap() 
{
}

std::vector<HGCCellMap::Cell>
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
HGCCellMap::uv2xy(int u, int v)
{
  pair<int, int> key(u, v);
  if ( _xymap.find(key) != _xymap.end() )
    return _xymap[key];
  else
    return pair<double, double>(-123456, -123456);
}


int
HGCCellMap::uv2cellid(int u, int v)
{
  pair<int, int> key(u, v);
  if ( _cidmap.find(key) != _cidmap.end() )
    return _cidmap[key];
  else
    return -123456;
}

pair<int, int>
HGCCellMap::uv2eid(int u, int v)
{
  pair<int, int> key(u, v);
  if ( _eidmap.find(key) != _eidmap.end() )
    return _eidmap[key];
  else
    return pair<int, int>(-123456, -123456);
}

pair<int, int>
HGCCellMap::xy2uv(double x, double y)
{
  size_t cellid = _map->FindBin(x, y);
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
