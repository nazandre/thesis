/*
 * dee.cpp
 *
 *  Created on: 31/08/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>

#include "dee.h"
#include "scheduler.h"
#include "messages.h"
#include "../utils/fm.h"

#include "../simulation.h"
#include "../logicalId.h"

#include "../utils/fm.h"
#include "../utils/hyperLogLog.h"

using namespace BaseSimulator;
using namespace std;

//#define ECCENTRICITY_INIT_DEBUG

//#define ECCENTRICITY_UPDATE_DEBUG
//#define ECCENTRICITY_UPDATE_MESSAGE_DEBUG
//#define MEAN_ECCENTRICITY_DEBUG

#define USE_START_MSG

/*************** Round Class ***************/
DEERoundData::DEERoundData() {
  received = 0;
}

DEERoundData::DEERoundData(const DEERoundData &rd) {
  neighborhoods = neighborhoods;  
  received = rd.received;
}
  
DEERoundData::~DEERoundData() {

}

void DEERoundData::updateNeighborhoods(std::vector<myShortDouble_t> &x) {
  DEE::mergeInto(neighborhoods,x);
}

size_t DEERoundData::size() {
  size_t s = neighborhoods.size() * sizeof(myShortDouble_t) + sizeof(degree_t);
  return s;
}


/*************** DEE Class ***************/

DEE::DEE(BuildingBlock *m): ElectionAlgorithm(m) {
  initiatorElection = new InitiatorElectionSP(m);
  traversal = &(initiatorElection->traversal);
  minEccentricity = new TreeElection<longDistance_t>(traversal->tree);
}

DEE::~DEE(){
  delete initiatorElection;
  delete minEccentricity;
}

size_t DEE::size() {
  degree_t d = module->getNbInterfaces();
  
  size_t s = 
    // leadere election + diameter estimation
    traversal->size()
#ifdef TWO_WAY_BFS_SP
    + sizeof(distance_t) // max height
#else
    + d * sizeof(distance_t) // branchHeights
#endif
    + local.size() * sizeof(myShortDouble_t) // local data
    + 2 * current.size() // round data
    + 2 * sizeof(distance_t) // bound/round
    + sizeof(longDistance_t) // ecc/far
    // final election on tree
    + sizeof(longDistance_t) // min ecc / min far
    + sizeof(degree_t) // received
    + sizeof(degree_t) // next hop    
    ;
  return s;
}

void DEE::init(){

  initiatorElection->init();

  unsigned int nh = 0; // m in the original paper

  if (sizeof(myShortDouble_t) == 2) {
    nh = 5;
  } else if (sizeof(myShortDouble_t) == 4) {
    nh = 2;
  }

  local.resize(nh,myShortDouble_t(0.0));
  current.neighborhoods.resize(nh,myShortDouble_t(0.0));
  previous.neighborhoods.resize(nh,myShortDouble_t(0.0));
  previous.received = 0;
  current.received = 0;

  doubleRNG gen = BaseSimulator::utils::Random::getUniformDoubleRNG(module->getRandomUint(),0,1);
  for (unsigned int i = 0; i < local.size(); i++) {
    myDouble_t u = gen();
    
    local[i] = u;
    current.neighborhoods[i] = local[i];
    previous.neighborhoods[i] = local[i];
  }
  

  round = 0;
  bound = numeric_limits<distance_t>::max();
  eccentricity = 0;

  minEccentricity->value = numeric_limits<longDistance_t>::max();
  minEccentricity->id = numeric_limits<mID_t>::max();
  minEccentricity->path = NULL;
}

void DEE::start() {
  initiatorElection->start();
}

void DEE::handle(EventPtr e) {}

