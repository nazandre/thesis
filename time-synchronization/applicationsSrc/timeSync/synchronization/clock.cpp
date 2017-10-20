#include "clock.h"

#include <cmath>
#include <iomanip>

#include "scheduler.h"
#include "layers.h"

#include "../utils/def.h"
#include "../simulation.h"

//#define DEBUG_LINEAR_REGRESSION

using namespace std;

namespace Synchronization {

  // Clock class
  Clock::Clock(Layers &l): layers(l) {
    assert(layers.localClock);
    maxReached = 0;
    //cerr << "sizeof(myDouble_t)" << sizeof(myDouble_t) << " vs "
//	 << "sizeof(float)" << sizeof(float)
//	 << endl;
  }

  Clock::Clock(const Clock &c): layers(c.layers) {
    maxReached = c.maxReached;
  }

  Clock::~Clock() {

  }

  Time Clock::getTime(Time localTime, bool msResolution) {
    Time globalTime = _getTime(localTime, msResolution);

    if (msResolution) {
      globalTime = US_TO_US_MS_RESOLUTION(globalTime);
    }
 
    return globalTime;
  }

  Time Clock::getTimeLast(Time local, bool msResolution) {
    return getTime(local,msResolution);
  }
  
  Time Clock::getTimeNow(bool msResolution) {
    Time simTimeNow = BaseSimulator::getScheduler()->now();
    Time localTime = layers.localClock->getLocalTime(simTimeNow, msResolution); 
    Time globalTime = getTime(localTime, msResolution);

    if (msResolution) {
      globalTime = US_TO_US_MS_RESOLUTION(globalTime);
    }
    
    updateMaxReached(globalTime);
    
    globalTime = max(globalTime, maxReached);
    
    return globalTime;
  }
  
  void Clock::updateMaxReached(Time globalTime) {
    if(MAX_REACHED_ENABLED) {
      maxReached = max(maxReached,globalTime);
    }
  }

  void Clock::synchronize(Point &p) {
    lastSync = p;
    _synchronize(p);
  }

  void Clock::setTime(Point &p) {
    lastSync = p;
    _setTime(p);
  } 
  
  // OffsetClock class
  OffsetClock::OffsetClock(Layers &l): Clock(l) {
    offset = 0;
  }

  OffsetClock::OffsetClock(const OffsetClock &oc): Clock(oc) {
    offset = oc.offset;
  }
  
  OffsetClock::~OffsetClock() {

  }

  Time OffsetClock::_getTime(Time local, bool msResolution) { // t: local
    int64_t global = local + offset;
    global = std::max(global,(int64_t)0);
    return global;
  }
 
  void OffsetClock::_synchronize(Point &p) {
    offset = (int64_t)p.global - (int64_t)p.local;
    getTimeNow(MS_RESOLUTION_CLOCK); // update maxReached
  }

  void OffsetClock::_setTime(Point &p) {
    synchronize(p);
  }

  // Linear clock
  LinearClock::LinearClock(Layers &l, size_t _max): Clock(l) {
    y0 = 1;
    x0 = 0;
    max = _max;
    offsetHW = 0;
  }

  LinearClock::LinearClock(const LinearClock &oc): Clock(oc) {
    y0 = oc.y0;
    x0 = oc.x0;
    max = oc.max;
    offsetHW = oc.offsetHW;
  }
  
  LinearClock::~LinearClock() {

  }

  Time LinearClock::_getTime(Time local, bool msResolution) { // t: local
    Time localMS = local / 1000;
    myDouble_t t = round(y0*(myDouble_t)localMS) + x0;
    t = std::max(t,myDouble_t(0.));
    Time res = Time(t)*1000;
    return res;
  }

  Time LinearClock::getTimeLast(Time local, bool msResolution) {
    Time localMS = local/1000;
    myDouble_t t = round(y0*(myDouble_t)localMS) + offsetHW;
    t = std::max(t,myDouble_t(0.));
    Time res = Time(t)*1000;
    return res;
  }
  
