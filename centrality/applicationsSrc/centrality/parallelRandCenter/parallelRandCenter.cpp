/*
 * parallelRandCenter.cpp
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

#include "parallelRandCenter.h"

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

#define NUM_BFSS 10

unsigned int ParallelRandCenter::numBFS = 0;

ParallelRandCenter::ParallelRandCenter(BuildingBlock *host): ElectionAlgorithm(host) {
  reset();
}

ParallelRandCenter::~ParallelRandCenter() {
  delete bfsAlg;
  delete bfs;
}

size_t ParallelRandCenter::size() {
  size_t s =
    + (bfs->size() + sizeof(ParallelBFSTraversalSPKey)) * NUM_BFSS
    + sizeof(uint8_t) // finishedBFSes
    + sizeof(longDistance_t) // estimatedEcc/Farness
    //+ sizeof(uint8_t) // NUM_BFSSS hardcoded!
    + sizeof(distance_t) // randomNumber
    + sizeof(longDistance_t) + sizeof(degree_t) + sizeof(mID_t) // children min farness/ecc + interface index + id
    ;
  return s;
}

void ParallelRandCenter::reset() {
 
  bfsAlg = new BFSTraversalSPAlgorithm(module);
  bfs = new BFSTraversalSP();
    
  treeElection = new TreeElection<longDistance_t>(bfs->tree);
  treeElection->id = numeric_limits<mID_t>::max();
  treeElection->value = numeric_limits<longDistance_t>::max();
  treeElection->path = NULL;
   
  estimatedEcc = 0;
  finishedBFSes = 0;
  
  randomNumber = module->getRandomUint();
}

void ParallelRandCenter::init() {

  // Test set:
#if 0
  if (module->blockId == 1) {

    for (unsigned int i = 0; i < NUM_BFSS + 5; i++) {
      distance_t u = rand();
      ParallelBFSTraversalSPKey key(i,u);
      find(key);
      cerr << "looking for BFS(" << i << "," << u << ")" << endl;
      //printBFSS();
    }
    printBFSS();

    //exit(0);
  }
#endif

  // every module 
  
}

void ParallelRandCenter::start() {
  ParallelBFSTraversalSPKey key(module->blockId,randomNumber);
  bool converged = false;
  
  ParallelBFSTraversalSP *bfs = find(key);
  converged = bfsAlg->start(bfs);
  
  /*if (converged) {
    win(); // a single module!
    MY_CERR << "Elected with value: " << estimatedEcc << endl;
    }*/
}

