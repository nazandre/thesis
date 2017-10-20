#ifndef ASYNCHRONOUS_DIFFUSION_H_
#define ASYNCHRONOOUS_DIFFUSION_H_

#include <fstream>
#include <string>

#include "random.h"

#include "../synchronization/synchronization.h"
#include "../utils/def.h"

#include "../simulation.h"

class Layers;

namespace Synchronization {

  class ADNode: public Node {
  public:
    
    ADNode(Layers &l);
    ADNode(const ADNode &n);
    ~ADNode();

    void startup();
    void handle(MessagePtr m);
    void handle(EventPtr e);

    void synchronizeNeighbors();

    //void readNeighborClock();
    
    void handleRoundTripSync(MessagePtr m);

    void updateClock(Point &reading);
  };
  
  
}

#endif
