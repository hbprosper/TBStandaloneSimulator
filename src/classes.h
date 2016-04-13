#include "HGCal/TBStandaloneSimulator/interface/HGCSSSimHit.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSGenParticle.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSSamplingSection.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSCollections.h"
#include "HGCal/TBStandaloneSimulator/interface/G4SiHit.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCSSGeometryConversion.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellMap.h"

#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Common/interface/Wrapper.h"
#include "DataFormats/Common/interface/RefToBase.h"
#include "DataFormats/Common/interface/Holder.h"
#include <vector>

namespace HGCSS
{
  struct dictionary 
  {
    HGCSSSimHit _a1;
    std::vector<HGCSSSimHit> _v1;
    edm::SortedCollection<HGCSSSimHit> _s1;
    edm::Wrapper<HGCSSSimHitCollection> _w1;

    HGCSSGenParticle _a2;
    std::vector<HGCSSGenParticle> _v2;
    edm::SortedCollection<HGCSSGenParticle> _s2;
    edm::Wrapper<HGCSSGenParticleCollection> _w2;

    HGCSSSamplingSection _a3;
    std::vector<HGCSSSamplingSection> _v3;
    edm::SortedCollection<HGCSSSamplingSection> _s3;
    edm::Wrapper<HGCSSSamplingSectionCollection> _w3;

    HGCCellMap _a4;
  };
}

