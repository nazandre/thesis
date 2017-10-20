#ifndef MLE_SYNCHRONIZATION_CLOCK_H_
#define MLE_SYNCHRONIZATION_CLOCK_H_

#include "clock.h"

#include <vector>

class Layers;

typedef double mleDouble_t;

namespace Synchronization {
  
class MLEClock: public Clock {
  public:
  mleDouble_t skew;
  mleDouble_t offset;
  int64_t offsetHW;
  int index;
  int nbSync;
  
  std::vector<Time> t1;
  std::vector<Time> t2;
  std::vector<Time> t3;
  std::vector<Time> t4;
  
  MLEClock(Layers &l);
  MLEClock(const MLEClock &oc);
  ~MLEClock();

  Time _getTime(Time t, bool msResolution);
  void _synchronize(Point &p);
  void _setTime(Point &p);

  Time getTimeLast(Time t, bool msResolution); 
  
  void synchronize(Time t1, Time t2, Time t3, Time t4,Time globalTime);
  void performMLE();
};

}

#endif
