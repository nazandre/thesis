#include "alphaSynchronizedBFS.h"

#include "world.h"

//#define DEBUG_AS_BFS_ALGORITHM

namespace AlphaSynchronizedBFS {

  std::string State::toString() {
    if (state == EXACTLY) {
      return "EXACTLY(" + std::to_string(distance) + ")"; 
    }

    if (state == MORE_THAN) {
      return "MORE_THAN(" + std::to_string(distance) +")";
    }

    return "UNKNOWN";
  }
  
  Traversal::Traversal() {
    degree_t d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
    neighborStates.resize(d);
    started = false;
  }
  
  Traversal::Traversal(Traversal &t): BFSTraversalSP(t) {
    neighborStates = t.neighborStates;
    started = t.started;
  }
  
  Traversal::~Traversal() {

  }

  size_t Traversal::size() {
    return tree.size() +
      // numBackReceived:
      sizeof(degree_t) +
      // neighbor and local states:
      (neighborStates.size() + 1) * AlphaSynchronizedBFS::State::getSize()
      ;
  }

  void Traversal::reset() {
    State unknown;
    for (degree_t i = 0; i < neighborStates.size(); i++) {
      neighborStates[i] = unknown;
    }

    tree.children.clear();
    tree.parent = NULL;
    tree.distance = 0;
    tree.rootID = 0;
    state = unknown;

    backSent = false;
    numBackReceived = 0;
    sysSize = 0;
    height = 0;
  }

  void Traversal::subscribe(mID_t id) {
    tree.rootID = id;
  }

  distance_t Traversal::getHeight() {
    return height;
  }
  
  sysSize_t Traversal::getSize() {
    return sysSize;
  }
}


AlphaSynchronizedBFSAlgorithm::AlphaSynchronizedBFSAlgorithm(BaseSimulator::BuildingBlock *m) {
    module = m;
}

AlphaSynchronizedBFSAlgorithm::~AlphaSynchronizedBFSAlgorithm() {
  
}

bool AlphaSynchronizedBFSAlgorithm::start(AlphaSynchronizedBFS::Traversal* t) {
  t->tree.rootID = module->blockId;
  t->state = AlphaSynchronizedBFS::State(0,AlphaSynchronizedBFS::EXACTLY);
  t->started = true;
  //sendStart(t,NULL);
  sendGo(t);
  return false;
}

