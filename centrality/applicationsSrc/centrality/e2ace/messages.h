/*
 * messages.h
 *
 *  Created on: 31/08/2016
 *      Author: Andre Naz
 */

#ifndef E2ACE_MESSAGES_H_
#define E2ACE_MESSAGES_H_

#include "../eaMsg.h"
#include "../utils/probabilisticCounter.h"
#include <vector>

#define START_FARNESS_MESSAGE 1003
#define FARNESS_UPDATE_MESSAGE 1004
#define FARNESS_FEEDBACK_MESSAGE 1005
#define FARNESS_LEADER_MESSAGE 1006

//==============================================================================
//
//          StartFarnessMsg  (class)
//
//==============================================================================

class StartFarnessMsg;
typedef std::shared_ptr<StartFarnessMsg>  StartFarnessMsg_ptr;

class StartFarnessMsg : public EAMessage {
 public :
  distance_t bound;
	
  StartFarnessMsg(longDistance_t b) {
    type = START_FARNESS_MESSAGE;
    dataSize = sizeof(distance_t);
    bound = b;
  }
  
  StartFarnessMsg(StartFarnessMsg &m) {
    type = m.type;
    dataSize = m.dataSize;
    bound = m.bound;
  }
  
  ~StartFarnessMsg() {};

  virtual StartFarnessMsg* clone() { return new StartFarnessMsg(*this); }
};

//==============================================================================
//
//          FarnessUpdateMsg  (class)
//
//==============================================================================

class FarnessUpdateMsg;
typedef std::shared_ptr<FarnessUpdateMsg>  FarnessUpdateMsg_ptr;

class FarnessUpdateMsg : public EAMessage {
 public :
  ProbabilisticCounter *pCounter;
  distance_t round;
  distance_t bound; // bound: not needed in last version.

  FarnessUpdateMsg(distance_t r, ProbabilisticCounter& p, distance_t bo) {
    type = FARNESS_UPDATE_MESSAGE;
    round = r;
    bound = bo;
    dataSize = sizeof(distance_t) + p.sizeDataToSendInMsg();
    pCounter = p.clone();
  }

  static size_t availableDataSize() {
    return EAMessage::availableDataSize() - sizeof(distance_t); 
  }
  
  ~FarnessUpdateMsg() {};
  
};

//==============================================================================
//
//          FarnessFeedbackMsg  (class)
//
//==============================================================================

class FarnessFeedbackMsg;
typedef std::shared_ptr<FarnessFeedbackMsg>  FarnessFeedbackMsg_ptr;

class FarnessFeedbackMsg : public EAMessage {
 public :
  longDistance_t farness;
  mID_t id;
	
  FarnessFeedbackMsg(longDistance_t e, mID_t i) {
    type = FARNESS_FEEDBACK_MESSAGE;
    farness = e;
    id = i;
    dataSize = sizeof(mID_t) + sizeof(longDistance_t);
  }
  ~FarnessFeedbackMsg() {};
};


//==============================================================================
//
//          FarnessLeaderMsg  (class)
//
//==============================================================================

class FarnessLeaderMsg;
typedef std::shared_ptr<FarnessLeaderMsg>  FarnessLeaderMsg_ptr;

class FarnessLeaderMsg : public EAMessage {
 public :
	
  mID_t id;
	
  FarnessLeaderMsg(mID_t i) {
    type = FARNESS_LEADER_MESSAGE;
    dataSize = sizeof(mID_t);
    id = i;
  }
  ~FarnessLeaderMsg() {};
};

#endif // E2ACE_MESSAGES_H_
