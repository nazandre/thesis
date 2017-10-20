#include "ftsp.h"

#include "scheduler.h"

#include "../utils/utils.h"
#include "../simulation.h"
#include "../layers.h"

#include "../appMsgID.h"
#include "../appEventsID.h"

#include "ftspMsg.h"
#include "ftspClock.h"
#include "ftspDef.h"

#include "../synchronization/clock.h"

//#define DEBUG_FTSP_TIME_SYNC
//#define INSTANTANEOUS_RETRANSMISSION
//#define QUASI_SYNCHRONOUS_SYNC

#define FTSP_CLOCK
#define REINIT_WHEN_NEW_MATSER

// Source: http://tinyos.cvs.sourceforge.net/viewvc/tinyos/tinyos-2.x/tos/lib/ftsp/TimeSyncP.nc?content-type=text%2Fplain

#define MIN_ID_LEADER
//#define MAX_ID_LEADER

//#define DEBUG_FTSP_LEADER

#include <climits>

namespace Synchronization {

  using namespace FTSP_PROTOCOL;
  
  FTSPNode::FTSPNode(Layers &l): Node(l) {

    assert(LINEAR_REGRESSION_ENABLED);

    if (IS_RUNNING_PROTOCOL(PULSE_SYNC)) {
      clock = new LinearClock(layers,NUM_MAX_POINTS_LINEAR_MODEL);
    } else {
#ifdef FTSP_CLOCK
    clock = new FTSPClock(layers);
#else
    clock = new LinearClock(layers,NUM_MAX_POINTS_LINEAR_MODEL);
#endif
    }
	     
    round = 0;

#ifdef MIN_ID_LEADER
    myRootId = UINT_MAX; //layers.module->blockId;
#elif defined(MAX_ID_LEADER)
    myRootId = 0;
#endif
    
    heartBeats = 0;
    numIgnoredTimeouts = 0;
  }
  
  FTSPNode::FTSPNode(const FTSPNode &n): Node(n)
  {
    round = n.round;
    myRootId = n.myRootId;
    heartBeats = n.heartBeats;
    numIgnoredTimeouts = n.numIgnoredTimeouts;
    assert(false);
  }

  FTSPNode::~FTSPNode() {
  }

  void FTSPNode::startup() {
    Time now = BaseSimulator::getScheduler()->now();
    startDecentralized(); // ftsp is kind of a mix between centralized/decentralized
  }
  
