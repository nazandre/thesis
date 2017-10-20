#include "wmts.h"

#include "scheduler.h"

#include "../utils/utils.h"
#include "../simulation.h"
#include "../layers.h"

#include "../appMsgID.h"
#include "../appEventsID.h"

#include "wmtsMsg.h"

#include "../synchronization/clock.h"

//#define GET_GLOBAL_TIME timeMaster->getTimeNow()

//#define DEBUG_WMTS_TIME_SYNC

#define WMTS_OUTPUT_FILE_NAME "wmts.dat"

//#define SKEW_ESTIMATION_PERIOD (120*ONE_MINUTE)
#define SKEW_ESTIMATION_PERIOD (60*ONE_MINUTE)
//#define SKEW_ESTIMATION_PERIOD (30*ONE_MINUTE)
//#define SKEW_ESTIMATION_PERIOD (5*ONE_MINUTE)

//#define ENABLE_KEEP_OLD_POINT

#define ENABLE_WMTS
//#define ENABLE_MTS
//#define ENABLE_MMTS

namespace Synchronization {

  WMTSNode::WMTSNode(Layers &l): Node(l), neighbors() {
    _clock = new WMTSClock(layers);
    clock = _clock;

    omega = 0;
    r = layers.module->blockId;
    mu = 0;
    nu = 0;
  }
  
  WMTSNode::WMTSNode(const WMTSNode &n): Node(n), neighbors(n.neighbors)
  {
    round = n.round;
    assert(false);
  }

  WMTSNode::~WMTSNode() {
    delete clock;
    _clock = NULL;
  }

  void WMTSNode::startup() {
    round = 0;

    startDecentralized();
    
  }
  
