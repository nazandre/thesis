#include "ftspMsg.h"

#include <iostream>
#include "scheduler.h"

#include "../appMsgID.h"
#include "../timeSyncBlockCode.h"

#include "../simulation.h"

#include "../synchronization/clock.h"

//#define FTSP_TIMESTAMPING_DEBUG 

using namespace std;
using namespace BaseSimulator;

FTSPTimeSyncMessage::FTSPTimeSyncMessage(mID_t id, uint32_t r, hopDistance_t h): MyMessage() {
  type = FTSP_TIME_SYNC_MSG;
  sendTime = 0;
  receptionTime = 0;
  receptionSimTime = 0;
  rootID = id;
  round = r;
  hops = h;
}

FTSPTimeSyncMessage::FTSPTimeSyncMessage(FTSPTimeSyncMessage *m): MyMessage() {
  type = m->type;
  sendTime = m->sendTime;
  receptionTime = m->receptionTime;
  receptionSimTime = m->receptionSimTime;
  rootID = m->rootID;
  round = m->round;
  hops = m->hops;
}

FTSPTimeSyncMessage::~FTSPTimeSyncMessage() {
}

FTSPTimeSyncMessage* FTSPTimeSyncMessage::clone() {
	return new FTSPTimeSyncMessage(this);
}
  
Time FTSPTimeSyncMessage::getSendTime() {
  return sendTime;
}

Time FTSPTimeSyncMessage::getReceptionTime() {
  return receptionTime;
}

unsigned int FTSPTimeSyncMessage::dataSize() {
  // send time
  // root id
  // round #
  return sizeof(myTime_t) + sizeof(mID_t) + sizeof(uint32_t);
}

unsigned int FTSPTimeSyncMessage::numEscapedBytes() {
  // all fields are uint32_t (in practice)
  unsigned int n = 0;
  n += MyMessage::numEscapedBytes(sendTime);
  n += MyMessage::numEscapedBytes((byte_t*)&rootID,sizeof(mID_t));
  n += MyMessage::numEscapedBytes((byte_t*)&round,sizeof(uint32_t));
  return n;
}

void FTSPTimeSyncMessage::beginningTransmissionMacLayer() {
  Time now = BaseSimulator::getScheduler()->now();
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);

  if (IS_RUNNING_PROTOCOL(PULSE_SYNC)) {
    Synchronization::LinearClock *clock = (Synchronization::LinearClock *)synchronization->clock;
    sendTime = clock->getTimeLast(localTime, MS_RESOLUTION_CLOCK);
  } else {
    sendTime = synchronization->clock->getTime(localTime,MS_RESOLUTION_CLOCK);
  }
  
#ifdef FTSP_TIMESTAMPING_DEBUG
   cerr << "@" << module->blockId
       << ": " << " timestamps FTSP_TIME_SYNC msg (send)"
       << sendTime
       << endl;
#endif
}

void FTSPTimeSyncMessage::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;
  Time now = BaseSimulator::getScheduler()->now();
  
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Time localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);

  receptionSimTime = now;
  receptionTime = localTime;

#ifdef FTSP_TIMESTAMPING_DEBUG 
  cerr << "@" << module->blockId
       << ": " << " timestamps FTSP_TIME_SYNC msg (reception): "
       << receptionTime
       << " at simTime "
       << receptionSimTime
       << endl;
#endif
}