  void LinearClock::_synchronize(Point &p1) {

    Point p = p1;
    
    p.local /= 1000;
    p.global /= 1000;

    getTimeNow(MS_RESOLUTION_CLOCK); // update maxReached

    if (p.global == 0 || p.local == 0) {
      y0 = 1.0;
      x0 = (myDouble_t)p.global - (myDouble_t)p.local;
    } else {
      
      int index = (int) points.size() - 1;
      if (index >= 0 && points[index].local == p.local) {
	assert(points[index].global != p.global);
	points[index].global = (points[index].global + p.global)/2;
      } else {
	points.push_back(p);
      }

      /*
      if (index > 0 && points[index].global == p.global) {
	points[index].local = (points[index].global + p.global)/2;
	}*/

      if (points.size() > max) {
	points.erase(points.begin());
      }
      
      if (points.size() == 1) {
	/*
	y0 = (myDouble_t)p.global / (myDouble_t)p.local;
	x0 = 0;*/

	y0 = 1.0;
	x0 = (myDouble_t)p.global - (myDouble_t)p.local;
      } else {
	int32_t o = p.global - p.local;
        Time simTimeNow = BaseSimulator::getScheduler()->now();
	Time localTime = layers.localClock->getLocalTime(simTimeNow, MS_RESOLUTION_CLOCK); 
	Time globalTime = (int64_t) localTime + o;
	maxReached = std::max(maxReached,globalTime);	
	performLinearRegression();
      }
    }

    //
    offsetHW = (int32_t)p.global - (int32_t) round(y0*(myDouble_t)p.local);

    //MY_CERR << "y0: " << y0 << ", ohw: " << offsetHW << endl;
    /*if (points.size() == 1) {
      if (p.global != 0 && p.local != 0) {
	x0 = (myDouble_t)p.global - (myDouble_t)p.local;
	y0 = 1.0;
      }
    } else {
      performLinearRegression();
      }*/
    
    //getTimeNow(MS_RESOLUTION_CLOCK); // update maxReached
  }

  void LinearClock::performLinearRegression() {
    // Linear regression (same as in hardware bb)
    // https://github.com/claytronics/oldbb/blob/master/build/src-bobby/system/clock.bb
    // x: local time
    // y: global time
    myDouble_t xAvg = 0, yAvg = 0;
    myDouble_t sum1 = 0, sum2 = 0;

    //MY_CERR << "linear regression!" << endl;
    //printLinearRegressionTable();
    
    for (vector<Point>::iterator it = points.begin() ; it != points.end(); it++){
      xAvg += (myDouble_t)it->local;
      yAvg += (myDouble_t)it->global;
    }

    xAvg = xAvg/ ((myDouble_t)points.size());
    yAvg = yAvg/ ((myDouble_t)points.size());

    for (vector<Point>::iterator it = points.begin() ; it != points.end(); it++){
      sum1 += ((myDouble_t)it->local - xAvg) * ((myDouble_t)it->global - yAvg);
      sum2 += ((myDouble_t)it->local - xAvg) * ((myDouble_t)it->local - xAvg);
	//pow((myDouble_t)it->local - xAvg,2);
    }

    y0 = sum1/sum2;
    x0 = yAvg - y0 * xAvg;
    
    //cerr << "\ty0: " << y0 << endl;
    //cerr << "\tx0: " << x0 << endl;
    //getchar();
    //cerr << "===============" << endl;
  }

  void LinearClock::_setTime(Point &p) {
    x0 = ((myDouble_t)p.global) - ((myDouble_t)p.local);
    x0 /= 1000;
    offsetHW = x0;
    getTimeNow(MS_RESOLUTION_CLOCK); // update maxReached
  }

  void LinearClock::clear() {
    points.clear();
  }

  void LinearClock::printLinearRegressionTable() {
    MY_CERR << "\t Linear regression table: " << endl;
    for (size_t i = 0; i < points.size(); i++) {
      cerr << "\t\t Local time " << points[i].local
	   << " => "
	   << "global time " << points[i].global
	   << endl;
    }
  }
 
}
