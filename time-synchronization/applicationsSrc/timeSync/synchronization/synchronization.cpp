#include "synchronization.h"

#include "scheduler.h"

#include "synchronizationEv.h"

#include "../layers.h"

#include "../simulation.h"

#include "random.h"
#include "../appMsgID.h"

#include "../utils/utils.h"

using namespace BaseSimulator;

namespace Synchronization {
  
  // Node class
  Node::Node(Layers &l): layers(l), clock(NULL), round(0) {
    setLowProcessingDelays();
    //lastMsgSent = std::vector<Time> (layers.module->getNbInterfaces(),0);
    reachedLocalTime = 0;
    timeoutCall = false;
  }

  Node::Node(const Node &n): layers(n.layers), clock(n.clock), round(n.round), highProcessingDelaysFlag(n.highProcessingDelaysFlag), timeoutCall(n.timeoutCall) {
			     //, lastMsgSent(n.lastMsgSent) {
    
  }
  
  Node::~Node() {
    delete clock;
  }

  void Node::scheduleTimeMasterElected(Time t) {
    BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
    sched->schedule(new TimeMasterElectedEvent(t,layers.module));
  }
  
  void Node::scheduleTimeSync(Time t) {
    BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
    sched->schedule(new TimeSyncEvent(t,layers.module));
  }
  
  void Node::setHighProcessingDelays() {
    highProcessingDelaysFlag = true;
  }
  
  void Node::setLowProcessingDelays() {
    highProcessingDelaysFlag = false;
  }

  void Node::initRandomDelays() {
    lowProcessingDelays= BaseSimulator::utils::Random::getUniformIntRNG(layers.module->getRandomUint(), 250,300);
    highProcessingDelays = BaseSimulator::utils::Random::getUniformIntRNG(layers.module->getRandomUint(), 475,525);
    timeoutDelays = BaseSimulator::utils::Random::getUniformIntRNG(layers.module->getRandomUint(),0,500);
    
    if (Simulation::Parameters::parameters.networkLoad) {
      // add incomming
      queueOccupancy = BaseSimulator::utils::Random::getPoissonIntRNG(layers.module->getRandomUint(),1);
    }
  }

  Time Node::getProcessingDelays() {
    Time delay = 0;

    if (highProcessingDelaysFlag) {
      delay = highProcessingDelays();
    } else {
      delay = lowProcessingDelays();
    }
    
    return delay;
  }

  Time Node::getTimeoutDelays() {
    return timeoutDelays();
  }

  void Node::populateQueue(P2PNetworkInterface *p2p) {
    assert(false);
    int queueLength = queueOccupancy();       
  }

  Time Node::getQueueingDelays() {
    assert(false);
    return 0;
  }

  Time Node::getHandlingTime(MessagePtr &m) {
    //assert(false);
    
    Time now = BaseSimulator::getScheduler()->now();
    Time localTimeNow = layers.localClock->getLocalTime(now,false);
    Time msgSendingTime = 0;
    
    Time localMsgSendingTime = max(localTimeNow,reachedLocalTime);

    // incomming queuing delays
    if (Simulation::Parameters::parameters.networkLoad) {
      int incommingQueueLength = queueOccupancy();
      localMsgSendingTime += incommingQueueLength*lowProcessingDelays();
    }
    
    // processing delay
    if ( (LINEAR_REGRESSION_ENABLED && Simulation::Parameters::parameters.protocol != Simulation::protocol_t::TPSN) ||
	 (LINEAR_REGRESSION_ENABLED && Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TPSN && m->type ==  MRTP_TIME_SYNC_3_MSG) ||
	 (Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TPSN_MLE && m->type ==  MRTP_TIME_SYNC_3_MSG)
	 ) {
      setHighProcessingDelays();
    }

    Time delay = getProcessingDelays();
    localMsgSendingTime += delay;

    setLowProcessingDelays();
    
    reachedLocalTime = localMsgSendingTime;
    return layers.localClock->getSimTime(localMsgSendingTime);
  }
  
