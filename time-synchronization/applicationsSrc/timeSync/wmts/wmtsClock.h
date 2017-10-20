#ifndef WMTS_CLOCK_H_
#define WMTS_CLOCK_H_

#include "../synchronization/clock.h"

class Layers;

namespace Synchronization {

  class WMTSClock: public Clock {
  public:
    wmtsDouble_t skew;
    wmtsDouble_t offset;

    WMTSClock(Layers &l);
    WMTSClock(const WMTSClock &c);
    ~WMTSClock();

    Time _getTime(Time t, bool msResolution);
    void _synchronize(Point &p);
    void _setTime(Point &p);

  };

}
#endif
