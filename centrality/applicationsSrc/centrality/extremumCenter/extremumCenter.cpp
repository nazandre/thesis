/*
 * extremumCenter.cpp
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

#include "extremumCenter.h"

#include "scheduler.h"
#include "network.h"
#include "trace.h"
#include "world.h"
#include "lattice.h"

//#include "messages.h"
#include "../msgID.h"
#include "../bfsTreeMsg.h"
#include "../simulation.h"

#include "messages.h"
#include "bfsData.h"

using namespace std;
using namespace BaseSimulator;

//#define DEBUG_ALGORITHM
//#define DEBUG_ALGORITHM_MSG

#define MAX_ITERATIONS 10

//#define USE_BFS_TREE_HEIGHT

unsigned int ExtremumCenter::numBFS = 0;

ExtremumCenter::ExtremumCenter(BuildingBlock *host): ElectionAlgorithm(host) {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  childrenMinEcc.resize(d,0);
  childrenMaxSumDistances.resize(d,0);
  
  reset();
}

ExtremumCenter::~ExtremumCenter() {
  delete initiatorElection;
  delete bfsAlg;
  //  delete bfs;
}

size_t ExtremumCenter::size() {
  degree_t d = childrenMinEcc.size();
  size_t s = bfs->size()
    + sizeof(uint8_t) // currIteration
    + sizeof(longDistance_t) // sum
    + sizeof(longDistance_t) // estimatedEcc/Farness
    + sizeof(bool) // previously activated
    //+ sizeof(uint8_t) // maxIteration => hardcoded
#ifdef TWO_WAY_BFS_SP
    + 2*sizeof(longDistance_t) + 2 * sizeof(degree_t) // children min farness/ecc + max farness + interface indices
    //+ sizeof(distance_t) + sizeof(degree_t) // branchHeight => can use an ohter array
#else
    + 2 * d * sizeof(longDistance_t) // children min farness/ecc + max farness
    //+ d*sizeof(distance_t) // branchHeight => can use an ohter array
    + (d+1)*sizeof(sysSize_t) // used to ensure the correctness of the agg computation (one to keep / the other computed each time)
#endif
    ;
    return s;
}

void ExtremumCenter::reset() {
  initiatorElection = new InitiatorElectionSP(module);
  bfs = &(initiatorElection->traversal);
    //new BFSTraversalSP();
  bfsAlg = new BFSTraversalSPAlgorithm(module);
  
  currIteration = 0;
  maxIteration = MAX_ITERATIONS;
  sum = 0;
  estimatedEcc = 0;
  activated = true;
  
  resetHandler();
}

void ExtremumCenter::init() {
  module->setColor(WHITE);
  
  /*if (module->blockId == 67) { // theoretical centroid
    module->setColor(DARKGREY);
  }
  if (module->blockId == 93) { // elected centroid
    module->setColor(CYAN);
    }*/
  
  initiatorElection->init();
}

void ExtremumCenter::start() {
  bool converged = initiatorElection->start();
  if (converged) {
    win(); // a single module!
    MY_CERR << "Elected with value: " << estimatedEcc << endl;
  }
}