  Time Node::getSendingTime(P2PNetworkInterface *p2p) {
    Time now = BaseSimulator::getScheduler()->now();
    Time localTimeNow = layers.localClock->getLocalTime(now,false);
    Time msgSendingTime = 0;
    
    Time localMsgSendingTime = max(localTimeNow,reachedLocalTime);

    if(timeoutCall) {
      // add timeout processing time
      Time delay = getProcessingDelays();
      localMsgSendingTime += delay;
    }
    
    // incomming queuing delays
    /*if (Simulation::Parameters::parameters.networkLoad && !timeoutCall) {
      int incommingQueueLength = queueOccupancy();
      localMsgSendingTime += incommingQueueLength*lowProcessingDelays();
    }
    
    // processing delay
    Time delay = getProcessingDelays();
    localMsgSendingTime += delay;
    
    reachedLocalTime = localMsgSendingTime;*/

    // assumptions: all queue same size
    // not send more than two msgs to a single node without having it responds in between
    // otherwise, msg order is not guaranteed (thus queue is not well simulated!)
    
    // outgoing queing delays
    // is Sending a neighborhood msg ?
    Time keepAlivePeriod = 500*ONE_MILLISECOND;
    Time remainder = localTimeNow % keepAlivePeriod;
    Time transferDelay = 6*ONE_MILLISECOND +
      layers.module->getRandomUint()%ONE_MILLISECOND/2;
    bool keepAlive = false;
    if (remainder <= transferDelay ) {
      keepAlive = true;
      Time compensation = transferDelay - remainder;
      localMsgSendingTime += compensation;
    }

    // warning: this totally kill queue length counting in the simulator kill
    if (Simulation::Parameters::parameters.networkLoad) {
      int outgoingQueueLength = queueOccupancy();
      if (!keepAlive && outgoingQueueLength >= 1) {
	Time remainder = transferDelay - layers.module->getRandomUint()%transferDelay;
        localMsgSendingTime += remainder;
	outgoingQueueLength--;
      }
      localMsgSendingTime += outgoingQueueLength*transferDelay;
    }
    
    msgSendingTime = layers.localClock->getSimTime(localMsgSendingTime);

    //MY_CERR << "Delay at node: " << localMsgSendingTime - localTimeNow << endl;
    
    return msgSendingTime;
  }
  
  void Node::startCentralized(bool root) {
    initRandomDelays();
    
    if (root) {
      Time now = BaseSimulator::getScheduler()->now();
      Time leaderElectedTime = max(now,SYNCHRONIZATION_START + Simulation::Parameters::parameters.timeToElectTimeMaster);
      scheduleTimeMasterElected(leaderElectedTime);
    }
  }
  
  void Node::startDecentralized() {
    Time now = BaseSimulator::getScheduler()->now();
    Time sync = max(now,SYNCHRONIZATION_START);

    initRandomDelays();
    scheduleTimeSync(sync);
  }

  Time Node::compensateForCommDelays(Time time, unsigned int size, hopDistance_t hops) {
    Time delay = 0;
    
    //assert(!ROUND_TRIP_DELAY_COMPENSATION_ENABLED); // shouldn't be called!

#ifdef BITRATE_DELAY_COMPENSATION
    double v = (((double) (size*8)) / MEAN_BITRATE_BIT_S) * pow(10,3);
    //v = std::round(v);
    v = v * 1000;
    delay = US_TO_US_MS_RESOLUTION((Time)v);
#endif
    
#ifdef STATIC_DELAY_COMPENSATION
    delay = COMM_DELAY_US;
#endif
    
#ifdef RANDOM_DELAY_COMPENSATION
    // 5
    // 6
    // 7
    BaseSimulator::doubleRNG gen = BaseSimulator::utils::Random::getUniformDoubleRNG(layers.module->getRandomUint(),0,1);
    double p = gen();
    p = gen();
    if (p < 0.80) {
      delay = COMM_DELAY_US;
    } else if (p < 0.95) {
      delay = COMM_DELAY_US - ONE_MILLISECOND;
    } else {
      delay = COMM_DELAY_US + ONE_MILLISECOND;
    }
#endif

#ifdef STATIC_HOP_DELAY_COMPENSATION
    Time hopError = 0.35 * (double) ONE_MILLISECOND;
    Time hopCompensation = US_TO_US_MS_RESOLUTION(hopError * h);

    cout << "h: " << h << ", compensation => " << hopCompensation << endl;
    delay = COMM_DELAY_US + hopCompensation;
#endif
    
    return time + delay;
  }

  void Node::scheduleNextTimeSync(Time period) {

    // ms resolution
    // blocktick ~2000Hz => 500us
    // timeout occurs at t + [0,500us]
    Time now = BaseSimulator::getScheduler()->now();
    Time localTime = layers.localClock->getLocalTime(now,false);
    Time localTimeNextSync = localTime + period;
    
    Time p =  ONE_MILLISECOND;
    Time r = localTimeNextSync%p;
    Time us = getTimeoutDelays();
    Time res = localTimeNextSync - p + us; // local scale

    res = layers.module->getSimulationTime(res); // sim scale

    assert(res >= now);
    if (res <  SIMULATION_END) {
      scheduleTimeSync(res);
    }
  }

  void Node::scheduleNextTimeSyncSimScale(Time period) {

    // ms resolution
    // blocktick ~2000Hz => 500us
    // timeout occurs at t + [0,500us]
    Time now = BaseSimulator::getScheduler()->now();
    Time nextSync = now + period;
    
    Time p =  ONE_MILLISECOND;
    Time r = nextSync%p;
    Time us = getTimeoutDelays();
    Time res = nextSync - p + us; // local scale

    assert(res >= now);
    if (res <  SIMULATION_END) {
      scheduleTimeSync(res);
    }
  }
}
