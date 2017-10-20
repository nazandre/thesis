#include "atsMsg.h"

#include <iostream>
#include "scheduler.h"
#include "../appMsgID.h"
#include "../timeSyncBlockCode.h"
#include "../simulation.h"
#include "../synchronization/clock.h"

using namespace std;
using namespace BaseSimulator;

ATSTimeMsg::ATSTimeMsg(myDouble_t s): MyMessage() {
  type = ATS_TIME_MSG;
  senderSkew = s;
}

ATSTimeMsg::ATSTimeMsg(ATSTimeMsg *m): MyMessage() {
  type = m->type;

  senderSkew = m->senderSkew;

  localSendTime = m->localSendTime;
  globalSendTime = m->globalSendTime;

  localReceiveTime = m->localReceiveTime;
  globalReceiveTime = m->globalReceiveTime;
}

ATSTimeMsg::~ATSTimeMsg() {
}

ATSTimeMsg* ATSTimeMsg::clone() {
	return new ATSTimeMsg(this);
}

unsigned int ATSTimeMsg::dataSize() {
  unsigned int data = 2*sizeof(myTime_t)+sizeof(myDouble_t);
  return data;
}

unsigned int ATSTimeMsg::numEscapedBytes() {
  unsigned int n = 0;
  
  n += MyMessage::numEscapedBytes(senderSkew);
  n += MyMessage::numEscapedBytes(localSendTime);
  n += MyMessage::numEscapedBytes(globalSendTime);
  return n;
}

void ATSTimeMsg::beginningTransmissionMacLayer() {
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;

  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time now = BaseSimulator::getScheduler()->now();

  localSendTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  globalSendTime = synchronization->clock->getTime(localSendTime,MS_RESOLUTION_CLOCK);
}

void ATSTimeMsg::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;

  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time now = BaseSimulator::getScheduler()->now();
  localReceiveTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  globalReceiveTime = synchronization->clock->getTime(localReceiveTime,MS_RESOLUTION_CLOCK);
}
