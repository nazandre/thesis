/*
 * events.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef EXACT_EVENTS_H_
#define EXACT_EVENTS_H_

#include "buildingBlock.h"
#include "events.h"

#include "centralityBlockCode.h"
#include "exact.h"

#define EVENT_ELECTION_TIMEOUT 15000

//===========================================================================================================
//
//          ElectionTimeOutEvent  (class)
//
//===========================================================================================================

class ElectionTimeOutEvent : public BlockEvent {
 public:
  bool cancelled;
	
 ElectionTimeOutEvent(uint64_t t, BaseSimulator::BuildingBlock *conBlock) : BlockEvent(t, conBlock) {
    //randomNumber = conBlock->getRandomUint();
    cancelled = false;
    eventType = EVENT_ELECTION_TIMEOUT;
  }
	
 ElectionTimeOutEvent(ElectionTimeOutEvent *ev) : BlockEvent(ev) {
    cancelled = ev->cancelled;
  }

  ~ElectionTimeOutEvent() {

  }

  void cancel() {
    cancelled = true;
  }
	
  void consumeBlockEvent() {
    if (!cancelled) {
      CentralityBlockCode *c = (CentralityBlockCode*) concernedBlock->blockCode;
      Exact *a = (Exact*) c->algorithm;
      a->electionTimeOut();
    }
  }
	
  const virtual string getEventName() {
    return ("ElectionTimeOut Event");
  }
};


#endif //EXACT_EVENTS_H_