  void WMTSNode::handle(MessagePtr m) {
    P2PNetworkInterface *recv = m->destinationInterface;
							  
    switch(m->type) {
    case WMTS_TIME_MSG:
      {
	WMTSTimeMsgPtr msg =
	  std::static_pointer_cast<WMTSTimeMsg>(m);

	WMTSNeighbor * neighbor = neighbors.getWMTSNeighbor(recv);
	
	Time localSendTime = msg->localSendTime;
	Time globalSendTime = msg->globalSendTime;
	Time localLocalTime = msg->localReceiveTime;
	Time localGlobalTime = msg->globalReceiveTime;

	unsigned int msgSize = msg->size();
	Time neighborLocalTime =
	  Synchronization::Node::compensateForCommDelays(localSendTime,msgSize,1);
	Time neighborGlobalTime =
	  Synchronization::Node::compensateForCommDelays(globalSendTime,msgSize,1);
	
	//Point pNew(localLocalTime,localSendTime);
	// pNew.local
	// pNew.global = remote local!

	Point pNew(localLocalTime,neighborLocalTime);

	if (neighbor->round >= 1) {
#ifdef ENABLE_WMTS
	  // relative skew
	  neighbor->updateRelativeSkewWMTS(pNew);
	  double q = neighbor->relativeSkew * msg->skew / _clock->skew;
	  
	  if ((r != msg->r && q>1) || (r == msg->r && omega > msg->omega)) {
	    omega = msg->omega + 1;
	    r = msg->r;
	    _clock->skew = neighbor->relativeSkew * msg->skew;
	    _clock->offset = msg->skew * pNew.global + msg->offset - _clock->skew * pNew.local; 
	  }
	  
	  if (r != msg->r && q==1) {
	    Time li, lj;
	    li = _clock->skew * pNew.local + _clock->offset;
	    lj = msg->skew * pNew.global + msg->offset;
	    if (li < lj) {
	      r = msg->r;
	      omega = msg->omega + 1;
	      _clock->offset = lj - _clock->skew*pNew.local;
	    }
	  }
#endif
#ifdef ENABLE_MTS
	  // classical MTS
	  neighbor->updateRelativeSkewMTS(pNew);
	  double q = neighbor->relativeSkew * msg->skew / _clock->skew;

	  if (q > 1) {
	    _clock->skew = neighbor->relativeSkew * msg->skew;
	    _clock->offset = msg->skew * pNew.global + msg->offset - _clock->skew * pNew.local; 
	  } else if (q == 1) {
	    // _clock->offset = std::max(msg->skew * pNew.global  + msg->offset,
	    //_clock->skew * pNew.local + _clock->offset) - _clock->skew*pNew.local;
	    Time li, lj;
	    li = _clock->skew * pNew.local + _clock->offset;
	    lj = msg->skew * pNew.global + msg->offset;
	    if (li < lj) {
	      _clock->offset = lj - _clock->skew*pNew.local;
	    }
	  }
#endif
#ifdef ENABLE_MMTS
	  neighbor->updateRelativeSkewWMTS(pNew);

	  wmtsDouble_t amax, amin, bmax, bmin;

	  amax = _clock->skew + mu;
	  amin = _clock->skew - mu;

	  bmax = _clock->offset + nu;
	  bmin = _clock->offset - nu;
	  
	  double q = neighbor->relativeSkew * (msg->skew + msg->mu) / amax;
	  double p = neighbor->relativeSkew * (msg->skew - msg->mu) / amin;

	  // Max consensus
	  if (p > 1) {
	    amax = neighbor->relativeSkew * (msg->skew + msg->mu);
	    bmax = (msg->skew + msg->mu)*pNew.global + msg->offset + msg->nu - neighbor->relativeSkew*(msg->skew + msg->mu)*pNew.local;
	  } else if (p == 1) {
	    bmax = std::max( (msg->skew + msg->mu)* pNew.global  + msg->offset + msg->nu,
			     (_clock->skew + mu)* pNew.local  + _clock->offset + nu) - (_clock->skew + mu)*pNew.local;
	  }	  
	  // Min consensus
	   if (q < 1) {
	    amin = neighbor->relativeSkew * (msg->skew - msg->mu);
	    bmin = (msg->skew - msg->mu)*pNew.global + msg->offset - msg->nu - neighbor->relativeSkew*(msg->skew - msg->mu)*pNew.local;
	  } else if (q == 1) {
	    bmin = std::min( (msg->skew - msg->mu)* pNew.global  + msg->offset - msg->nu,
				       (_clock->skew - mu)* pNew.local  + _clock->offset - nu) - (_clock->skew - mu)*pNew.local;
	  }

	   _clock->skew = (amax+amin)/2;
	   _clock->offset = (bmax+bmin)/2;
	   mu = (amax - amin)/2;
	   nu = (bmax - bmin)/2;
#endif
	  
	   if (pNew.local - neighbor->pOld.local >= (SKEW_ESTIMATION_PERIOD)) {
	     neighbor->pOld = pNew;
	   }
	  
	} else {
	  neighbor->pOld = pNew;
	}
	
#ifndef ENABLE_KEEP_OLD_POINT
	neighbor->pOld = pNew;
#endif
	
	neighbor->round++;
	
#ifdef DEBUG_WMTS_TIME_SYNC
	MY_CERR << "TIME_MSG!" << endl;

	cerr <<  "\tLocal (i): " << pNew.local << endl;
	cerr <<  "\tRemote (j): " << pNew.global << endl;
	cerr <<  "\tskew: " << _clock->skew << endl;
	cerr <<  "\toffset: " << _clock->offset << endl;
	cerr <<  "\tmu: " << mu << endl;
	cerr <<  "\tnu: " << nu << endl;
	cerr <<  "\tNeighbor: " << endl;
	cerr <<  "\t\tRelative skew: " << neighbor->relativeSkew << endl;
	cerr <<  "\t\tRound: " << neighbor->round << endl;
#endif
	
      }
      break;
    default:
      assert(false);
    }
  }
  
  void WMTSNode::handle(EventPtr e) {
    
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

  void WMTSNode::synchronizeNeighbors() {
    WMTSTimeMsg m(_clock->offset, _clock->skew,
		  omega, r, // WMTS
		  mu,nu); // MMTS
    Utils::broadcast(layers,m,NULL);    
  }

  void WMTSNode::init(std::string &config) {
    
  }
}
