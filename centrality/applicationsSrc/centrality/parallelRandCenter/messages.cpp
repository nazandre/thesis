#include "messages.h"
#include "../msgID.h"

//===========================================================================================================
//
//          ParallelRandCenterMessageNext  (class)
//
//===========================================================================================================

ParallelRandCenterMessageNext::ParallelRandCenterMessageNext(distance_t r, mID_t i) {
  type = PARALLEL_RAND_CENTER_NEXT;
  dataSize = sizeof(distance_t) + sizeof(mID_t);
  randomNumber = r;
  id = i;
}

ParallelRandCenterMessageNext::ParallelRandCenterMessageNext(ParallelRandCenterMessageNext &m): EAMessage(m) {
  type = m.type;
  dataSize = m.dataSize;
  randomNumber = m.randomNumber;
  id = m.id;
}

ParallelRandCenterMessageNext::~ParallelRandCenterMessageNext() { }

ParallelRandCenterMessageNext* ParallelRandCenterMessageNext::clone() {
  return new ParallelRandCenterMessageNext(*this);
}

//===========================================================================================================
//
//          ParallelRandCenterGoLeaderMsg  (class)
//
//===========================================================================================================

ParallelRandCenterGoLeaderMsg::ParallelRandCenterGoLeaderMsg() {
    type = PARALLEL_RAND_CENTER_GO_LEADER_MESSAGE;
    dataSize = 0;
}

ParallelRandCenterGoLeaderMsg::~ParallelRandCenterGoLeaderMsg() {

}

ParallelRandCenterGoLeaderMsg::ParallelRandCenterGoLeaderMsg(ParallelRandCenterGoLeaderMsg &m) {
  type = m.type;
}

ParallelRandCenterGoLeaderMsg* ParallelRandCenterGoLeaderMsg::clone() {
  return new ParallelRandCenterGoLeaderMsg(*this);
}

//===========================================================================================================
//
//          ParallelRandCenterBackLeaderMsg  (class)
//
//===========================================================================================================

ParallelRandCenterBackLeaderMsg::ParallelRandCenterBackLeaderMsg(longDistance_t v, mID_t i) {
  type = PARALLEL_RAND_CENTER_BACK_LEADER_MESSAGE;
  value = v;
  id = i;
  dataSize = sizeof(longDistance_t) + sizeof(mID_t);
}

ParallelRandCenterBackLeaderMsg::~ParallelRandCenterBackLeaderMsg() {
  
}

//===========================================================================================================
//
//          ParallelRandCenterLeaderMsg  (class)
//
//===========================================================================================================


ParallelRandCenterLeaderMsg::ParallelRandCenterLeaderMsg(longDistance_t v, mID_t i) {
  type = PARALLEL_RAND_CENTER_LEADER_MESSAGE;
  value = v; // not necessary, just for testing/debugging
  id = i;
  dataSize = sizeof(mID_t);
}

ParallelRandCenterLeaderMsg::~ParallelRandCenterLeaderMsg() {
  
}
