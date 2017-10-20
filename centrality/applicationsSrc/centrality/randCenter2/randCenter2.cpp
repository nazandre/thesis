/*
 * randCenter2.cpp
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

#include "randCenter2.h"

#include "scheduler.h"
#include "network.h"
#include "trace.h"
#include "world.h"
#include "lattice.h"

//#include "messages.h"
#include "../msgID.h"
#include "../bfsTreeMsg.h"
#include "../initiatorMsg.h"
#include "../simulation.h"

#include "messages.h"
#include "bfsData.h"

using namespace std;
using namespace BaseSimulator;

//#define DEBUG_ALGORITHM

#define MAX_ITERATIONS 10

unsigned int RandCenter2::numBFS = 0;

RandCenter2::RandCenter2(BuildingBlock *host): ElectionAlgorithm(host) {
  reset();
}

RandCenter2::~RandCenter2() {
  delete bfsAlg;
  delete initiatorElection;
}

size_t RandCenter2::size() {
  size_t s = bfs->size()
    + sizeof(uint8_t) // currIteration
    + sizeof(longDistance_t) // estimatedEcc/Farness
    + sizeof(bool) // previously activated
    //+ sizeof(uint8_t) // maxIteration => hardcoded
    + sizeof(distance_t) // randomNumber
    + sizeof(longDistance_t) + sizeof(degree_t) + sizeof(mID_t) // children min farness/ecc + interface index + id
    //+ 2*sizeof(distance_t) + sizeof(degree_t) // (use above variable) random number + interface index
    ;
  return s;
}

void RandCenter2::reset() {
  initiatorElection = new InitiatorElectionSP(module);
  bfs = &(initiatorElection->traversal);
  //new BFSTraversalSP();
  bfsAlg = new BFSTraversalSPAlgorithm(module);
  bfs->enableConvergecast = false;
  treeElection = new TreeElection<longDistance_t>(bfs->tree);

  treeElection->id = numeric_limits<mID_t>::max();
  treeElection->value = numeric_limits<longDistance_t>::max();
  treeElection->path = NULL;
  
  setHandlers();
  
  currIteration = 0;
  maxIteration = MAX_ITERATIONS;
  estimatedEcc = 0;
  activated = true;
  
  randomNumber = module->getRandomUint();
  
  resetHandler();
}

void RandCenter2::init() {
  initiatorElection->init();
}

void RandCenter2::start() {
  bool converged = initiatorElection->start();
  if (converged) {
    win(); // a single module!
    MY_CERR << "Elected with value: " << estimatedEcc << endl;
  }
}

void RandCenter2::handle(MessagePtr m) {
  P2PNetworkInterface *from = m->destinationInterface;
  
  if (initiatorElection->hasToHandle(m)) {
    bool initiatorElected = initiatorElection->handle(m);
    if (initiatorElected) {
      numBFS++;
      // check size
#ifdef DEBUG_ALGORITHM
      sysSize_t n = initiatorElection->traversal.getSize();
      
      MY_CERR << "Initiator elected!"
	      << " " << n << " nodes in the system."
	      << endl;
#endif

      // update ecc/far
      updateEstimatedEcc();

      //convergecastAlg->startBroadcast(convergecast);
      activated = false;
      broadcastNext(maxRandomNumber,maxRandomNumberId);
    }
  } else if (bfsAlg->hasToHandle(m)) {
    // check iteration
    BFSTraversalSPMessagePtr mes =  std::static_pointer_cast<BFSTraversalSPMessage>(m);
    RandCenter2BFSHeader* header = (RandCenter2BFSHeader*) mes->header;
    
    if(header->iteration > currIteration) {
      assert(header->iteration - currIteration == 1);
      currIteration = header->iteration;
      initNextIteration(mes->rootID); // update ecc/far
    }

    if (header->iteration == currIteration) {
      bool bfsCompleted = bfsAlg->handle(mes,bfs);
      if (bfsCompleted) {
	// update ecc/far
	updateEstimatedEcc();
	numBFS++;
	MY_CERR << "BFS " << (unsigned int) header->iteration + 1
		<< " completed!"
		<< " (random number: " << randomNumber << ")"
		<< " at " << getScheduler()->now()
		<< endl;
	activated = false;
	if (currIteration == (maxIteration-1)) {
	  
	  treeElection->id = module->blockId;
	  treeElection->value = estimatedEcc;
	  treeElection->path = NULL;	  

	  broadcastLeaderGoMsg();	  
    	} else {
	  broadcastNext(maxRandomNumber,maxRandomNumberId);
	}
      }
    }
  } else


    switch(m->type) {
    case RAND_CENTER_2_NEXT: {
      RandCenter2MessageNextPtr mes =  std::static_pointer_cast<RandCenter2MessageNext>(m);

#ifdef DEBUG_ALGORITHM
      MY_CERR << "NEXT<"
	      << mes->value << ">"
	      << endl;
#endif

      if (currIteration < (maxIteration-1)) {
	if (mes->id == module->blockId) {
	  assert(randomNumber == mes->randomNumber);
	  initNextIteration(module->blockId); // update ecc/far
	  currIteration++;
	  bfsAlg->start(bfs);
	} else {
	  //if (mes->id == maxRandomNumberId) { // do not continue broadcasting if this node has never been on a path to the targeted node. (error...)
	    broadcastNext(mes->randomNumber,mes->id);
	    //}
	}
      }
    }
      break;
    case RAND_CENTER_2_GO_LEADER_MESSAGE: {
      
      updateEstimatedEcc();
      
      treeElection->id = module->blockId;
      treeElection->value = estimatedEcc;
      treeElection->path = NULL;

      if (bfs->tree.isALeaf()) {
	sendLeaderBackMsg();
      } else {
	broadcastLeaderGoMsg();
      }
    }
      break;
    case RAND_CENTER_2_BACK_LEADER_MESSAGE: {
      RandCenter2BackLeaderMsg_ptr recv = std::static_pointer_cast<RandCenter2BackLeaderMsg>(m);
      
      if (recv->value < treeElection->value ||
	  (recv->value == treeElection->value && recv->id < treeElection->id)) {
        treeElection->value = recv->value;
	treeElection->id = recv->id;
	treeElection->path = from;
      }

      treeElection->received.insert(from);

      if (treeElection->received.size() == bfs->tree.children.size()) {
	if (bfs->tree.isRoot()) {
	  if (treeElection->id == module->blockId) {
	    MY_CERR << "Elected with value: " << estimatedEcc << endl;
	    win();
	  } else {
	    assert(treeElection->path);
	    sendLeaderMsg();
	  }
	} else {
	  sendLeaderBackMsg();
	}
      }
    }
      break;
    case RAND_CENTER_2_LEADER_MESSAGE: {
      RandCenter2LeaderMsg_ptr recv = std::static_pointer_cast<RandCenter2LeaderMsg>(m);
      if (recv->id == module->blockId) {
	MY_CERR << "Elected with value: " << estimatedEcc << endl;
	win();
      } else {
	assert(treeElection->path != NULL);
	sendLeaderMsg();
      }
    }
      break;
    default:
      assert(false);
    }
}

void RandCenter2::initFirstIteration() {
  assert(false);
}

void RandCenter2::updateEstimatedEcc() {
  longDistance_t d =  bfs->tree.distance;
  
  if (IS_RUNNING_VERSION(VERSION_CENTER)) {
    estimatedEcc = std::max(estimatedEcc,d);
  } else if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
    estimatedEcc += d;
  }
}

void RandCenter2::initNextIteration(mID_t rootID) {
  updateEstimatedEcc();  
  bfsAlg->reset(bfs);
  bfs->subscribe(rootID);
  resetHandler();
}

void RandCenter2::handle(EventPtr e) {
}

void RandCenter2::addChildHandler(MessagePtr mes) {
  RandCenter2BFSBackData* data = NULL;
  
  if(mes->type == BFS_TRAVERSAL_SP_BACK ||
     mes->type == BFS_TRAVERSAL_SP_CONVERGECAST_BACK) {
    BFSTraversalSPMessagePtr m =  std::static_pointer_cast<BFSTraversalSPMessage>(mes);
    data = (RandCenter2BFSBackData*) m->backData;
  } else if (mes->type == IE_SP_BACK_MESSAGE ||
	     mes->type == IE_SP_CONVERGECAST_BACK_MESSAGE) {
    IESPBackMsgPtr m =  std::static_pointer_cast<IESPBackMsg>(mes);
    data = (RandCenter2BFSBackData*) m->backData;
  }
  
  assert(data != NULL);
  
#ifdef DEBUG_ALGORITHM
  MY_CERR << "ADD_CHILD<"
	  << "random=" << data->maxRandomNumber << ","
	  << "id=" << data->maxRandomNumberId << ">"
	  << endl;
#endif

  if (maxRandomNumber < data->maxRandomNumber ||
      (maxRandomNumber == data->maxRandomNumber &&
       maxRandomNumberId < data->maxRandomNumberId)) {
    maxRandomNumber = data->maxRandomNumber;
    maxRandomNumberId = data->maxRandomNumberId;
  }
}

void RandCenter2::removeChildHandler(MessagePtr mes) {
  // don't care
}

void RandCenter2::resetHandler() {
  if (activated && !bfs->tree.isRoot()) { // will be disactivated upon completion
    maxRandomNumber = randomNumber;
    maxRandomNumberId = module->blockId;
  } else {
    maxRandomNumber = numeric_limits<distance_t>::min();
    maxRandomNumberId = numeric_limits<mID_t>::min();
  }
}

BFSData* RandCenter2::getBFSDataHeader() {
  return new RandCenter2BFSHeader(currIteration);
}

BFSData* RandCenter2::getBFSDataBack() {
  // this node data
  return new RandCenter2BFSBackData(maxRandomNumber,maxRandomNumberId);
}

void RandCenter2::setHandlers() {
  bfs->resetHandler = std::bind(&RandCenter2::resetHandler,this);
  bfs->addChild = std::bind(&RandCenter2::addChildHandler,this,std::placeholders::_1);
  bfs->removeChild = std::bind(&RandCenter2::removeChildHandler,this,std::placeholders::_1);

  bfs->getBFSDataHeader = std::bind(&RandCenter2::getBFSDataHeader,this);
  bfs->getBFSDataBack = std::bind(&RandCenter2::getBFSDataBack,this);
}

void RandCenter2::broadcastNext(distance_t r, mID_t i) {  
  Message *m = new RandCenter2MessageNext(r,i);
  //assert(!bfs->tree.isALeaf());
  bfs->tree.broadcast(m);
}

void RandCenter2::broadcastLeaderGoMsg() {
  Message *m = new RandCenter2GoLeaderMsg();
  assert(!bfs->tree.isALeaf());
  bfs->tree.broadcast(m);
}

void RandCenter2::sendLeaderBackMsg() {
  Message *m = new RandCenter2BackLeaderMsg(treeElection->value,treeElection->id);
  assert(!bfs->tree.isRoot());
  bfs->tree.sendParent(m);
}

void RandCenter2::sendLeaderMsg() {
  Message *m = new RandCenter2LeaderMsg(treeElection->value,treeElection->id);
  assert(treeElection->path!=NULL);
  treeElection->path->send(m);
}
