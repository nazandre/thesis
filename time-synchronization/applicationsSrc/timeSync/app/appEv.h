#ifndef APP_EVENTS_H_
#define APP_EVENTS_H_

#include "events.h"

class AppRoundEvent;
typedef std::shared_ptr<AppRoundEvent> AppRoundEventPtr;

class AppRoundEvent : public BlockEvent {
public:
  
  AppRoundEvent(Time t, BaseSimulator::BuildingBlock *conBlock);
  AppRoundEvent(AppRoundEvent *ev);
  ~AppRoundEvent();
  void consumeBlockEvent();
  const virtual string getEventName();
};

#endif
