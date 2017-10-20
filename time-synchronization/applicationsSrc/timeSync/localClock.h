#ifndef LOCAL_CLOCK_H_
#define LOCAL_CLOCK_H_

#include "clock.h"

class Layers;

class LocalClock {
public:
  Layers &layers;

  LocalClock(Layers &l);
  LocalClock(const LocalClock &lc);
  ~LocalClock();

  Time getLocalTime(Time simTime, bool msResolution = true);
  Time getSimTime(Time localTime);

  Time getSimTimeLocalDelay(Time delay);
};

#endif
