// --------------------------------------------------------------------------
// File: createIDMap
// --------------------------------------------------------------------------
#include <cmath>
#include <fstream>
#include <iostream>

#include "TCanvas.h"
#include "TStyle.h"
#include "TH2Poly.h"
#include "TIterator.h"
#include "TList.h"
#include "TText.h"

#include "HGCal/TBStandaloneSimulator/interface/HGCSSGeometryConversion.hh"
#include "HGCal/Geometry/interface/HGCalTBCellVertices.h"

using namespace std;

namespace {
  int MODEL=5;                   // TB2016 model
  int NCELL=11;                  // number of pixels from side to side in sensor
  int CELL_SIZE_X=6.496345;      // side length of one pixel (cell)
  double SIDE=NCELL*CELL_SIZE_X; // side length of sensor
  double WIDTH=2*SIDE;           // width of sensor corner to corner
};

int main()
{
  // create a 2-D histogram with hexagonal bins, a 
  // subset of which lie within the hexagonal boundary
  // that defines a sensor
  HGCSSGeometryConversion geom(MODEL, CELL_SIZE_X);
  geom.initialiseHoneyComb(WIDTH, CELL_SIZE_X);
  TH2Poly* map = geom.hexagonMap();
  map->SetTitle("TB2016 Standalone Simulator Cell IDs");

  // create a single hexagonal bin to represent sensor.
  // we shall use this hexagon region to determine which 
  // sim cellids lie within a sensor.
  TH2Poly hsensor;
  hsensor.SetName("hsensor");
  hsensor.SetTitle("sensor");

  // make slightly larger than reality, so that we include
  // mouse bitten cells
  double S = 1.0001*SIDE;
  double H = S*sqrt(3)/2;  // center to side distance
  double x[7], y[7];
  x[0] = -S/2; y[0] = -H;
  x[1] = -S;   y[1] =  0;
  x[2] = -S/2; y[2] =  H;
  x[3] =  S/2; y[3] =  H;
  x[4] =  S;   y[4] =  0;
  x[5] =  S/2; y[5] = -H;
  x[6] = -S/2; y[6] = -H;
  hsensor.AddBin(7, x, y);
  // get the single hexagonal bin that represents
  // the boundary of sensor
  TIter it(hsensor.GetBins());
  TH2PolyBin* sensor = (TH2PolyBin*)it();

  // loop over bins in 2-D histogram, determine which
  // ones lie within sensor, and write out the bin
  // numbers (cellid) and the (x,y) centers of each 
  // pixel.
  gStyle->SetPalette(1);
  gStyle->SetOptStat("");

  TCanvas csensor("sensor_cellids", "cellid", 10, 10, 600, 600);
  map->GetXaxis()->CenterTitle();
  map->GetXaxis()->SetTitle("#font[12]{x} axis");
  map->GetYaxis()->CenterTitle();
  map->GetYaxis()->SetTitle("#font[12]{y} axis");
  map->Draw();

  hsensor.SetMinimum(0.0);
  hsensor.SetMaximum(1.0);
  hsensor.SetBinContent(1, 0.72);
  hsensor.Draw("col same");
  map->Draw("same");

  ofstream fout("sensor_cellids.txt");
  char record[80];
  sprintf(record, "%6s\t%6s\t%10s %10s",
	  "", "cellid", "x", "y");
  cout << record << endl;
  fout << record << endl;

  ofstream sout("sensor_cellid_uv_map.txt");
  sprintf(record, "%6s\t%6s %6s %6s\t%10s %10s",
	  "", "cellid", "u", "v", "x", "y");
  cout << record << endl;
  sout << record << endl;

  // loop
  TList* bins = map->GetBins();
  TIter next(bins);
  int ncell = 0;
  TText text;
  text.SetTextSize(0.02);
  text.SetTextAlign(22);  // centered

  // number of cells in y (either 12 or 11)
  bool new_column = true;
  bool odd_column = true;
  int colnumber = 0;
  int u = 0;
  int v = 0;
  int u_start = 1;
  int v_start = 8;
  int number=1;

  while ( TH2PolyBin* bin=(TH2PolyBin*)next() )
    {
      int binnumber = bin->GetBinNumber();
      double x = (bin->GetXMax()+bin->GetXMin())/2;
      double y = (bin->GetYMax()+bin->GetYMin())/2;
      csensor.cd();
      sprintf(record, "%d", binnumber);
      text.DrawText(x, y, record); 

      // We get the starting
      // values of (u, v) per column as follows:
      //   1. every two columns, increment u 
      //   2. every column, decrement v
      // Thereafter:
      //   1. decrement u

      new_column = binnumber == number;
      if ( new_column )
	{
	  // decrement v
	  v_start--;

	  colnumber++;
	  if ( colnumber % 2 == 1 ) u_start++;

	  // initialize (u, v) for current column
	  u = u_start+1;
	  v = v_start;

	  new_column = false;
	  if ( odd_column )
	    number += 12;
	  else
	    number += 11;
	  odd_column = !odd_column;
	}

      // decrement u
      u--;

      if ( sensor->IsInside(x, y) )
	{
	  sprintf(record, "%6d\t%6d\t%10.3f %10.3f", 
		  ncell, binnumber, x, y);
	  //cout << record << endl;
	  fout << record << endl;

	  sprintf(record, "%6d\t%6d %6d %6d\t%10.3f %10.3f", 
		  ncell, binnumber, u, v, x, y);
	  cout << record << endl;
	  sout << record << endl;

	  ncell++;
	}	   
    }
  fout.close();
  sout.close();

  csensor.Update();
  csensor.SaveAs(".png");

  // ---------------------------------------------

  TCanvas cuv("sensor_u_v", "u, v", 10, 10, 600, 600);
  map->SetTitle("TB2016 Sensor (u,v) Coordinates");
  map->Draw();
  hsensor.Draw("col same");
  map->Draw("same");

  //cout << endl;
  fout.open("sensor_u_v.txt");
  sprintf(record, "%6s\t%6s %6s\t%10s %10s",
	  "", "u",  "v", "x", "y");
  //cout << record << endl;
  fout << record << endl;

  HGCalTBCellVertices vertices;
  int layer = 1;
  int sensor_iu = 0;
  int sensor_iv = 0;
  int ncells  = 128;
  ncell = 0;
  for(int iu=-15; iu < 15; iu++)
    for(int iv=-15; iv < 15; iv++)
      {
	pair<double, double> 
	  xy = vertices.GetCellCentreCoordinates(layer,
						 sensor_iu, sensor_iv,
						 iu, iv,
						 ncells);
	double x = xy.first;
	double y = xy.second;
	if ( x < -10000 ) continue;

	x *= 10; // change to mm
	y *= 10;
	sprintf(record, "%6d\t%6d %6d\t%10.3f %10.3f", 
		ncell, iu, iv, x, y);
	//cout << record << endl;
	fout << record << endl;
	ncell++;

	cuv.cd();
	sprintf(record, "%d,%d", iu, iv);
	text.DrawText(x, y, record); 
      }
  fout.close();
  cuv.Update();
  cuv.SaveAs(".png");

  return 0;
}
