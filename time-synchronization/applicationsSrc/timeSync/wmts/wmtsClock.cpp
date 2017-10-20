#include "wmtsClock.h"

#include "../layers.h"

#include "../simulation.h"

namespace Synchronization {
  
  WMTSClock::WMTSClock(Layers &l): Clock(l) {
    skew = 1.0;
    offset = 0;
  }
  
  WMTSClock::WMTSClock(const WMTSClock &c): Clock(c), skew(c.skew), offset(c.offset) {

  }
  
  WMTSClock::~WMTSClock() {

  }
  
  Time WMTSClock::_getTime(Time local, bool msResolution) {
    wmtsDouble_t t = skew * ((wmtsDouble_t)local) + offset;

    t = std::max(t,wmtsDouble_t(0.0));
    
    return t;
  }
  
  void WMTSClock::_synchronize(Point &p) {
    assert(false);
  }
  
  void WMTSClock::_setTime(Point &p) {
    assert(false);
  }
  
}
