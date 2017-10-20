#ifndef FTSP_DEF_H_
#define FTSP_DEF_H_

#include "../simulation.h"

namespace Synchronization {

  namespace FTSP_PROTOCOL {

// (us) how often send the beacon msg    
//#define BEACON_RATE SYNC_PERIOD_1_US
    
    enum {
        MAX_ENTRIES           = 5,              // number of entries in the table
        ROOT_TIMEOUT          = 5,              //time to declare itself the root if no msg was received (in sync periods)
        IGNORE_ROOT_MSG       = 3,              // after becoming the root ignore other roots messages (in send period)
        ENTRY_VALID_LIMIT     = 3,              // number of entries to become synchronized
        ENTRY_SEND_LIMIT      = 3,              // number of entries to send sync messages
        ENTRY_THROWOUT_LIMIT  = 5000,            // (ms) if time sync error is bigger than this clear the table
    };

    typedef struct TableItem
    {
      uint8_t     state;
      uint64_t    localTime;
      int64_t     timeOffset; // globalTime - localTime
    } TableItem;
    
    enum {
        ENTRY_EMPTY = 0,
        ENTRY_FULL = 1,
    };
  }
}

#endif
