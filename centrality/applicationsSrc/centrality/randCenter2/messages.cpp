#include "messages.h"
#include "../msgID.h"

//===========================================================================================================
//
//          RandCenter2MessageNext  (class)
//
//===========================================================================================================

RandCenter2MessageNext::RandCenter2MessageNext(distance_t r, mID_t i) {
  type = RAND_CENTER_2_NEXT;
  dataSize = sizeof(distance_t) + sizeof(mID_t);
  randomNumber = r;
  id = i;
}

RandCenter2MessageNext::RandCenter2MessageNext(RandCenter2MessageNext &m): EAMessage(m) {
  type = m.type;
  dataSize = m.dataSize;
  randomNumber = m.randomNumber;
  id = m.id;
}

RandCenter2MessageNext::~RandCenter2MessageNext() { }

RandCenter2MessageNext* RandCenter2MessageNext::clone() {
  return new RandCenter2MessageNext(*this);
}

//===========================================================================================================
//
//          RandCenter2GoLeaderMsg  (class)
//
//===========================================================================================================

RandCenter2GoLeaderMsg::RandCenter2GoLeaderMsg() {
    type = RAND_CENTER_2_GO_LEADER_MESSAGE;
    dataSize = 0;
}

RandCenter2GoLeaderMsg::~RandCenter2GoLeaderMsg() {

}

RandCenter2GoLeaderMsg::RandCenter2GoLeaderMsg(RandCenter2GoLeaderMsg &m) {
  type = m.type;
}

RandCenter2GoLeaderMsg* RandCenter2GoLeaderMsg::clone() {
  return new RandCenter2GoLeaderMsg(*this);
}

//===========================================================================================================
//
//          RandCenter2BackLeaderMsg  (class)
//
//===========================================================================================================

RandCenter2BackLeaderMsg::RandCenter2BackLeaderMsg(longDistance_t v, mID_t i) {
  type = RAND_CENTER_2_BACK_LEADER_MESSAGE;
  value = v;
  id = i;
  dataSize = sizeof(longDistance_t) + sizeof(mID_t);
}

RandCenter2BackLeaderMsg::~RandCenter2BackLeaderMsg() {
  
}

//===========================================================================================================
//
//          RandCenter2LeaderMsg  (class)
//
//===========================================================================================================


RandCenter2LeaderMsg::RandCenter2LeaderMsg(longDistance_t v, mID_t i) {
  type = RAND_CENTER_2_LEADER_MESSAGE;
  value = v; // not necessary, just for testing/debugging
  id = i;
  dataSize = sizeof(mID_t);
}

RandCenter2LeaderMsg::~RandCenter2LeaderMsg() {
  
}
