#include "ats.h"

#include "scheduler.h"

#include "../utils/utils.h"
#include "../simulation.h"
#include "../layers.h"

#include "../appMsgID.h"
#include "../appEventsID.h"

#include "atsMsg.h"

#include "../synchronization/clock.h"

//#define GET_GLOBAL_TIME timeMaster->getTimeNow()

//#define DEBUG_ATS_TIME_SYNC

#define ATS_OUTPUT_FILE_NAME "ats.dat"

//#define SKEW_ESTIMATION_PERIOD (120*ONE_MINUTE)
#define SKEW_ESTIMATION_PERIOD (60*ONE_MINUTE)
//#define SKEW_ESTIMATION_PERIOD (30*ONE_MINUTE)
//#define SKEW_ESTIMATION_PERIOD (5*ONE_MINUTE)

namespace Synchronization {

  ATSNode::ATSNode(Layers &l): Node(l), neighbors() {
    _clock = new ATSClock(layers);
    clock = _clock;
  }
  
  ATSNode::ATSNode(const ATSNode &n): Node(n), neighbors(n.neighbors)
  {
    round = n.round;
    assert(false);
  }

  ATSNode::~ATSNode() {
  }

  void ATSNode::startup() {
    round = 0;

    startDecentralized();
    
  }
  
  void ATSNode::handle(MessagePtr m) {
    P2PNetworkInterface *recv = m->destinationInterface;
							  
    switch(m->type) {
    case ATS_TIME_MSG:
      {
	ATSTimeMsgPtr msg =
	  std::static_pointer_cast<ATSTimeMsg>(m);

	ATSNeighbor * neighbor = neighbors.getATSNeighbor(recv);
	

	Time localSendTime = msg->localSendTime;
	Time globalSendTime = msg->globalSendTime;
	  
	Time localLocalTime = msg->localReceiveTime;
	Time localGlobalTime = msg->globalReceiveTime;

	unsigned int msgSize = msg->size();
	
	Time neighborLocalTime =
	Synchronization::Node::compensateForCommDelays(localSendTime,msgSize,1);
	Time neighborGlobalTime =
	  Synchronization::Node::compensateForCommDelays(globalSendTime,msgSize,1);

	Point pNew(localLocalTime,neighborLocalTime);
	
	//Time localGlobalTimeUpdate = 0;

#ifdef DEBUG_ATS_TIME_SYNC
	MY_CERR << "TIME_MSG!" << endl;

	cerr <<  "\tlocalLocalTime: " << localLocalTime << endl;
	cerr <<  "\tneighborLocallTime: " << neighborLocalTime << endl;
	cerr << "\tlocalGlobalTime: " << localGlobalTime << endl;
	cerr << "\tneighborGlobalTime: " << neighborGlobalTime << endl;

	cerr << "\tsender skew: " << msg->senderSkew << endl;
	cerr << "\tskew: " << _clock->skew << endl;
#endif
	neighbor->round++;

	// 4 +- 2.5
	
	if (neighbor->round > 1) {

	  if (pNew.local - neighbor->pOld.local >= (SKEW_ESTIMATION_PERIOD)) {
	    if (neighbor->points.size() > 0) {
	      neighbor->pOld = neighbor->points.front();
	      neighbor->points.pop_back();
	    }
	    neighbor->points.push_back(pNew);
	  }
	  
	  neighbor->updateRelativeSkew(pNew);
	  _clock->updateSkew(neighbor->relativeSkew, msg->senderSkew);
	} else{
	  neighbor->pOld = pNew;
	}
	
	//neighbor->pOld = pNew;

	//neighbor->add(pNew);
	
	_clock->updateOffset(localGlobalTime, neighborGlobalTime);
	
      }
      break;
    default:
      assert(false);
    }
  }
  
  void ATSNode::handle(EventPtr e) {
    
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

  void ATSNode::synchronizeNeighbors() {

    if (ROUND_TRIP_DELAY_COMPENSATION_ENABLED) {
      assert(false);
    } else {
      ATSTimeMsg m(_clock->skew);
      Utils::broadcast(layers,m,NULL);
    }
    
  }

  void ATSNode::init(std::string &config) {
    
  }

  // Round trip sync (test)
  void ATSNode::handleRoundTripSync(MessagePtr m) {
    #if 0
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
	
	vector<Time>& simTime = msg->simTime;
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
#endif
  }  
}
