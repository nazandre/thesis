#include "ctmEv.h"

#include "../appEventsID.h"

ConvergecastTimeMaxDone::ConvergecastTimeMaxDone(Time t, BaseSimulator::BuildingBlock *conBlock):
  BlockEvent(t, conBlock) {
  eventType = EVENT_CONVERGECAST_TIME_MAX_DONE;
}
  
ConvergecastTimeMaxDone::ConvergecastTimeMaxDone(ConvergecastTimeMaxDone *ev): BlockEvent(ev) {
  
}
  
ConvergecastTimeMaxDone::~ConvergecastTimeMaxDone() {

}
  
void ConvergecastTimeMaxDone::consumeBlockEvent() {
  concernedBlock->scheduleLocalEvent(EventPtr(new ConvergecastTimeMaxDone(this)));
}
  
const string ConvergecastTimeMaxDone::getEventName() {
  return("ConvergecastTimeMaxDone Event");
}

