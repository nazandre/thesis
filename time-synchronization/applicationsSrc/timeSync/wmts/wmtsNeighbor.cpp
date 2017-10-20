#include "wmtsNeighbor.h"

#include "world.h"
#include "lattice.h"

#include "../simulation.h"


namespace Synchronization {

  // WMTSNeighbor class
  
  WMTSNeighbor::WMTSNeighbor() {
    relativeSkew = 1.0;
    pOld = Point(0,0);
    round = 0;
  }

  WMTSNeighbor::WMTSNeighbor(const WMTSNeighbor &n): relativeSkew(n.relativeSkew), pOld(n.pOld), round(n.round) {
  
  }

  WMTSNeighbor::~WMTSNeighbor() {

  }

  void WMTSNeighbor::updateRelativeSkewWMTS(Point &pNew) {
    wmtsDouble_t rskew = 0;
    wmtsDouble_t diffNum = ((wmtsDouble_t) pNew.global) - ((wmtsDouble_t) pOld.global);
    wmtsDouble_t diffDen = ((wmtsDouble_t) pNew.local) - ((wmtsDouble_t) pOld.local);
    wmtsDouble_t dround = round;
    
    rskew = diffNum/diffDen;
    relativeSkew = (rskew +  relativeSkew * (dround-1))/dround;
  }

  void WMTSNeighbor::updateRelativeSkewMTS(Point &pNew) {
    wmtsDouble_t diffNum = ((wmtsDouble_t) pNew.global) - ((wmtsDouble_t) pOld.global);
    wmtsDouble_t diffDen = ((wmtsDouble_t) pNew.local) - ((wmtsDouble_t) pOld.local);
    relativeSkew = diffNum/diffDen;
  }

  // WMTSNeighbors class
  WMTSNeighbors::WMTSNeighbors() {
    degree_t d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
    neighbors.resize(d);
  }

  WMTSNeighbors::WMTSNeighbors(const WMTSNeighbors &n): neighbors(n.neighbors) {
  }
  
  WMTSNeighbors::~WMTSNeighbors() {

  }
  
  WMTSNeighbor* WMTSNeighbors::getWMTSNeighbor(P2PNetworkInterface *p2p) {
    degree_t index = p2p->hostBlock->getInterfaceIndex(p2p);
    return &neighbors[index];
  }

}
