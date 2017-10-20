#include "app.h"

#include "random.h"
#include "scheduler.h"

#include "../layers.h"
#include "../simulation.h"
#include "../utils/utils.h"

#include "appMsg.h"
#include "appEv.h"

#define APP_PERIOD (250*ONE_MILLISECOND)
#define APP_FIRST_ROUND_TIME (3*ONE_SECOND)
#define APP_RESEND_PROB 0.2

using namespace BaseSimulator;

namespace Application {
  
  // Node class
  Node::Node(Layers &l): layers(l) {
    reachedSimTime = 0;
  }

  Node::Node(const Node &n): layers(n.layers), reachedSimTime(n.reachedSimTime) {
    
  }
  
  Node::~Node() {
  }
  
  void Node::initRandomDelays() {
    processingDelays= BaseSimulator::utils::Random::getUniformIntRNG(layers.module->getRandomUint(), 250,300);
    nextBroadcast = BaseSimulator::utils::Random::getUniformIntRNG(layers.module->getRandomUint(), 250*ONE_MILLISECOND,500*ONE_MILLISECOND);
    timeoutDelays = BaseSimulator::utils::Random::getUniformIntRNG(layers.module->getRandomUint(),0,500);
    resendProbability = BaseSimulator::utils::Random::getUniformDoubleRNG(layers.module->getRandomUint(),0,1);
    howManyMsgs = BaseSimulator::utils::Random::getPoissonIntRNG(layers.module->getRandomUint(),1);
  }

  Time Node::getProcessingDelays() {
    return processingDelays();
  }

  Time Node::getTimeoutDelays() {
    return timeoutDelays();
  }

  void Node::handle(MessagePtr m) {
    Time receptionSimTime = getScheduler()->now();
    Time processingSimTime = max(reachedSimTime, receptionSimTime);
    Time queingSimTime = processingSimTime - receptionSimTime;
    reachedSimTime = processingSimTime+getProcessingDelays();

    //if (layers.module->getNbNeighbors() == 6) {
    if (layers.module->blockId == 177) {
      MY_CERR << "queingTime: " << queingSimTime << endl;
    }
    
    /*MY_CERR << "APP MSG!"
	    << " at time " << getScheduler()->now()
	    << endl;*/
    double p = resendProbability();
    if (p < APP_RESEND_PROB) {
      AppMessage msg;
      Utils::directBroadcast(layers,msg,NULL,reachedSimTime);
    }
    
  }
  
  void Node::handle(EventPtr p) {
    //MY_CERR << "APP ROUND EV!" << endl;
    int nb = min(howManyMsgs(),4);
    
    for (int i = 0; i < nb; i++) {
      AppMessage m;
      uint b = Utils::directBroadcast(layers,m,NULL,p->date);
    }
    //MY_CERR << "broadcasted to " << b << " neighbors at " << p->date << endl;
    scheduleNextAppRound();
  }

  void Node::startup() {
    initRandomDelays();
    scheduleAppRound(APP_FIRST_ROUND_TIME);
  }

  void Node::scheduleAppRound(Time s) {
    BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
    sched->schedule(new AppRoundEvent(s,layers.module));
  }
  
  void Node::scheduleNextAppRound() {

    // ms resolution
    // blocktick ~2000Hz => 500us
    // timeout occurs at t + [0,500us]
    Time now = BaseSimulator::getScheduler()->now();
    Time localTime = layers.localClock->getLocalTime(now,false);
    Time localTimeNextSync = localTime + nextBroadcast();
    
    Time p =  ONE_MILLISECOND;
    Time r = localTimeNextSync%p;
    Time us = getTimeoutDelays();
    Time res = localTimeNextSync - p + us; // local scale

    res = layers.module->getSimulationTime(res); // sim scale

    res = now + nextBroadcast();
    assert(res >= now);
    if (res <  SIMULATION_END) {
      scheduleAppRound(res);
    }
  }
}
