#ifndef ATS_NEIGHBOR_H_
#define ATS_NEIGHBOR_H_

#include <vector>

#include "network.h"

#include "../utils/def.h"
#include "../synchronization/point.h"

namespace Synchronization {

  class ATSNeighbor {
  public:
    Time rtt_T1; // rtt
    myDouble_t relativeSkew;
    Point pOld;
    Point pOld2;
    
    unsigned int round;

    std::vector<Point> points;
    
    ATSNeighbor();
    ATSNeighbor(const ATSNeighbor &n);
    ~ATSNeighbor();

    void updateRelativeSkew(Point &pNew);
    void add(Point p);
    myDouble_t avgSkew();
  };
  
  class ATSNeighbors {
    std::vector<ATSNeighbor> neighbors;

  public:
    ATSNeighbors();
    ATSNeighbors(const ATSNeighbors &n);
    ~ATSNeighbors();
    
    ATSNeighbor* getATSNeighbor(P2PNetworkInterface *p2p);
  };
  
}


#endif
