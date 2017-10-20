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

#include "abcCenterV2.h"

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
//#define DEBUG_FARTHEST
//#define DEBUG_FORWARD_CLOSE_CANDIDATE

ABCCenterV2::ABCCenterV2(BuildingBlock *host): ElectionAlgorithm(host) {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();

  childrenMinCandidateGradient.resize(d,0);
  childrenMaxCandidateDistance.resize(d,0);
  childrenNumCandidates.resize(d,0);
  
  initiatorElection = NULL;
  bfsAlg = NULL;
  convergecast = NULL;
  convergecastAlg = NULL;
  
  reset();
}

ABCCenterV2::~ABCCenterV2() {
  delete bfsAlg;
  delete convergecastAlg;
  delete convergecast;
  delete initiatorElection;
}

void ABCCenterV2::reset() {
  
  delete bfsAlg;
  delete convergecastAlg;
  delete convergecast;
  delete initiatorElection;
  
  initiatorElection = new InitiatorElectionSP(module);
  bfs = &(initiatorElection->traversal);
    //new BFSTraversalSP();
  bfsAlg = new BFSTraversalSPAlgorithm(module);
  //bfsAlg->reset(bfs);
  
  convergecast = new TreeConvergecast(bfs->tree);
  convergecastAlg = new TreeConvergecastAlgorithm(module);

  candidate = true;
  iterations = 0;
  
  resetHandler();
}

size_t ABCCenterV2::size() {
  degree_t d = childrenMinCandidateGradient.size();
  size_t s = convergecast->size()
    + sizeof(bool) // candidate
    + sizeof(uint8_t) // iterations
    + sizeof(distancesBC_t)
    + bfs->size()
    //+ d * sizeof(distance_t) // branchHeight => can use an ohter array below
#ifdef TWO_WAY_BFS_SP
    + 2 * sizeof(distance_t) + 2 * sizeof(degree_t) // max candidate distance + min gradient + interface index
    + sizeof(uint8_t) // num candidates
#else
    + 2 * d * sizeof(distance_t) // max candidate distance + min gradient
    + d * sizeof(uint8_t) // num candidates
    + (d+1)*sizeof(sysSize_t) // used to ensure the correctness of the agg computation (one to keep / the other computed each time)
#endif
    ;
  return s;
}

void ABCCenterV2::init() {
  initiatorElection->init();
}

void ABCCenterV2::start() {
  bool converged = initiatorElection->start();
  if (converged) {
    win(); // a single module!
    cerr << "number of steps: " << 0 << endl;
  }
}

