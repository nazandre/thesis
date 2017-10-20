
#include <cassert>

#include "initiator.h"
#include "initiatorMsg.h"
#include "simulation.h"

#include "world.h"

using namespace std;
using namespace BaseSimulator;

// check aggrgate computation
//#define CHECK_ENSURE_WE_NEED_ARRAY_TO_COMPUTE_AGGREGATE

//#define DEBUG_IE_SP

//#define DEBUG_IE

/******* Class InitiatorElection ********/
InitiatorElection::InitiatorElection(BuildingBlock *m) {
  degree_t maxDegree = m->getNbInterfaces();
  Tree tree(m->blockId);

  module = m;
  tree.parent = NULL;
  tree.distance = 0;
  traversal = BFSTraversal(tree);
  traversal.waiting = 0;
  traversal.broadcastWait = 0;
  traversal.prevGoAcked.resize(maxDegree,true);
}

void InitiatorElection::init() {}

bool InitiatorElection::start() {
  if (checkAndBroadcastGo()) {
    if (checkAndForwardBack()) {
      return true;
    }
  }
  return false;
}

bool InitiatorElection::hasToHandle(MessagePtr m) {
  return (m->type == IE_GO_MESSAGE ||
	  m->type == IE_GO_ACK_MESSAGE ||
	  m->type == IE_BACK_MESSAGE);
}

bool InitiatorElection::handle(MessagePtr m) {
  assert(hasToHandle(m));

  P2PNetworkInterface *from = m->destinationInterface;
  
  switch (m->type) {
  case IE_GO_MESSAGE :
    {
      IEGoMsgPtr recm = std::static_pointer_cast<IEGoMsg>(m);

#ifdef DEBUG_IE
      MY_CERR << "received GO<id=" << recm->rootID
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << " (traversal: id=" << traversal.tree.rootID << ")"
	      << " msgId = " << recm->id
	      << endl;
#endif

      // Ack this GO message
      if (IS_CONTROLLED_BROADCAST_ENABLED)
	ackGo(from,recm->rootID);

      if (recm->rootID < traversal.tree.rootID) {
	traversal.tree.rootID = recm->rootID;
	traversal.tree.parent = from;
	
        if (checkAndBroadcastGo()) {
	  if (checkAndForwardBack()) {
	    assert(false);
	  }
	}
      } else if (recm->rootID == traversal.tree.rootID) { // send back anyway!
	Message *m = new IEBackMsg(recm->rootID);
        from->send(m);
      }
    }
    break;
  case IE_GO_ACK_MESSAGE :
    {
      IEGoAckMsgPtr recm = std::static_pointer_cast<IEGoAckMsg>(m);
      
#ifdef DEBUG_IE
      MY_CERR << "received GO_ACK<id=" << recm->rootID
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << " (traversal: id=" << traversal.tree.rootID << ")"
	      << " msgId = " << recm->id
	      << endl;
#endif
      
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
      
      degree_t fromIndex = module->getDirection(from);
      assert(!traversal.prevGoAcked[fromIndex]);
      traversal.prevGoAcked[fromIndex] = true;

      if (traversal.tree.rootID != recm->rootID &&
	  traversal.tree.parent != from) { // traversal has been updated
	sendGo(from);
      }
      
#else
      traversal.broadcastWait--;
      if (traversal.broadcastWait == 0) {
	if (traversal.tree.rootID != recm->rootID) {
	  // traversal has been updated, send broadcast!
	  if (checkAndBroadcastGo()) {
	    if (checkAndForwardBack()) {
	      return true;
	    }
	  }
	}
      }
#endif
    }
    break;
  case IE_BACK_MESSAGE : {
    IEBackMsgPtr recm = std::static_pointer_cast<IEBackMsg>(m);

#ifdef DEBUG_IE
      MY_CERR << "received BACK<id=" << recm->rootID
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << " (traversal: id=" << traversal.tree.rootID << ")"
	      << " msgId = " << recm->id
	      << endl;
#endif
    
    if (traversal.tree.rootID == recm->rootID) {
      traversal.waiting--;

      //traversal.tree.children.insert(from);
      // need boolean yes or no.
      
      if (checkAndForwardBack()) {
	return true;
      }
    }
  }
    break;
  default:
    cerr << "Not an InitiatorElection message..." << endl;
  }
  return false;
}

bool InitiatorElection::checkAndBroadcastGo() {

#ifndef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  if (!IS_CONTROLLED_BROADCAST_ENABLED ||
      (IS_CONTROLLED_BROADCAST_ENABLED && traversal.broadcastWait == 0)) {
#endif

    broadcastGo();
    
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
    return true;
#else
    return true;
  }
  return false;
