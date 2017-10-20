#include "localClock.h"

#include "qclock.h"
#include "scheduler.h"

#include "layers.h"
#include "utils/def.h"

#include "simulation.h"

//#define DEBUG_LOCAL_CLOCK

LocalClock::LocalClock(Layers &l): layers(l) {

}

LocalClock::LocalClock(const LocalClock &lc): layers(lc.layers) {
  
}

LocalClock::~LocalClock() {

}

Time LocalClock::getLocalTime(Time simTime, bool msResolution) {
  Time localTime = layers.module->getLocalTime(simTime);
  
  if (msResolution) {
    localTime = US_TO_US_MS_RESOLUTION(localTime);
  }

#ifdef DEBUG_LOCAL_CLOCK
  MY_CERR << "SimTime: " << simTime
	  << " => localTime: " << localTime
	  << endl;
#endif
  
  return localTime;
}

Time LocalClock::getSimTime(Time localTime) {
  Time simTime = layers.module->getSimulationTime(localTime);
  return simTime;
}

Time LocalClock::getSimTimeLocalDelay(Time delay) {
  Time simNow = BaseSimulator::getScheduler()->now();
  Time localNow = layers.module->getLocalTime(simNow);
  Time localTime = localNow + delay;
  
#ifdef FAST_APPROX_LOCAL_TIME_TO_SIM_TIME_CONV
  BaseSimulator::QClock* _clock =
    (BaseSimulator::QClock*) layers.module->clock; 
  Time simTime = _clock->BaseSimulator::QClock::getSimulationTime(localTime);
#else
  Time simTime = layers.module->getSimulationTime(localTime);
  Time localTimeCheck = layers.module->getLocalTime(simTime);
  assert(std::max(localTime,localTimeCheck)-min(localTime,localTimeCheck) <= 1);
#endif  
  return simTime;
}
