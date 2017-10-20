#ifndef SYNCHRONIZATION_H_
#define SYNCHRONIZATION_H_

#include "network.h"
#include "events.h"

#include "../utils/def.h"

#include "clock.h"

#include "random.h"

class Layers;

namespace Synchronization {

  // Synchronization node
  class Node {
  public:
    Layers &layers;
    
    Clock *clock;
    unsigned int round;

    // Last sent
    //std::vector<Time> lastMsgSent;
    Time reachedLocalTime;

    bool timeoutCall;
    
    // Processing delay
    BaseSimulator::intRNG lowProcessingDelays;
    BaseSimulator::intRNG highProcessingDelays;
    BaseSimulator::intRNG timeoutDelays;
    BaseSimulator::intRNG queueOccupancy;
    
    bool highProcessingDelaysFlag;
    
    Node(Layers &l);
    Node(const Node &n);
    virtual ~Node();

    virtual void handle(MessagePtr m) = 0;
    virtual void handle(EventPtr p) = 0;
    
    virtual void startup() = 0;

    
    void initRandomDelays();
    void setHighProcessingDelays();
    void setLowProcessingDelays();
    Time getProcessingDelays();
    Time getTimeoutDelays();
    Time getQueueingDelays();

    void populateQueue(P2PNetworkInterface *p2p);
    
    void incrReachedTime(MessagePtr &m);

    Time getHandlingTime(MessagePtr &m);
    Time getSendingTime(P2PNetworkInterface *p2p);
    
    void startCentralized(bool root);
    void startDecentralized();
    
    // Simulator time!
    void scheduleTimeMasterElected(Time t);
    
    void scheduleTimeSync(Time simTime);
    void scheduleNextTimeSync(Time period);
    void scheduleNextTimeSyncSimScale(Time period);
    
    Time compensateForCommDelays(Time time, unsigned int size, hopDistance_t hops);

  };

}

#endif
