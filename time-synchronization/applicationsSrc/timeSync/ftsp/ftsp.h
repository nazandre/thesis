#ifndef FTSP_H_
#define FTSP_H_

#include <fstream>
#include <string>

#include "../synchronization/synchronization.h"
#include "../utils/def.h"

#include "../simulation.h"

class Layers;

namespace Synchronization {

  class FTSPNode: public Node {
  public:

    mID_t myRootId;
    uint32_t heartBeats;
    uint32_t numIgnoredTimeouts;
    
    FTSPNode(Layers &l);
    FTSPNode(const FTSPNode &n);
    ~FTSPNode();

    void startup();
    void handle(MessagePtr m);
    void handle(EventPtr e);

    void synchronizeNeighbors(mID_t i, uint32_t r, hopDistance_t h, P2PNetworkInterface *ignore);

    void setNewRoot(mID_t i);    
  };
  
}

#endif
