#ifndef BFS_TREE_EVENTS_H_
#define BFS_TREE_EVENTS_H_

#include "events.h"

class SpanningTreeConstructedEvent;
typedef std::shared_ptr<SpanningTreeConstructedEvent> SpanningTreeConstructedEventPtr;

class SpanningTreeConstructedEvent : public BlockEvent {
public:
  
  SpanningTreeConstructedEvent(Time t, BaseSimulator::BuildingBlock *conBlock);
  SpanningTreeConstructedEvent(SpanningTreeConstructedEvent *ev);
  ~SpanningTreeConstructedEvent();
  void consumeBlockEvent();
  const virtual string getEventName();

};

#endif
