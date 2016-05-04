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
    _posid(map<pair<int, int>, int>()),
    _xymap(map<pair<int, int>, pair<double, double> >()),
    _celltype(map<int, map<pair<int, int>, int> >()),
    _eidmap(map<int, map<pair<int, int>, pair<int, int> > >()),
    _geom(HGCSSGeometryConversion(MODEL, WIDTH, CELL_SIDE)),
    _map(0),
    _cells(map<int, vector<HGCCellMap::Cell> >())
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

  int layer, posid, cellid,
    sensor_u, sensor_v, u, v,  
    skiroc, channel, celltype; 
  double x, y;

  while (fin 
	 >> layer >> posid >> cellid 
	 >> sensor_u >> sensor_v >> u >> v
	 >> skiroc >> channel >> celltype 
	 >> x >> y)
    {
      int key = makeKey(layer, sensor_u, sensor_v);

      pair<int, int> uv(u, v);
      if ( layer == 1 )
	{
	  _uvmap[cellid] = uv;
	  _posid[uv] = posid;

	  pair<double, double> xy(x, y);
	  _xymap[uv] = xy;
	}
      HGCCellMap::Cell cell;
      cell.skiroc  = skiroc;
      cell.channel = channel;
      cell.u = u;
      cell.v = v;
      cell.x = x;
      cell.y = y;
      cell.z = 0;
      cell.count = 0;
      cell.posid = posid;
      cell.celltype = celltype;

      if ( _cells.find(key) == _cells.end() ) 
	_cells[key] = std::vector<HGCCellMap::Cell>(); 
      _cells[key].push_back(cell);

      if ( _celltype.find(key) == _celltype.end() )
	_celltype[key] = map<pair<int, int>, int>();
      _celltype[key][uv] = celltype;

      if ( _eidmap.find(key) == _eidmap.end() ) 
	_eidmap[key] = map<pair<int, int>, pair<int, int> >();
      _eidmap[key][uv] = pair<int, int>(skiroc, channel);
    }
  fin.close();
}

HGCCellMap::~HGCCellMap() 
{
}

std::vector<HGCCellMap::Cell>
HGCCellMap::cells(int layer, int sensor_u, int sensor_v) 
{
  int key = makeKey(layer, sensor_u, sensor_v);
  if ( _cells.find(key) != _cells.end() )
    return _cells[key];
  else
    return std::vector<HGCCellMap::Cell>();
}


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

pair<int, int>
HGCCellMap::uv2eid(int layer, int sensor_u, int sensor_v, int u, int v)
{
  int key = makeKey(layer, sensor_u, sensor_v);
  if ( _eidmap.find(key) != _eidmap.end() )
    {
      pair<int, int> uv(u, v);
      if ( _eidmap[key].find(uv) != _eidmap[key].end() )
	return _eidmap[key][uv];
      else
	return pair<int, int>(-123456, -123456);
    }
  else
    return pair<int, int>(-123456, -123456);
}

pair<int, int>
HGCCellMap::xy2uv(double x, double y)
{
  size_t cellid = _map->FindBin(x, y);
  return (*this)(cellid);
}

int
HGCCellMap::celltype(int layer, int sensor_u, int sensor_v, int u, int v)
{
  int key = makeKey(layer, sensor_u, sensor_v);
  if ( _celltype.find(key) != _celltype.end() )
    {
      pair<int, int> uv(u, v);
      if ( _celltype[key].find(uv) != _celltype[key].end() )
	return _celltype[key][uv];
      else
	return -123456;
    }
  else
    return -123456;
}

int
HGCCellMap::posid(int u, int v)
{
  pair<int, int> key(u, v);
  if ( _posid.find(key) != _posid.end() )
    return _posid[key];
  else
    return -1;
}
