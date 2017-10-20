#include "myNeighborhood.h"

#include "scheduler.h"

#include "appEventsID.h"
#include "appMsgID.h"

#include "layers.h"
#include "timeSyncBlockCode.h"

#include "simulation.h"
#include "utils/utils.h"

#define KEEPALIVE_PERIOD (500*ONE_MILLISECOND)

using namespace BaseSimulator;

NeighborhoodKeepAliveEvent::NeighborhoodKeepAliveEvent(Time t, BaseSimulator::BuildingBlock *conBlock): BlockEvent(t, conBlock) {
  eventType = EVENT_NEIGHBORHOOD_KEEPALIVE;
  randomNumber = conBlock->getRandomUint();
}

NeighborhoodKeepAliveEvent::NeighborhoodKeepAliveEvent(NeighborhoodKeepAliveEvent *ev): BlockEvent(ev) {
  randomNumber = ev->randomNumber;

}

NeighborhoodKeepAliveEvent::~NeighborhoodKeepAliveEvent() {

}

void NeighborhoodKeepAliveEvent::consumeBlockEvent() {
  // send msg + reschedule
  Time now = date;
  Layers &layers = ((TimeSyncBlockCode*)concernedBlock->blockCode)->layers;
  //Time localNow = layers.localClock->getLocalTime(now,true);
  //Time localNext = localNow +  KEEPALIVE_PERIOD + layers.module->getRandomUint()%1000;
  //Time simNext = layers.localClock->getSimTime(localNext);
  Time simNext = now + KEEPALIVE_PERIOD ;
  
  NeighborKeepAliveMessage m;
  Utils::broadcast(layers,m,NULL);
  
  if (simNext <  SIMULATION_END) {
    BaseSimulator::getScheduler()->schedule(new NeighborhoodKeepAliveEvent(simNext, layers.module));
  }
}

const string NeighborhoodKeepAliveEvent::getEventName() {
  return "NEIGHBORHOOD_KEEPALIVE EVENT";
}

void NeighborhoodKeepAliveEvent::init(Layers &layers) {
  Time now = BaseSimulator::getScheduler()->now();
  BaseSimulator::getScheduler()->schedule(new NeighborhoodKeepAliveEvent(now, layers.module));
}

// msg
NeighborKeepAliveMessage::NeighborKeepAliveMessage() {
  type = NEIGHBOR_KEEPALIVE_MSG;

}

NeighborKeepAliveMessage::NeighborKeepAliveMessage(NeighborKeepAliveMessage *m) {
  type = m->type;
}

NeighborKeepAliveMessage::~NeighborKeepAliveMessage() {

}

NeighborKeepAliveMessage* NeighborKeepAliveMessage::clone() {
  return new NeighborKeepAliveMessage(this);
}

unsigned int NeighborKeepAliveMessage::numEscapedBytes() {
  return 0;
}

unsigned int NeighborKeepAliveMessage::dataSize() {
  return 0;
}
