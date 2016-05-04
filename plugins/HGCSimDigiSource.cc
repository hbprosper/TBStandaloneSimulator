// -------------------------------------------------------------------------
// Description: convert output of standalone simulator to TB 2016 test beam
// data format: SKIROC2DataFrames
// Created  April 2016 Harrison B. Prosper
// Updated: 04-23-2016 HBP use HGCCellMap to get mapping from (u, v) to
//                     (skiroc, channel id)
//          05-03
// -------------------------------------------------------------------------
#include <iostream>
#include <algorithm>
#include "FWCore/Framework/interface/InputSourceMacros.h"
#include "HGCal/TBStandaloneSimulator/plugins/HGCSimDigiSource.h"
#include "HGCal/DataFormats/interface/HGCalTBDataFrameContainers.h"
#include "HGCal/DataFormats/interface/SKIROCParameters.h"
#include "HGCal/DataFormats/interface/HGCalTBDetId.h"
// -------------------------------------------------------------------------
using namespace std;

HGCSimDigiSource::HGCSimDigiSource
(const edm::ParameterSet& pset, edm::InputSourceDescription const& desc) 
   : edm::ProducerSourceFromFiles(pset, desc, true),
     _run(pset.getUntrackedParameter<int>("runNumber", 101)),
     _maxevents(pset.getUntrackedParameter<int>("maxEvents", -1)),
     _minadccount(pset.getUntrackedParameter<int>("minADCCount", 1)),
     _adcpermev(pset.getUntrackedParameter<double>("ADCperMeV", 182.5)),
     _filenames(pset.getUntrackedParameter<std::vector<std::string> >
		("fileNames")),
     _chain(0),              // chain of files
     _tree(0),               // sim tree
     _entries(0),            // number of simulated events
     _entry(0),              // entry number,
     _cellmap(HGCCellMap()), // cell id to (u, v) map and (x, y) to (u, v)
     _simhits(0)
{
  // collections to be produced
  produces<SKIROC2DigiCollection>();
  //produces<HGCSSGenParticleVec>();

  // create a chain of files
  _chain = new TChain("HGCSSTree");
  if ( !_chain )
    throw cms::Exception("ChainCreationFailed", "chain: HGCSSTree");

  for(size_t c=0; c < _filenames.size(); c++)
    _chain->Add(_filenames[c].c_str());
  
  // determine number of events to read
  _entries = _chain->GetEntries(); 
  _entries = _maxevents < 0  
    ? _entries 
    : (_entries < (size_t)_maxevents ? _entries : _maxevents);
  std::cout << std::endl
  	    << "==> Number of simulated events to read: " 
  	    << _entries
  	    << std::endl;

  // map input tree to sim object pointers
  _tree = (TTree*)_chain;
  _tree->SetBranchAddress("HGCSSSimHitVec", &_simhits);
}

HGCSimDigiSource::~HGCSimDigiSource() 
{
  if ( _chain ) delete _chain;
}

bool HGCSimDigiSource::
setRunAndEventInfo(edm::EventID& id, 
		   edm::TimeValue_t& time, 
		   edm::EventAuxiliary::ExperimentType&)
{
  if ( !_chain )
    throw cms::Exception("ChainNotFound") << "sim file chain not open";

  if ( _entry >= _entries ) return false;

  // load sim objects into memory
  long localentry = _chain->LoadTree(_entry);
  _tree->GetEntry(localentry);
  _entry++;

  // construct event info
  id = edm::EventID(_run, 1, _entry);

  // time is a hack
  edm::TimeValue_t present_time = presentTime();
  unsigned long time_between_events = timeBetweenEvents();
  time = present_time + time_between_events;

  return true;
}