void ABCCenterV2::handle(MessagePtr m) {
  if (initiatorElection->hasToHandle(m)) {
    bool initiatorElected = initiatorElection->handle(m);
    if (initiatorElected) {
#ifdef DEBUG_ALGORITHM
      MY_CERR << "Initiator elected!" << endl;
#endif

      bfsAlg->setSysSize(bfs->getSize());
      
      MY_CERR << getProgressionString(iterations) << endl;
      
      // check size
      
      //forward to farthest
      forwardFarthestCandidate();
    }
  }
  else if (bfsAlg->hasToHandle(m)) {
    // check iteration
    BFSTraversalSPMessagePtr mes =  std::static_pointer_cast<BFSTraversalSPMessage>(m);
    ABCCenterV2BFSHeader* header = (ABCCenterV2BFSHeader*) mes->header;
    
    if (iterations == 0) { // first BFS computed
      initFirstIteration();
    }
    
    if(header->iteration > iterations) {
      assert(header->iteration - iterations == 1);
      iterations = header->iteration;
      initNextIteration(mes->rootID);
    }

    if (header->iteration == iterations) {
      bool bfsCompleted = bfsAlg->handle(mes,bfs);
      if (bfsCompleted) {
#ifdef DEBUG_ALGORITHM
	MY_CERR << "BFS Completed!"
		<< getProgressionString(iterations)
		<< endl;
#endif

	role_t r = getRole(iterations);

	if (r == A || r == B) {
	  forwardFarthestCandidate();
	} else if (r == C) {
	  // Locally update 
	  if (getGradient() > getMinGradient()){
	    candidate = false;
	  }
	  // check num candidates
	  convergecastAlg->startBroadcast(convergecast);
	}
      }
    } 
  } else if (m->type == ABCCENTERV2_NEXT) {
    ABCCenterV2MessageNextPtr mes =  std::static_pointer_cast<ABCCenterV2MessageNext>(m); 
    role_t r = getRole(iterations);

    bfsAlg->setSysSize(mes->sysSize);
    
#ifdef DEBUG_ALGORITHM
    MY_CERR << "NEXT<"
	    << mes->value << ">"
	    << endl;
#endif

    if((iterations == 0 && bfs->tree.isALeaf()) ||
       ((candidate) &&
	((r == C && candidate) ||
	 (r < C && iterations > 0 && bfs->tree.distance == mes->value)))) {
      
      if (iterations == 0) {
	initFirstIteration();
      }
#ifdef DEBUG_ALGORITHM
      MY_CERR << "Farthest from initiator reached!" << endl;
#endif
      initNextIteration(module->blockId);
      iterations++;
#ifdef DEBUG_ALGORITHM
      MY_CERR << "BFS Starting..."
	      << getProgressionString(iterations)
	      << endl;
#endif

      r = getRole(iterations);
      if (r == B || r == C) {
	candidate = false;
      }

      MY_CERR << getProgressionString(iterations) << " (candidate: " << candidate << ") at " << getScheduler()->now() << endl;

      bfsAlg->start(bfs);
      
    } else {
      if (r < C) {
	forwardFarthestCandidate();
      } else {
	forwardCloseCandidate();
      }
    }
    
  } else if (convergecastAlg->hasToHandle(m)) {
    bool completed = convergecastAlg->handle(m,convergecast);
    if (completed) {
      TreeConvergecastBackMessagePtr mes = std::static_pointer_cast<TreeConvergecastBackMessage>(m);
      //ABCCenterV2ConvergecastBackData* data = (ABCCenterV2ConvergecastBackData*) mes->backData;

      uint c = getNumCandidates();
      
#ifdef DEBUG_ALGORITHM
      MY_CERR << "Convergecast terminated "
	      << getProgressionString(iterations)
	      << ": " << c << " candidates."
	      << endl;
#endif

      
      if (c >= 3) {
	// next step
	forwardCloseCandidate();
      } else {
	// over, forward to center
	forwardCenter();
      }
    }
  } else if (m->type == ABCCENTERV2_CENTER) {
    ABCCenterV2MessageCenterPtr mes =  std::static_pointer_cast<ABCCenterV2MessageCenter>(m); 
    distance_t g = mes->gradient;
    if (candidate && getGradient() == g) {
      uint s = getNumStep(iterations);
      win();      
      cerr << "number of steps: " << s << endl; 
    } else {
      forwardCenter();
    }
  }
  else {
    assert(false); // unknown msg
  }
}

void ABCCenterV2::initFirstIteration() {
  //estimatedEcc = initiatorElection->traversal.tree.distance;
  //sum += estimatedEcc;
  setHandlers();
}

void ABCCenterV2::initNextIteration(mID_t rootID) {
  //sum += bfs->tree.distance; // previous iteration
  //estimatedEcc = max(estimatedEcc,bfs->tree.distance);

  bfsAlg->reset(bfs);
  bfs->subscribe(rootID);
}

void ABCCenterV2::handle(EventPtr e) {

}

void ABCCenterV2::addChildHandler(MessagePtr msg) {
  BFSTraversalSPMessagePtr m = std::static_pointer_cast<BFSTraversalSPMessage>(msg);
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  ABCCenterV2BFSBackData* data = (ABCCenterV2BFSBackData*) m->backData;

#ifdef DEBUG_ALGORITHM
  MY_CERR << "ADD_CHILD<"
	  << data->minCandidateGradient << ","
	  << data->maxCandidateDistance << ","
	  << (unsigned) data->numCandidates
	  << ">" << endl;
#endif

  childrenMinCandidateGradient[fromIndex] = data->minCandidateGradient;
  childrenMaxCandidateDistance[fromIndex] = data->maxCandidateDistance;
  childrenNumCandidates[fromIndex] = data->numCandidates;
}

void ABCCenterV2::removeChildHandler(MessagePtr m) {
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  
  childrenMinCandidateGradient[fromIndex] = numeric_limits<distance_t>::max();
  childrenMaxCandidateDistance[fromIndex] = 0;
  childrenNumCandidates[fromIndex] = 0;
}

void ABCCenterV2::resetHandler() {
  role_t r = getRole(iterations);
  
  for (size_t i = 0; i < childrenNumCandidates.size(); i++) {
    childrenMinCandidateGradient[i] = numeric_limits<distance_t>::max();
    childrenMaxCandidateDistance[i] = 0;
    childrenNumCandidates[i] = 0;
  }

  if (r == B) {
    distancesBC.B = bfs->tree.distance;
  }

  if (r == C) {
    distancesBC.C = bfs->tree.distance;
  }
  
}

