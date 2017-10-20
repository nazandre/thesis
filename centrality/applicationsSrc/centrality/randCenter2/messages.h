#ifndef RAND_CENTER_2_MESSAGE_H_
#define RAND_CENTER_2_MESSAGE_H_

#include "../utils/def.h"
#include "../eaMsg.h"

class RandCenter2MessageNext;
typedef std::shared_ptr<RandCenter2MessageNext>  RandCenter2MessageNextPtr;

class RandCenter2MessageNext : public EAMessage {
 public:
  distance_t randomNumber;
  mID_t id;
  
  RandCenter2MessageNext(distance_t r, mID_t i);
  RandCenter2MessageNext(RandCenter2MessageNext &m);
  virtual ~RandCenter2MessageNext();
  
  virtual RandCenter2MessageNext* clone();
};

//===========================================================================================================
//
//          RandCenter2GoLeaderMsg  (class)
//
//===========================================================================================================

class RandCenter2GoLeaderMsg;
typedef std::shared_ptr<RandCenter2GoLeaderMsg> RandCenter2GoLeaderMsg_ptr;

class RandCenter2GoLeaderMsg : public EAMessage {
 public:
	
  RandCenter2GoLeaderMsg();
  ~RandCenter2GoLeaderMsg();
  RandCenter2GoLeaderMsg(RandCenter2GoLeaderMsg &m);
  virtual RandCenter2GoLeaderMsg* clone();
};

//===========================================================================================================
//
//          RandCenter2BackLeaderMsg  (class)
//
//===========================================================================================================

class RandCenter2BackLeaderMsg;
typedef std::shared_ptr<RandCenter2BackLeaderMsg> RandCenter2BackLeaderMsg_ptr;

class RandCenter2BackLeaderMsg : public EAMessage {
 public :
  longDistance_t value;
  mID_t id;

  RandCenter2BackLeaderMsg(longDistance_t v, mID_t i);
  ~RandCenter2BackLeaderMsg();
};

//===========================================================================================================
//
//          RandCenter2LeaderMsg  (class)
//
//===========================================================================================================

class RandCenter2LeaderMsg;
typedef std::shared_ptr<RandCenter2LeaderMsg> RandCenter2LeaderMsg_ptr;

class RandCenter2LeaderMsg: public EAMessage {
 public :

  longDistance_t value; // not necessary, just for correctness testing/debug
  mID_t id;
	
  RandCenter2LeaderMsg(longDistance_t v, mID_t i); 
  ~RandCenter2LeaderMsg();
};

#endif
