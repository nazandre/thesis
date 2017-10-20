#ifndef WMTS_H_
#define WMTS_H_

#include <fstream>
#include <string>

#include "../synchronization/synchronization.h"
#include "../utils/def.h"
#include "../simulation.h"

#include "wmtsNeighbor.h"
#include "wmtsClock.h"

class Layers;

namespace Synchronization {

  class WMTSNode: public Node {
  public:
    WMTSNeighbors neighbors;
    WMTSClock *_clock;

    uint32_t omega;
    bID r;

    wmtsDouble_t mu;
    wmtsDouble_t nu;
    
    WMTSNode(Layers &l);
    WMTSNode(const WMTSNode &n);
    ~WMTSNode();

    void startup();
    void handle(MessagePtr m);
    void handle(EventPtr e);
    
    static void init(std::string &config);

    void synchronizeNeighbors();
    
    void handleRoundTripSync(MessagePtr m);
  };
  
  
}

#endif
