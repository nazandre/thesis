/*
 * randCenter.cpp
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

#include "randCenter.h"

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

//#define USE_BFS_TREE_HEIGHT

unsigned int RandCenter::numBFS = 0;

RandCenter::RandCenter(BuildingBlock *host): ElectionAlgorithm(host) {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  childrenMinEcc.resize(d,0);
  childrenMaxActivatedRandomNumber.resize(d,0);
  
  //convergecast = NULL;
  //convergecastAlg = NULL;

  reset();
}

RandCenter::~RandCenter() {
  delete bfsAlg;
  //delete convergecastAlg;
  //delete convergecast;
  delete initiatorElection;
}

size_t RandCenter::size() {
  degree_t d = childrenMinEcc.size();
  size_t s = bfs->size()
    + sizeof(uint8_t) // currIteration
    + sizeof(longDistance_t) // estimatedEcc/Farness
    + sizeof(bool) // previously activated
    //+ sizeof(uint8_t) // maxIteration => hardcoded
#ifdef TWO_WAY_BFS_SP
    + sizeof(longDistance_t) + sizeof(degree_t) // children min farness/ecc + interface index
    + 2*sizeof(distance_t) + sizeof(degree_t) // random number + interface index
#else
    + d * sizeof(longDistance_t) // children min farness/ecc
    + (d + 1)* sizeof(distance_t) // random number
    + (d+1)*sizeof(sysSize_t) // used to ensure the correctness of the agg computation (one to keep / the other computed each time)
#endif
    ;
  return s;
}

void RandCenter::reset() {
  initiatorElection = new InitiatorElectionSP(module);
  bfs = &(initiatorElection->traversal);
    //new BFSTraversalSP();
  bfsAlg = new BFSTraversalSPAlgorithm(module);
  
  setHandlers();
  
  currIteration = 0;
  maxIteration = MAX_ITERATIONS;
  estimatedEcc = 0;
  activated = true;
  
  randomNumber = module->getRandomUint();
  
  resetHandler();
}

void RandCenter::init() {
  initiatorElection->init();
}

void RandCenter::start() {
  bool converged = initiatorElection->start();
  if (converged) {
    win(); // a single module!
    MY_CERR << "Elected with value: " << estimatedEcc << endl;
  }
}

void RandCenter::handle(MessagePtr m) {
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

      bfsAlg->setSysSize(bfs->getSize());
      // update ecc/far
      updateEstimatedEcc();
      
      //convergecastAlg->startBroadcast(convergecast);
      activated = false;      
      forwardMaxActivatedRandomNumber(); // consider only children!
    }
  } else if (bfsAlg->hasToHandle(m)) {
    // check iteration
    BFSTraversalSPMessagePtr mes =  std::static_pointer_cast<BFSTraversalSPMessage>(m);
    RandCenterBFSHeader* header = (RandCenterBFSHeader*) mes->header;
    
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
	  vector<longDistance_t>::iterator it = std::min_element(childrenMinEcc.begin(), childrenMinEcc.end());
	  if(*it > estimatedEcc) { // local min (possible? at least infrequent)
	    assert(numBFS == MAX_ITERATIONS); // if size > 10 
	    win();
	    MY_CERR << "Elected with value: " << estimatedEcc << endl;
	  } else {
	    forwardCenter();
	  }
	} else {
	  // forward to trigger next iteration: maxSum
	  forwardMaxActivatedRandomNumber();
	}
      }
    }
  } else if (m->type == RAND_CENTER_NEXT) {
    RandCenterMessageNextPtr mes =  std::static_pointer_cast<RandCenterMessageNext>(m);

#ifdef DEBUG_ALGORITHM
    MY_CERR << "NEXT<"
	    << mes->value << ">"
	    << endl;
#endif

    bfsAlg->setSysSize(mes->sysSize);
    
    if (currIteration < (maxIteration-1)) {
      if (mes->value == randomNumber) {
	initNextIteration(module->blockId); // update ecc/far
	currIteration++;
	bfsAlg->start(bfs);
      } else {
	forwardMaxActivatedRandomNumber();
      }
    } else {
      updateEstimatedEcc();
      if (mes->value == estimatedEcc) {
	assert(numBFS == MAX_ITERATIONS);
	win();
	MY_CERR << "Elected with value: " << estimatedEcc << endl;
      } else {
	forwardCenter();
      }
    }
  }
}

void RandCenter::initFirstIteration() {
  assert(false);
}

void RandCenter::updateEstimatedEcc() {
  longDistance_t d =  bfs->tree.distance;
  
  if (IS_RUNNING_VERSION(VERSION_CENTER)) {
#ifdef USE_BFS_TREE_HEIGHT
    longDistance_t h = bfs->getHeight();
    estimatedEcc = std::max(estimatedEcc,std::max(h,d));
#else
    estimatedEcc = std::max(estimatedEcc,d);
#endif
  } else if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
    estimatedEcc += d;
  }
}

void RandCenter::initNextIteration(mID_t rootID) {
  updateEstimatedEcc();  
  bfsAlg->reset(bfs);
  bfs->subscribe(rootID);
  resetHandler();
}

void RandCenter::handle(EventPtr e) {
}

void RandCenter::addChildHandler(MessagePtr mes) {
  RandCenterBFSBackData* data = NULL;
  P2PNetworkInterface *from = mes->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  
  if(mes->type == BFS_TRAVERSAL_SP_BACK ||
     mes->type == BFS_TRAVERSAL_SP_CONVERGECAST_BACK) {
    BFSTraversalSPMessagePtr m =  std::static_pointer_cast<BFSTraversalSPMessage>(mes);
    data = (RandCenterBFSBackData*) m->backData;
  } else if (mes->type == IE_SP_BACK_MESSAGE ||
	     mes->type == IE_SP_CONVERGECAST_BACK_MESSAGE) {
    IESPBackMsgPtr m =  std::static_pointer_cast<IESPBackMsg>(mes);
    data = (RandCenterBFSBackData*) m->backData;
  }
  
  assert(data != NULL);
  
#ifdef DEBUG_ALGORITHM
   MY_CERR << "ADD_CHILD<"
	   << data->minEcc << ","
	   << data->maxRandomNumber << ">"
	   << endl;
#endif
  
  childrenMinEcc[fromIndex] = data->minEcc;
  childrenMaxActivatedRandomNumber[fromIndex] = data->maxRandomNumber;
}

void RandCenter::removeChildHandler(MessagePtr mes) {
  BFSTraversalSPMessagePtr m =  std::static_pointer_cast<BFSTraversalSPMessage>(mes);
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  
  childrenMinEcc[fromIndex] = numeric_limits<distance_t>::max();
  childrenMaxActivatedRandomNumber[fromIndex] = 0;
}

void RandCenter::resetHandler() {
  for (size_t i = 0; i < childrenMinEcc.size(); i++) {
    childrenMinEcc[i] = numeric_limits<distance_t>::max();
    childrenMaxActivatedRandomNumber[i] = 0;
  }
}

BFSData* RandCenter::getBFSDataHeader() {
  return new RandCenterBFSHeader(currIteration);
}

BFSData* RandCenter::getBFSDataBack() {
  // this node data
  longDistance_t e = estimatedEcc;
  distance_t r = 0;
  longDistance_t d = bfs->tree.distance;
  
  if (activated) {
    r = randomNumber;
  }

  if (IS_RUNNING_VERSION(VERSION_CENTER)) {
#ifdef USE_BFS_TREE_HEIGHT
    longDistance_t h = bfs->getHeight();
    e = std::max(e,std::max(h,d));
#else
    e = std::max(e,d);
#endif
  } else if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
    e += d;
  }

  // aggergate with children info
  r = std::max(r,*std::max_element(childrenMaxActivatedRandomNumber.begin(),childrenMaxActivatedRandomNumber.end()));
  e = std::min(e,*std::min_element(childrenMinEcc.begin(),childrenMinEcc.end()));

  return new RandCenterBFSBackData(r,e);
}

void RandCenter::setHandlers() {
  bfs->resetHandler = std::bind(&RandCenter::resetHandler,this);
  bfs->addChild = std::bind(&RandCenter::addChildHandler,this,std::placeholders::_1);
  bfs->removeChild = std::bind(&RandCenter::removeChildHandler,this,std::placeholders::_1);

  bfs->getBFSDataHeader = std::bind(&RandCenter::getBFSDataHeader,this);
  bfs->getBFSDataBack = std::bind(&RandCenter::getBFSDataBack,this);
}

void RandCenter::forwardMaxActivatedRandomNumber() {  
  vector<distance_t>::iterator it = std::max_element(childrenMaxActivatedRandomNumber.begin(), childrenMaxActivatedRandomNumber.end());
  int indexMax = std::distance(childrenMaxActivatedRandomNumber.begin(), it);
  P2PNetworkInterface *next = module->getInterface(indexMax);
  Message *m = new RandCenterMessageNext(*it, bfsAlg->sysSize);
 
#ifdef DEBUG_ALGORITHM
  MY_CERR << "Max Activated Random Number: "
	  << *it
	  << endl;
#endif

  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}

void RandCenter::forwardCenter() {
  vector<longDistance_t>::iterator it = std::min_element(childrenMinEcc.begin(), childrenMinEcc.end());
  int indexMin = std::distance(childrenMinEcc.begin(), it);
  P2PNetworkInterface *next = module->getInterface(indexMin);
  Message *m = new RandCenterMessageNext(*it, bfsAlg->sysSize);
  
#ifdef DEBUG_ALGORITHM
  MY_CERR << "Forward center: "
	  << *it
	  << endl;
#endif

  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}
