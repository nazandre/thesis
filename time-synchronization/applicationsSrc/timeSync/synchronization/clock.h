#ifndef SYNCHRONIZATION_CLOCK_H_
#define SYNCHRONIZATION_CLOCK_H_

#include <vector>

#include "../utils/def.h"

#include "tDefs.h"
#include "point.h"

class Layers;

namespace Synchronization {
  
  // clocks:
  class Clock {
  public:
    // to avoid run backward + to jump to estimated global time
    Time maxReached;
    Layers &layers;
    Point lastSync;
    
    Clock(Layers &l);
    Clock(const Clock &c);
    virtual ~Clock();

    // input: Time t => local time
    // returned : Time => synchronized global time (estimation)
    Time getTime(Time t, bool msResolution = true);

    void synchronize(Point &p);
    void setTime(Point &p);
    
    Time getTimeNow(bool msResolution = true); // update max reached

    void updateMaxReached(Time globalTime);
    
    virtual Time _getTime(Time t, bool msResolution) = 0;
    virtual void _synchronize(Point &p) = 0;
    virtual void _setTime(Point &p) = 0;
    virtual Time getTimeLast(Time t, bool msResolution);
    
    virtual void clear() {};

    virtual void synchronize(Time t1, Time t2, Time t3, Time t4, Time globalTime) {};
  };

  class OffsetClock: public Clock {
  public:
    int64_t offset;
    
    OffsetClock(Layers &l);
    OffsetClock(const OffsetClock &oc);
    ~OffsetClock();

    Time _getTime(Time t, bool msResolution);
    void _synchronize(Point &p);
    void _setTime(Point &p);
  };

  class LinearClock: public Clock {
  public:
    myDouble_t y0; 
    int32_t x0;
    int64_t offsetHW; 
    size_t max;
    std::vector<Point> points;
    
    LinearClock(Layers &l,size_t _max);
    LinearClock(const LinearClock &lc);
    ~LinearClock();

    Time getTimeLast(Time t, bool msResolution);
    Time _getTime(Time t, bool msResolution);
    void _synchronize(Point &p);
    void _setTime(Point &p);
    
    void performLinearRegression();

    void clear();
    void printLinearRegressionTable();

  };
}

#endif
