/*
 * e2ace.cpp
 *
 *  Created on: 31/08/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>

#include "e2ace.h"
#include "scheduler.h"
#include "messages.h"
#include "../utils/fm.h"

#include "../simulation.h"
#include "../logicalId.h"

#include "../utils/fm.h"
#include "../utils/hyperLogLog.h"

using namespace BaseSimulator;
using namespace std;

//#define FARNESS_INIT_DEBUG

//#define FARNESS_UPDATE_DEBUG
//#define FARNESS_UPDATE_MESSAGE_DEBUG
//#define MEAN_FARNESS_DEBUG
//#define ROUND_VALUE_DEBUG

#define USE_START_MSG

#define ECC_STATE_DIFF

/*************** Round Class ***************/
RoundData::RoundData() {
  neighborhoods = NULL;
  received = 0;
}

RoundData::RoundData(const RoundData &rd) {
  neighborhoods = rd.neighborhoods->clone();  
  received = rd.received;
}
  
RoundData::~RoundData() {
  delete neighborhoods;
}

void RoundData::mergeNeighborhood(ProbabilisticCounter &p) {
  neighborhoods->merge(p);
}

size_t RoundData::size() {
  assert(neighborhoods);
  size_t s = sizeof(degree_t) + neighborhoods->size();
  return s;
}

/*************** E2ACE Class ***************/

E2ACE::E2ACE(BuildingBlock *m): ElectionAlgorithm(m) {
  initiatorElection = new InitiatorElectionSP(m);
  traversal = &(initiatorElection->traversal);
  minFarness = new TreeElection<longDistance_t>(traversal->tree);
}

E2ACE::~E2ACE(){
  delete initiatorElection;
  delete minFarness;
}

size_t E2ACE::size() {
  degree_t d = module->getNbInterfaces();
  size_t s =
    // leadere election + diameter estimation
    traversal->size()
#ifdef TWO_WAY_BFS_SP
    + sizeof(distance_t) // max height
#else
    + d * sizeof(distance_t) // branchHeights
#endif
    + 2*current.size() // roundData current + previous
    + local->size() // local counter
    + 2*sizeof(distance_t) // round + bound 
    + sizeof(longDistance_t) // farness / eccentricity
    
    // final election on tree
    + sizeof(longDistance_t) // min ecc / min far
    + sizeof(degree_t) // received
    + sizeof(degree_t) // next hop
    ;
  return s;
}

void E2ACE::setHashFuncs(unsigned int nh) {
  size_t space = FarnessUpdateMsg::availableDataSize();

#ifndef USE_START_MSG
  space -= sizeof(distance_t); // need to send bound every time.
#endif

  cerr << "Available space in message (byte): " << space << endl;
  
  if (IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG) ||
      IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG_HIP)) {
    if (nh > 1) {
      MY_CERR << "WARNING: HYPER_LOG_LOG(-HIP) counter uses only 1 hashfunc!" << endl;
    }
    HyperLogLog::init(space);
  } else if (IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN) ||
	     IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN_M)) {
    if (nh == 0) {
      FlajolletMartin::init(space);
    } else {
      FlajolletMartin::init(nh*FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE);
    }
  } else {
    assert(false);
  }
}

void E2ACE::init(){
  static bool initHashFuncs = false;
  
  mID_t logicalId = LogicalID::getID(module->blockId);

  //MY_CERR << "my logicalID: " << logicalId << endl;
  
  initiatorElection->init();

  if (!initHashFuncs) {
    initHashFuncs = true;
    //ProbabilisticCounter::init(NUMBER_OF_HASH_FUNCTIONS);
    setHashFuncs(NUMBER_OF_HASH_FUNCTIONS);
  }

  // awful code :)
  if (IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN) || IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN_M)) {
    local = new FlajolletMartin(module);
    current.neighborhoods = new FlajolletMartin(module);
    previous.neighborhoods = new FlajolletMartin(module);
  } else if (IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG)) {
    local = new HyperLogLog(module);
    current.neighborhoods = new HyperLogLog(module);
    previous.neighborhoods = new HyperLogLog(module);
  } else if (IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG_HIP)) {
    local = new HyperLogLogHIP(module);
    current.neighborhoods = new HyperLogLogHIP(module);
    previous.neighborhoods = new HyperLogLogHIP(module);
  }
  
  local->add(logicalId);
  previous.mergeNeighborhood(*local);
  current.mergeNeighborhood(*local);
  
  if (IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN)) {
    for (unsigned int i = 0; i < ProbabilisticCounter::getNumHashFuncs(); i++) {
      farnesses.push_back(0);
    }
  } else {
    farnesses.push_back(0);
  }