BFSData* ABCCenterV2::getBFSDataHeader() {
  return new ABCCenterV2BFSHeader(iterations);
}

BFSData* ABCCenterV2::getBFSDataBack() {
  distance_t f = 0;
  distance_t g = 0;
  uint8_t c = 0;
  
  if (candidate) {
    f = bfs->tree.distance;
    g = getGradient();
  } else {
    g = numeric_limits<distance_t>::max();
    f = 0;
  }
 
  // aggergate with children info
  f = std::max(f,*std::max_element(childrenMaxCandidateDistance.begin(),childrenMaxCandidateDistance.end()));
  g = std::min(g,*std::min_element(childrenMinCandidateGradient.begin(),childrenMinCandidateGradient.end()));
  c = getNumCandidates();
  
  return new ABCCenterV2BFSBackData(g,f,c);
}

void ABCCenterV2::convergecastGoHandler(MessagePtr msg) {
  TreeConvergecastGoMessagePtr m = std::static_pointer_cast<TreeConvergecastGoMessage>(msg);
  ABCCenterV2ConvergecastGoHeader* data = (ABCCenterV2ConvergecastGoHeader*) m->header;

  /*for (size_t i = 0; i < childrenNumCandidates.size(); i++) {
    childrenMinCandidateGradient[i] = numeric_limits<distance_t>::max();
    childrenMaxCandidateDistance[i] = 0;
    childrenNumCandidates[i] = 0;
    }*/
  
  if (getGradient() > data->minCandidateGradient) {
    candidate = false;
  }
}

void ABCCenterV2::convergecastBackHandler(MessagePtr msg) {
  TreeConvergecastBackMessagePtr m = std::static_pointer_cast<TreeConvergecastBackMessage>(msg);
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  ABCCenterV2ConvergecastBackData* data = (ABCCenterV2ConvergecastBackData*) m->backData;

  childrenMinCandidateGradient[fromIndex] = data->minCandidateGradient;
  childrenMaxCandidateDistance[fromIndex] = data->maxCandidateDistance;
  childrenNumCandidates[fromIndex] = data->numCandidates;
  
}

BFSData* ABCCenterV2::getConvergecastDataHeader() {

  if (convergecast->tree.isRoot()) {
    distance_t g = numeric_limits<distance_t>::max();
    if (candidate) {
      g = getGradient();
    }    
    g = std::min(g,*std::min_element(childrenMinCandidateGradient.begin(),childrenMinCandidateGradient.end()));
    return new ABCCenterV2ConvergecastGoHeader(g);
  }
  
  return NULL;
}

BFSData* ABCCenterV2::getConvergecastDataBack() {

  distance_t f = 0;
  distance_t g = 0;
  uint8_t c = 0;
  
  if (candidate) {
    f = bfs->tree.distance;
    g = getGradient();
  } else {
    g = numeric_limits<distance_t>::max();
    f = 0;
  }
 
  // aggergate with children info
  f = std::max(f,*std::max_element(childrenMaxCandidateDistance.begin(),childrenMaxCandidateDistance.end()));
  g = std::min(g,*std::min_element(childrenMinCandidateGradient.begin(),childrenMinCandidateGradient.end()));
  c = getNumCandidates();
  
  return new ABCCenterV2ConvergecastBackData(g,f,c);
}

void ABCCenterV2::setHandlers() {
  bfs->resetHandler = std::bind(&ABCCenterV2::resetHandler,this);
  bfs->addChild = std::bind(&ABCCenterV2::addChildHandler,this,std::placeholders::_1);
  bfs->removeChild = std::bind(&ABCCenterV2::removeChildHandler,this,std::placeholders::_1);

  bfs->getBFSDataHeader = std::bind(&ABCCenterV2::getBFSDataHeader,this);
  bfs->getBFSDataBack = std::bind(&ABCCenterV2::getBFSDataBack,this);

  convergecast->goHandler = std::bind(&ABCCenterV2::convergecastGoHandler,this,std::placeholders::_1);
  convergecast->backHandler = std::bind(&ABCCenterV2::convergecastBackHandler,this,std::placeholders::_1);
  
  convergecast->getHeader = std::bind(&ABCCenterV2::getConvergecastDataHeader,this);
  convergecast->getBackData = std::bind(&ABCCenterV2::getConvergecastDataBack,this);

}

