#include "ftspClock.h"
#include "../layers.h"
#include "../simulation.h"

//#define ENABLE_FILTERING

namespace Synchronization {

  using namespace FTSP_PROTOCOL;
  
  FTSPClock::FTSPClock(Layers &l): Clock(l) {
    reinit();
  }
  
  FTSPClock::FTSPClock(const FTSPClock &c): Clock(c) {
    assert(false);
  }
  
  FTSPClock::~FTSPClock() {

  }

  void FTSPClock::reinit() {
    skew = 1.0;
    localAverage = 0;
    offsetAverage = 0;
    numErrors = 0;
    
    clearTable();
  }
  
  void FTSPClock::clearTable() {
    int8_t i;
    for(i = 0; i < MAX_ENTRIES; ++i) {
      table[i].state = ENTRY_EMPTY;
      table[i].localTime = 0;
      table[i].timeOffset = 0;
    }
    
    numEntries = 0;
  }
  
  Time FTSPClock::_getTime(Time local, bool msResolution) {
    assert(msResolution);
    
    Time localMS = local / 1000;
    int64_t globalMS = localMS + offsetAverage + (int64_t)(skew * (int64_t)(localMS - localAverage));
    globalMS = std::max(globalMS,(int64_t)0);
    Time res = Time(globalMS)*1000;

    if (!is_synced()) {
      res = local;
    }
    
    return res;
  }
  
  void FTSPClock::_synchronize(Point &p) {
    assert(false);
  }
  
  void FTSPClock::_setTime(Point &p) {
    assert(false);
  }

  void FTSPClock::calculate() {
    float newSkew = skew;
    uint32_t newLocalAverage;
    int32_t newOffsetAverage;
    int32_t localAverageRest;
    int32_t offsetAverageRest;
    
    int64_t localSum;
    int64_t offsetSum;
    
    int8_t i;
    
    for(i = 0; i < MAX_ENTRIES && table[i].state != ENTRY_FULL; ++i);
    
    if( i >= MAX_ENTRIES )  // table is empty
      return;
    /*
      We use a rough approximation first to avoid time overflow errors. The idea
        is that all times in the table should be relatively close to each other.
    */
    newLocalAverage = table[i].localTime;
    newOffsetAverage = table[i].timeOffset;
    
    localSum = 0;
    localAverageRest = 0;
    offsetSum = 0;
    offsetAverageRest = 0;
    
    while( ++i < MAX_ENTRIES ) {

      if( table[i].state == ENTRY_FULL ) {
	/*
	  This only works because C ISO 1999 defines the signe for modulo the same as for the Dividend!
                */ 
	localSum += (int32_t)(table[i].localTime - newLocalAverage) / tableEntries;
	localAverageRest += (table[i].localTime - newLocalAverage) % tableEntries;
	offsetSum += (int32_t)(table[i].timeOffset - newOffsetAverage) / tableEntries;
	offsetAverageRest += (table[i].timeOffset - newOffsetAverage) % tableEntries;
      }
    }
    
    newLocalAverage += localSum + localAverageRest / tableEntries;
    newOffsetAverage += offsetSum + offsetAverageRest / tableEntries;
    
    localSum = offsetSum = 0;
    
    for(i = 0; i < MAX_ENTRIES; ++i)
      if( table[i].state == ENTRY_FULL ) {
	int32_t a = table[i].localTime - newLocalAverage;
	int32_t b = table[i].timeOffset - newOffsetAverage;
	
	localSum += (int64_t)a * a;
	offsetSum += (int64_t)a * b;
      }
    
    if( localSum != 0 ) {
      newSkew = (float)offsetSum / (float)localSum;
    }
    
    skew = newSkew;
    offsetAverage = newOffsetAverage;
    localAverage = newLocalAverage;
    numEntries = tableEntries;
  }
    
  bool FTSPClock::is_synced() {
    if (numEntries >= ENTRY_VALID_LIMIT)
      return true;
    else
      return false;
  }
  
  void FTSPClock::checkAndAddNewEntry(Point &pUS)
    {
        int8_t i, freeItem = -1, oldestItem = 0;
        uint32_t age, oldestTime = 0;
        int32_t timeError;
	Point p = Point(pUS.local/1000,pUS.global/1000);
	
        // clear table if the received entry's been inconsistent for some time
        Time hereGlobal = _getTime(pUS.local,MS_RESOLUTION_CLOCK); // us
	hereGlobal /= 1000; // get in ms
	
	timeError = hereGlobal - p.global;

#ifdef ENABLE_FILTERING
        if( (is_synced()) &&
            (timeError > ENTRY_THROWOUT_LIMIT || timeError < -ENTRY_THROWOUT_LIMIT)) {
	  MY_CERR << "BAD READINGS: " << timeError << endl;
	  if (++numErrors>3) {
	      clearTable();
	  }
            return; // don't incorporate a bad reading
	}
#endif

        tableEntries = 0; // don't reset table size unless you're recounting
        numErrors = 0;

        for(i = 0; i < MAX_ENTRIES; ++i) {
	  age = p.local - table[i].localTime;

            //logical time error compensation
            if( age >= 0x7FFFFFFFL )
                table[i].state = ENTRY_EMPTY;

            if( table[i].state == ENTRY_EMPTY )
                freeItem = i;
            else
                ++tableEntries;

            if( age >= oldestTime ) {
                oldestTime = age;
                oldestItem = i;
            }
        }

        if( freeItem < 0 )
            freeItem = oldestItem;
        else
            ++tableEntries;

        table[freeItem].state = ENTRY_FULL;
	
        table[freeItem].localTime = p.local;
        table[freeItem].timeOffset = (int64_t)p.global - (int64_t)p.local;
    }
  
}
