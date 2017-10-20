#include "synchronizationEv.h"

#include <limits>
#include <iostream>
#include <vector>

#include "scheduler.h"

#include "../appEventsID.h"

#include "../simulation.h"

// clock readings:
#include "world.h"
#include "buildingBlock.h"
#include "blockCode.h"

#include "../timeSyncBlockCode.h"
#include "../layers.h"
#include "../localClock.h"
#include "../synchronization/synchronization.h"
#include "../synchronization/clock.h"

//#define DEBUG_MIN_MAX_ERROR

//#define COMPUTE_ERROR_AVG

//#define TEST_COMPUTE_ABS_MEAN_AVG

using namespace std;
using namespace BaseSimulator;

TimeSyncEvent::TimeSyncEvent(Time t, BaseSimulator::BuildingBlock *conBlock):
  BlockEvent(t, conBlock) {
  eventType = EVENT_TIME_SYNC;
  randomNumber = conBlock->getRandomUint();
}
	
TimeSyncEvent::TimeSyncEvent(TimeSyncEvent *ev) : BlockEvent(ev) {
  randomNumber = ev->randomNumber;
}
	
TimeSyncEvent::~TimeSyncEvent() {};
	
void TimeSyncEvent::consumeBlockEvent() {
  concernedBlock->scheduleLocalEvent(EventPtr(new TimeSyncEvent(this)));
}
	
const string TimeSyncEvent::getEventName() { 
  return "TIME_SYNC EVENTS";
}


// TimeMasterElectedEvent class

TimeMasterElectedEvent::TimeMasterElectedEvent(Time t, BaseSimulator::BuildingBlock *conBlock):
  BlockEvent(t, conBlock) {
  eventType = EVENT_TIME_MASTER_ELECTED;
}
  
TimeMasterElectedEvent::TimeMasterElectedEvent(TimeMasterElectedEvent *ev): BlockEvent(ev) {
  
}
  
TimeMasterElectedEvent::~TimeMasterElectedEvent() {

}
  
void TimeMasterElectedEvent::consumeBlockEvent() {
  concernedBlock->scheduleLocalEvent(EventPtr(new TimeMasterElectedEvent(this)));
}
  
const string TimeMasterElectedEvent::getEventName() {
  return("TimeMaster Elected Event");
}

// ErrorMeasurementEvent class
ErrorMeasurementEvent::ErrorMeasurementEvent(Time t):
  Event(t) {
  eventType = EVENT_ERROR_MEASUREMENT;
}
  
ErrorMeasurementEvent::ErrorMeasurementEvent(ErrorMeasurementEvent *ev): Event(ev) {
  
}
  
ErrorMeasurementEvent::~ErrorMeasurementEvent() {

}
  
void ErrorMeasurementEvent::consume() {
  World *world = getWorld(); 
  map<bID, BuildingBlock*> &map = world->getMap();
  std::map<bID, BuildingBlock*>::iterator it;
  BuildingBlock *b = NULL;
  TimeSyncBlockCode *blockCode = NULL;
  Synchronization::Node *synchronization = NULL;
  Layers *layers = NULL;

  Time simTimeNow = BaseSimulator::getScheduler()->now();
  Time localTimeNow = 0;
  Time globalTimeNow = 0;

  Time minGlobalTimeNow = std::numeric_limits<Time>::max();
  Time maxGlobalTimeNow = std::numeric_limits<Time>::min();
  
  ofstream output;

  Time error;

  long double n = world->getSize();
  long double mean = 0;
  long double aMean = 0; // https://en.wikipedia.org/wiki/Mean_absolute_difference
  long double sd = 0;
  int i = 0;
  
#ifdef COMPUTE_ERROR_AVG
  std::vector<long double> globalTimes(n);
  std::vector<long double>::iterator itv;
#endif
  
  for(it = map.begin(); it != map.end(); ++it) {
    b = it->second;
    blockCode = (TimeSyncBlockCode*) b->blockCode;
    layers = &blockCode->layers;
    synchronization = layers->synchronization;
    
    localTimeNow = layers->localClock->getLocalTime(simTimeNow,
						    MS_RESOLUTION_CLOCK);
    globalTimeNow = synchronization->clock->getTime(localTimeNow,
						    MS_RESOLUTION_CLOCK);
#ifdef DEBUG_MIN_MAX_ERROR
    cerr << b->blockId << ": " << endl;
    cerr << "\t Local time: " << localTimeNow << endl;
    cerr << "\t Global time: " << globalTimeNow << endl;
    
    Synchronization::LinearClock *clock = (Synchronization::LinearClock*) synchronization->clock;
    cerr << "\t Skew (y0): " << clock->y0 << endl;
    cerr << "\t Offset (x0): " << clock->x0 << endl;
#endif
    
    minGlobalTimeNow = std::min(minGlobalTimeNow,globalTimeNow);
    maxGlobalTimeNow = std::max(maxGlobalTimeNow,globalTimeNow);

#ifdef COMPUTE_ERROR_AVG
    mean += globalTimeNow;
    globalTimes[i] = globalTimeNow;
    i++;
#endif
  }
  
  error = maxGlobalTimeNow - minGlobalTimeNow;

#ifdef COMPUTE_ERROR_AVG
  mean /= n;
  for (itv = globalTimes.begin(); itv != globalTimes.end(); itv++) {
    sd += pow(*itv -mean,2);
    aMean += abs(*itv - mean);
  }
  
  sd = sqrt(1.0/n * sd);
  aMean /= n;
#endif
  
#ifdef DEBUG_MIN_MAX_ERROR
  cerr << "***" << endl;
  cerr << "max: " << maxGlobalTimeNow << endl;
  cerr << "min: " << minGlobalTimeNow << endl;
  cerr << "error: " << error << endl;
  //getchar();
#endif

#ifdef TEST_COMPUTE_ABS_MEAN_AVG
  long double aMeanTest = 0;
  std::vector<long double>::iterator itv2;
  for (itv = globalTimes.begin(); itv != globalTimes.end(); itv++) {
    for (itv2 = globalTimes.begin(); itv2 != globalTimes.end(); itv2++) {
      aMeanTest += abs(*itv-*itv2);
    }
  }
  aMeanTest /= pow(n, 2);
#endif
    
  output.open(ERROR_MEASUREMENT_FILE_NAME, fstream::app);

  output << simTimeNow << " "
	 << error 
#ifdef COMPUTE_ERROR_AVG    
	 << " " << sd << " "
	 << aMean << " "
#ifdef TEST_COMPUTE_ABS_MEAN_AVG
	 << aMeanTest
#endif
#endif
	 << endl;
  
  output.close(); 
  
  if (simTimeNow < SIMULATION_END) {
    scheduleNext();
  }
  
}

void ErrorMeasurementEvent::init() {
  ofstream output;
  Time first = 0;  
  Time beforeSyncStart = 10*ONE_SECOND;
  output.open(ERROR_MEASUREMENT_FILE_NAME, fstream::trunc);
  output.close();

  if (SYNCHRONIZATION_START > beforeSyncStart) {
    first = SYNCHRONIZATION_START - beforeSyncStart;
  }
  
  BaseSimulator::getScheduler()->schedule(new ErrorMeasurementEvent(first)); 
}
  
const string ErrorMeasurementEvent::getEventName() {
  return("ErrorMeasurement Event");
}

void ErrorMeasurementEvent::scheduleNext() {
  Time next = BaseSimulator::getScheduler()->now() +
    ERROR_MEASUREMENT_PERIOD;
  
  BaseSimulator::getScheduler()->schedule(new ErrorMeasurementEvent(next)); 

}