#ifdef FARNESS_UPDATE_DEBUG
  //MY_CERR << " initial: " << current.locals[0].bitString << endl;
#endif
  current.received = 0;
  previous.received = 0;

  round = 0;
  farness = 0;
  bound = numeric_limits<distance_t>::max();
  
  minFarness->value = numeric_limits<longDistance_t>::max();
  minFarness->id = numeric_limits<mID_t>::max();
  minFarness->path = NULL;
}

void E2ACE::start() {
  initiatorElection->start();
}

void E2ACE::handle(EventPtr e) {}

void E2ACE::handle(MessagePtr m) {

  stringstream info;

  P2PNetworkInterface *from = m->destinationInterface;
  
  if (initiatorElection->hasToHandle(m)) {
    bool converged = initiatorElection->handle(m);
    if (converged) {
      distance_t height = initiatorElection->traversal.getHeight();
      cout << "E2ACE initiator elected: "
	   << module->blockId
	   << " at " << BaseSimulator::getScheduler()->now()
	   << endl;
      cout << "Test tree height: " << height << endl;
      //cout << "Num hashFuncs: " << ::getNumHashFuncs() << endl;

      assert(height < numeric_limits<distance_t>::max()/2);
      bound = 2*height;
      
#ifdef USE_START_MSG
      broadcastStart();
#endif
      sendFarnessUpdate(round,local,bound);
      round = 1;
    }
  } else {
    switch (m->type) {
    case START_FARNESS_MESSAGE : {
      StartFarnessMsg_ptr recv = std::static_pointer_cast<StartFarnessMsg>(m);
      
      assert(round == 0);
      bound = recv->bound;
      
      if (!traversal->tree.isALeaf()) {
	broadcastStart();
      }
      
      sendFarnessUpdate(round,local,bound);
      round = 1;

      initNextRound();

      // cannot happen in fifo channel!
      if (previous.received == module->getNbNeighbors()) {

	assert(false);	

	updateFarnesses(); // |prev.neighborhood - prev.local|
	sendFarnessUpdate(round,local,bound);
	round++;
        initNextRound();
      }
      
    }
      break;
    case FARNESS_UPDATE_MESSAGE: {
      FarnessUpdateMsg_ptr recv = std::static_pointer_cast<FarnessUpdateMsg>(m);

#ifdef USE_START_MSG
      assert(recv->bound == 0);
#else
      bound = recv->bound;
      if(round == 0) {
	sendFarnessUpdate(round,local,bound);
	round = 1;
      }
#endif
      
#ifdef FARNESS_UPDATE_MESSAGE_DEBUG
      bID fromId = recv->sourceInterface->hostBlock->blockId;
      bID thisId = module->blockId;

      if (module->blockId == 643) {
      MY_CERR << "round " << round << " received FARNESS_UPDATE_MSG<"
	      << "r=" << recv->round
	      << ">"
	      << " from @" << fromId
	      << endl;
      }
#endif

      if (round < bound) { // no need to send the d round
	if (round == recv->round + 1) {
	  previous.received++;
	  previous.mergeNeighborhood(*recv->pCounter);

#ifdef FARNESS_UPDATE_MESSAGE_DEBUG
	  if (module->blockId == 643) {
	    MY_CERR << " updated size estimate: " << recv->pCounter->getSizeEstimation() << endl;
	    MY_CERR << " neighborhood size estimate: " << previous.neighborhoods->getSizeEstimation() << endl;
	  }
	  
#endif
	  if (previous.received == module->getNbNeighbors()) {

	    updateFarnesses(); // |prev.neighborhood - prev.local|

	    // updateFarnesses() => local = merge(local,prev.neighborhood)
	    sendFarnessUpdate(round,local,bound);

	    // prepare next round
	    round++;
	    
	    initNextRound();
	    
	    if (round == bound-1) {
	      // compute mean farness / eccentricity
	      if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
		farness = meanFarness();
	      } else if (IS_RUNNING_VERSION(VERSION_CENTER)) {
		farness = maxFarness(); // => eccentricity, lazy to rename variable names...
	      }
	      // send feedback
	      checkMinFarnessElection(farness, module->blockId, NULL);
	    }
	  }
	} else {
	  assert(round == recv->round);

	  current.received++;
	  current.mergeNeighborhood(*recv->pCounter);

#ifdef FARNESS_UPDATE_MESSAGE_DEBUG
	  if (module->blockId == 643) {
	    MY_CERR << " next updated size estimate: " << recv->pCounter->getSizeEstimation() << endl;
	    MY_CERR << " next neighborhood size estimate: " << previous.neighborhoods->getSizeEstimation() << endl;
	  }
#endif
	}
      }
    }
      break;
    case FARNESS_FEEDBACK_MESSAGE: {
      FarnessFeedbackMsg_ptr recv = std::static_pointer_cast<FarnessFeedbackMsg>(m);
      checkMinFarnessElection(recv->farness, recv->id, from);      
    }
      break;
    case FARNESS_LEADER_MESSAGE: {
      FarnessLeaderMsg_ptr recv = std::static_pointer_cast<FarnessLeaderMsg>(m);
      if (recv->id == module->blockId) {
	cout << "E2ACE center elected: " << minFarness->id << " (" << minFarness->value << ")" << endl;
	win();
      } else {
	if (minFarness->path != NULL) {
	  spreadFarnessLeaderMsg(minFarness->path,recv->id);
	}
      }
    }
      break;
    default:
      cout << "unknown message" << endl;
    }
  }
  info << "message received" << " from " << m->sourceInterface->hostBlock->blockId << endl; // at face " << NeighborDirection::getString(module->getDirection(m->sourceInterface->connectedInterface)) 
}

