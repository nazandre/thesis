/*
 * messages.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef EXACT_MESSAGES_H_
#define EXACT_MESSAGES_H_

#include "../eaMsg.h"

#define DISTANCE_MESSAGE 36003
#define SUM_REQUEST_MESSAGE 36004
#define SUM_INFO_MESSAGE 36005

//===========================================================================================================
//
//          DistanceMsg  (class)
//
//===========================================================================================================

class DistanceMsg;
typedef std::shared_ptr<DistanceMsg> DistanceMsg_ptr;

class DistanceMsg: public EAMessage {
 public:
  unsigned int id;
  unsigned int distance;
	
  DistanceMsg(unsigned int i, unsigned int d) {
    type = DISTANCE_MESSAGE;
    dataSize = sizeof(distance_t) + sizeof(mID_t);
    id = i;
    distance = d;
  }
  
  ~DistanceMsg() {};
};

//===========================================================================================================
//
//         SumRequestMsg  (class)
//
//===========================================================================================================

class SumRequestMsg;
typedef std::shared_ptr<SumRequestMsg> SumRequestMsg_ptr;

class SumRequestMsg: public EAMessage {
 public:

  SumRequestMsg() {
    type = SUM_REQUEST_MESSAGE;
    dataSize = 0; // ?
  }
  
  ~SumRequestMsg() {};
};

//===========================================================================================================
//
//          SumInfoMsg  (class)
//
//===========================================================================================================

class SumInfoMsg;
typedef std::shared_ptr<SumInfoMsg> SumInfoMsg_ptr;

class SumInfoMsg: public EAMessage {
 public:
  unsigned int sum;
	
  SumInfoMsg(unsigned int s) {
    type = SUM_INFO_MESSAGE;
    dataSize = sizeof(longDistance_t); // ?
    sum = s;
  }
  
  ~SumInfoMsg() {};
};

#endif
