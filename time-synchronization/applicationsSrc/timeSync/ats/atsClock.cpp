#include "atsClock.h"

#include "../layers.h"

#include "../simulation.h"

namespace Synchronization {
  
  ATSClock::ATSClock(Layers &l): Clock(l) {
    skew = 1.0;
    offset = 0;
  }
  
  ATSClock::ATSClock(const ATSClock &c): Clock(c), skew(c.skew), offset(c.offset) {

  }
  
  ATSClock::~ATSClock() {

  }
  
  Time ATSClock::_getTime(Time local, bool msResolution) {
    myDouble_t t = skew * ((myDouble_t)local) + offset;

    t = std::max(t,myDouble_t(0.0));
    
    return t;
  }
  
  void ATSClock::_synchronize(Point &p) {
    assert(false);
  }
  
  void ATSClock::_setTime(Point &p) {
    assert(false);
  }

  void ATSClock::updateSkew(myDouble_t relativeNeighborSkew, myDouble_t neighborSkew) {
    myDouble_t p = ATS_PROBA_SKEW_UPDATE;
    skew =  p * skew + (1.0 - p) * relativeNeighborSkew *neighborSkew;
  }

  void ATSClock::updateOffset(Time here, Time neighbor) {
    myDouble_t p = ATS_PROBA_OFFSET_UPDATE;
    myDouble_t diff = ((myDouble_t) neighbor) - ((myDouble_t) here);
    offset = offset + (1.0-p)*diff;
  }
  
}
