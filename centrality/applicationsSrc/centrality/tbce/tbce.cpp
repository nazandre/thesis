/*
 * tbce.cpp
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>

#include "tbce.h"

#include "scheduler.h"
#include "network.h"
#include "trace.h"
#include "world.h"

#include "../simulation.h"

#include "../initiatorMsg.h"
#include "../utils/def.h"

#include "messages.h"
#include "bfsData.h"

//#define DEBUG_TBCE

using namespace std;
using namespace BaseSimulator;

TBCE::TBCE(BuildingBlock *host): ElectionAlgorithm(host) {
  initiatorElection = new InitiatorElectionSP(host);
  bfs = &(initiatorElection->traversal);
  treeElection = new TreeElection<mID_t>(bfs->tree);
}

TBCE::~TBCE(){
  delete initiatorElection;
  delete treeElection;
}

/*
  bool candidate = (tbCentrality == height);
					
  if (candidate) {
  //cout << "candidate " << module->blockId << "(" << tbCentrality << "," << height << ")" << endl;
  minTBCenterId = module->blockId;
  }	
*/

size_t TBCE::size() {
  degree_t d = module->getNbInterfaces();
  
  size_t s = bfs->size() +
#ifdef TWO_WAY_BFS_SP
    + sizeof(longDistance_t) // initiator farness
    + sizeof(distance_t) // height
    + sizeof(sysSize_t) // system size
#else
    + d * sizeof(longDistance_t) // initiator farness
    + d * sizeof(distance_t) // branchHeight
    + d * sizeof(sysSize_t) // size of system
#endif
    // final election on tree
    + sizeof(distance_t)
    + sizeof(degree_t) // received
    + sizeof(degree_t) // next hop
    ;
  return s;
}

void TBCE::init(){
  setHandlers();

  treeElection->id = numeric_limits<mID_t>::max();
  treeElection->path = NULL;
  height = numeric_limits<mID_t>::max();
  initiatorFarness = 0;
  
  initiatorElection->init();
}

void TBCE::start() {

  bool converged = initiatorElection->start();
  if (converged) {
    win(); // a single module!
  }
}

void TBCE::handle(MessagePtr m) {
  P2PNetworkInterface *from = m->destinationInterface;
  
  if (initiatorElection->hasToHandle(m)) {
    bool initiatorElected = initiatorElection->handle(m);
    if (initiatorElected) {
      sysSize_t sysSize = bfs->getSize();
      distance_t tbCentrality = initiatorFarness / (sysSize - 1);
      height = bfs->getHeight();
      
      cerr << "distributed system size: " << sysSize << endl;
      cerr << "distributed tree height: " << height << endl;
      cerr << "distributed depthSum: " << initiatorFarness << endl;
      cerr << "Distributed TB Centrality: " << tbCentrality << endl;

      assert(sysSize == getWorld()->getSize());

      if (tbCentrality == height) {
	MY_CERR << "candidate!" << endl;
	treeElection->id = module->blockId;
      }
      
      spreadTBCentralityGoLeaderMsg(tbCentrality);
    }
  } else {
    switch(m->type) {
    case TB_CENTRALITY_GO_LEADER_MESSAGE: {
      TBCentralityGoLeaderMsg_ptr recv = std::static_pointer_cast<TBCentralityGoLeaderMsg>(m);
      distance_t tbc = recv->tbCentrality;
      height = bfs->getHeight();
      
#ifdef DEBUG_TBCE
      MY_CERR << "TBCE_LEADER_GO<"
	      << "tbc=" << tbCentrality
	      << ">"
	      << endl;
#endif
      
      // only one go msg
      if (tbc == height) {
	MY_CERR << "candidate!" << endl;
	treeElection->id = module->blockId;
      }
      
      if (bfs->tree.isALeaf())  {
	 spreadTBCentralityBackLeaderMsg();
      } else {
	spreadTBCentralityGoLeaderMsg(tbc);
      }
    }
      break;
    case TB_CENTRALITY_BACK_LEADER_MESSAGE: {
      TBCentralityBackLeaderMsg_ptr recv = std::static_pointer_cast<TBCentralityBackLeaderMsg>(m);
#ifdef DEBUG_TBCE
      MY_CERR << "TBCE_LEADER_BACK<"
	      << "id=" << recv->id
	      << ">"
	      << endl;
#endif

      if (recv->id < treeElection->id) {
	treeElection->id = recv->id;
	treeElection->path = from;
      }

      treeElection->received.insert(from);

      if (treeElection->received.size() == bfs->tree.children.size()) {
	if (bfs->tree.isRoot()) {
	  if (treeElection->id == module->blockId) {
	    MY_CERR << "Tree-based center distributively elected: " << module->blockId << "(" << 0 << ")" << endl;
	    win();
	  } else {
	    assert(treeElection->path);
	    spreadTBCentralityLeaderMsg();
	  }   
	} else {
	   spreadTBCentralityBackLeaderMsg();
	}
      }
    }
      break;
    case TB_CENTRALITY_LEADER_MESSAGE: {
      TBCentralityLeaderMsg_ptr recv = std::static_pointer_cast<TBCentralityLeaderMsg>(m);
      if (recv->id == module->blockId) {
	cout << "Tree-based center distributively elected: " << module->blockId << "(" << height << ")" << endl;
	win();
      } else {
	assert(treeElection->path != NULL);
	spreadTBCentralityLeaderMsg();
      }
    }
      break;
    default:
      cerr << "unknown message" << endl;
    }
  }
}

void TBCE::handle(EventPtr e) {

}

void TBCE::setHandlers() {
  bfs->resetHandler = std::bind(&TBCE::resetHandler,this);
  bfs->addChild = std::bind(&TBCE::addChildHandler,this,std::placeholders::_1);
  bfs->getBFSDataBack = std::bind(&TBCE::getBFSDataBack,this);
}

void TBCE::resetHandler() {
  initiatorFarness = bfs->tree.distance;
}

BFSData* TBCE::getBFSDataBack() {
  // insert initiatorFarness;
  return new TBCEBFSBackData(initiatorFarness);
}

void TBCE::addChildHandler(MessagePtr mes) {
  TBCEBFSBackData* data = NULL;

  if(mes->type == IE_SP_BACK_MESSAGE) {
    cerr << "ONE-PHASE BFS_SP + CONVERGECAST not supported in TBCE." << endl;
    assert(false);
  }

  if (mes->type == IE_SP_CONVERGECAST_BACK_MESSAGE) {
    IESPBackMsgPtr m =  std::static_pointer_cast<IESPBackMsg>(mes);
    data = (TBCEBFSBackData*) m->backData;
  }

  assert(data);

  initiatorFarness += data->farness;  
}

void TBCE::spreadTBCentralityGoLeaderMsg(distance_t tbc) {
  P2PNetworkInterface *p = NULL;
  std::set<P2PNetworkInterface*>::iterator it;
  
  for (it = bfs->tree.children.begin(); it !=bfs->tree.children.end(); it++) {
    TBCentralityGoLeaderMsg * message = new TBCentralityGoLeaderMsg(tbc);
    p = *it;
    p->send(message);
  }
}

void TBCE::spreadTBCentralityBackLeaderMsg() {	
  TBCentralityBackLeaderMsg * message = new TBCentralityBackLeaderMsg(treeElection->id);
  assert(bfs->tree.parent);
  bfs->tree.parent->send(message);
}

void TBCE::spreadTBCentralityLeaderMsg() {
  TBCentralityLeaderMsg * message = new TBCentralityLeaderMsg(treeElection->id);
  assert(treeElection->path);
  treeElection->path->send(message);
}
