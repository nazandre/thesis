#include "ad.h"

#include "scheduler.h"

#include "../utils/utils.h"
#include "../simulation.h"
#include "../layers.h"

#include "../appMsgID.h"
#include "../appEventsID.h"

#include "adMsg.h"

#include "../synchronization/clock.h"

//#define GET_GLOBAL_TIME timeMaster->getTimeNow()

//#define DEBUG_AD_TIME_SYNC

#define AD_OUTPUT_FILE_NAME "ad.dat"

namespace Synchronization {

  ADNode::ADNode(Layers &l): Node(l) {

    clock = new OffsetClock(layers);
    round = 0;    
  }
  
  ADNode::ADNode(const ADNode &n): Node(n)
  {
    round = n.round;
    assert(false);
  }

  ADNode::~ADNode() {
  }

  void ADNode::startup() {
    round = 0;
    startDecentralized();
  }
  
  void ADNode::handle(MessagePtr m) {
    switch(m->type) {
    case AD_READCLOCK_1_MSG:
    case AD_READCLOCK_2_MSG:
      {
	handleRoundTripSync(m);
      }
      break;
    case AD_TIME_MSG:
      {
	
	ADSyncMsgPtr msg =
	  std::static_pointer_cast<ADSyncMsg>(m);
	
	//vector<Time>& simTime = msg->simTime;
	vector<Time>& localTime = msg->localTime;
	vector<Time>& globalTime = msg->globalTime;
	
        // Compensate for synchro
	Time syncNeighborGlobalTime =
	  Synchronization::Node::compensateForCommDelays(globalTime[AD_T1_INDEX],msg->size(),1);
	
	// Update clock
	Point p(localTime[AD_T2_INDEX],syncNeighborGlobalTime);
	updateClock(p);
      }
      break;
    default:
      assert(false);
    }
  }

  void ADNode::updateClock(Point& reading) {
    Time localTime = reading.local;
    Time neighborGlobalTime = reading.global;
    Time thisGlobalTime = clock->getTime(localTime, MS_RESOLUTION_CLOCK);

#ifdef AD_MAX
    Time globalTime = max(neighborGlobalTime,thisGlobalTime);
#endif
	
#ifdef AD_AVG
    Time globalTime = (neighborGlobalTime+thisGlobalTime)/2;
    globalTime = US_TO_US_MS_RESOLUTION(globalTime);
#endif

#ifdef DEBUG_AD_TIME_SYNC
    double error = (double)thisGlobalTime - (double)globalTime;
    
    MY_CERR << "Time sync:" << endl;
    cerr << "\t local time: " << localTime << endl;
    cerr << "\t neighbor global time: " << neighborGlobalTime << endl;
    cerr << "\t this global time: "<< thisGlobalTime << endl;
    cerr << "\t avg/max time: " << globalTime << endl;
    cerr << "\t error: " << error << endl;
    //getchar();
#endif
    
    Point sync(localTime,globalTime);
    clock->synchronize(sync);
  }
  
  void ADNode::handle(EventPtr e) {
    
    switch(e->eventType) {     
    case EVENT_TIME_SYNC:
      { 
	synchronizeNeighbors();
	round++;
	scheduleNextTimeSync(SYNC_PERIOD_1_US);
      }
      break;
    default:
      MY_CERR << " unknown event type: " << e->getEventName() << endl;
      assert(false);
    }
  }

  void ADNode::synchronizeNeighbors() {
    if (ROUND_TRIP_DELAY_COMPENSATION_ENABLED) {
      ADSyncMsg m(AD_READCLOCK_1_MSG);
      Utils::broadcast(layers,m,NULL);
    } else {
      ADSyncMsg m(AD_TIME_MSG);
      Utils::broadcast(layers,m,NULL);
    }   
  }
  
  // Round trip sync (test)
  void ADNode::handleRoundTripSync(MessagePtr m) {
    P2PNetworkInterface *recv = m->destinationInterface;
    
    switch(m->type) {
    case AD_READCLOCK_1_MSG:
      {
	ADSyncMsgPtr msg =
	  std::static_pointer_cast<ADSyncMsg>(m);

	ADSyncMsg* reply = new ADSyncMsg(AD_READCLOCK_2_MSG,
					 msg->simTime,
					 msg->localTime,
					 msg->globalTime
					 );
	MyMessage::send(layers,recv,reply);
      }
      break;
    case AD_READCLOCK_2_MSG:
      {	
	ADSyncMsgPtr msg =
	  std::static_pointer_cast<ADSyncMsg>(m);
	
	//vector<Time>& simTime = msg->simTime;
	vector<Time>& localTime = msg->localTime;
	vector<Time>& globalTime = msg->globalTime;

	// compute transfer delay
	Time rttLocal = (localTime[AD_T4_INDEX] - localTime[AD_T1_INDEX]) -
	  (localTime[AD_T3_INDEX] - localTime[AD_T2_INDEX]);
	Time oneWayLocal = rttLocal/2;

	oneWayLocal = US_TO_US_MS_RESOLUTION(oneWayLocal);
	
        // Compensate for synchro
	Time syncNeighborGlobalTime = globalTime[AD_T3_INDEX] + oneWayLocal;
	Time syncLocalTime = localTime[AD_T4_INDEX];

	/*cerr << "delay:" << oneWayLocal << endl;
	  getchar();*/
	
	// Update clock
	Point p(syncLocalTime,syncNeighborGlobalTime);
	updateClock(p);
      }
      break;
    default:
      assert(false);
    }
  }  
}
