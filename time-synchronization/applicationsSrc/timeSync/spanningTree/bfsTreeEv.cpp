#include "bfsTreeEv.h"

#include "../appEventsID.h"

SpanningTreeConstructedEvent::SpanningTreeConstructedEvent(Time t, BaseSimulator::BuildingBlock *conBlock):
  BlockEvent(t, conBlock) {
  eventType = EVENT_SPANNING_TREE_CONSTRUCTED;
}
  
SpanningTreeConstructedEvent::SpanningTreeConstructedEvent(SpanningTreeConstructedEvent *ev): BlockEvent(ev) {
  
}
  
SpanningTreeConstructedEvent::~SpanningTreeConstructedEvent() {

}
  
void SpanningTreeConstructedEvent::consumeBlockEvent() {
  concernedBlock->scheduleLocalEvent(EventPtr(new SpanningTreeConstructedEvent(this)));
}
  
const string SpanningTreeConstructedEvent::getEventName() {
  return("SpanningTree Constructed Event");
}

