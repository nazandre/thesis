#include "mleClock.h"

#include <cmath>
#include <iomanip>

#include "scheduler.h"
#include "layers.h"

#include "../utils/def.h"
#include "../simulation.h"

//#define DEBUG_MLE_COMPUTATION

#define MAX_TIMESTAMPS_IN_MEMORY 5

#define P_I(i) ((mleDouble_t)(t2[i] + t3[i]))
#define S_I(i) ((mleDouble_t)(t1[i] + t4[i]))

using namespace std;

namespace Synchronization {

  // MLEClock class
  MLEClock::MLEClock(Layers &l): Clock(l) {
    offset = 0;
    skew = 1.0;
    index = 0;
    nbSync = 0;
    offsetHW = 0;

    t1 = std::vector<Time> (MAX_TIMESTAMPS_IN_MEMORY,0);
    t2 = std::vector<Time> (MAX_TIMESTAMPS_IN_MEMORY,0);
    t3 = std::vector<Time> (MAX_TIMESTAMPS_IN_MEMORY,0);
    t4 = std::vector<Time> (MAX_TIMESTAMPS_IN_MEMORY,0);
  }

  MLEClock::MLEClock(const MLEClock &mc): Clock(mc) {
    offset = mc.offset;
    skew = mc.skew;
    index = mc.index;
    nbSync = mc.nbSync;
    
    t1 = mc.t1;
    t2 = mc.t2;
    t3 = mc.t3;
    t4 = mc.t4;
  }
  
  MLEClock::~MLEClock() {

  }

  Time MLEClock::_getTime(Time local, bool msResolution) { // t: local
    Time localMS = local / 1000;
    mleDouble_t t = round(skew*(mleDouble_t)localMS) + offset;
    t = std::max(t,mleDouble_t(0.));
    Time res = Time(t)*1000;
    return res;
  }

  Time MLEClock::getTimeLast(Time local, bool msResolution) {
    
    Time localMS = local/1000;
    mleDouble_t t = round(skew*(mleDouble_t)localMS) + offsetHW;
    t = std::max(t,(mleDouble_t)0.0);
    Time res = Time(t)*1000;
    return res;
  }
 
  void MLEClock::_synchronize(Point &p) {
    assert(false);
  }

  void MLEClock::_setTime(Point &p) {
    assert(false);
  }
  
  void MLEClock::performMLE() {
    mleDouble_t sumPow2 = 0, sum2 = 0, diff2 = 0;
    mleDouble_t coeff = 4*MAX_TIMESTAMPS_IN_MEMORY;
    int i = 0;
    mleDouble_t theta[2] = {0,0};
    mleDouble_t p_i = 0, s_i = 0;
    mleDouble_t den = 0;
    
    for (i = 0; i < MAX_TIMESTAMPS_IN_MEMORY; i++) {
      p_i = P_I(i);
      diff2 += -2*p_i;
      sumPow2 += pow(p_i,2);
      sum2 += 2*p_i;
    }

    den = -pow(diff2,2) + coeff * sumPow2;
      
    for (i = 0; i < MAX_TIMESTAMPS_IN_MEMORY; i++) {
      s_i = S_I(i);
      p_i = P_I(i);
      theta[0] += (coeff*p_i - 2*sum2) * s_i;
      theta[1] += (p_i*sum2 - 2 * sumPow2) * s_i;
    }

    theta[0] /= den;
    theta[1] /= den;

    skew = 1.0/theta[0];
    offset = theta[1]/theta[0];    
  }

  void MLEClock::synchronize(Time __t1, Time __t2, Time __t3, Time __t4, Time __globalTime) {

    Time _t1 = __t1/1000; 
    Time _t2 = __t2/1000;
    Time _t3 = __t3/1000;
    Time _t4 = __t4/1000;
    Time _globalTime = __globalTime/1000;
    
    nbSync++;
    
    t1[index] = _t1;
    t2[index] = _t2;
    t3[index] = _t3;
    t4[index] = _t4;
    
    index = (index+1) % MAX_TIMESTAMPS_IN_MEMORY;

    if (nbSync >= MAX_TIMESTAMPS_IN_MEMORY) {
      // estimate clock skew + offset
      performMLE();
    } else { // traditionnal rtt
      offset = ( ( (int64_t)_t2 - (int64_t)_t1 )
		 +
		 ( (int64_t)_t3 - (int64_t)_t4 )
		 
		 )/2;
      skew = 1;
    }

    offsetHW = (int64_t)_globalTime - (int64_t) round(skew*(mleDouble_t) _t4);
    
    getTimeNow(MS_RESOLUTION_CLOCK); // update maxReached
    
  };

}
