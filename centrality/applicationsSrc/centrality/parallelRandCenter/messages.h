#ifndef PARALLEL_RAND_CENTER_MESSAGE_H_
#define PARALLEL_RAND_CENTER_MESSAGE_H_

#include "../utils/def.h"
#include "../eaMsg.h"

class ParallelRandCenterMessageNext;
typedef std::shared_ptr<ParallelRandCenterMessageNext>  ParallelRandCenterMessageNextPtr;

class ParallelRandCenterMessageNext : public EAMessage {
 public:
  distance_t randomNumber;
  mID_t id;
  
  ParallelRandCenterMessageNext(distance_t r, mID_t i);
  ParallelRandCenterMessageNext(ParallelRandCenterMessageNext &m);
  virtual ~ParallelRandCenterMessageNext();
  
  virtual ParallelRandCenterMessageNext* clone();
};

//===========================================================================================================
//
//          ParallelRandCenterGoLeaderMsg  (class)
//
//===========================================================================================================

class ParallelRandCenterGoLeaderMsg;
typedef std::shared_ptr<ParallelRandCenterGoLeaderMsg> ParallelRandCenterGoLeaderMsg_ptr;

class ParallelRandCenterGoLeaderMsg : public EAMessage {
 public:
	
  ParallelRandCenterGoLeaderMsg();
  ~ParallelRandCenterGoLeaderMsg();
  ParallelRandCenterGoLeaderMsg(ParallelRandCenterGoLeaderMsg &m);
  virtual ParallelRandCenterGoLeaderMsg* clone();
};

//===========================================================================================================
//
//          ParallelRandCenterBackLeaderMsg  (class)
//
//===========================================================================================================

class ParallelRandCenterBackLeaderMsg;
typedef std::shared_ptr<ParallelRandCenterBackLeaderMsg> ParallelRandCenterBackLeaderMsg_ptr;

class ParallelRandCenterBackLeaderMsg : public EAMessage {
 public :
  longDistance_t value;
  mID_t id;

  ParallelRandCenterBackLeaderMsg(longDistance_t v, mID_t i);
  ~ParallelRandCenterBackLeaderMsg();
};

//===========================================================================================================
//
//          ParallelRandCenterLeaderMsg  (class)
//
//===========================================================================================================

class ParallelRandCenterLeaderMsg;
typedef std::shared_ptr<ParallelRandCenterLeaderMsg> ParallelRandCenterLeaderMsg_ptr;

class ParallelRandCenterLeaderMsg: public EAMessage {
 public :

  longDistance_t value; // not necessary, just for correctness testing/debug
  mID_t id;
	
  ParallelRandCenterLeaderMsg(longDistance_t v, mID_t i); 
  ~ParallelRandCenterLeaderMsg();
};

#endif