void ExtremumCenter::handle(MessagePtr m) {
  if (initiatorElection->hasToHandle(m)) {
    bool initiatorElected = initiatorElection->handle(m);
    if (initiatorElected) {

      if (bfs->getSize() > 1 && MAX_ITERATIONS > 1) {
      
	bfsAlg->setSysSize(bfs->getSize());
	numBFS++;
	module->setColor(BROWN);
	MY_CERR << "Initiator elected at "
		<< getScheduler()->now()
		<< endl;
	activated = false;
	updateEstimatedEcc(); // update ECC/Farness + SUM
	forwardFarthest();
      } else {
	win();
      }
    }
  } else if (bfsAlg->hasToHandle(m)) {
    // check iteration
    BFSTraversalSPMessagePtr mes =  std::static_pointer_cast<BFSTraversalSPMessage>(m);
    ExtremumCenterBFSHeader* header = (ExtremumCenterBFSHeader*) mes->header;

    if (currIteration == 0) { // first BFS computed
      initFirstIteration();
    }
    
    if(header->iteration > currIteration) {
      assert(header->iteration - currIteration == 1);
      currIteration = header->iteration;
      initNextIteration(mes->rootID); // update ECC/Farness + SUM
    }

    if (header->iteration == currIteration) {
      bool bfsCompleted = bfsAlg->handle(mes,bfs);
      if (bfsCompleted) {
	
	updateEstimatedEcc();// update ECC/Farness + SUM
	numBFS++;
	module->setColor(YELLOW);

	//activated = false;
	
	MY_CERR << "BFS " << (unsigned int) currIteration+1 << " completed." << endl;
	if (currIteration > maxIteration || currIteration > bfsAlg->sysSize) {
	  vector<longDistance_t>::iterator it = std::min_element(childrenMinEcc.begin(), childrenMinEcc.end());
	  if(*it > estimatedEcc) { // local min (possible? at least infrequent)
	    win();
	    assert(numBFS == MAX_ITERATIONS);
	    MY_CERR << "Elected with value: " << estimatedEcc << endl;
	  } else {
	    forwardCenter();
	  }
	} else {
	  // forward to trigger next iteration: maxSum
	  forwardMaxSumDistances();
	}
      }
    }
  } else if (m->type == EXTREMUM_CENTER_NEXT) {
    ExtremumCenterMessageNextPtr mes =  std::static_pointer_cast<ExtremumCenterMessageNext>(m);

#ifdef DEBUG_ALGORITHM_MSG
    MY_CERR << "NEXT<"
	    << mes->value << ">"
	    << endl;
#endif

    bfsAlg->setSysSize(mes->sysSize);
    
    if (currIteration == 0) {
      if (bfs->tree.isALeaf()) {
	initFirstIteration();

#ifdef DEBUG_ALGORITHM
	MY_CERR << "Farthest from initiator reached!" << endl;
#endif
	initNextIteration(module->blockId); // update ECC/Farness + SUM

#ifdef DEBUG_ALGORITHM
	MY_CERR << "BFS Starting..." << endl;
#endif
	currIteration++;
	bfsAlg->start(bfs);
      } else {
	forwardFarthest();
      }
    } else if (currIteration < (maxIteration-1)) {
      longDistance_t tmp = sum + bfs->tree.distance;      
      if (mes->value == tmp) {
	initNextIteration(module->blockId); // update ECC/Farness + SUM
	currIteration++;
	activated = false;
	bfsAlg->start(bfs);
      } else {
	forwardMaxSumDistances();
      }
    } else {
      // this was the final BFS
      updateEstimatedEcc(); // update ECC/Farness + SUM
      if (mes->value == estimatedEcc) {
	win();
	assert(numBFS == MAX_ITERATIONS);
	MY_CERR << "Elected with value: " << estimatedEcc << endl;
      } else {
	forwardCenter();
      }
    }
  }
}

void ExtremumCenter::initFirstIteration() {
  setHandlers();
}

void ExtremumCenter::updateEstimatedEcc() {
  longDistance_t d = bfs->tree.distance; 
  if (IS_RUNNING_VERSION(VERSION_CENTER)) {
#ifdef USE_BFS_TREE_HEIGHT
    longDistance_t h = bfs->getHeight();
    estimatedEcc = std::max(estimatedEcc,
			    std::max(h,d));
#else
    estimatedEcc = std::max(estimatedEcc,d);
#endif
  } else if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
    estimatedEcc += d;
  }
}

void ExtremumCenter::initNextIteration(mID_t rootID) {
  sum += bfs->tree.distance; // previous iteration
  updateEstimatedEcc();

  bfsAlg->reset(bfs);
  bfs->subscribe(rootID);
  resetHandler();
}

void ExtremumCenter::handle(EventPtr e) {

}

