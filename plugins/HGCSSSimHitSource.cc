// -------------------------------------------------------------------------
// Description: copy output of standalone simulator to TB 2016 test beam
// data format and create HGCalTBRecHits
// Harrison B. Prosper
// -------------------------------------------------------------------------
#include "HGCal/TBStandaloneSimulator/plugins/HGCSSSimHitSource.h"
// -------------------------------------------------------------------------

HGCSSSimHitSource::HGCSSSimHitSource
(const edm::ParameterSet& pset, edm::InputSourceDescription const& desc) 
   : edm::ProducerSourceFromFiles(pset, desc, true),
     _run(pset.getUntrackedParameter<int>("runNumber", 101)),
     _maxevents(pset.getUntrackedParameter<int>("maxEvents",-1)),
     _filenames(pset.getUntrackedParameter<std::vector<std::string> >
	       ("fileNames")),
     _file(0),           // sim file
     _tree(0),           // sim tree
     _genparts(0),       // pointer to vector<HGCSSGenParticle>
     _simhits(0),        // pointer to vector<HGCSSSimHit>
     _samsecs(0),        // pointer to vector<HGCSSSamplingSection>
     _entries(0),        // number of simulated events
     _entry(0)           // entry number
{
  // collections to be produced
  produces<HGCSSGenParticleCollection>("HGCSSGenParticles");
  produces<HGCSSSimHitCollection>("HGCSSSimHits");
  produces<HGCSSSamplingSectionCollection>("HGCSSSamplingSections");
  produces<HGCalTBRecHitCollection>("HGCCalTBRecHits");

  // open sim file
  _file = new TFile(_filenames[0].c_str());
  if ( !_file->IsOpen() )
    throw cms::Exception("FileOpenFailed", 
			 std::string("unable to open file ") + _filenames[0]);

  // get sim tree
  _tree = (TTree*)_file->Get("HGCSSTree");
  if ( !_tree )
    throw cms::Exception("GetTreeFailed", 
			 "unable to access tree HGCSSTree");

  // determine number of events to read
  _entries = (size_t)_tree->GetEntries(); 
  _entries = _maxevents < 0  
    ? _entries 
    : (_entries < (size_t)_maxevents ? _entries : _maxevents);
  std::cout << std::endl
	    << "==> Number of simulated events to read: " 
	    << _entries
	    << std::endl;

  // map input tree to sim object pointers
  _tree->SetBranchAddress("HGCSSGenParticleVec", &_genparts);
  _tree->SetBranchAddress("HGCSSSimHitVec", &_simhits);
  _tree->SetBranchAddress("HGCSSSamplingSectionVec", &_samsecs);
}

HGCSSSimHitSource::~HGCSSSimHitSource() 
{
  if ( _tree ) delete _tree;
  if ( _file ) delete _file;
}

bool HGCSSSimHitSource::
setRunAndEventInfo(edm::EventID& id, 
		   edm::TimeValue_t& time, 
		   edm::EventAuxiliary::ExperimentType&)
{
  if ( !_file )
    throw cms::Exception("FileNotFound") << "sim file not open";

  if ( _entry >= _entries ) return false;

  // load sim objects into memory
  _tree->GetEntry(_entry);
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

  // copy gen particles
  std::auto_ptr<HGCSSGenParticleCollection> 
    genparts(new HGCSSGenParticleCollection);
  for(size_t c=0; c < _genparts->size(); c++)
    genparts->push_back((*_genparts)[c]);

  // copy sampling sections
  std::auto_ptr<HGCSSSamplingSectionCollection> 
    samsecs(new HGCSSSamplingSectionCollection);
  for(size_t c=0; c < _samsecs->size(); c++)
    samsecs->push_back((*_samsecs)[c]);

  // copy sim hit and map to rec hits
  std::auto_ptr<HGCSSSimHitCollection> 
    simhits(new HGCSSSimHitCollection);

  std::auto_ptr<HGCalTBRecHitCollection>  
    rechits(new HGCalTBRecHitCollection);

  for(size_t c=0; c < _simhits->size(); c++)
    {
      // get a reference to sim hit NOT a copy
      HGCSSSimHit& hit = (*_simhits)[c];

      simhits->push_back(hit);

      // get data from sim hit
      // TODO: 1) add noise to energy
      //       2) add noise to time
      float energy = hit.energy();
      float time   = hit.time();
      // layer: 0, 1, etc. starting with layer facing beam
      int layer    = hit.layer();
      // si-layer 0, 1, 2
      int silayer  = hit.silayer();
      size_t cellid= hit.cellid();

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
      // map sim cell id to (u,v) coordinates
      if ( silayer == 2 )
	{
	  // offline starts counting at layer 0 or 1?
	  // assume 1 for now.
	  layer++;
	  int sensor_u=0;
	  int sensor_v=0;
	  int u=0;
	  int v=0;
	  int celltype=0; // calibration cell ids differ depending on layer!
	  HGCalTBDetId detid(layer, sensor_u, sensor_v, u, v, celltype);
	  HGCalTBRecHit rechit(detid, energy, time);

	  rechits->push_back(rechit);
	}
    }

  event.put(genparts, "HGCSSGenParticles");
  event.put(simhits,  "HGCSSSimHits");
  event.put(samsecs,  "HGCSSSamplingSections");
  event.put(rechits,  "HGCCalTBRecHits");
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

#include "FWCore/Framework/interface/InputSourceMacros.h"

DEFINE_FWK_INPUT_SOURCE(HGCSSSimHitSource);
