// -------------------------------------------------------------------------
// Description: map sim cell id to (u, v) coordinates and (x, y) to (u, v)
// Created: 06-Apr-2016 Harrison B. Prosper
// -------------------------------------------------------------------------
#include <fstream>
#include <iostream>
#include "TSystem.h"
#include "TH2Poly.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellMap.h"
// -------------------------------------------------------------------------
using namespace std;

HGCCellMap::HGCCellMap(string inputFilename)
  : _uvmap(map<size_t, pair<int, int> >()),
    _type(map<pair<int, int>, int>()),
    _xymap(map<pair<int, int>, pair<double, double> >())
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

      // assumes that centers given with an
      // accuracy of at least one decimal
      // place.
      int ix = 10*x;
      int iy = 10*y;
      pair<int, int> ixiy(ix, iy);
      _xy2uv[ixiy] = uv;

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
HGCCellMap::operator()(double x, double y)
{
  // assumes that centers given with an
  // accuracy of at least one decimal
  // place.
  int ix = 10*x;
  int iy = 10*y;
  pair<int, int> key(ix, iy);
  if ( _xy2uv.find(key) != _xy2uv.end() )
    return _xy2uv[key];
  else
    return pair<int, int>(-123456, -123456);
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