void ExtremumCenter::addChildHandler(MessagePtr mes) {
  BFSTraversalSPMessagePtr m =  std::static_pointer_cast<BFSTraversalSPMessage>(mes);
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  ExtremumCenterBFSBackData* data = (ExtremumCenterBFSBackData*) m->backData;

#ifdef DEBUG_ALGORITHM_MSG
  MY_CERR << "ADD_CHILD<"
	  << data->minEcc << ","
	  << data->maxSum << ">"
	  << endl;
#endif
  
  childrenMinEcc[fromIndex] = data->minEcc;
  childrenMaxSumDistances[fromIndex] = data->maxSum;
}

void ExtremumCenter::removeChildHandler(MessagePtr mes) {
  BFSTraversalSPMessagePtr m =  std::static_pointer_cast<BFSTraversalSPMessage>(mes);
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  
  childrenMinEcc[fromIndex] = numeric_limits<distance_t>::max();
  childrenMaxSumDistances[fromIndex] = 0;
}

void ExtremumCenter::resetHandler() {
  for (size_t i = 0; i < childrenMinEcc.size(); i++) {
    childrenMinEcc[i] = numeric_limits<distance_t>::max();
    childrenMaxSumDistances[i] = 0;
  }
}

BFSData* ExtremumCenter::getBFSDataHeader() {
  return new ExtremumCenterBFSHeader(currIteration);
}

BFSData* ExtremumCenter::getBFSDataBack() {
  // this node data
  longDistance_t s = 0;
  longDistance_t e = estimatedEcc;
  longDistance_t d = bfs->tree.distance;
    
  if (activated) {
    s = sum + d;
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
  s = std::max(s,*std::max_element(childrenMaxSumDistances.begin(),childrenMaxSumDistances.end()));
  e = std::min(e,*std::min_element(childrenMinEcc.begin(),childrenMinEcc.end()));

  return new ExtremumCenterBFSBackData(s,e);
}

void ExtremumCenter::setHandlers() {
  bfs->resetHandler = std::bind(&ExtremumCenter::resetHandler,this);
  bfs->addChild = std::bind(&ExtremumCenter::addChildHandler,this,std::placeholders::_1);
  bfs->removeChild = std::bind(&ExtremumCenter::removeChildHandler,this,std::placeholders::_1);

  bfs->getBFSDataHeader = std::bind(&ExtremumCenter::getBFSDataHeader,this);
  bfs->getBFSDataBack = std::bind(&ExtremumCenter::getBFSDataBack,this);
}

void ExtremumCenter::forwardFarthest() {
  vector<distance_t>& distances = initiatorElection->traversal.branchHeight;
  vector<distance_t>::iterator it = std::max_element(distances.begin(), distances.end());
  int indexMax = std::distance(distances.begin(), it);
  Message *m = new ExtremumCenterMessageNext(0, bfsAlg->sysSize);
  P2PNetworkInterface *next = module->getInterface(indexMax);
  
  assert(initiatorElection->traversal.tree.children.find(next) != initiatorElection->traversal.tree.children.end());
  next->send(m);
}

void ExtremumCenter::forwardMaxSumDistances() {  
  vector<longDistance_t>::iterator it = std::max_element(childrenMaxSumDistances.begin(), childrenMaxSumDistances.end());
  int indexMax = std::distance(childrenMaxSumDistances.begin(), it);
  P2PNetworkInterface *next = module->getInterface(indexMax);
  Message *m = new ExtremumCenterMessageNext(*it, bfsAlg->sysSize);
 
#ifdef DEBUG_ALGORITHM
  MY_CERR << "MaxSumDistances: "
	  << *it
	  << endl;
#endif

  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}

void ExtremumCenter::forwardCenter() {
  vector<longDistance_t>::iterator it = std::min_element(childrenMinEcc.begin(), childrenMinEcc.end());
  int indexMin = std::distance(childrenMinEcc.begin(), it);
  P2PNetworkInterface *next = module->getInterface(indexMin);
  Message *m = new ExtremumCenterMessageNext(*it,bfsAlg->sysSize);
  
#ifdef DEBUG_ALGORITHM
  MY_CERR << "Forward center: "
	  << *it
	  << endl;
#endif

  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}
