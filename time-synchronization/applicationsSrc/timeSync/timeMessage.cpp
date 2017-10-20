#include "timeMessage.h"

#include <iostream>
#include "scheduler.h"

#include "appMsgID.h"

#define TIMESTAMPING_DEBUG 

using namespace std;
using namespace BaseSimulator;

TimeMessage::TimeMessage(): MyMessage() {
  type = TIME_MSG_ID;
  sendTime = 0;
  receptionTime = 0;
}

TimeMessage::TimeMessage(TimeMessage *m): MyMessage() {
  type = m->type;
  sendTime = m->sendTime;
  receptionTime = m->receptionTime;
}

TimeMessage::~TimeMessage() {};
  
Time TimeMessage::getSendTime() {
  return sendTime;
}

Time TimeMessage::getReceptionTime() {
  return receptionTime;
}

unsigned int TimeMessage::dataSize() {
  return 2*sizeof(myTime_t);
}

void TimeMessage::beginningTransmissionMacLayer() {
  BuildingBlock *module = sourceInterface->hostBlock;

#ifdef TIMESTAMPING_DEBUG
  cerr << "@" << module->blockId
       << " starts to send msg " << id
       << " at " << BaseSimulator::getScheduler()->now()
       << endl;
#endif
}

void TimeMessage::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;
  
#ifdef TIMESTAMPING_DEBUG 
  cerr << "@" << module->blockId
       << " has just received msg " << id
       << " at " << BaseSimulator::getScheduler()->now()
       << endl;
#endif
}
