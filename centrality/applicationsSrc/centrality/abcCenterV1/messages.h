/*
 * messages.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef ABCCENTERV1_MESSAGES_H_
#define ABCCENTERV1_MESSAGES_H_


#include "../eaMsg.h"
#include "../utils/def.h"

#define GO_MESSAGE 1000
#define BACK_MESSAGE 1001
#define DELETE_MESSAGE 1002
#define LEADER_ELECTED_MESSAGE 1003
#define NEXT_STEP_MESSAGE 1004

#define ROLE_N 0
#define ROLE_A 1
#define ROLE_B 2
#define ROLE_C 3
#define ROLE_BARYCENTER 4

class GoMsg;
typedef std::shared_ptr<GoMsg>  GoMsg_ptr;

class GoMsg : public EAMessage {
 public :
  unsigned int step;
  unsigned int role;
  unsigned int valid_gradient;
  unsigned int id;
  unsigned int value;
  unsigned int value2;
  unsigned int distance;

  GoMsg(unsigned int s, unsigned int r, unsigned int vg, unsigned int v, unsigned int v2, unsigned int i, unsigned int d) {
    type = GO_MESSAGE;

    dataSize = sizeof(uint8_t) // step
      + sizeof(uint8_t) // role
      + sizeof(distance_t) // valid_gradient
      + sizeof(mID_t)
      + 2*sizeof(distance_t) // value, value2
      + sizeof(distance_t);
    
    step = s;
    role = r;
    valid_gradient = vg;
    id = i;
    value = v;
    value2 = v2;
    distance = d;
  }
  ~GoMsg() {};
};

//===========================================================================================================
//
//          BackMsg  (class)
//
//===========================================================================================================

class BackMsg;
typedef std::shared_ptr<BackMsg> BackMsg_ptr;

class BackMsg : public EAMessage {
 public :
  unsigned int id;
  unsigned int distance;
  unsigned int value;
  unsigned int value2;
  unsigned int remaining_id[3];
  unsigned int role;
  unsigned int step;
	
  BackMsg(unsigned int s, unsigned int r, unsigned int v, unsigned int v2, unsigned int i, unsigned int d, unsigned int *ri) {
    type = BACK_MESSAGE;

    dataSize = sizeof(mID_t)
      + 3*sizeof(distance_t) // distance, value, value2
      + 2*sizeof(mID_t) // 2 is enough
      + 2*sizeof(uint8_t); // role, step

    id = i;
    distance = d;
    value = v;
    value2 = v2;
    step = s;
    role = r;
    memcpy(remaining_id, ri, 3*sizeof(unsigned int));
  }
  ~BackMsg() {};
};

class DeleteMsg;
typedef std::shared_ptr<DeleteMsg>  DeleteMsg_ptr;

class DeleteMsg : public EAMessage {
 public :
  unsigned int id;
  unsigned int distance;
  unsigned int value;
  unsigned int value2;
  unsigned int remaining_id[3];
  unsigned int role;
  unsigned int step;
	
  DeleteMsg(unsigned int s, unsigned int r, unsigned int v, unsigned int v2, unsigned int i, unsigned int d, unsigned int *ri) {
    type = DELETE_MESSAGE;
    
    dataSize = sizeof(mID_t)
      + 3*sizeof(distance_t) // distance, value, value2
      + 2*sizeof(mID_t) // 2 is enough
      + 2*sizeof(uint8_t); // role, step

    id = i;
    distance = d;
    value = v;
    value2 = v2;
    step = s;
    role = r;
    memcpy(remaining_id, ri, 3*sizeof(unsigned int));
  }
  ~DeleteMsg() {};
};

//===========================================================================================================
//
//          NextStepMsg  (class)
//
//===========================================================================================================


class NextStepMsg;
typedef std::shared_ptr<NextStepMsg>  NextStepMsg_ptr;

class NextStepMsg : public EAMessage {
 public :
  unsigned int step;
	
  NextStepMsg(unsigned int s) {
    type = NEXT_STEP_MESSAGE;
    dataSize = sizeof(uint8_t);
    step = s;
  }
  ~NextStepMsg() {};
};

//===========================================================================================================
//
//          ElectMsg  (class)
//
//===========================================================================================================

class LeaderElectedMsg;
typedef std::shared_ptr<LeaderElectedMsg>  LeaderElectedMsg_ptr;

class LeaderElectedMsg : public EAMessage {
 public :
	
  unsigned int id;
	
  LeaderElectedMsg(unsigned int i) {
    type = LEADER_ELECTED_MESSAGE;
    dataSize = sizeof(mID_t);
    id = i;
  }
  ~LeaderElectedMsg() {};
};

#endif