bool AlphaSynchronizedBFSAlgorithm::handle(MessagePtr m, AlphaSynchronizedBFS::Traversal *t) {
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);

  assert(hasToHandle(m));
  
  //assert(m = t->tree.rootID);
  switch(m->type) {
  case ALPHA_SYNCHRONIZED_BFS_START_MSG:
    {
      AlphaSynchronizedBFSStartMessagePtr recm = std::static_pointer_cast<AlphaSynchronizedBFSStartMessage>(m);

#ifdef DEBUG_AS_BFS_ALGORITHM
      MY_CERR << "START<id=" << recm->rootID << "> from " << from->connectedInterface->hostBlock->blockId << endl;
#endif
      
      /*if (!t->started) {
	assert(t->state.state == AlphaSynchronizedBFS::MORE_THAN && t->state.distance == -1);
	t->tree.rootID = recm->rootID;
	t->started = true;
	t->state.distance = 0;
	sendStart(t,from);
	sendGo(t);
	}*/
    }
    break; 
  case ALPHA_SYNCHRONIZED_BFS_GO_MSG:
    {
      AlphaSynchronizedBFSGoMessagePtr recm = std::static_pointer_cast<AlphaSynchronizedBFSGoMessage>(m);      
#ifdef DEBUG_AS_BFS_ALGORITHM
      MY_CERR << "GO<id=" << recm->rootID << ",d=" << recm->state.toString() << "> from " << from->connectedInterface->hostBlock->blockId << endl;
#endif
      
      if (!t->started) {
	assert(t->state.state == AlphaSynchronizedBFS::MORE_THAN && t->state.distance == -1);
	t->tree.rootID = recm->rootID;
	t->started = true;
	t->state.distance = 0;
	
	//sendStart(t,from);
	sendGo(t);
      }
      
      if(t->election) {
	if (recm->rootID < t->tree.rootID) {
	  //distance_t round = t->state.distance;
	  //t->reset();
	  
	  for (degree_t i = 0; i < t->neighborStates.size(); i++) {
	    t->neighborStates[i].state =  AlphaSynchronizedBFS::MORE_THAN;
	  }

	  t->tree.children.clear();
	  t->tree.parent = NULL;
	  t->tree.distance = 0;
	  t->tree.rootID = 0;
	  //state = unknown;
	  
	  t->backSent = false;
	  t->numBackReceived = 0;
	  t->sysSize = 0;
	  t->height = 0;

	  t->subscribe(recm->rootID);
	  t->tree.parent = from;
	  t->state.state = AlphaSynchronizedBFS::MORE_THAN;
	  //t->state.distance = recm->state.distance - 1;
	  //t->state.distance = round;
	  //sendGo(t);
	} else if (recm->rootID > t->tree.rootID) {
	  return false; // ignore that message
	}
      }

      assert(recm->rootID == t->tree.rootID);
      assert(t->started);
      
      t->neighborStates[fromIndex] = recm->state;

      if (recm->state.state == AlphaSynchronizedBFS::EXACTLY && recm->state.distance == 0) {
	t->state.distance = 1;
	t->state.state = AlphaSynchronizedBFS::EXACTLY;
	t->tree.parent = from;
	sendGo(t);
      }

      if (t->state.state != AlphaSynchronizedBFS::EXACTLY) {

	// rule 1: I can send exactly(d) as soon as I have received exactly(d-1) from at least one neighbor and more-than(d-2) from all neighbors.
	// rule 2: I can send more-than(d) if d = 0 or as soon as I have received more-than(d-1) from all neighbors.

	// better neighbor (min excatly, if any)	
        int16_t b = -1;

	for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
	  P2PNetworkInterface *p = module->getInterface(i);

	  if (p->isConnected()) {

	    if (t->neighborStates[i].distance == -1) {
	      return false;
	    }
	    
	    if ((b == -1 && t->neighborStates[i].state == AlphaSynchronizedBFS::EXACTLY) ||
		(b != -1 && t->neighborStates[i].state == AlphaSynchronizedBFS::EXACTLY && t->neighborStates[i].distance < t->neighborStates[b].distance)) {
	      b = i;
	    }
	  }
	}

	// Minimum MORE-THAN received
	distance_t minDistance = 999;
	for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
	  P2PNetworkInterface *p = module->getInterface(i);
	  if (p->isConnected() && t->neighborStates[i].state == AlphaSynchronizedBFS::MORE_THAN) {
	    minDistance = std::min(minDistance, (distance_t)t->neighborStates[i].distance);
	  }
	}

	// Received an EXACTLY
	if (b != -1) {
	  distance_t d = t->neighborStates[b].distance;
	  distance_t criteria = d-1;
#ifdef DEBUG_AS_BFS_ALGORITHM
	  cerr << "\t best: " << t->neighborStates[b].toString() << ", criteria: " << criteria << ", minD: " << minDistance << endl;
#endif
	  // EXACTLY(d) received, at least MORE-THAN(d-1) received from all the other neighbors ?
	  if (minDistance >= criteria) {
	    t->state.distance = d+1;
	    t->state.state = AlphaSynchronizedBFS::EXACTLY;
	    t->tree.parent = module->getInterface(b);
#ifdef DEBUG_AS_BFS_ALGORITHM	    
	    MY_CERR << "send " << t->state.toString() << endl;
#endif
	    sendGo(t); // advertise the modules
	  }
	} else {
	  // no exact msg received
	  // received more-than(current state) from all the neighbors ?
	  
#ifdef DEBUG_AS_BFS_ALGORITHM
	  cerr << "\t nextRound?: " << " minDistance: " << minDistance << " thisDistance: " << t->state.distance << endl;
#endif
	  
	  // d-1 
	  if (minDistance >= t->state.distance) {
	    t->state.distance++; // d
	    sendGo(t);
	  }
	}
      }

      if (t->state.state == AlphaSynchronizedBFS::EXACTLY) {
	// if converges
	bool leafAndConverged = true;

	// all neighbor have converged with a value <= to mine (I'm a leaf)
	for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
	  P2PNetworkInterface *p = module->getInterface(i);
	  if (p->isConnected()) {
	    
	    if (t->neighborStates[i].state != AlphaSynchronizedBFS::EXACTLY) {
	      leafAndConverged = false;
	    } else if (t->neighborStates[i].state == AlphaSynchronizedBFS::EXACTLY &&
		       t->neighborStates[i].distance > t->state.distance) {
	      leafAndConverged = false;
	    }

	  }
	}

	if (leafAndConverged) {
	  // send back<yes> to parent, and send back<no> to others with same value (to unlock them)
	  AlphaSynchronizedBFSBackMessage *msg = NULL;

	  if (t->backSent) {
	    return false;
	  }
	  
	  assert(!t->backSent);
	  t->backSent = true;
	  
	  msg = new AlphaSynchronizedBFSBackMessage(t->tree.rootID,1,0,true);
	  t->tree.parent->send(msg);

	  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
	    P2PNetworkInterface *p = module->getInterface(i);
	    if (p->isConnected() &&
		t->neighborStates[i].distance == (t->state.distance-1) &&
		t->tree.parent != p) {
	      msg = new AlphaSynchronizedBFSBackMessage(t->tree.rootID,0,0,false);
	      p->send(msg);
	    }
	  }
	}
      }
      
    }
    break;    
  case ALPHA_SYNCHRONIZED_BFS_BACK_MSG:
    {
      AlphaSynchronizedBFSBackMessagePtr recm = std::static_pointer_cast<AlphaSynchronizedBFSBackMessage>(m);

      if (t->election) {
	assert(recm->rootID >= t->tree.rootID);
	if (recm->rootID != t->tree.rootID) {
	  return false;
	}
      }
      
      assert(recm->rootID == t->tree.rootID);
#ifdef DEBUG_AS_BFS_ALGORITHM
      MY_CERR << "BACK<id=" << recm->rootID
	      << ", h=" << recm->height
	      << ", s=" << recm->sysSize
	      << ", c=" << recm->child
	      << "> from " << from->connectedInterface->hostBlock->blockId << endl;
#endif

      degree_t numWaitingBack = 0;

      t->numBackReceived++;

      t->height = std::max(t->height,(distance_t)(recm->height+1));
      t->sysSize += recm->sysSize;
      
      for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
	P2PNetworkInterface *p = module->getInterface(i);
	if (p->isConnected()) {
	  
	  if (t->neighborStates[i].state != AlphaSynchronizedBFS::EXACTLY) {
	    return false;
	  }
	  
	  if (t->neighborStates[i].distance == (t->state.distance+1)) {
	    numWaitingBack++;
	  }
	}
      }
      
