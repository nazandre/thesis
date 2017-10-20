#ifndef ATS_CLOCK_H_
#define ATS_CLOCK_H_

#include "../synchronization/clock.h"

class Layers;

namespace Synchronization {

  class ATSClock: public Clock {
  public:
    myDouble_t skew;
    myDouble_t offset;

    ATSClock(Layers &l);
    ATSClock(const ATSClock &c);
    ~ATSClock();

    Time _getTime(Time t, bool msResolution);
    void _synchronize(Point &p);
    void _setTime(Point &p);

    void updateSkew(myDouble_t relativeNeighborSkew, myDouble_t neighborSkew);
    void updateOffset(Time here, Time neighbor);
  };

}
#endif
