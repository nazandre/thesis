#include "bfsTree.h"

#include <climits>

#include "scheduler.h"

#include "../timeSyncBlockCode.h"
#include "../layers.h"

#include "bfsTreeMsg.h"
#include "bfsTreeEv.h"

using namespace std;
using namespace BaseSimulator;

/*********** Tree Class ************/
Tree::Tree() {
  rootID = NOT_A_VALID_ID;
  parent = NULL;
  distance = numeric_limits<hopDistance_t>::max();
}

Tree::Tree(mID_t rID) {
  rootID = rID;
  parent = NULL;
  distance = numeric_limits<hopDistance_t>::max();
}

Tree::Tree(const Tree &t) {
  rootID = t.rootID;
  children = t.children;
  parent = t.parent;
  distance = t.distance;  
}

Tree::~Tree() {}

void Tree::sendParent(Message *m) {
  if (parent != NULL) {
    //parent->send(m);
    Layers &layers = ((TimeSyncBlockCode*)parent->hostBlock->blockCode)->layers;
    MyMessage::send(layers,parent,m);
  }
}

void Tree::broadcast(Message &m) {
  std::set<P2PNetworkInterface*>::iterator it;
 
  if (children.size() == 0) {
    return;
  } else {
    // hack to get layers object
    Layers &layers = ((TimeSyncBlockCode*)(*children.begin())->hostBlock->blockCode)->layers;
    Time sendTime = getScheduler()->now();
    
    //if (parent != NULL) {
    sendTime = layers.synchronization->getSendingTime(NULL);
    //}
    
    for(it = children.begin(); it != children.end(); ++it) {
      P2PNetworkInterface *p = *it;
      Message *msg = m.clone();
      p->send(sendTime,msg);
      //MyMessage::send(layers, p, msg);
    }
  }
}

/*********** BFSTraversal Class ************/
BFSTraversal::BFSTraversal() {
  waiting = 0;
  broadcastWait = 0;
}

BFSTraversal::BFSTraversal(Tree &t) {
  tree = t;
  waiting = 0;
  broadcastWait = 0;
}

BFSTraversal::BFSTraversal(const BFSTraversal &t) {
  tree = t.tree;
  waiting = t.waiting;
  broadcastWait = t.broadcastWait;
}

BFSTraversal::~BFSTraversal() {}

/*********** BFSTraversalSP Class ************/

BFSTraversalSP::BFSTraversalSP() {
  broadcastWait = 0;
}

BFSTraversalSP::BFSTraversalSP(Tree &t) {
  tree = t;
  broadcastWait = 0;
}

BFSTraversalSP::~BFSTraversalSP() {}

BFSTraversalSP::BFSTraversalSP(const BFSTraversalSP &t) {
  tree = t.tree;
  waiting = t.waiting;
  broadcastWait = t.broadcastWait;
}

/*********** BFSTraversalSPAlgorithm Class ************/

BFSTraversalSPAlgorithm::BFSTraversalSPAlgorithm(Layers &l, bool cb): layers(l) {
  module = layers.module;
  controlledBroadcast = cb;
}

BFSTraversalSPAlgorithm::~BFSTraversalSPAlgorithm() {

}

BFSTraversalSP* BFSTraversalSPAlgorithm::create() {
  BFSTraversalSP* t = new BFSTraversalSP();
  reset(t);
  t->tree.rootID = module->blockId;
  t->tree.distance = 0;
  return t;
}

bool BFSTraversalSPAlgorithm::start(BFSTraversalSP* t) {
  assert(t);
  if (checkAndBroadcastGo(t)) {
    if (checkAndForwardBack(t)) {
      if (module->blockId == t->tree.rootID) { // test required ?
	scheduleSpanningTreeConstructed();
      }
      return true;
    }
  }
  return false;
}

BFSTraversalSP* BFSTraversalSPAlgorithm::create(MessagePtr m) {
  assert(hasToHandle(m));
  BFSTraversalSPMessagePtr mes = std::static_pointer_cast<BFSTraversalSPMessage>(m);
  BFSTraversalSP* t = new BFSTraversalSP();
  reset(t);
  t->tree.rootID = mes->rootID;
  return t;
}

bool BFSTraversalSPAlgorithm::hasToHandle(MessagePtr m) {
  return BFSTraversalSPMessage::isABFSTraversalSPMessage(m);
}