void HGCSimDigiSource::produce(edm::Event& event)
{
  // auto_ptr own objects they point to and are 
  // automatically deleted when out of scope

  // // add sim hits to event
  // std::auto_ptr<HGCSSSimHitVec> simhits(new HGCSSSimHitVec());
  // for(size_t c=0; c < _simhits->size(); c++)
  //   simhits->push_back((*_simhits)[c]);
  // event.put(simhits);

  // create skiroc digi objects and put in event
  std::auto_ptr<SKIROC2DigiCollection> 
    digis(new SKIROC2DigiCollection(SKIROC::MAXSAMPLES));

  // sum energies of sim hits in each cell
  // and convert sums from MeV to ADC count
  vector< HGCSimDigiSource::Cell> channels;
  digitize(channels);

  // store digitized data
  for(size_t c = 0; c < channels.size(); c++)
    {
      HGCSimDigiSource::Cell& cell = channels[c];
      digis->addDataFrame(cell.detid);
      digis->backDataFrame().setSample(0, 
				       cell.ADClow, 
				       cell.ADChigh,
				       cell.TDC);
      if ( _entry < 2 )
	{
	  char record[80];
	  sprintf(record, 
		  "(SKIROC,channel)=(%2d,%2d)"
		  " (u,v)=(%2d,%2d), (x,y)=(%6.2f,%6.2f)",
		  cell.skiroc, cell.channel, 
		  cell.u, cell.v, cell.x, cell.y);
	  cout << record << endl;
	}
    }
  event.put(digis);
}

void HGCSimDigiSource::digitize(std::vector<HGCSimDigiSource::Cell>& channels)
{
  // histogram sim hits
  map<int, HGCSimDigiSource::Cell> hits;
  for(size_t c = 0; c < _simhits->size(); c++)
    {
      HGCSSSimHit& simhit = (*_simhits)[c];
      double energy = simhit.energy();
      int cellid    = simhit.cellid();
      int layer     = simhit.layer();
      if ( hits.find(cellid) != hits.end() )
	{
	  HGCSimDigiSource::Cell cell;
	  cell.ADClow = 0;
	  cell.TDC    = 0;
	  cell.energy = 0.0;
	  cell.layer  = layer + 1;

	  pair<int, int> uv = _cellmap(cellid); 
	  cell.u = uv.first;
	  cell.v = uv.second;

	  pair<double, double> xy =_cellmap.uv2xy(cell.u, cell.v);
	  cell.x = xy.first;
	  cell.y = xy.second;

	  // FIXME: for now, hard code sensor_u and sensor_v
	  cell.sensor_u = 0;
	  cell.sensor_v = 0;
	  hits[cellid]  = cell;
	}	
      hits[cellid].energy += energy;
    }

  // now map to ADC counts
  for(map<int, HGCSimDigiSource::Cell>::iterator it=hits.begin();
      it != hits.end(); it++)
    {
      int cellid = it->first;
      HGCSimDigiSource::Cell& cell = hits[cellid];

      cell.ADChigh = static_cast<uint16_t>(cell.energy*_adcpermev);

      // now add noise
      addNoise(cell.ADChigh);

      // apply "zero" suppression
      if ( cell.ADChigh < _minadccount ) continue;

      cell.celltype = _cellmap.celltype(cell.layer,
					cell.sensor_u, cell.sensor_v, 
					cell.u, cell.v);

      cell.detid = HGCalTBDetId(cell.layer, 
				cell.sensor_u, cell.sensor_v, 
				cell.u, cell.v, 
				cell.celltype);

      // map to SKIROC and channel id numbers
      pair<int, int> eid = _cellmap.uv2eid(cell.layer,
					   cell.sensor_u, cell.sensor_v, 
					   cell.u, cell.v);
      cell.skiroc = eid.first;
      cell.channel= eid.second;
      channels.push_back(cell); 
    }

  // sort so that SKIROC 2 comes before SKIROC 1
  // and channels increase monotonically
  sort(channels.begin(), channels.end());
}

// TODO
void HGCSimDigiSource::addNoise(uint16_t& /** adc*/)
{
}

void HGCSimDigiSource::fillDescriptions
(edm::ConfigurationDescriptions& descriptions)
{
  edm::ParameterSetDescription desc;
  desc.setComment("Test Beam 2016");
  desc.addUntracked<int>("runNumber", 101);
  desc.addUntracked<int>("maxEvents", -1);
  desc.addUntracked<int>("minADCCount", 1);
  desc.addUntracked<double>("ADCperMeV", 182.5);
  desc.addUntracked<std::vector<std::string> >("fileNames");
  descriptions.add("source", desc);
}

DEFINE_FWK_INPUT_SOURCE(HGCSimDigiSource);