  void FTSPNode::handle(MessagePtr m) {

    P2PNetworkInterface *recv = m->destinationInterface;
    
    switch(m->type) {
    case FTSP_TIME_SYNC_MSG:
      {
	FTSPTimeSyncMessagePtr msg = std::static_pointer_cast<FTSPTimeSyncMessage>(m);
	hopDistance_t hops = 1;
	Time receptionLocalTime = msg->receptionTime;
	Time receptionGlobalTime = clock->getTime(receptionLocalTime, MS_RESOLUTION_CLOCK);
	Time receptionEstimatedGlobalTime =
	  Synchronization::Node::compensateForCommDelays(msg->sendTime, msg->size(), hops);
	
	Point p(receptionLocalTime,receptionEstimatedGlobalTime);

	//Point p(receptionGlobalTime,receptionEstimatedGlobalTime);
	
#ifdef DEBUG_FTSP_LEADER
	MY_CERR << "SYNC by @" << msg->rootID << "(cur root: @" << myRootId << ")" << endl;
#endif
	
#ifdef MIN_ID_LEADER
	if (msg->rootID < myRootId) {
#elif defined(MAX_ID_LEADER)
	if (msg->rootID > myRootId) {
#endif
	  //if (myRootId == THIS_ID && numIgnoredTimeouts < IGNORE_ROOT_MSG) {
	  if (heartBeats < IGNORE_ROOT_MSG && myRootId == THIS_ID){
	    //numIgnoredTimeouts++;
	    return;
	  }
	  
	  myRootId = msg->rootID;
	  
#ifdef REINIT_WHEN_NEW_MATSER
	  if (IS_RUNNING_PROTOCOL(PULSE_SYNC)) {
	    LinearClock *_clock = (LinearClock*) clock;
	    _clock->clear();
	    _clock->x0 = 0;
	    _clock->y0 = 1.0;
	  } else if (IS_RUNNING_PROTOCOL(PULSE_SYNC)) {
	    
#ifdef FTSP_CLOCK
	    FTSPClock *_clock = (FTSPClock*) clock;
#else
	    LinearClock *_clock = (LinearClock*) clock;
#endif


#ifdef FTSP_CLOCK
	    _clock->reinit(); 
#else
	    _clock->clear();
	    _clock->x0 = 0;
	    _clock->y0 = 1.0;
#endif
	  }
#endif

#ifdef MIN_ID_LEADER
	} else if (msg->rootID > myRootId || msg->round <= round) {
#elif defined(MAX_ID_LEADER)
	} else if (msg->rootID < myRootId || msg->round <= round) {
#endif

#ifdef DEBUG_FTSP_TIME_SYNC
	  if (layers.module->blockId == 2727) {
	    MY_CERR << "FTSP TIME-SYNC MSG IGNORED!!! :" << endl;
	    cerr << "\t myRootID:" << myRootId << " vs msg->rootID: " << msg->rootID << endl;
	    cerr << "\t myRound:" << round << " vs msg->round: " << msg->round << endl;
	  }
#endif
	  return;
	}
	
	round = msg->round;

#ifdef MIN_ID_LEADER
	if (myRootId < layers.module->blockId) {
#elif defined(MAX_ID_LEADER)
	if (myRootId > layers.module->blockId) {
#endif
	  heartBeats = 0;
	}
	
#ifdef DEBUG_FTSP_TIME_SYNC
	MY_CERR << "FTSP TIME-SYNC MSG:" << endl;
	cerr << "\t rootID:" << msg->rootID << endl;
	cerr << "\t round:" << msg->round << endl;
	cerr << "\t reception local time: " << receptionLocalTime << endl;
	cerr << "\t reception estimated global time: " << receptionEstimatedGlobalTime << endl;
	cerr << "\t predicted global time: " << clock->getTime(receptionLocalTime,MS_RESOLUTION_CLOCK) << endl;
	cerr << "\t from: " << recv->connectedInterface->hostBlock->blockId << endl;
	cerr << "Interval last: " << intervalLast << endl;
#endif
	
	if (IS_RUNNING_PROTOCOL(PULSE_SYNC)) {
	  LinearClock *_clock = (LinearClock*) clock;
	  _clock->synchronize(p);
	  setHighProcessingDelays();
	  synchronizeNeighbors(myRootId, round, 0, recv);
	  setLowProcessingDelays();
	} else if (IS_RUNNING_PROTOCOL(FTSP)) {
#ifdef FTSP_CLOCK
	   FTSPClock *_clock = (FTSPClock*) clock;
	  // update clock
	  _clock->checkAndAddNewEntry(p);
	  _clock->calculate();
#else
	  LinearClock *_clock = (LinearClock*) clock;
	  _clock->synchronize(p);
#endif
#ifdef INSTANTANEOUS_RETRANSMISSION
	  // synchronize neighbors
	  synchronizeNeighbors(myRootId, round, 0, recv);
#endif
	}	
      }
      break;
    default:
      assert(false);
    }
  }

  void FTSPNode::handle(EventPtr e) {
    
    switch(e->eventType) {
    case EVENT_TIME_SYNC:
      {

	Time simNow = BaseSimulator::getScheduler()->now();
	uint8_t nbEntries = 0;
	
	heartBeats++;

	if (myRootId != layers.module->blockId && heartBeats >= ROOT_TIMEOUT) {
	  myRootId = layers.module->blockId;
	  numIgnoredTimeouts = 0;
	}

	if (IS_RUNNING_PROTOCOL(PULSE_SYNC)) {
	  if (myRootId == layers.module->blockId) {
	    synchronizeNeighbors(myRootId, round, 0, NULL);
	    round++;
	  }	  
	} else if (IS_RUNNING_PROTOCOL(FTSP)) {

#ifdef FTSP_CLOCK
	  FTSPClock *_clock = (FTSPClock*) clock;
	  nbEntries = _clock->numEntries;
#else
	  LinearClock *_clock = (LinearClock*) clock;
	  nbEntries = _clock->points.size();
#endif
	  if (nbEntries >= ENTRY_SEND_LIMIT || myRootId == layers.module->blockId) {
	    synchronizeNeighbors(myRootId, round, 0, NULL);
	    if (myRootId == layers.module->blockId) {
	      round++;
	    }
	  }
	}
	
#ifdef QUASI_SYNCHRONOUS_SYNC
	scheduleNextTimeSyncSimScale(SYNC_PERIOD_1_US);
#else
	scheduleNextTimeSync(SYNC_PERIOD_1_US);
#endif
      }
      break;
    default:
      MY_CERR << " unknown event type: " << e->getEventName() << endl;
      assert(false);
    }
  }

  void FTSPNode::synchronizeNeighbors(mID_t i, uint32_t r, hopDistance_t h, P2PNetworkInterface *ignore) {
    FTSPTimeSyncMessage m(i,r,h);
    Utils::broadcast(layers,m,ignore);
  } 
}
