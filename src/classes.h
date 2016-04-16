#include <vector>
#include "HGCal/TBStandaloneSimulator/interface/HGCSSRecoHit.hh"
#include "HGCal/TBStandaloneSimulator/interface/HGCCellMap.h"
#include "DataFormats/Common/interface/Wrapper.h"

namespace HGCSS
{
  struct dictionary
  {
    HGCSSRecoHit _a1;
    std::vector<HGCSSRecoHit> _v1;
    edm::Wrapper<std::vector<HGCSSRecoHit> > _w1;
  };
}