degree_t E2ACE::sendFarnessUpdate(longDistance_t r, ProbabilisticCounter *pc, longDistance_t b) {
  P2PNetworkInterface *p = NULL;
  degree_t s = 0;
  vector<P2PNetworkInterface*>::iterator it;
  for (it = module->getP2PNetworkInterfaces().begin();
       it != module->getP2PNetworkInterfaces().end();
       ++it) {
    p = *it;
    if (p->isConnected()) {
#ifdef USE_START_MSG
      FarnessUpdateMsg *message = new FarnessUpdateMsg(r,*pc,0);
#else
      FarnessUpdateMsg *message = new FarnessUpdateMsg(r,*pc,b);
#endif
      p->send(message);
      s++;
    }
  }
  return s;
}

void E2ACE::sendFarnessFeedbackMsg(P2PNetworkInterface *p, longDistance_t e, mID_t i) {
  FarnessFeedbackMsg * message = new FarnessFeedbackMsg(e,i);
  p->send(message);
}

void E2ACE::spreadFarnessLeaderMsg(P2PNetworkInterface *p, mID_t i) {
  FarnessLeaderMsg * message = new FarnessLeaderMsg(i);
  p->send(message);
}

void E2ACE::updateFarnesses() {
  longSysSize_t sizePrev = 0, sizeCur = 0;

  if (IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN)) {
    FlajolletMartin* prevLocal = (FlajolletMartin*) local;
    FlajolletMartin* prevNeighborhoods = (FlajolletMartin*) previous.neighborhoods;
    FlajolletMartin::bitString_t localCopy = 0;
    
    for (unsigned int i = 0; i < farnesses.size(); i++) {
      sizePrev = prevLocal->getSizeEstimation(i);
      localCopy = prevLocal->bitStrings[i];
      prevLocal->merge(*prevNeighborhoods,i);     
      sizeCur = prevLocal->getSizeEstimation(i);
      
#ifdef ROUND_VALUE_DEBUG
      MY_CERR << "r" << round << " => " << sizePrev << " -> " << sizeCur << endl;
#endif

      if (IS_RUNNING_VERSION(VERSION_CENTROID)) {	
	farnesses[i] += round * (sizeCur-sizePrev);
      } else if (IS_RUNNING_VERSION(VERSION_CENTER)) {
#ifdef ECC_STATE_DIFF
	if (localCopy != prevLocal->bitStrings[i]) {
#else
	if(sizeCur != sizePrev) {
#endif
	  farnesses[i] = round;
	}
	}
    }
  } else if (IS_RUNNING_PROBABILISTIC_COUNTER(FLAJOLLET_MARTIN_M) ||
      IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG) ||
      IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG_HIP)) {
	
      ProbabilisticCounter* localCopy = NULL;

      sizePrev = local->getSizeEstimation();

#ifdef ECC_STATE_DIFF
      if (IS_RUNNING_VERSION(VERSION_CENTER)) {
         localCopy = local->clone();
      }
#endif
      
      local->merge(*(previous.neighborhoods));
      sizeCur = local->getSizeEstimation();
      //MY_CERR << sizePrev << " -> " << sizeCur << endl;

#ifdef ROUND_VALUE_DEBUG
    //if (module->blockId == 948)
    if (round <= 3)
      MY_CERR << "r" << round << " => " << sizePrev << " -> " << sizeCur << endl;
#endif
    
    if(IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG) ||
       IS_RUNNING_PROBABILISTIC_COUNTER(HYPER_LOG_LOG_HIP)) {
      sizeCur = std::max(sizeCur,sizePrev);
    }
    
    if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
      farnesses[0] += round * (sizeCur-sizePrev);
    } else if (IS_RUNNING_VERSION(VERSION_CENTER)) {
#ifdef ECC_STATE_DIFF
      if (!localCopy->isEqual(local)) {
#else
	if(sizeCur != sizePrev) {
#endif
	farnesses[0] = round;
      }
    }
  }

  if (sizeCur < sizePrev) {
    MY_CERR << "ERROR: sizeCur < sizePrev "
	    << "(" << sizeCur << " < " << sizePrev << ")"
	    << endl;
  }
  assert(sizeCur >= sizePrev);
}

