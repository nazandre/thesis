#include "wmtsMsg.h"

#include <iostream>
#include "scheduler.h"
#include "../appMsgID.h"
#include "../timeSyncBlockCode.h"
#include "../simulation.h"
#include "../synchronization/clock.h"

using namespace std;
using namespace BaseSimulator;

WMTSTimeMsg::WMTSTimeMsg(int64_t _offset, wmtsDouble_t _skew,
			 uint32_t _omega,
			 bID _r,
			 wmtsDouble_t _mu, wmtsDouble_t _nu
			 ): MyMessage() {
  type = WMTS_TIME_MSG;
  offset = _offset;
  skew = _skew;
  omega = _omega;
  r = _r;

  mu = _mu;
  nu = _nu;
}

WMTSTimeMsg::WMTSTimeMsg(WMTSTimeMsg *m): MyMessage() {
  type = m->type;

  offset = m->offset;
  skew = m->skew;
  omega = m->omega;
  r = m->r;

  mu = m->mu;
  nu = m->nu;

  localSendTime = m->localSendTime;
  globalSendTime = m->globalSendTime;

  localReceiveTime = m->localReceiveTime;
  globalReceiveTime = m->globalReceiveTime;
}

WMTSTimeMsg::~WMTSTimeMsg() {
}

WMTSTimeMsg* WMTSTimeMsg::clone() {
	return new WMTSTimeMsg(this);
}

unsigned int WMTSTimeMsg::dataSize() {
  unsigned int data = 1*sizeof(myTime_t)+ 2*sizeof(myDouble_t) +
    1*sizeof(uint16_t);
    //+ 1*sizeof(int32_t); // ignore because it does not fit!
  return data;
}

unsigned int WMTSTimeMsg::numEscapedBytes() {
  unsigned int n = 0;
  
  //n += MyMessage::numEscapedBytes(skew);
  //n += MyMessage::numEscapedBytes(offset);
  n += MyMessage::numEscapedBytes(localSendTime);
  n += MyMessage::numEscapedBytes(globalSendTime);
  return n;
}

void WMTSTimeMsg::beginningTransmissionMacLayer() {
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;

  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time now = BaseSimulator::getScheduler()->now();

  localSendTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  globalSendTime = synchronization->clock->getTime(localSendTime,MS_RESOLUTION_CLOCK);
}

void WMTSTimeMsg::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;

  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time now = BaseSimulator::getScheduler()->now();
  localReceiveTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  globalReceiveTime = synchronization->clock->getTime(localReceiveTime,MS_RESOLUTION_CLOCK);
}
