#include "adMsg.h"

#include <iostream>
#include "scheduler.h"
#include "../appMsgID.h"
#include "../timeSyncBlockCode.h"
#include "../simulation.h"
#include "../synchronization/clock.h"

using namespace std;
using namespace BaseSimulator;

ADSyncMsg::ADSyncMsg(int t): MyMessage() {
  type = t;

  if (type == AD_TIME_MSG) {
    simTime = std::vector<Time> (2,0);
    localTime = std::vector<Time> (2,0);
    globalTime = std::vector<Time> (2,0); 
  } else {
    simTime = std::vector<Time> (4,0);
    localTime = std::vector<Time> (4,0);
    globalTime = std::vector<Time> (4,0); 
  }
}

ADSyncMsg::ADSyncMsg(int t,
		     std::vector<Time> &s,
		     std::vector<Time> &l,
		     std::vector<Time> &g): MyMessage() {
  type = t;
  simTime = s;
  localTime = l;
  globalTime = g;
}

ADSyncMsg::ADSyncMsg(ADSyncMsg *m): MyMessage() {
  type = m->type;
  simTime = m->simTime;
  localTime = m->localTime;
  globalTime = m->globalTime;
}

ADSyncMsg::~ADSyncMsg() {
}

ADSyncMsg* ADSyncMsg::clone() {
	return new ADSyncMsg(this);
}

unsigned int ADSyncMsg::dataSize() {
  unsigned int n = 0;
  
  switch(type) {
  case AD_READCLOCK_1_MSG:
    {
    n = sizeof(myTime_t);
    }
    break;
  case AD_READCLOCK_2_MSG:
    {
      n = 4*sizeof(myTime_t);
    }
    break; 
  case AD_TIME_MSG:
    {
      n = sizeof(myTime_t);
    }
    break;
  }
  return n; 
}

unsigned int ADSyncMsg::numEscapedBytes() {
  // send time
  unsigned int n = 0;
  
  assert(globalTime.size() > 0);

  switch(type) {
  case AD_READCLOCK_1_MSG:
    {
      n += MyMessage::numEscapedBytes(localTime[0]);
    }
    break;
  case AD_READCLOCK_2_MSG:
    {
      n += MyMessage::numEscapedBytes(localTime[0]);
      n += MyMessage::numEscapedBytes(localTime[1]);
      n += MyMessage::numEscapedBytes(localTime[2]);
      n += MyMessage::numEscapedBytes(globalTime[2]);
    }
    break;
  case AD_TIME_MSG:
    {
      n += MyMessage::numEscapedBytes(globalTime[0]);
    }
    break;
  default:
    assert(false);
  }
  return n;
}

void ADSyncMsg::beginningTransmissionMacLayer() {
  BaseSimulator::BuildingBlock *module = sourceInterface->hostBlock;

  // remove time-stamps from any previous unsuccessful transmission!
  int index = 0;
  
  switch(type) {
  case AD_READCLOCK_1_MSG:
  case AD_TIME_MSG:
    {
      index = 0;
    }
    break;
  case AD_READCLOCK_2_MSG:
    {
      index = 2;
    }
    break;
  default:
    assert(false);
  }
  
  timeStamp(module,index);
}

void ADSyncMsg::endReceptionMacLayer() {
  BuildingBlock *module = destinationInterface->hostBlock;
  int index = 0;
  
  switch(type) {
  case AD_READCLOCK_1_MSG:
  case AD_TIME_MSG:
    {
      index = 1;
    }
    break;
  case AD_READCLOCK_2_MSG:
    {
      index = 3;
    }
    break;
  default:
    assert(false);
  }
  
  timeStamp(module,index);
}


void ADSyncMsg::timeStamp(BaseSimulator::BuildingBlock *module, int index) {
  TimeSyncBlockCode *blockCode = (TimeSyncBlockCode *) module->blockCode;
  Synchronization::Node *synchronization = (Synchronization::Node*)
    blockCode->layers.synchronization;
  
  Time now = BaseSimulator::getScheduler()->now();
  Time _localTime = blockCode->layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  
#ifdef LINEAR_MODEL_SYNCHRONIZED_CLOCK
  Time _globalTime = synchronization->clock->getTime(_localTime,MS_RESOLUTION_CLOCK);
#else
  Time _globalTime = synchronization->clock->getTime(_localTime,MS_RESOLUTION_CLOCK);
#endif

  simTime[index] = now;
  localTime[index] = _localTime;
  globalTime[index] = _globalTime;
}