#endif
}

void InitiatorElection::ackGo(P2PNetworkInterface *p, mID_t i) {
  Message *m = new IEGoAckMsg(i);
  p->send(m);
}

void InitiatorElection::sendGo(P2PNetworkInterface *p) {
  degree_t pIndex = module->getDirection(p);
  
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  if (traversal.prevGoAcked[pIndex]) {
#endif

    Message *m = new IEGoMsg(traversal.tree.rootID);

    if (IS_CONTROLLED_BROADCAST_ENABLED) {
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
      traversal.prevGoAcked[pIndex] = false;
#else
      traversal.broadcastWait++;
#endif
    }
    p->send(m);
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  }
#endif
}

void InitiatorElection::broadcastGo() {
  degree_t numGoShouldBeSent = 0;

  traversal.waiting = 0;
  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = module->getInterface(i);
    if (p->isConnected() && p != traversal.tree.parent) {
      sendGo(p);
      numGoShouldBeSent++;
    }
  }
  traversal.waiting = numGoShouldBeSent;
}

void InitiatorElection::forwardBack() {
  Message *m = new IEBackMsg(traversal.tree.rootID);
  traversal.tree.parent->send(m);
}

bool InitiatorElection::checkAndForwardBack() {
  if (traversal.waiting == 0) {
    if (traversal.tree.rootID == module->blockId) {
      return true;
    }
    forwardBack();
  }
  return false;
}

/******* Class InitiatorElectionSP ********/

InitiatorElectionSP::InitiatorElectionSP(BuildingBlock *m) {
  degree_t maxDegree = m->getNbInterfaces();
  module = m;
  traversal = BFSTraversalSP();

  traversal.branchHeight.resize(maxDegree,0);
  traversal.branchSize.resize(maxDegree,0);

  traversal.prevGoAcked.resize(maxDegree,true);
  
  reset();
  
  traversal.tree.rootID = module->blockId;  
  traversal.broadcastWait = 0;
}

void InitiatorElectionSP::init() {}

// TODO: change return type of start, the algorithm may terminate here: system
//       with a single module.
bool InitiatorElectionSP::start() {
  if (checkAndBroadcastGo()) {
    if (checkAndForwardBack()) {
      return true;
    }
  }
  return false;
}
  
bool InitiatorElectionSP::hasToHandle(MessagePtr m) {
  return (m->type == IE_SP_GO_MESSAGE ||
	  m->type == IE_SP_GO_ACK_MESSAGE ||
	  m->type == IE_SP_BACK_MESSAGE ||
	  m->type == IE_SP_DELETE_MESSAGE ||
	  m->type == IE_SP_CONVERGECAST_GO_MESSAGE ||
	  m->type == IE_SP_CONVERGECAST_BACK_MESSAGE
	  );
}

