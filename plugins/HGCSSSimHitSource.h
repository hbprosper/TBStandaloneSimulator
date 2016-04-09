#ifndef HGCSSSIMHITSOURCE_H
#define HGCSSSIMHITSOURCE_H
/** \class

	\author Harrison B. Prosper 
	(basically a copy of Shervin´s code with mods)
 */
#include <stdio.h>
#include <iostream>
#include <vector>

#include "TChain.h"
#include "TTree.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Sources/interface/ProducerSourceFromFiles.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "HGCal/DataFormats/interface/HGCalTBRecHitCollections.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSCollections.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellIDUVMap.h"

class HGCSSSimHitSource : public edm::ProducerSourceFromFiles
{
 public:
  explicit HGCSSSimHitSource(const edm::ParameterSet& pset,
			     edm::InputSourceDescription 
			     const& desc);

  virtual ~HGCSSSimHitSource(); 

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual bool setRunAndEventInfo(edm::EventID& id, 
				  edm::TimeValue_t& time, 
				  edm::EventAuxiliary::ExperimentType&);

  virtual void produce(edm::Event & e);

  virtual int cellType(int layer, int cellid);

  int _run;
  int _maxevents;
  std::vector<std::string> _filenames;  ///<name of input sim files

  /// Sim objects
  TChain* _chain;
  TTree*  _tree;
  std::vector<HGCSSGenParticle>*     _genparts;
  std::vector<HGCSSSimHit>*          _simhits;
  std::vector<HGCSSSamplingSection>* _samsecs;
  size_t _entries;
  size_t _entry;
  HGCCellIDUVMap _cellidmap;
  std::string _outputname;
};


#endif
