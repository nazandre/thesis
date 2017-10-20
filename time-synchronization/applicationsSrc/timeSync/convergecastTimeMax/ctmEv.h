#ifndef CTM_EVENTS_H_
#define CTM_EVENTS_H_

#include "events.h"

class ConvergecastTimeMaxDone;
typedef std::shared_ptr<ConvergecastTimeMaxDone> ConvergecastTimeMaxDonePtr;

class ConvergecastTimeMaxDone : public BlockEvent {
public:
  
  ConvergecastTimeMaxDone(Time t, BaseSimulator::BuildingBlock *conBlock);
  ConvergecastTimeMaxDone(ConvergecastTimeMaxDone *ev);
  ~ConvergecastTimeMaxDone();
  void consumeBlockEvent();
  const virtual string getEventName();

};

#endif
