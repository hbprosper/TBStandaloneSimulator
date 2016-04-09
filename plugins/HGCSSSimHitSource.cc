// -------------------------------------------------------------------------
// Description: copy output of standalone simulator to TB 2016 test beam
// data format and create HGCalTBRecHits
// Harrison B. Prosper
// -------------------------------------------------------------------------
#include "HGCal/TBStandaloneSimulator/plugins/HGCSSSimHitSource.h"
#include "FWCore/Framework/interface/InputSourceMacros.h"
// -------------------------------------------------------------------------

HGCSSSimHitSource::HGCSSSimHitSource
(const edm::ParameterSet& pset, edm::InputSourceDescription const& desc) 
   : edm::ProducerSourceFromFiles(pset, desc, true),
     _run(pset.getUntrackedParameter<int>("runNumber", 101)),
     _maxevents(pset.getUntrackedParameter<int>("maxEvents",-1)),
     _filenames(pset.getUntrackedParameter<std::vector<std::string> >
		("fileNames")),
     _chain(0),          // chain of files
     _tree(0),           // sim tree
     _genparts(0),       // pointer to vector<HGCSSGenParticle>
     _simhits(0),        // pointer to vector<HGCSSSimHit>
     _samsecs(0),        // pointer to vector<HGCSSSamplingSection>
     _entries(0),        // number of simulated events
     _entry(0),          // entry number,
     _cellidmap(HGCCellIDUVMap()), // cell id to (u, v) map
     _outputname("simulator")
{
  // collections to be produced
  produces<HGCSSGenParticleCollection>(_outputname);
  produces<HGCSSSimHitCollection>(_outputname);
  produces<HGCSSSamplingSectionCollection>(_outputname);
  produces<HGCalTBRecHitCollection>(_outputname);

  // create a possible chain of files
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
  _tree->SetBranchAddress("HGCSSGenParticleVec", &_genparts);
  _tree->SetBranchAddress("HGCSSSimHitVec", &_simhits);
  _tree->SetBranchAddress("HGCSSSamplingSectionVec", &_samsecs);
}

HGCSSSimHitSource::~HGCSSSimHitSource() 
{
  if ( _chain ) delete _chain;
}

bool HGCSSSimHitSource::
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

void HGCSSSimHitSource::produce(edm::Event& event)
{
  // auto_ptr own objects they point to and are 
  // automatically deleted when out of scope

  // put gen particles in event
  std::auto_ptr<HGCSSGenParticleCollection> 
    genparts(new HGCSSGenParticleCollection);
  for(size_t c=0; c < _genparts->size(); c++)
    genparts->push_back((*_genparts)[c]);
  event.put(genparts, _outputname);

  // put sampling sections in event
  std::auto_ptr<HGCSSSamplingSectionCollection> 
    samsecs(new HGCSSSamplingSectionCollection);
  for(size_t c=0; c < _samsecs->size(); c++)
    samsecs->push_back((*_samsecs)[c]);
  event.put(samsecs,  _outputname);

  // put sim hits in event
  std::auto_ptr<HGCSSSimHitCollection> 
    simhits(new HGCSSSimHitCollection);
  for(size_t c=0; c < _simhits->size(); c++)
    simhits->push_back((*_simhits)[c]);
  event.put(simhits,  _outputname);

  // create rec hits and put in event
  std::auto_ptr<HGCalTBRecHitCollection>  
    rechits(new HGCalTBRecHitCollection);

  for(size_t c=0; c < _simhits->size(); c++)
    {
      // get a reference to sim hit NOT a copy
      HGCSSSimHit& hit = (*_simhits)[c];

      // si-layer 0, 1, 2
      int silayer  = hit.silayer();

      // construct a RecHit for sim hit in the depletion 
      // layer only since that is the part of the silicon
      // that is instrumented.
      //
      // we assume that silayer = 2 is always the depletion
      // layer. this is how it is in the sim. but if the 
      // adjacent sensors are flipped about y, I would expect
      // that so too are the depletion layers. CHECK.
      //
      // -----------------------------------------------
      if ( silayer == 2 )
      	{
      	  // create DetId
	  
      	  int sensor_u = 0;
      	  int sensor_v = 0;
      	  // map sim cell id to (u,v) coordinates
      	  size_t cellid = hit.cellid();
      	  std::pair<int, int> uv = _cellidmap(cellid);
      	  int iu = uv.first;
      	  int iv = uv.second;
      	  // layer: 0, 1, etc. starting with layer facing beam
      	  int layer    = hit.layer();
      	  int celltype = cellType(layer, cellid);
      	  HGCalTBDetId detid(layer, sensor_u, sensor_v, iu, iv, celltype);

      	  // create rec hit
      	  // TODO: 1) add noise to energy
      	  //       2) add noise to time
      	  float energy = hit.energy();
      	  float time   = hit.time();
      	  HGCalTBRecHit rechit(detid, energy, time);
      	  rechits->push_back(rechit);
      	}
    }
  event.put(rechits,  _outputname);
}

void HGCSSSimHitSource::fillDescriptions
(edm::ConfigurationDescriptions& descriptions)
{
  edm::ParameterSetDescription desc;
  desc.setComment("Test Beam 2016");
  desc.addUntracked<int>("runNumber", 101);
  desc.addUntracked<int>("maxEvents", -1);
  desc.addUntracked<std::vector<std::string> >("fileNames");
  descriptions.add("source", desc);
}

int HGCSSSimHitSource::cellType(int layer, int cellid)
{
  // try to figure out the cell type (for 128-cell sensor)
  // I understand that
  // layers 0, 2, 4, etc. have inner calibration cell
  // at (u,v) = (-1,2), that is, cellid = 64 and outer
  // calibration cell at (u,v) = (2,-4), that is, cellid=133.
  // This is reversed for layers 1, 2, 4, etc.
  int celltype=0; // standard cell

  int innercell=0, outercell=0;
  if ( layer % 2 == 0 )
    {
      innercell=64; // see sensor_cellid.png
      outercell=133;
    }
  else
    {
      innercell=133;
      outercell=64;
    }
  if      ( cellid == innercell )
    celltype = 1; // inner calibration cell
  else if ( cellid == outercell )
    celltype = 2; // outer calibration cell
  return celltype;
}

DEFINE_FWK_INPUT_SOURCE(HGCSSSimHitSource);
