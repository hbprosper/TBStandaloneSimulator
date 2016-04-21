#ifndef HGCSIMDIGISOURCE_H
#define HGCSIMDIGISOURCE_H
/** \class

	\author Harrison B. Prosper 
 */
#include <stdio.h>
#include <iostream>
#include <vector>

#include "TChain.h"
#include "TTree.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Sources/interface/ProducerSourceFromFiles.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "HGCal/CondObjects/interface/HGCalElectronicsMap.h"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSRecoHit.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellMap.h"

class HGCSimDigiSource : public edm::ProducerSourceFromFiles
{
 public:
  explicit HGCSimDigiSource(const edm::ParameterSet& pset,
			    edm::InputSourceDescription 
			    const& desc);

  virtual ~HGCSimDigiSource(); 

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual bool setRunAndEventInfo(edm::EventID& id, 
				  edm::TimeValue_t& time, 
				  edm::EventAuxiliary::ExperimentType&);

  virtual void produce(edm::Event & e);

  int _run;
  int _maxevents;
  int _minadccount;
  std::vector<std::string> _filenames;  ///<name of input sim files

  /// Sim objects
  TChain* _chain;
  TTree*  _tree;
  size_t  _entries;
  size_t  _entry;
  HGCCellMap  _cellmap;
  HGCalElectronicsMap _emap;
  HGCSSRecoHitVec*  _recohits;
};


#endif
