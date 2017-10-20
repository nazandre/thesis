#ifndef WMTS_NEIGHBOR_H_
#define WMTS_NEIGHBOR_H_

#include <vector>

#include "network.h"

#include "../utils/def.h"
#include "../synchronization/point.h"

namespace Synchronization {

  class WMTSNeighbor {
  public:
    wmtsDouble_t relativeSkew;
    Point pOld;
    Point pOld2;
    
    unsigned int round;

    WMTSNeighbor();
    WMTSNeighbor(const WMTSNeighbor &n);
    ~WMTSNeighbor();

    void updateRelativeSkewWMTS(Point &pNew);
    void updateRelativeSkewMTS(Point &pNew);
  };
  
  class WMTSNeighbors {
    std::vector<WMTSNeighbor> neighbors;

  public:
    WMTSNeighbors();
    WMTSNeighbors(const WMTSNeighbors &n);
    ~WMTSNeighbors();
    
    WMTSNeighbor* getWMTSNeighbor(P2PNetworkInterface *p2p);
  };
  
}


#endif