#ifdef DEBUG_AS_BFS_ALGORITHM
      MY_CERR << (uint16_t)t->numBackReceived << " vs " << (uint16_t)numWaitingBack << endl;
#endif
      
      //assert(t->numBackReceived <= numWaitingBack);
      
      if (numWaitingBack == t->numBackReceived) {

	if (t->tree.rootID == module->blockId) {
	  t->sysSize += 1;

	  MY_CERR << " BFS completed (ALPHA-SYNCHRONIZED_BFS-SP) at " << getScheduler()->now()
		  << " size: " << t->sysSize
		  << " ecc: " << t->height
		  << endl;

	  assert(t->sysSize == (sysSize_t) getWorld()->getSize());	  
	  return true;
	} else {
	  AlphaSynchronizedBFSBackMessage *msg = NULL;

	  assert(!t->backSent);
	  t->backSent = true;
	
	  msg = new AlphaSynchronizedBFSBackMessage(t->tree.rootID,t->sysSize+1,t->height,true);
	  t->tree.parent->send(msg);
	
	  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
	    P2PNetworkInterface *p = module->getInterface(i);
	    if (p->isConnected() &&
		t->neighborStates[i].distance == (t->state.distance-1) &&
		t->tree.parent != p) {
	      msg = new AlphaSynchronizedBFSBackMessage(t->tree.rootID,0,0,false);
	      p->send(msg);
	    }
	  }
	}
      }
    }
    break;
  default:
    assert(false);
  };

  return false;
}

bool AlphaSynchronizedBFSAlgorithm::hasToHandle(MessagePtr m) {
  return m->type == ALPHA_SYNCHRONIZED_BFS_GO_MSG || m->type == ALPHA_SYNCHRONIZED_BFS_BACK_MSG || m->type == ALPHA_SYNCHRONIZED_BFS_START_MSG;
}

//t->neighborStates[i].state != AlphaSynchronizedBFS::EXACTLY
void AlphaSynchronizedBFSAlgorithm::sendGo(AlphaSynchronizedBFS::Traversal *t) {
  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = module->getInterface(i);
    if (p->isConnected()) {
      AlphaSynchronizedBFSGoMessage *msg = new AlphaSynchronizedBFSGoMessage(t->tree.rootID, t->state);
      p->send(msg);
    }
  }
}

void AlphaSynchronizedBFSAlgorithm::sendStart(AlphaSynchronizedBFS::Traversal *t, P2PNetworkInterface *ignore) {
  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = module->getInterface(i);
    if (p->isConnected() && p != ignore) {
      AlphaSynchronizedBFSStartMessage *msg = new AlphaSynchronizedBFSStartMessage(t->tree.rootID);
      p->send(msg);
    }
  }
}
// messages
