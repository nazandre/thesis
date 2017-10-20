#include "ctmMsg.h"

#include "buildingBlock.h"
#include "../timeSyncBlockCode.h"
#include "ctm.h"
#include "synchronization/clock.h"
#include "mrtp/mrtp.h"

#include "../simulation.h"

/******** CTMGoMessage Class ********/

CTMGoMessage::CTMGoMessage(): MyMessage() {
  type = CTM_GO;
}

CTMGoMessage::CTMGoMessage(CTMGoMessage *m): MyMessage() {
  type = m->type;
}
  

CTMGoMessage::~CTMGoMessage() {}

CTMGoMessage* CTMGoMessage::clone() {
  return new CTMGoMessage(this);
}

unsigned int CTMGoMessage::dataSize() {
  return 0;
}

unsigned int CTMGoMessage::numEscapedBytes() { // always returns 0, don't really care, important only for precision in sync. msgs
  return 0;
}

/******** CTMBackMessage Class ********/

CTMBackMessage::CTMBackMessage(hopDistance_t h): MyMessage() {
  type = CTM_BACK;
  hops = h;
  sendTime = 0;
  receptionTime = 0;
}

CTMBackMessage::CTMBackMessage(CTMBackMessage *m): MyMessage() {
  type = m->type;
  hops = m->hops;
  sendTime = m->sendTime;
  receptionTime = m->receptionTime;
}
  
CTMBackMessage::~CTMBackMessage() {}


CTMBackMessage* CTMBackMessage::clone() {
  return new CTMBackMessage(this);
}

void CTMBackMessage::beginningTransmissionMacLayer() {
  Time now = BaseSimulator::getScheduler()->now();
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::MRTPNode *mrtp =
    (Synchronization::MRTPNode*) blockCode->layers.synchronization;
  ConvergecastTimeMaxAlgorithm *ctm = &(mrtp->convergecastTimeMax);
  
  Time localTime = blockCode->layers.localClock->getLocalTime(now,
							      MS_RESOLUTION_CLOCK);
  Time maxLocalTime = ctm->maxClock.getTime(localTime,MS_RESOLUTION_CLOCK);
  
  sendTime = maxLocalTime;
}

void CTMBackMessage::endReceptionMacLayer() {
  Time now = BaseSimulator::getScheduler()->now();
  BaseSimulator::BuildingBlock *module = destinationInterface->hostBlock;
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  
  Time localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  
  receptionTime = localTime;
}

unsigned int CTMBackMessage::dataSize() {
  return sizeof(hopDistance_t) + 2*sizeof(myTime_t);
}

unsigned int CTMBackMessage::numEscapedBytes() { // always returns 0, don't really care, important only for precision in sync. msgs
  return 0;
}