void ParallelRandCenter::handle(MessagePtr m) {
  P2PNetworkInterface *from = m->destinationInterface;
  
  if (bfsAlg->hasToHandle(m)) {
    // check iteration
    BFSTraversalSPMessagePtr mes =  std::static_pointer_cast<BFSTraversalSPMessage>(m);
    ParallelRandCenterBFSHeader* header = (ParallelRandCenterBFSHeader*) mes->header;
    assert(header);
    ParallelBFSTraversalSPKey &key = header->key;
    ParallelBFSTraversalSP *bfs = find(key);
    
    if (bfs && !(key.id == 0 && key.randomNumber == 0)) {
      bool bfsCompleted = bfsAlg->handle(mes,bfs);      
      if (bfsCompleted) {
    	numBFS++;
	aBFSHasFinished();
	
	MY_CERR << "BFS"
		<< " completed!"
		<< " (random number: " << randomNumber << ")"
		<< " at " << getScheduler()->now()
		<< endl;
	
	//  treeElection->id = module->blockId;
	//  treeElection->value = estimatedEcc;
	// treeElection->path = NULL;	  

	//  broadcastLeaderGoMsg();	  
      }
    }
  } else {

    switch(m->type) {
    case PARALLEL_RAND_CENTER_BACK_LEADER_MESSAGE: {
      ParallelRandCenterBackLeaderMsg_ptr recv = std::static_pointer_cast<ParallelRandCenterBackLeaderMsg>(m);
      
      if (recv->value < treeElection->value ||
	  (recv->value == treeElection->value && recv->id < treeElection->id)) {
        treeElection->value = recv->value;
	treeElection->id = recv->id;
	treeElection->path = from;
      }
      
      treeElection->received.insert(from);
      
      if (finishedBFSes == NUM_BFSS && treeElection->received.size() == bfs->tree.children.size()) {
	
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
    case PARALLEL_RAND_CENTER_LEADER_MESSAGE: {
      ParallelRandCenterLeaderMsg_ptr recv = std::static_pointer_cast<ParallelRandCenterLeaderMsg>(m);
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
}


void ParallelRandCenter::handle(EventPtr e) {
}

void ParallelRandCenter::resetHandler() { // conv go
  aBFSHasFinished();
}

BFSData* ParallelRandCenter::getBFSDataHeader(ParallelBFSTraversalSPKey k) {
  return new ParallelRandCenterBFSHeader(k);
}

void ParallelRandCenter::setHandlers(BFSTraversalSP* t, ParallelBFSTraversalSPKey k) {
  t->getBFSDataHeader = std::bind(&ParallelRandCenter::getBFSDataHeader,
				  this,k);
  t->resetHandler = std::bind(&ParallelRandCenter::resetHandler,this);
}

void ParallelRandCenter::sendLeaderBackMsg() {
  Message *m = new ParallelRandCenterBackLeaderMsg(treeElection->value,treeElection->id);
  assert(!bfs->tree.isRoot());
  bfs->tree.sendParent(m);
}

void ParallelRandCenter::sendLeaderMsg() {
  Message *m = new ParallelRandCenterLeaderMsg(treeElection->value,treeElection->id);
  assert(treeElection->path!=NULL);
  treeElection->path->send(m);
}

ParallelBFSTraversalSP* ParallelRandCenter::find(const ParallelBFSTraversalSPKey &k) {
  std::map<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>::iterator exact;
  std::map<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>::iterator min;
  ParallelBFSTraversalSP *bfs = NULL;
  
  exact = bfss.find(k);
  if (exact != bfss.end()) {
    bfs = exact->second;
  } else if (bfss.size() < NUM_BFSS) {

    bfs = new ParallelBFSTraversalSP();
    bfs->enableConvergecast = true;
    bfss.insert(std::pair<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>
		(k,bfs));
    setHandlers(bfs,k);
    bfs->subscribe(k.id);
  } else {
    //up = bfss.upper_bound(k);
    min = bfss.begin();
    if (min->first < k) {
      delete min->second;
      bfss.erase(min); // really need to re-search by key ?

      bfs = new ParallelBFSTraversalSP();
      bfs->enableConvergecast = true;
      bfss.insert(std::pair<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>
		  (k,bfs));
      setHandlers(bfs,k);
      bfs->subscribe(k.id);
    }
  }
  
  return bfs; 
}

void ParallelRandCenter::printBFSS() {
  std::map<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>::iterator it;
  MY_CERR << "BFSes:" << endl;
  for (it=bfss.begin(); it!=bfss.end(); ++it) {
    ParallelBFSTraversalSPKey k = it->first;
    ParallelBFSTraversalSP *bfs = it->second;
    
    MY_CERR << "BFS( "
	    << "r=" << k.randomNumber
	    << ",id=" << k.id
	    << ",dist=" << bfs->tree.distance
	    << ")"
	    << endl;
  }
}


void ParallelRandCenter::aBFSHasFinished() {
  finishedBFSes++;
  
  if (finishedBFSes == NUM_BFSS) {
    
    setElectionTree();
    computeEccentricity();

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
}

void ParallelRandCenter::setElectionTree() {
  std::map<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>::iterator min = bfss.begin();
  treeElection->tree = min->second->tree;  
}

void ParallelRandCenter::computeEccentricity() {
  std::map<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*>::iterator it;

  for (it=bfss.begin(); it!=bfss.end(); ++it) {
    ParallelBFSTraversalSP *bfs = it->second;
    longDistance_t d =  bfs->tree.distance;  
    if (IS_RUNNING_VERSION(VERSION_CENTER)) {
      estimatedEcc = std::max(estimatedEcc,d);
    } else if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
      estimatedEcc += d;
    }
  }

  if (estimatedEcc < treeElection->value ||
      (estimatedEcc == treeElection->value && module->blockId < treeElection->id)
      ){
    treeElection->id = module->blockId;
    treeElection->value = estimatedEcc;
    treeElection->path = NULL;
  }
}
