#ifndef SYNCHRONIZATION_EVENTS_H_
#define SYNCHRONIZATION_EVENTS_H_

#include "events.h"

class TimeSyncEvent;
typedef std::shared_ptr<TimeSyncEvent> TimeSyncEventPtr;

class TimeSyncEvent : public BlockEvent {
public:

  TimeSyncEvent(Time t, BaseSimulator::BuildingBlock *conBlock);
  TimeSyncEvent(TimeSyncEvent *ev);
  ~TimeSyncEvent();
  void consumeBlockEvent();
  const string getEventName();
};

class TimeMasterElectedEvent;
typedef std::shared_ptr<TimeMasterElectedEvent> TimeMasterElectedEventPtr;

class TimeMasterElectedEvent : public BlockEvent {
public:
  
  TimeMasterElectedEvent(Time t, BaseSimulator::BuildingBlock *conBlock);
  TimeMasterElectedEvent(TimeMasterElectedEvent *ev);
  ~TimeMasterElectedEvent();
  void consumeBlockEvent();
  const virtual string getEventName();

};

class ErrorMeasurementEvent;
typedef std::shared_ptr<ErrorMeasurementEvent> ErrorMeasurementEventPtr;

class ErrorMeasurementEvent: public Event {
public:
  
  ErrorMeasurementEvent(Time t);
  ErrorMeasurementEvent(ErrorMeasurementEvent *ev);
  ~ErrorMeasurementEvent();
  
  void consume();
  const virtual string getEventName();

  static void scheduleNext();
  static void init();
};


#endif
