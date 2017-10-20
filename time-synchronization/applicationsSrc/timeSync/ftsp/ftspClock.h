#ifndef FTSP_CLOCK_H_
#define FTSP_CLOCK_H_

#include "../synchronization/clock.h"

#include "ftspDef.h"

class Layers;

namespace Synchronization {

  class FTSPClock: public Clock {
  public:
    float skew;
    uint32_t localAverage;
    int32_t offsetAverage;
    uint8_t numEntries; // the number of full entries in the table
    uint8_t tableEntries;
    uint8_t numErrors;
    
    FTSP_PROTOCOL::TableItem  table[FTSP_PROTOCOL::MAX_ENTRIES];
        
    FTSPClock(Layers &l);
    FTSPClock(const FTSPClock &c);
    ~FTSPClock();

    Time _getTime(Time t, bool msResolution);
    
    void _synchronize(Point &p);
    void _setTime(Point &p);

    bool is_synced();
    void checkAndAddNewEntry(Point &pUS);
    void calculate();

    void clearTable();
    void reinit();
    
  };

}
#endif