void DEE::handle(MessagePtr m) {

  stringstream info;

  P2PNetworkInterface *from = m->destinationInterface;
  
  if (initiatorElection->hasToHandle(m)) {
    bool converged = initiatorElection->handle(m);
    if (converged) {
      distance_t height = initiatorElection->traversal.getHeight();
      cout << "DEE initiator elected: "
	   << module->blockId
	   << " at " << BaseSimulator::getScheduler()->now()
	   << endl;
      cout << "Test tree height: " << height << endl;
      cout << "Number of registers: " << local.size() << endl;

      assert(height < numeric_limits<distance_t>::max()/2);
      bound = 2*height;

#ifdef USE_START_MSG
      broadcastStart();
#endif
      
      sendEccentricityUpdate(round,local,bound);
      round = 1;
    }
  } else {
    switch (m->type) {
    case START_ECCENTRICITY_MESSAGE: {
      StartEccentricityMsg_ptr recv = std::static_pointer_cast<StartEccentricityMsg>(m);
      
      assert(round == 0);
      bound = recv->bound;
      
      if (!traversal->tree.isALeaf()) {
	broadcastStart();
      }
      
      sendEccentricityUpdate(round,local,bound);
      round = 1;

      initNextRound();

      // cannot happen in fifo channel!
      assert(previous.received != module->getNbNeighbors());     
    }
      break;
    case ECCENTRICITY_UPDATE_MESSAGE: {
      EccentricityUpdateMsg_ptr recv = std::static_pointer_cast<EccentricityUpdateMsg>(m);

#ifdef USE_START_MSG
      assert(recv->bound == 0);
#else
      bound = recv->bound;
      if (round == 0) {
	sendEccentricityUpdate(round,local,bound);
	round = 1;
      }
#endif
      
#ifdef ECCENTRICITY_UPDATE_MESSAGE_DEBUG
      bID fromId = recv->sourceInterface->hostBlock->blockId;
      bID thisId = module->blockId;

      //if(thisId == 2 && fromId == 1) {
      MY_CERR << "ECCENTRICITY_UPDATE_MSG<"
	      << "r=" << recv->round << ","
	      << ">"
	      << " from @" << fromId
	      << endl;
      //}
#endif

      if (round < bound) {
	if (round == recv->round + 1) {
	  previous.received++;
	  previous.updateNeighborhoods(recv->x);
#ifdef ECCENTRICITY_UPDATE_DEBUG
	  MY_CERR << " receive: " 
		  << " b_" << recv->round << " = "
		  <<  recv->bitStrings[0].bitString
		  << " from @" << recv->sourceInterface->hostBlock->blockId
		  << " "
		  << (unsigned int) previous.received << "/"
		  <<  module->getNbNeighbors()
		  << endl;
#endif
	  if (previous.received == module->getNbNeighbors()) {
	    updateEccentricity();
	    
	    sendEccentricityUpdate(round,local,bound);

	    // prepare next round
	    round++;
	    
	    //previous = current; // merging do in updateEccentricity
	    initNextRound();
	    
	    if (round == bound-1) {
	      // send feedback
	      checkMinEccentricityElection(eccentricity, module->blockId, NULL);
	    }
	  }
	} else {
	  assert(round == recv->round);

	  current.received++;
	  current.updateNeighborhoods(recv->x);
	}
      }
    }
      break;
    case ECCENTRICITY_FEEDBACK_MESSAGE: {
      EccentricityFeedbackMsg_ptr recv = std::static_pointer_cast<EccentricityFeedbackMsg>(m);
      checkMinEccentricityElection(recv->eccentricity, recv->id, from);      
    }
      break;
    case ECCENTRICITY_LEADER_MESSAGE: {
      EccentricityLeaderMsg_ptr recv = std::static_pointer_cast<EccentricityLeaderMsg>(m);
      if (recv->id == module->blockId) {
	cout << "DEE center elected: " << minEccentricity->id << " (" << minEccentricity->value << ")" << endl;
	win();
      } else {
	if (minEccentricity->path != NULL) {
	  spreadEccentricityLeaderMsg(minEccentricity->path,recv->id);
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

degree_t DEE::sendEccentricityUpdate(longDistance_t r, std::vector<myShortDouble_t> &x, longDistance_t b) {
  P2PNetworkInterface *p = NULL;
  degree_t s = 0;
  vector<P2PNetworkInterface*>::iterator it;
  for (it = module->getP2PNetworkInterfaces().begin();
       it != module->getP2PNetworkInterfaces().end();
       ++it) {
    p = *it;
    if (p->isConnected()) {
#ifdef USE_START_MSG
      EccentricityUpdateMsg *message = new EccentricityUpdateMsg(r,x,0);
#else      
      EccentricityUpdateMsg *message = new EccentricityUpdateMsg(r,x,b);
#endif
      p->send(message);
      s++;
    }
  }
  return s;
}

void DEE::sendEccentricityFeedbackMsg(P2PNetworkInterface *p, longDistance_t e, mID_t i) {
  EccentricityFeedbackMsg * message = new EccentricityFeedbackMsg(e,i);
  p->send(message);
}

void DEE::spreadEccentricityLeaderMsg(P2PNetworkInterface *p, mID_t i) {
  EccentricityLeaderMsg * message = new EccentricityLeaderMsg(i);
  p->send(message);
}

void DEE::updateEccentricity() {
  // update previous locals and eccentricity/farness value
  if (IS_RUNNING_VERSION(VERSION_CENTROID)) {    
    myDouble_t sizeCur, sizePrev;
    sizePrev = getSize(local);
    mergeInto(local,previous.neighborhoods);
    sizeCur = getSize(local);
    assert(sizeCur >= sizePrev);
    eccentricity += round * (sizeCur-sizePrev);
  } else if (IS_RUNNING_VERSION(VERSION_CENTER)) {
    std::vector<myShortDouble_t> copy_previous_locals = local;
    mergeInto(local,previous.neighborhoods);
    for (unsigned int i = 0; i < local.size(); i++) {
      if (copy_previous_locals[i] != local[i]) {
	eccentricity = round;
	break;
      }
    }    
  }
}

void DEE::checkMinEccentricityElection(longDistance_t value, mID_t id, P2PNetworkInterface *path) {

  minEccentricity->received.insert(path);
      
  if ((value < minEccentricity->value) || ((value == minEccentricity->value) && (id < minEccentricity->id))) {
    minEccentricity->value = value;
    minEccentricity->id = id;
    minEccentricity->path = path;
  }
      
  if (minEccentricity->received.size() == minEccentricity->tree.children.size() + 1) {
    if (minEccentricity->tree.parent == NULL) {
      if (minEccentricity->id == module->blockId) {
	cout << "DEE center elected: " << minEccentricity->id << "(" << minEccentricity->value << ")" << endl;
	win();
      } else {
	spreadEccentricityLeaderMsg(minEccentricity->path,minEccentricity->id);
      }
    } else {
      sendEccentricityFeedbackMsg(minEccentricity->tree.parent,minEccentricity->value,minEccentricity->id);
    }
  }
}

void DEE::initNextRound() {
  mergeInto(previous.neighborhoods,current.neighborhoods);
  previous.received = current.received;
  current.received = 0;
}

void DEE::broadcastStart() {
  assert(!traversal->tree.isALeaf());
  Message *m = new StartEccentricityMsg(bound);
  traversal->tree.broadcast(m);
}

void DEE::mergeInto(std::vector<myShortDouble_t> &x, std::vector<myShortDouble_t> &y) {
  assert(x.size() == y.size());
  for (unsigned int i = 0; i < x.size(); i++) {
    x[i] = std::max(x[i],y[i]);
  }
}

myDouble_t DEE::getSize(std::vector<myShortDouble_t> &x) {
  myDouble_t s = 0;

  for (unsigned int i = 0; i < x.size(); i++) {
    s += log(x[i]);
  }
  s = s * (-1.0) / (myDouble_t)x.size();
  s = 1.0/s;
  return s;
}
