#include "mrtpMsg.h"

#include <iostream>
#include "scheduler.h"

#include "../appMsgID.h"
#include "../timeSyncBlockCode.h"

#include "../simulation.h"

#include "../synchronization/clock.h"

//#define MRTP_TIMESTAMPING_DEBUG 

using namespace std;
using namespace BaseSimulator;

MRTPTimeSyncMessage::MRTPTimeSyncMessage(hopDistance_t h): MyMessage() {
  type = MRTP_TIME_SYNC_MSG;
  sendTime = 0;
  receptionTime = 0;
  receptionSimTime = 0;
  hops = h;
}

MRTPTimeSyncMessage::MRTPTimeSyncMessage(MRTPTimeSyncMessage *m): MyMessage() {
  type = m->type;
  sendTime = m->sendTime;
  receptionTime = m->receptionTime;
  receptionSimTime = m->receptionSimTime;
  hops = m->hops;
}

MRTPTimeSyncMessage::~MRTPTimeSyncMessage() {
}

MRTPTimeSyncMessage* MRTPTimeSyncMessage::clone() {
	return new MRTPTimeSyncMessage(this);
}
  
Time MRTPTimeSyncMessage::getSendTime() {
  return sendTime;
}

Time MRTPTimeSyncMessage::getReceptionTime() {
  return receptionTime;
}

unsigned int  MRTPTimeSyncMessage::numEscapedBytes() {
  // send time;
  byte_t hM = hops + 100;
  unsigned int n = MyMessage::numEscapedBytes(sendTime) +
    MyMessage::numEscapedBytes((byte_t*)&hM,sizeof(byte_t));
  return n;
}

unsigned int MRTPTimeSyncMessage::dataSize() {
  // only 1 time is inserted before msg is sent!
  return 1*sizeof(myTime_t) + 1*sizeof(byte_t);
}

void MRTPTimeSyncMessage::beginningTransmissionMacLayer() {
  Time now = BaseSimulator::getScheduler()->now();
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;

  Time localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  Time globalTime = 0;
  
  if (LINEAR_REGRESSION_ENABLED) {
    Synchronization::LinearClock *clock = (Synchronization::LinearClock *)synchronization->clock;
    globalTime = clock->getTimeLast(localTime, MS_RESOLUTION_CLOCK);
  } else {
    globalTime = synchronization->clock->getTime(localTime,MS_RESOLUTION_CLOCK);
  }
  
  sendTime = globalTime;
  
#ifdef MRTP_TIMESTAMPING_DEBUG
   cerr << "@" << module->blockId
       << ": " << " timestamps MRTP_TIME_SYNC msg (send)"
       << sendTime
       << endl;
#endif
}

void MRTPTimeSyncMessage::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;
  Time now = BaseSimulator::getScheduler()->now();
  
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Time localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  
  receptionSimTime = now;
  receptionTime = localTime;

#ifdef MRTP_TIMESTAMPING_DEBUG 
  cerr << "@" << module->blockId
       << ": " << " timestamps MRTP_TIME_SYNC msg (reception)"
       << receptionTime
       << endl;
#endif
}


// Round-trip sync
MRTPSyncRT::MRTPSyncRT(int t): MyMessage() {
  type = t;
  sizePrevious = 0;

  simTime = std::vector<Time> (6,0);
  localTime = std::vector<Time> (6,0);
  globalTime = std::vector<Time> (6,0); 
}

MRTPSyncRT::MRTPSyncRT(int t, std::vector<Time> &s,
	     std::vector<Time> &l,
	   std::vector<Time> &g): MyMessage() {
  type = t;
  sizePrevious = 0;
  
  simTime = s;
  localTime = l;
  globalTime = g;
  
}

MRTPSyncRT::MRTPSyncRT(MRTPSyncRT *m): MyMessage() {
  type = m->type;
  sizePrevious = m->sizePrevious;
  
  simTime = m->simTime;
  localTime = m->localTime;
  globalTime = m->globalTime;
}

MRTPSyncRT::~MRTPSyncRT() {
}

MRTPSyncRT* MRTPSyncRT::clone() {
	return new MRTPSyncRT(this);
}

unsigned int MRTPSyncRT::dataSize() {
  if (type == MRTP_TIME_SYNC_1_MSG) {
    return 0;
  } else if (type == MRTP_TIME_SYNC_2_MSG) {
    return sizeof(myTime_t);
  } else if (type == MRTP_TIME_SYNC_3_MSG) {
    return 3*sizeof(myTime_t) + sizeof(byte_t);
  }
  return 0;
}


unsigned int MRTPSyncRT::numEscapedBytes() {
  unsigned int n = 0;
  unsigned int i = 0;

  if (EXPORT_RTT_DATA_ENABLED) {

    for (i = 0; i <= 3; i++) {
      n+= MyMessage::numEscapedBytes(localTime[i]);
    }
    
  } else {
    
    if (type >= MRTP_TIME_SYNC_2_MSG) {
      n += MyMessage::numEscapedBytes(localTime[2]);
    }
    
    if (type == MRTP_TIME_SYNC_3_MSG) {
      byte_t d = localTime[4] - localTime[3];
      n += MyMessage::numEscapedBytes(&d, sizeof(byte_t));
      n += MyMessage::numEscapedBytes(globalTime[4]);
    }
    
  }
  return n;
}

void MRTPSyncRT::beginningTransmissionMacLayer() {
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;
  int index = 0;
  // remove previous transmission!
  if (type == MRTP_TIME_SYNC_1_MSG) {
    index = 0;
  } else if (type == MRTP_TIME_SYNC_2_MSG) {
    index = 2;
  } else if (type == MRTP_TIME_SYNC_3_MSG) {
    index = 4;
  }
  timeStamp(module,index);
}

void MRTPSyncRT::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;

  int index = 0;

  if (type == MRTP_TIME_SYNC_1_MSG) {
    index = 1;
  } else if (type == MRTP_TIME_SYNC_2_MSG) {
    index = 3;
  } else if (type == MRTP_TIME_SYNC_3_MSG) {
    index = 5;
  }
  
  timeStamp(module,index);
}


void MRTPSyncRT::timeStamp(BaseSimulator::BuildingBlock *module, int index) {
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time now = BaseSimulator::getScheduler()->now();
  
  Time _localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);

  Time _globalTime = 0;

  /*
  if (LINEAR_REGRESSION_ENABLED) {
    Synchronization::LinearClock *clock = (Synchronization::LinearClock *)synchronization->clock;
    _globalTime = clock->getTimeLast(_localTime, MS_RESOLUTION_CLOCK);
  } else {
    _globalTime = synchronization->clock->getTime(_localTime,MS_RESOLUTION_CLOCK);
  }
  */

  _globalTime = synchronization->clock->getTimeLast(_localTime, MS_RESOLUTION_CLOCK);
  
  simTime[index] = now;
  localTime[index] = _localTime;
  globalTime[index] = _globalTime;
}
