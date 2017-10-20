#ifndef AVERAGE_TIME_SYNC_H_
#define AVERAGE_TIME_SYNC_H_

#include <fstream>
#include <string>

#include "../synchronization/synchronization.h"
#include "../utils/def.h"

#include "../simulation.h"

#include "atsNeighbor.h"
#include "atsClock.h"

class Layers;

namespace Synchronization {

  class ATSNode: public Node {
  public:
    ATSNeighbors neighbors;
    ATSClock *_clock;
    
    ATSNode(Layers &l);
    ATSNode(const ATSNode &n);
    ~ATSNode();

    void startup();
    void handle(MessagePtr m);
    void handle(EventPtr e);
    
    static void init(std::string &config);

    void synchronizeNeighbors();
    
    void handleRoundTripSync(MessagePtr m);
  };
  
  
}

#endif