bool InitiatorElectionSP::handle(MessagePtr m) {
  assert(hasToHandle(m));
  
  P2PNetworkInterface *from = m->destinationInterface;
  degree_t fromIndex = module->getDirection(from);

  switch (m->type) {
  case IE_SP_GO_MESSAGE:
    {
      IESPGoMsgPtr recm = std::static_pointer_cast<IESPGoMsg>(m);
      
#ifdef DEBUG_IE_SP
      MY_CERR << "received GO<id=" << recm->rootID
	      << ",dist=" << recm->distance
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << " (traversal: id=" << traversal.tree.rootID << ",dist=" << traversal.tree.distance << ")"
	      << " msgId = " << recm->id
	      << endl;
#endif

      assert(!(
	       (traversal.tree.rootID == recm->rootID) &&
	       (traversal.tree.distance == (recm->distance+1)) &&
	       (traversal.tree.parent == from)
	       ));
      
      // Ack this GO message
      if (IS_CONTROLLED_BROADCAST_ENABLED)
	ackGo(from,recm->rootID,recm->distance);


      if (recm->rootID < traversal.tree.rootID) {
	traversal.backSent = false;
      }
      
      if ( recm->rootID < traversal.tree.rootID || 
	   (traversal.tree.rootID == recm->rootID
	    && traversal.tree.distance > (recm->distance+1))) {
	
	assert(!traversal.finished); // can not have elected the leader and receive a better GO msg
	  
	if (traversal.tree.rootID == recm->rootID &&
	    traversal.tree.distance > (recm->distance+1) &&
	    traversal.tree.parent != NULL) {

	  /*
	  if(traversal.backSent) {
	    MY_CERR << " wow back sent! " << endl;
	    //assert(!traversal.backSent);
	    }*/
	  
	  sendDelete(traversal.tree.parent, traversal.tree.rootID, traversal.tree.distance-1);
	  }
		
	// Update traversal
	reset();
	
	traversal.tree.parent = from;
	traversal.tree.rootID = recm->rootID;
	traversal.tree.distance = recm->distance + 1;

#ifndef TWO_WAY_IE_SP
	traversal.resetHandler();
#else
	if(!traversal.enableConvergecast) {
	  traversal.resetHandler();
	}
#endif
	
	// Check if can broadcast GO msg (avoid clogging up the message queue)
	if (checkAndBroadcastGo()) {
	  if (checkAndForwardBack()) {
	    assert(false); // impossible to both be the winning root and receive a better go msg!
	  }
	}
      }  else if (recm->rootID == traversal.tree.rootID) {
	sendDelete(from,recm->rootID,recm->distance);
      }
    }
    break;
  case IE_SP_BACK_MESSAGE:
    {
      IESPBackMsgPtr recm = std::static_pointer_cast<IESPBackMsg>(m);
      
      if ((recm->rootID == traversal.tree.rootID) &&
	  (recm->distance == traversal.tree.distance)
	  && !traversal.finished) {

#ifdef DEBUG_IE_SP
      MY_CERR << "received BACK<id=" << recm->rootID
	      << ",dist=" << recm->distance
#ifndef TWO_WAY_IE_SP
	      << ",height=" << recm->height + 1
	      << ",size=" << recm->size
#endif
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << " (traversal: id=" << traversal.tree.rootID << ",dist=" << traversal.tree.distance << ")"
	
	      << " msgId = " << recm->id
	      << endl;
#endif
      
#ifdef CHECK_ENSURE_WE_NEED_ARRAY_TO_COMPUTE_AGGREGATE
	//assert(traversal.waiting.contains(from));
	assert(traversal.waiting.find(from) != traversal.waiting.end());
#endif
	
	traversal.waiting.erase(from);
        traversal.tree.children.insert(from);

#ifndef TWO_WAY_IE_SP
	traversal.branchHeight[fromIndex] = recm->height + 1;
	traversal.branchSize[fromIndex] = recm->bSize;
#else
	if (!traversal.enableConvergecast) {
	  traversal.addChild(m);
	}
#endif
	
	if (checkAndForwardBack()) {
#ifdef TWO_WAY_IE_SP
	  return !traversal.enableConvergecast;
#else
	  return true;
#endif
	}
      }
    }
    break;
  case IE_SP_DELETE_MESSAGE:
    {
      IESPDeleteMsgPtr recm = std::static_pointer_cast<IESPDeleteMsg>(m);
      
      if ((recm->rootID == traversal.tree.rootID) &&
	  (recm->distance == traversal.tree.distance) &&
	  !traversal.finished) {

#ifdef DEBUG_IE_SP
      MY_CERR << "received DELETE <id=" << recm->rootID
	      << ",dist=" << recm->distance
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << " (traversal: id=" << traversal.tree.rootID << ",dist=" << traversal.tree.distance << ")"
	      << " msgId = " << recm->id
	      << endl;
#endif
      
#ifdef CHECK_ENSURE_WE_NEED_ARRAY_TO_COMPUTE_AGGREGATE
	//assert(traversal.waiting.contains(from));
	assert(traversal.waiting.find(from) != traversal.waiting.end());
#endif	
	traversal.waiting.erase(from);
	traversal.tree.children.erase(from);
	
#ifndef TWO_WAY_IE_SP
	traversal.branchHeight[fromIndex] = 0;
	traversal.branchSize[fromIndex] = 0;
	// remove child
	traversal.removeChild(m);
#else
	if (!traversal.enableConvergecast) {
	  traversal.removeChild(m);
	}
#endif	
	if (checkAndForwardBack()) {
	  assert(false); // should not happend!
	}
      }
    }
    break;
  case IE_SP_GO_ACK_MESSAGE:
    {
      IESPGoAckMsgPtr recm = std::static_pointer_cast<IESPGoAckMsg>(m);

#ifdef DEBUG_IE_SP
      MY_CERR << "received ACK_GO <id=" << recm->rootID
	      << ",dist=" << recm->distance
	      << "> from " << from->connectedInterface->hostBlock->blockId
	      << endl;
#endif

#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
      assert(!traversal.prevGoAcked[fromIndex]);
      traversal.prevGoAcked[fromIndex] = true;
      if ( (traversal.tree.rootID != recm->rootID ||
	    traversal.tree.distance != recm->distance) &&
	   traversal.tree.parent != from) { // traversal has been updated
	sendGo(from);
      }
#else  
      traversal.broadcastWait--;
      if (traversal.broadcastWait == 0) {	
	//if (traversal.newUpdate) {
	if (traversal.tree.rootID != recm->rootID ||
	    traversal.tree.distance != recm->distance) {
      
	  // traversal has been updated, send broadcast!
	  if (checkAndBroadcastGo()) {
	    if (checkAndForwardBack()) {
	      assert(false);
	    }
	  }
	}
      }
#endif
    }
    break;
  case IE_SP_CONVERGECAST_GO_MESSAGE:
    {
      assert(!traversal.finished); // only one visit is possible!

      traversal.finished = true; 
      traversal.resetHandler(); // ie => inform this child of the visit
      
      if (traversal.tree.isALeaf()) {
	sendConvergecastBack();
      } else {
	sendConvergecastGo();
      }
    };
    break;
  case IE_SP_CONVERGECAST_BACK_MESSAGE:
    {
      IESPBackMsgPtr recm = std::static_pointer_cast<IESPBackMsg>(m);

      assert(traversal.finished);
      assert(traversal.waiting.find(from) != traversal.waiting.end());     

      traversal.waiting.erase(from);
      traversal.branchHeight[fromIndex] = recm->height + 1;
      traversal.branchSize[fromIndex] = recm->bSize;
      traversal.addChild(m);

      if (traversal.waiting.empty()) {
	if (traversal.tree.isRoot()) {
	  sysSize_t size = traversal.getSize();
	  distance_t height = traversal.getHeight();

	  MY_CERR << "Initiator elected (IE_BFS-SP_CONV) at " << getScheduler()->now()
		  << " size: " << size << " ecc: " << height
		  << endl;
	  
	  assert(size == (sysSize_t) getWorld()->getSize());
	  return true;
	} else {
	  sendConvergecastBack();
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

void InitiatorElectionSP::reset() {
  traversal.waiting.clear();
  traversal.tree.children.clear();
  traversal.tree.parent = NULL;
  traversal.tree.distance = 0;
  traversal.tree.rootID = NOT_A_VALID_MODULE_ID;

  for (degree_t i = 0; i < traversal.branchHeight.size(); i++) {
    traversal.branchHeight[i] = 0;
    traversal.branchSize[i] = 0;
  }

#ifdef TWO_WAY_IE_SP
  traversal.enableConvergecast = true;
#else
  traversal.enableConvergecast = false;
#endif
}

void InitiatorElectionSP::sendGo(P2PNetworkInterface *p) {
  degree_t pIndex = module->getDirection(p);
  
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  if (traversal.prevGoAcked[pIndex]) {
#endif

    BFSData *dataHeader =  traversal.getBFSDataHeader();
    Message *m = new IESPGoMsg(dataHeader,traversal.tree.rootID, traversal.tree.distance);
    
#ifdef DEBUG_IE_SP
    cerr << "\t sending GO<> to " << p->connectedInterface->hostBlock->blockId
	 << " msgId = " << m->id << endl;
#endif

    if (IS_CONTROLLED_BROADCAST_ENABLED) {
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
      traversal.prevGoAcked[pIndex] = false;
#else
      traversal.broadcastWait++;
#endif
    }
    
    p->send(m);
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  }
#endif
  traversal.waiting.insert(p); // even if not sent.  
}

bool InitiatorElectionSP::checkAndBroadcastGo() {

#ifndef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  if (!IS_CONTROLLED_BROADCAST_ENABLED || (IS_CONTROLLED_BROADCAST_ENABLED && traversal.broadcastWait == 0)) {
#endif
    traversal.waiting.clear();
    broadcastGo();    
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
    return true;
#else
    return true;
  }
  return false;
#endif
}

void InitiatorElectionSP::broadcastGo() {
  
#ifdef DEBUG_IE_SP
  MY_CERR << "broadcast GO <id=" << traversal.tree.rootID
	  << ",dist=" << traversal.tree.distance
    	  << ">" << endl;
#endif   
  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = module->getInterface(i);
    if (p->isConnected() && p != traversal.tree.parent) {
      sendGo(p);
    }
  }
}

void InitiatorElectionSP::forwardBack() {
  distance_t height = traversal.getHeight();
  sysSize_t size = traversal.getSize();
  BFSData *dataHeader =  traversal.getBFSDataHeader();
  BFSData *dataBack =  traversal.getBFSDataBack();
  Message *m = new IESPBackMsg(dataHeader,traversal.tree.rootID, traversal.tree.distance-1,height,size,dataBack);

#ifdef DEBUG_IE_SP
  MY_CERR << "send BACK <id=" << traversal.tree.rootID
	  << ",dist=" << traversal.tree.distance-1
#ifndef TWO_WAY_IE_SP
	  << ",height=" << height
	  << ",size=" << size
#endif
	  << "> to " << traversal.tree.parent->connectedInterface->hostBlock->blockId
	  << " msgId = " << m->id
	  << endl;
#endif

  traversal.backSent = true;
  traversal.tree.parent->send(m);
}

bool InitiatorElectionSP::checkAndForwardBack() {
  
  if (traversal.waiting.empty()) {
    if (traversal.tree.rootID == module->blockId) {    
      
      // initiate convergecast
      traversal.finished = true;

#ifdef TWO_WAY_IE_SP
      
      if(!traversal.enableConvergecast) {
	MY_CERR << "Initiator elected (IE_BFS-SP) at " << getScheduler()->now() << endl;
	return true;
      }
      
      sendConvergecastGo();
#else
      sysSize_t size = traversal.getSize();
      distance_t height = traversal.getHeight();
      
      MY_CERR << "Initiator elected (IE_BFS-SP_ONE-WAY) at " << getScheduler()->now()
	      << " size: " << size << " ecc: " << height
	      << endl;
      
      assert(traversal.getSize() == (sysSize_t) getWorld()->getSize());
      return true;
      //return false;
#endif
    } else {
      forwardBack();
    }
  }
  return false;
}

void InitiatorElectionSP::sendDelete(P2PNetworkInterface *p, mID_t i, distance_t d) {
  BFSData *dataHeader =  traversal.getBFSDataHeader();
  Message *m = new IESPDeleteMsg(dataHeader,i,d);
#ifdef DEBUG_IE_SP
  MY_CERR << "send DELETE <id=" << i
	  << ",dist=" << d
	  << "> to " << p->connectedInterface->hostBlock->blockId
	  << " msgId = " << m->id
	  << endl;
#endif
  p->send(m);
}

void InitiatorElectionSP::ackGo(P2PNetworkInterface *p, mID_t i, distance_t d) {
  BFSData *dataHeader =  traversal.getBFSDataHeader();
  Message *m = new IESPGoAckMsg(dataHeader,i,d);
  
#ifdef DEBUG_IE_SP
  MY_CERR << "send ACK_GO <id=" << i
	  << ",dist=" << d
	  << "> to " << p->connectedInterface->hostBlock->blockId
	  << " msgId = " << m->id
	  << endl;
#endif

  p->send(m);
}

void InitiatorElectionSP::sendConvergecastGo() {

#ifdef DEBUG_IE_SP
  MY_CERR << "broadcast tree Convergecast GO "
	  << "<id=" << traversal.tree.rootID
	  << ",dist=" << traversal.tree.distance
    	  << ">" << endl;
#endif

  std::set<P2PNetworkInterface*>::iterator it;
  
  for (it = traversal.tree.children.begin(); it != traversal.tree.children.end(); it++) {
    
    P2PNetworkInterface *p = *it;

    BFSData *dataHeader =  traversal.getBFSDataHeader();
    Message *m = new IESPConvergecastGoMsg(dataHeader,traversal.tree.rootID, traversal.tree.distance);
#ifdef DEBUG_IE_SP
      cerr << "\t sending Convergecast GO <> to " << p->connectedInterface->hostBlock->blockId
	   << " msgId = " << m->id << endl;
#endif
      traversal.waiting.insert(p);
      p->send(m);
  }
}

void InitiatorElectionSP::sendConvergecastBack() {
  distance_t height = traversal.getHeight();
  sysSize_t size = traversal.getSize();
  BFSData *dataHeader =  traversal.getBFSDataHeader();
  BFSData *dataBack =  traversal.getBFSDataBack();
  
  Message *m = new IESPConvergecastBackMsg(dataHeader,traversal.tree.rootID, traversal.tree.distance-1,height,size,dataBack);

  assert(traversal.waiting.empty());

#ifdef DEBUG_IE_SP
  MY_CERR << "send Convergecast BACK <id=" << traversal.tree.rootID
	  << ",dist=" << traversal.tree.distance-1
	  << ",height=" << height
	  << ",size=" << size
	  << "> to " << traversal.tree.parent->connectedInterface->hostBlock->blockId
	  << " msgId = " << m->id
	  << endl;
#endif
  
  traversal.tree.parent->send(m);
}
