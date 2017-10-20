#ifndef APP_H_
#define APP_H_

#include "network.h"
#include "events.h"
#include "random.h"
#include "../utils/def.h"

class Layers;

namespace Application {

  class Node {
  public:
    Layers &layers;

    Time reachedSimTime;
    
    // Processing delay
    BaseSimulator::intRNG processingDelays;
    BaseSimulator::intRNG timeoutDelays;
    BaseSimulator::doubleRNG resendProbability;
    BaseSimulator::intRNG howManyMsgs;
    BaseSimulator::intRNG nextBroadcast;
    
    Node(Layers &l);
    Node(const Node &n);
    virtual ~Node();

    void handle(MessagePtr m);
    void handle(EventPtr p);
    void startup();

    void initRandomDelays();
    Time getProcessingDelays();
    Time getTimeoutDelays();

    void scheduleAppRound(Time s);
    void scheduleNextAppRound();
  };

}
#endif