void ABCCenterV2::forwardFarthest(vector<distance_t>& distances) {
  vector<distance_t>::iterator it = std::max_element(distances.begin(), distances.end());
  int indexMax = std::distance(distances.begin(), it);
  Message *m = new ABCCenterV2MessageNext(*it,bfsAlg->sysSize);
  P2PNetworkInterface *next = module->getInterface(indexMax);

#ifdef DEBUG_FARTHEST
  MY_CERR << "v= " << *it << endl;
#endif
  
  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}

void ABCCenterV2::forwardFarthestCandidate() {
  uint8_t s = getNumStep(iterations);
  role_t r = getRole(iterations);

#ifdef DEBUG_FARTHEST_2
  MY_CERR << "iterations: " << (uint) iterations << endl;
  MY_CERR << "steps: " << (uint) s << endl;
  MY_CERR << "role: " << (uint) r << endl;
  MY_CERR << "A: " << (uint) A << endl;
#endif
  
  if(iterations == 0) {
#ifdef DEBUG_FARTHEST
    MY_CERR << "Farthest from initiator: ";
#endif    
    forwardFarthest(initiatorElection->traversal.branchHeight);
  } else {
    forwardFarthest(childrenMaxCandidateDistance);
  }
}

void ABCCenterV2::forwardCloseCandidate() {
  vector<distance_t> &distances = childrenMaxCandidateDistance;
  vector<distance_t> &candidates = childrenNumCandidates;

  P2PNetworkInterface *next;
  int8_t minIndex = -1;
  
  for (int8_t i = 0; i < distances.size(); i++) {
#ifdef DEBUG_FORWARD_CLOSE_CANDIDATE  
    MY_CERR << distances[i] << endl;
#endif
    next = module->getInterface(i);
    bool child = bfs->tree.children.find(next) != bfs->tree.children.end();
    if((minIndex == -1) && candidates[i] > 0) {
      minIndex = i;
    } else if ((distances[i] < distances[minIndex]) && candidates[i] > 0 ) {
      minIndex = i;
    }
  }

  distance_t v = distances[minIndex];
  Message *m = new ABCCenterV2MessageNext(v,bfsAlg->sysSize);
  next = module->getInterface(minIndex);
  
#ifdef DEBUG_FORWARD_CLOSE_CANDIDATE
  MY_CERR << "v= " << v << endl;
#endif
  
  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}

void ABCCenterV2::forwardCenter() {
  vector<distance_t>::iterator it = std::min_element(childrenMinCandidateGradient.begin(), childrenMinCandidateGradient.end());
  int indexMax = std::distance(childrenMinCandidateGradient.begin(), it);
  P2PNetworkInterface *next = module->getInterface(indexMax);
  Message *m = new ABCCenterV2MessageCenter(*it);
  
#ifdef DEBUG_ALGORITHM
  MY_CERR << "Forward center: "
	  << *it
	  << endl;
#endif

  assert(bfs->tree.children.find(next) != bfs->tree.children.end());
  next->send(m);
}

uint8_t ABCCenterV2::getNumStep(uint8_t i) {
  return (i/3) + 1;
}

role_t ABCCenterV2::getRole(uint8_t i) {
  uint8_t s = getNumStep(i)-1;
  uint8_t r = i - s * 3;
  
  assert(r <= 4);
  return role_t(r);
}

distance_t ABCCenterV2::getGradient() {
  return std::abs(((int32_t) distancesBC.B) -
		  ((int32_t) distancesBC.C));
}

uint8_t ABCCenterV2::getNumCandidates() {
  uint8_t r = 0;
  sysSize_t c = 0;
  if (candidate) {
    c++;
  }

  for (int i = 0; i < childrenNumCandidates.size(); i++) {
    c += childrenNumCandidates[i];
  }

  if (c > numeric_limits<uint8_t>::max()) {
    r = numeric_limits<uint8_t>::max();
  } else {
    r = (uint8_t) c;
  }

  return r;
}

distance_t ABCCenterV2::getMinGradient() {
  distance_t g = getGradient();
  g = std::min(g,*std::min_element(childrenMinCandidateGradient.begin(),childrenMinCandidateGradient.end()));
  return g;
}

std::string ABCCenterV2::toString(role_t r) {
  switch(r) {
  case A:
    return "A";
  case B:
    return "B";
  case C:
    return "C";
  case center:
    return "center";
  default:
    return "ERROR: Unknown role_t value!";
  }
}

std::string ABCCenterV2::getProgressionString(uint8_t i) {
  return "(step: " + to_string((uint) getNumStep(i)) + ",role:" + toString(getRole(i)) + ")" ; 
}