longDistance_t E2ACE::meanFarness() {
  longDistance_t s = 0;
  for (unsigned int i = 0; i < farnesses.size(); i++) {
#ifdef MEAN_FARNESS_DEBUG
    MY_CERR << "far(round=" << round
	    << ",h" << i << ") = "
	    << farnesses[i] << endl;
#endif
    s += farnesses[i];
  }
  s /= farnesses.size();
  return s;
}

longDistance_t E2ACE::maxFarness() {
  longDistance_t s = 0;
  for (unsigned int i = 0; i < farnesses.size(); i++) {
#ifdef MEAN_FARNESS_DEBUG
    MY_CERR << "far(round=" << round
	    << ",h" << i << ") = "
	    << farnesses[i] << endl;
#endif
    s = std::max(s,farnesses[i]);
  }
  return s;
}

void E2ACE::checkMinFarnessElection(longDistance_t value, mID_t id, P2PNetworkInterface *path) {

  minFarness->received.insert(path);
      
  if ((value < minFarness->value) || ((value == minFarness->value) && (id < minFarness->id))) {
    minFarness->value = value;
    minFarness->id = id;
    minFarness->path = path;
  }
      
  if (minFarness->received.size() == minFarness->tree.children.size() + 1) {
    if (minFarness->tree.parent == NULL) {
      if (minFarness->id == module->blockId) {
	cout << "E2ACE center elected: " << minFarness->id << "(" << minFarness->value << ")" << endl;
	win();
      } else {
	spreadFarnessLeaderMsg(minFarness->path,minFarness->id);
      }
    } else {
      sendFarnessFeedbackMsg(minFarness->tree.parent,minFarness->value,minFarness->id);
    }
  }
}

void E2ACE::initNextRound() {
  previous.neighborhoods->merge(*(current.neighborhoods));
  previous.received = current.received;
  current.received = 0;
}


void E2ACE::broadcastStart() {
  assert(!traversal->tree.isALeaf());
  Message *m = new StartFarnessMsg(bound);
  traversal->tree.broadcast(m);
}
