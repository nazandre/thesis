#include <limits>
#include <iostream>
#include <vector>

#include "scheduler.h"

#include "appEv.h"
#include "../appEventsID.h"
#include "../simulation.h"

using namespace std;
using namespace BaseSimulator;

// TimeMasterElectedEvent class

AppRoundEvent::AppRoundEvent(Time t, BaseSimulator::BuildingBlock *conBlock):
  BlockEvent(t, conBlock) {
  eventType = EVENT_APP_ROUND;
}
  
AppRoundEvent::AppRoundEvent(AppRoundEvent *ev): BlockEvent(ev) {
  
}
  
AppRoundEvent::~AppRoundEvent() {

}
  
void AppRoundEvent::consumeBlockEvent() {
  concernedBlock->scheduleLocalEvent(EventPtr(new AppRoundEvent(this)));
}
  
const string AppRoundEvent::getEventName() {
  return("AppRound Event");
}