bool BFSTraversalSPAlgorithm::handle(MessagePtr m, BFSTraversalSP* t) {
  assert(hasToHandle(m));
  
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);
  BFSTraversalSPMessagePtr mes = std::static_pointer_cast<BFSTraversalSPMessage>(m);

  assert(t);

  // Hack
  if (t->tree.rootID == NOT_A_VALID_ID) {
    t->tree.rootID = mes->rootID;
  }
  
  assert(t->tree.rootID == mes->rootID);

  switch (m->type) {
  case BFS_TRAVERSAL_SP_GO:
    {
      BFSTraversalSPGoMessagePtr recm = std::static_pointer_cast<BFSTraversalSPGoMessage>(mes);

      // Ack this GO message
      ackGo(from,recm->rootID,recm->distance);
      
      if (t->tree.distance > (recm->distance+1)) {
	if (t->tree.parent) {
	  sendDelete(t->tree.parent,t->tree.rootID,t->tree.distance-1);
	}
	reset(t);
	t->tree.rootID = recm->rootID;
	t->tree.parent = from;
	t->tree.distance = recm->distance + 1;
	if (checkAndBroadcastGo(t)) {
	  if (checkAndForwardBack(t)) {
	    return true;
	  }
	}
    } else {
	sendDelete(from,recm->rootID,recm->distance);
    }
    }
    break;
  case BFS_TRAVERSAL_SP_GO_ACK: {
     BFSTraversalSPGoAckMessagePtr recm = std::static_pointer_cast<BFSTraversalSPGoAckMessage>(m);
      t->broadcastWait--;
      if (t->broadcastWait == 0) {
        if (t->tree.rootID != recm->rootID ||
	    t->tree.distance != recm->distance) {
	  // traversal has been updated, send broadcast!
	  if (checkAndBroadcastGo(t)) {
	    if (checkAndForwardBack(t)) {
	      return true;
	    }
	  }
	}
      }
    
  }
    break;
  case BFS_TRAVERSAL_SP_BACK:
    {
      BFSTraversalSPBackMessagePtr recm = std::static_pointer_cast<BFSTraversalSPBackMessage>(m);
      if (recm->distance == t->tree.distance) {
	t->waiting.erase(from);
	t->tree.children.insert(from);
	if (checkAndForwardBack(t)) {
	  if (module->blockId == t->tree.rootID) {
	    scheduleSpanningTreeConstructed();
	  }
	  return true;
	}
      }
    }
    break;
  case BFS_TRAVERSAL_SP_DELETE:
    {
      BFSTraversalSPDeleteMessagePtr recm = std::static_pointer_cast<BFSTraversalSPDeleteMessage>(m);
      if (recm->distance == t->tree.distance) {
	t->waiting.erase(from);
	t->tree.children.erase(from);
	if (checkAndForwardBack(t)) {
	  assert(false); // should not happend!
	}
      }
    }
    break;
  default:
    cerr << "Not an InitiatorElectionSP message..." << endl;
    break;
  }
  return false;
}

void BFSTraversalSPAlgorithm::reset(BFSTraversalSP* t) { 
  degree_t maxDegree = module->getNbInterfaces();
  t->tree.parent = NULL;
  t->tree.distance = numeric_limits<hopDistance_t>::max();
  t->tree.rootID = NOT_A_VALID_ID;
  t->waiting.clear();
  t->tree.children.clear();
}

bool BFSTraversalSPAlgorithm::checkAndBroadcastGo(BFSTraversalSP* t) {
  if (!controlledBroadcast || (controlledBroadcast && t->broadcastWait == 0)) {
    broadcastGo(t);
    if (controlledBroadcast) {
      t->broadcastWait = t->waiting.size();
    }
    return true;
  }
  return false;
}

void BFSTraversalSPAlgorithm::broadcastGo(BFSTraversalSP* t) {
  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = module->getInterface(i);
    if (p->isConnected() && p != t->tree.parent) {
      Message *m = new BFSTraversalSPGoMessage(t->tree.rootID, t->tree.distance);
      t->waiting.insert(p);
      p->send(m);
    }
  }
}

void BFSTraversalSPAlgorithm::ackGo(P2PNetworkInterface *p, mID_t i, hopDistance_t d) {
  Message *m = new BFSTraversalSPGoAckMessage(i,d);
  p->send(m);
}

void BFSTraversalSPAlgorithm::forwardBack(BFSTraversalSP* t) {
  Message *m = new BFSTraversalSPBackMessage(t->tree.rootID, t->tree.distance-1);
  t->tree.parent->send(m);
}

bool BFSTraversalSPAlgorithm::checkAndForwardBack(BFSTraversalSP* t) {
  if (t->waiting.size() == 0) {
    if (t->tree.rootID == module->blockId) {
      return true;
    }
    forwardBack(t);
  }
  return false;
}

void BFSTraversalSPAlgorithm::sendDelete(P2PNetworkInterface *p, mID_t i, hopDistance_t d) {
  Message *m = new BFSTraversalSPDeleteMessage(i,d);
  p->send(m);
}

void BFSTraversalSPAlgorithm::scheduleSpanningTreeConstructed() {
  BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
  sched->schedule(new SpanningTreeConstructedEvent(sched->now(),layers.module));
}

								   
