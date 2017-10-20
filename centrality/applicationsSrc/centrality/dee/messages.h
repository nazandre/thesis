/*
 * messages.h
 *
 *  Created on: 31/08/2016
 *      Author: Andre Naz
 */

#ifndef DEE_MESSAGES_H_
#define DEE_MESSAGES_H_

#include "../eaMsg.h"
#include "../utils/probabilisticCounter.h"
#include <vector>

#define START_ECCENTRICITY_MESSAGE 1003
#define ECCENTRICITY_UPDATE_MESSAGE 1004
#define ECCENTRICITY_FEEDBACK_MESSAGE 1005
#define ECCENTRICITY_LEADER_MESSAGE 1006

//==============================================================================
//
//          StartEccentricityMsg  (class)
//
//==============================================================================

class StartEccentricityMsg;
typedef std::shared_ptr<StartEccentricityMsg>  StartEccentricityMsg_ptr;

class StartEccentricityMsg : public EAMessage {
 public :
  distance_t bound;
	
  StartEccentricityMsg(distance_t b) {
    type = START_ECCENTRICITY_MESSAGE;
    dataSize = sizeof(distance_t);
    bound = b;
  }
  
  StartEccentricityMsg(StartEccentricityMsg &m) {
    type = m.type;
    dataSize = m.dataSize;
    bound = m.bound;
  }
  
  ~StartEccentricityMsg() {};
  
   virtual StartEccentricityMsg* clone() { return new StartEccentricityMsg(*this); }
};


//==============================================================================
//
//          EccentricityUpdateMsg  (class)
//
//==============================================================================

class EccentricityUpdateMsg;
typedef std::shared_ptr<EccentricityUpdateMsg>  EccentricityUpdateMsg_ptr;

class EccentricityUpdateMsg : public EAMessage {
 public :
  std::vector<myShortDouble_t> x;
  distance_t round;
  distance_t bound; // not need bound in last version.

  EccentricityUpdateMsg(distance_t r, std::vector<myShortDouble_t>&_x, distance_t bo): x(_x) {
    type = ECCENTRICITY_UPDATE_MESSAGE;
    round = r;
    bound = bo;
    dataSize = sizeof(distance_t) + sizeof(myShortDouble_t)*x.size();
  }
  
  ~EccentricityUpdateMsg() {};
};

//==============================================================================
//
//          EccentricityFeedbackMsg  (class)
//
//==============================================================================

class EccentricityFeedbackMsg;
typedef std::shared_ptr<EccentricityFeedbackMsg>  EccentricityFeedbackMsg_ptr;

class EccentricityFeedbackMsg : public EAMessage {
 public :
  longDistance_t eccentricity;
  mID_t id;
	
  EccentricityFeedbackMsg(longDistance_t e, mID_t i) {
    type = ECCENTRICITY_FEEDBACK_MESSAGE;
    dataSize = sizeof(longDistance_t) + sizeof(mID_t);
    eccentricity = e;
    id = i;
  }
  ~EccentricityFeedbackMsg() {};
};


//==============================================================================
//
//          EccentricityLeaderMsg  (class)
//
//==============================================================================

class EccentricityLeaderMsg;
typedef std::shared_ptr<EccentricityLeaderMsg>  EccentricityLeaderMsg_ptr;

class EccentricityLeaderMsg : public EAMessage {
 public :
	
  mID_t id;
	
  EccentricityLeaderMsg(mID_t i) {
    type = ECCENTRICITY_LEADER_MESSAGE;
    dataSize = sizeof(mID_t);
    id = i;
  }
  ~EccentricityLeaderMsg() {};
};

#endif // DEE_MESSAGES_H_
