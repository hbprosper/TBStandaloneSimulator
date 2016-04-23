// -------------------------------------------------------------------------
// Description: convert output of standalone simulator to TB 2016 test beam
// data format: SKIROC2DataFrames
// Created  April 2016 Harrison B. Prosper
// Updated: 04-23-2016 HBP use HGCCellMap to get mapping from (u, v) to
//                     (skiroc, channel id)
// -------------------------------------------------------------------------
#include <iostream>
#include <algorithm>
#include "FWCore/Framework/interface/InputSourceMacros.h"
#include "HGCal/TBStandaloneSimulator/plugins/HGCSimDigiSource.h"
#include "HGCal/DataFormats/interface/HGCalTBDataFrameContainers.h"
#include "HGCal/DataFormats/interface/SKIROCParameters.h"
#include "HGCal/DataFormats/interface/HGCalTBElectronicsId.h"
#include "HGCal/DataFormats/interface/HGCalTBDetId.h"
// -------------------------------------------------------------------------
using namespace std;
namespace {
  struct Channel
  {
    int skiroc;      // SKIROC number
    int id;          // channel id
    DetId detid;     // detector id
    uint16_t ADClow;
    uint16_t ADChigh;
    uint16_t TDC;
    int layer;
    int sensor_u;
    int sensor_v;
    int u;
    int v;
    int type;
    double x;
    double y;
    bool operator<(Channel& o)
    {
      // negate skiroc number so that
      // SKIROC 2 occurs before SKIROC 1
      int lhs = -100000*skiroc + id;
      int rhs = -100000*o.skiroc + o.id;
      return lhs < rhs; 
    }
  };
};

HGCSimDigiSource::HGCSimDigiSource
(const edm::ParameterSet& pset, edm::InputSourceDescription const& desc) 
   : edm::ProducerSourceFromFiles(pset, desc, true),
     _run(pset.getUntrackedParameter<int>("runNumber", 101)),
     _maxevents(pset.getUntrackedParameter<int>("maxEvents", -1)),
     _minadccount(pset.getUntrackedParameter<int>("minADCCount", 1)),
     _filenames(pset.getUntrackedParameter<std::vector<std::string> >
		("fileNames")),
     _chain(0),          // chain of files
     _tree(0),           // sim tree
     _entries(0),        // number of simulated events
     _entry(0),          // entry number,
     _cellmap(HGCCellMap()), // cell id to (u, v) map and (x, y) to (u, v)
     _emap(HGCalElectronicsMap()),
     _recohits(0)
{
  // collections to be produced
  produces<SKIROC2DigiCollection>();
  produces<HGCSSRecoHitVec>();

  // create a chain of files
  _chain = new TChain("RecoTree");
  if ( !_chain )
    throw cms::Exception("ChainCreationFailed", "chain: RecoTree");

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
  _tree->SetBranchAddress("HGCSSRecoHitVec", &_recohits);
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

  // add sim hits to event
  std::auto_ptr<HGCSSRecoHitVec> simreco(new HGCSSRecoHitVec());
  for(size_t c=0; c < _recohits->size(); c++)
    simreco->push_back((*_recohits)[c]);
  event.put(simreco);

  // create skiroc digi objects and put in event
  std::auto_ptr<SKIROC2DigiCollection> 
    digis(new SKIROC2DigiCollection(SKIROC::MAXSAMPLES));

  vector<Channel> channels;
  for(size_t c=0; c < _recohits->size(); c++)
    {
      // get a reference to sim rechit NOT a copy
      HGCSSRecoHit& hit = (*_recohits)[c];

      Channel channel;
      channel.ADChigh = static_cast<uint16_t>(hit.adcCounts());

      // apply "zero" suppression
      if ( channel.ADChigh < _minadccount ) continue;

      channel.ADClow = 0;
      channel.TDC = 0;

      // layer count starts at 1 in offline
      // and at zero in sim!
      channel.layer = hit.layer() + 1;
      channel.x  = hit.get_x();
      channel.y  = hit.get_y();

      // create DetId
      channel.sensor_u = 0;
      channel.sensor_v = 0;
      // map sim cell center (x,y) to (u,v) coordinates
      std::pair<int, int> uv = _cellmap.xy2uv(channel.x, channel.y);
      channel.u = uv.first;
      channel.v = uv.second;

      channel.type = _cellmap.type(channel.u, channel.v);
      if ( channel.type == 0 )
	channel.type = 1;
      else
	channel.type = 2;
      channel.detid = HGCalTBDetId(channel.layer, 
				   channel.sensor_u, channel.sensor_v, 
				   channel.u, channel.v, 
				   channel.type);

      // map to SKIROC and channel id numbers
      pair<int, int> eid = _cellmap.uv2eid(channel.u, channel.v);
      channel.skiroc = eid.first;
      channel.id     = eid.second;
      channels.push_back(channel);
    }

  // sort so that SKIROC 2 comes before SKIROC 1
  // and channels increase monotonically
  sort(channels.begin(), channels.end());
  for(size_t c = 0; c < channels.size(); c++)
    {
      Channel& channel = channels[c];
      digis->addDataFrame(channel.detid);
      digis->backDataFrame().setSample(0, 
				       channel.ADClow, 
				       channel.ADChigh,
				       channel.TDC);
      if ( _entry < 2 )
	{
	  char record[80];
	  sprintf(record, 
		  "(SKIROC,channel)=(%2d,%2d)"
		  " (u,v)=(%2d,%2d), (x,y)=(%6.2f,%6.2f)",
		  channel.skiroc, channel.id, 
		  channel.u, channel.v, channel.x, channel.y);
	  cout << record << endl;
	}
    }
  event.put(digis);
}

void HGCSimDigiSource::fillDescriptions
(edm::ConfigurationDescriptions& descriptions)
{
  edm::ParameterSetDescription desc;
  desc.setComment("Test Beam 2016");
  desc.addUntracked<int>("runNumber", 101);
  desc.addUntracked<int>("maxEvents", -1);
  desc.addUntracked<int>("minADCCount", 1);
  desc.addUntracked<std::vector<std::string> >("fileNames");
  descriptions.add("source", desc);
}

DEFINE_FWK_INPUT_SOURCE(HGCSimDigiSource);
