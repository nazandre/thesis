#include <climits>

#include "bfsTree.h"
#include "simulation.h"

#include "world.h"
#include "lattice.h"

using namespace std;
using namespace BaseSimulator;

//#define DEBUG_BFS_ALGORITHM

//#define DEBUG_CONVERGECAST_ALGORITHM

// check aggrgate computation
//#define CHECK_ENSURE_WE_NEED_ARRAY_TO_COMPUTE_AGGREGATE

void emptyChildHandler(MessagePtr m) {};
void emptyResetHandler() {};

/*********** Tree Class ************/
Tree::Tree() {
  rootID = NOT_A_VALID_MODULE_ID;
  parent = NULL;
  distance = numeric_limits<distance_t>::max();
}

Tree::Tree(mID_t rID) {
  rootID = rID;
  parent = NULL;
  distance = numeric_limits<distance_t>::max();
}

Tree::Tree(const Tree &t) {
  rootID = t.rootID;
  children = t.children;
  parent = t.parent;
  distance = t.distance;
}

Tree::~Tree() {}

size_t Tree::size() {
  degree_t d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  size_t s =
    sizeof(mID_t) + // rootID
    sizeof(distance_t) + // distance
    d*sizeof(degree_t) + // children
    sizeof(degree_t); // parent
  return s;
}

void Tree::sendParent(Message *m) {

  if (parent != NULL) {
    parent->send(m);
  }
  
}

void Tree::broadcast(Message *m) {
  std::set<P2PNetworkInterface*>::iterator it;
 
  if (children.size() == 0) {
    return;
  } else {
    
    for(it = children.begin(); it != children.end(); ++it) {
      P2PNetworkInterface *p = *it;
      Message *msg = m->clone();
      p->send(msg);
    }
  }
}

/*********** BFSTraversalManager Class ************/
BFSTraversalManager::BFSTraversalManager() {}

BFSTraversalManager::~BFSTraversalManager() {
  // TODO: delete all trees
}

BFSTraversalSP* BFSTraversalManager::get(mID_t rootID) {
  map<tID_t, BFSTraversalSP*>::iterator it;
  it = traversals.find(rootID);
  if (it != traversals.end()) {
    return it->second;
  }
  return NULL;
}

void BFSTraversalManager::insert(mID_t rootID, BFSTraversalSP* traversal) {
  traversals.insert(pair<mID_t,BFSTraversalSP*>(rootID,traversal));
}

/*********** BFSTraversal Class ************/
BFSTraversal::BFSTraversal() {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  
  waiting = 0;

  broadcastWait = 0;
  prevGoAcked.resize(d,true);

  getBFSDataHeader = BFSData::getNULLBFSData;
  getBFSDataBack = BFSData::getNULLBFSData;

  addChild = emptyChildHandler;
  removeChild = emptyChildHandler;
  resetHandler = emptyResetHandler;
}

BFSTraversal::BFSTraversal(Tree &t) {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();

  tree = t;
  waiting = 0;

  prevGoAcked.resize(d,true);  
  broadcastWait = 0;
  
  getBFSDataHeader = BFSData::getNULLBFSData;
  getBFSDataBack = BFSData::getNULLBFSData;

  addChild = emptyChildHandler;
  removeChild = emptyChildHandler;
  resetHandler = emptyResetHandler;
}

BFSTraversal::BFSTraversal(const BFSTraversal &t) {
  tree = t.tree;
  waiting = t.waiting;

  broadcastWait = t.broadcastWait;
  prevGoAcked = t.prevGoAcked;
    
  getBFSDataHeader = t.getBFSDataHeader;
  getBFSDataBack = t.getBFSDataBack;

  addChild = t.addChild;
  removeChild = t.removeChild;
  resetHandler = t.resetHandler;
}

BFSTraversal::~BFSTraversal() {}

size_t BFSTraversal::size() {
  size_t s = sizeof(degree_t) + tree.size();

  if (IS_CONTROLLED_BROADCAST_ENABLED) {
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
    s += prevGoAcked.size() * sizeof(bool);
#else
    s += sizeof(degree_t);
#endif
  }
  
  return s;
}

/*********** BFSTraversalSP Class ************/

BFSTraversalSP::BFSTraversalSP() {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  
  broadcastWait = 0;
  getBFSDataHeader = BFSData::getNULLBFSData;
  getBFSDataBack = BFSData::getNULLBFSData;
  addChild = emptyChildHandler;
  removeChild = emptyChildHandler;
  resetHandler = emptyResetHandler;

  branchHeight.resize(d,0);
  branchSize.resize(d,0);

  prevGoAcked.resize(d,true);
  
  finished = false;
  
#ifdef TWO_WAY_BFS_SP
  enableConvergecast = true;
#else
  enableConvergecast = false;
#endif
  
  aggComputation = true;
}

BFSTraversalSP::BFSTraversalSP(Tree &t) {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  
  tree = t;
  broadcastWait = 0;
  getBFSDataHeader = BFSData::getNULLBFSData;
  getBFSDataBack = BFSData::getNULLBFSData;
  addChild = emptyChildHandler;
  removeChild = emptyChildHandler;
  resetHandler = emptyResetHandler;
  
  branchHeight.resize(d,0);
  branchSize.resize(d,0);
  
  prevGoAcked.resize(d,true);
  
  finished = false;

#ifdef TWO_WAY_BFS_SP
  enableConvergecast = true;
#else
  enableConvergecast = false;
#endif
  
  aggComputation = true;
}

BFSTraversalSP::~BFSTraversalSP() {}

BFSTraversalSP::BFSTraversalSP(const BFSTraversalSP &t) {
  tree = t.tree;
  waiting = t.waiting;
  branchHeight = t.branchHeight;
  branchSize = t.branchSize;
  broadcastWait = t.broadcastWait;
  getBFSDataHeader = t.getBFSDataHeader;
  getBFSDataBack = t.getBFSDataBack;
  addChild = t.addChild;
  removeChild = t.removeChild;
  resetHandler = t.resetHandler;
  
  prevGoAcked = t.prevGoAcked;
  
  finished = t.finished;
  enableConvergecast = t.enableConvergecast;
  aggComputation = t.aggComputation;
}

distance_t BFSTraversalSP::getHeight() {
  distance_t height = 0;
  vector<distance_t>::iterator it;
  for (it = branchHeight.begin(); it != branchHeight.end(); ++it) {
    height = max(height,*it);
  }
  return height;
}

sysSize_t BFSTraversalSP::getSize() {
  sysSize_t s = 1;
  vector<distance_t>::iterator it;
  for (it = branchSize.begin(); it != branchSize.end(); ++it) {
    s += *it;
  }
  return s;
}

void BFSTraversalSP::subscribe(mID_t id) {
  tree.rootID = id;
}

size_t BFSTraversalSP::size() {
  // branchSize is not used by any of the algorithm
  // branch height: depends on the app, added if necessary in each app
  degree_t d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
    
  size_t s = tree.size() + d*sizeof(degree_t) + sizeof(bool);
  // bool finished

  if (IS_CONTROLLED_BROADCAST_ENABLED) {
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
    s += prevGoAcked.size() * sizeof(bool);
#else
    s += sizeof(degree_t);
#endif
  }
  return s;
}

/*********** BFSTraversalSPAlgorithm Class ************/

BFSTraversalSPAlgorithm::BFSTraversalSPAlgorithm(BuildingBlock *m) {
  module = m;
  getBFSDataHeader2 = getNULLBFSData2;

  sysSize = 0; // unknwon
}

BFSTraversalSPAlgorithm::~BFSTraversalSPAlgorithm() {

}

BFSData* BFSTraversalSPAlgorithm::getNULLBFSData2(BFSTraversalSP *t) {
  return NULL;
}

BFSData* BFSTraversalSPAlgorithm::getBFSDataHeader(BFSTraversalSP *t) {
  BFSData *dataHeader =  getBFSDataHeader2(t);
  if (dataHeader == NULL) {
    dataHeader = t->getBFSDataHeader();
  }
  return dataHeader;
}

size_t BFSTraversalSPAlgorithm::getSizeAggComp(size_t s, bool a) {
  size_t r = 0;
  if (a) { // needs an array
     degree_t d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
     r = d*s;
  } else {
    r = s;
  }
  return r;
}

BFSTraversalSP* BFSTraversalSPAlgorithm::create() {
  BFSTraversalSP* t = new BFSTraversalSP();
  reset(t);
  t->tree.rootID = module->blockId;
  t->tree.distance = 0;

  if (!t->enableConvergecast) {
    t->resetHandler();
  }
  
  return t;
}

bool BFSTraversalSPAlgorithm::start(BFSTraversalSP* t) {
  assert(t);
  t->tree.distance = 0;
  if (checkAndBroadcastGo(t)) {
    if (checkAndForwardBack(t)) {
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
  assert(t->tree.rootID == mes->rootID);

  switch (m->type) {
  case BFS_TRAVERSAL_SP_GO:
    {
      BFSTraversalSPGoMessagePtr recm = std::static_pointer_cast<BFSTraversalSPGoMessage>(mes);

#ifdef DEBUG_BFS_ALGORITHM
      MY_CERR << "GO <"
	      << "id=" << recm->rootID << ","
	      << "d=" << recm->distance
	      << "> (traversal: " << t->tree.distance << ")" << " msgId= " << recm->id 
	      << " from " << from->connectedInterface->hostBlock->blockId
	      << endl;
#endif
      // Ack this GO message
      if (IS_CONTROLLED_BROADCAST_ENABLED)
	ackGo(t,from,recm->rootID,recm->distance);
      
      if (t->tree.distance > (recm->distance+1)) {

	assert(!t->finished);
	
	if (t->tree.parent) {
	  sendDelete(t,t->tree.parent,t->tree.rootID,t->tree.distance-1);
	}

	reset(t);

	t->tree.rootID = recm->rootID;
	t->tree.parent = from;
	t->tree.distance = recm->distance + 1;
	
	if (!t->enableConvergecast) {
	  t->resetHandler();
	}
	
	if (checkAndBroadcastGo(t)) {
	  if (checkAndForwardBack(t)) {
	    assert(false); // impossible the root can't receive a better go msg!
	  }
	}
      } else {
	sendDelete(t,from,recm->rootID,recm->distance);
      }
    }
    break;
  case BFS_TRAVERSAL_SP_GO_ACK: {
     BFSTraversalSPGoAckMessagePtr recm = std::static_pointer_cast<BFSTraversalSPGoAckMessage>(m);
     
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
      assert(!t->prevGoAcked[fromIndex]);
      t->prevGoAcked[fromIndex] = true;
      if ( (t->tree.rootID != recm->rootID ||
	    t->tree.distance != recm->distance) &&
	   t->tree.parent != from) { // traversal has been updated
	sendGo(t,from);
      }
#else
     t->broadcastWait--;
      if (t->broadcastWait == 0) {
        if (t->tree.distance != recm->distance) { //t->tree.rootID != recm->rootID || t->tree.distance != recm->distance)
	  // traversal has been updated, send broadcast!
	  if (checkAndBroadcastGo(t)) {
	    if (checkAndForwardBack(t)) {
	      assert(false);
	    }
	  }
	}
      }
#endif
  }
    break;
  case BFS_TRAVERSAL_SP_BACK:
    {
      BFSTraversalSPBackMessagePtr recm = std::static_pointer_cast<BFSTraversalSPBackMessage>(m);

#ifdef DEBUG_BFS_ALGORITHM
      MY_CERR << "BACK <"
	      << "id=" << recm->rootID
	      << ",d=" << recm->distance;
      if (!t->enableConvergecast) {
	cerr << ",height=" << recm->height
	     << ",size=" << recm->bSize;
      }
      cerr  << ">"
	    << " (traversal: " << t->tree.distance << ")" << " msgId= " << recm->id 
	    << " from " << from->connectedInterface->hostBlock->blockId
	    << endl;
#endif
      
      if (recm->distance == t->tree.distance &&
	  !t->finished) {

#ifdef CHECK_ENSURE_WE_NEED_ARRAY_TO_COMPUTE_AGGREGATE
	//assert(traversal.waiting.contains(from));
	assert(t->waiting.find(from) != t->waiting.end());
#endif
	
	t->waiting.erase(from);
	t->tree.children.insert(from);

	t->branchHeight[fromIndex] = recm->height + 1;
	t->branchSize[fromIndex] = recm->bSize;

	if (!t->enableConvergecast) {
	  // addChild;
	  t->addChild(mes);
	}
	
	if (checkAndForwardBack(t)) {
	  return !t->enableConvergecast;
	}
      }
    }
    break;
  case BFS_TRAVERSAL_SP_DELETE:
    {
      BFSTraversalSPDeleteMessagePtr recm = std::static_pointer_cast<BFSTraversalSPDeleteMessage>(m);

#ifdef DEBUG_BFS_ALGORITHM
      MY_CERR << "DELETE <"
	      << "id=" << recm->rootID << ","
	      << "d=" << recm->distance
	      << ">"
	      << " from " << from->connectedInterface->hostBlock->blockId
	      << endl;
#endif 

      if (recm->distance == t->tree.distance && !t->finished) {

#ifdef CHECK_ENSURE_WE_NEED_ARRAY_TO_COMPUTE_AGGREGATE
	//assert(traversal.waiting.contains(from));
	assert(t->waiting.find(from) != t->waiting.end());
#endif
	
	t->waiting.erase(from);
	t->tree.children.erase(from);

	t->branchHeight[fromIndex] = 0;	
	t->branchSize[fromIndex] = 0;
	
	if (!t->enableConvergecast) {
	// remove child
	  t->removeChild(mes);
	}
	
	if (checkAndForwardBack(t)) {
	  assert(false); // should not happend!
	}
      }
    }
    break;
  case BFS_TRAVERSAL_SP_CONVERGECAST_GO:
    {
      
#ifdef DEBUG_BFS_ALGORITHM
      MY_CERR << "BFSSP Convergecast GO msg" << endl;
#endif
      assert(!t->finished); // only one visit is possible!
      t->finished = true;

      // re-init all aggregate
      for (degree_t i = 0; i < t->branchHeight.size(); i++) {
	t->branchHeight[i] = 0;
	t->branchSize[i] = 0;
      }
      
      t->resetHandler(); // ie => inform this child of the visit
      
      if (t->tree.isALeaf()) {
	sendConvergecastBFSPBack(t);
      } else {
	sendConvergecastBFSPGo(t);
      }
    };
    break;
  case BFS_TRAVERSAL_SP_CONVERGECAST_BACK:
    {
      BFSTraversalSPConvergecastBackMessagePtr recm = std::static_pointer_cast<BFSTraversalSPConvergecastBackMessage>(m);

#ifdef DEBUG_BFS_ALGORITHM
      MY_CERR << "BFSSP Convergecast BACK msg" << endl;
#endif
      
      assert(t->finished);
      assert(t->waiting.find(from) != t->waiting.end());     

      t->waiting.erase(from);
      
      t->branchHeight[fromIndex] = recm->height + 1;
      t->branchSize[fromIndex] =recm->bSize;
	
      t->addChild(m);
      
      if (t->waiting.empty()) {
	if (t->tree.isRoot()) {
	  sysSize_t ssize = t->getSize();
	  distance_t height = t->getHeight();

	  MY_CERR << "BFS completed (BFS-SP_CONV) at " << getScheduler()->now()
		  << " size: " << ssize << " ecc: " << height
		  << endl;
	  
	  assert(ssize == (sysSize_t) getWorld()->getSize());
	  return true;
	} else {
	  sendConvergecastBFSPBack(t);
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
  t->tree.parent = NULL;
  t->tree.distance = numeric_limits<distance_t>::max();
  t->tree.rootID = NOT_A_VALID_MODULE_ID;
  t->waiting.clear();
  t->tree.children.clear();
  t->finished = false;

  for (degree_t i = 0; i < t->branchHeight.size(); i++) {
    t->branchHeight[i] = 0;
    t->branchSize[i] = 0;
  }

#ifdef TWO_WAY_BFS_SP
  t->enableConvergecast = true;
#else
  t->enableConvergecast = false;
#endif
}

bool BFSTraversalSPAlgorithm::checkAndBroadcastGo(BFSTraversalSP* t) {

#ifndef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL  
  if (!IS_CONTROLLED_BROADCAST_ENABLED ||
      (IS_CONTROLLED_BROADCAST_ENABLED && t->broadcastWait == 0)) {
#endif
    t->waiting.clear();
    broadcastGo(t);
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
    return true;
#else
    return true;
  }
  return false;
#endif
}

void BFSTraversalSPAlgorithm::sendGo(BFSTraversalSP* t, P2PNetworkInterface *p) {
  degree_t pIndex = module->getDirection(p);
  
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  if (t->prevGoAcked[pIndex]) {
#endif

    BFSData *dataHeader =  getBFSDataHeader(t);      
    Message *m = new BFSTraversalSPGoMessage(dataHeader,t->tree.rootID, t->tree.distance);     

    if (IS_CONTROLLED_BROADCAST_ENABLED) {
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
      t->prevGoAcked[pIndex] = false;
#else
      t->broadcastWait++;
#endif
    }
    
    p->send(m);
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
  }
#endif
  t->waiting.insert(p); // even if not sent.  
}


void BFSTraversalSPAlgorithm::broadcastGo(BFSTraversalSP* t) {
  for (degree_t i = 0; i < module->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = module->getInterface(i);
    if (p->isConnected() && p != t->tree.parent) {
      sendGo(t,p);
    }
  }
}

void BFSTraversalSPAlgorithm::ackGo(BFSTraversalSP* t,P2PNetworkInterface *p, mID_t i, distance_t d) {
  BFSData *dataHeader = getBFSDataHeader(t);  
  Message *m = new BFSTraversalSPGoAckMessage(dataHeader,i,d);
  p->send(m);
}

void BFSTraversalSPAlgorithm::forwardBack(BFSTraversalSP* t) {
  distance_t height = t->getHeight();
  sysSize_t size = t->getSize();
  BFSData *dataHeader =  getBFSDataHeader(t);
  BFSData* dataBack =  t->getBFSDataBack();
  Message *m = new BFSTraversalSPBackMessage(dataHeader, t->tree.rootID,
					     t->tree.distance-1, height,
					     size, dataBack);
  t->tree.parent->send(m);
}

bool BFSTraversalSPAlgorithm::checkAndForwardBack(BFSTraversalSP* t) {
  
  if (t->waiting.size() == 0) {

    if (t->tree.rootID == module->blockId) {  
       
      t->finished = true;
      
      if (t->enableConvergecast) {

	// re-init all aggregate
	for (degree_t i = 0; i < t->branchHeight.size(); i++) {
	  t->branchHeight[i] = 0;
	  t->branchSize[i] = 0;
	}
	
	// initiate convergecast     
	sendConvergecastBFSPGo(t);
      } else {

	sysSize_t ssize = t->getSize();
	distance_t height = t->getHeight();
	
	if(t->aggComputation) {
	  assert(sysSize != 0); // i.e., size is known!
	  if (ssize != sysSize) {
	    t->finished = false;
	    
	    MY_CERR << "size: " << ssize << " versus " << sysSize << endl;
	    MY_CERR << "wrong size aggregate, BFS-SP_ONE-WAY not finished!" << endl;
	    return false;
	  }
	}
	
	MY_CERR << "BFS completed (BFS-SP_ONE-WAY) at " << getScheduler()->now()
		<< " size: " << ssize << " ecc: " << height
		<< endl;
	
	assert(ssize == (sysSize_t) getWorld()->getSize());
	
	/*if(t->getSize() != (sysSize_t) getWorld()->getSize()) {
	  cerr << "error" << endl;
	  getchar();
	  }*/
	
	return true;
      }
    } else {
      forwardBack(t);
    }
  }
  return false;
}

void BFSTraversalSPAlgorithm::sendDelete(BFSTraversalSP* t, P2PNetworkInterface *p, mID_t i, distance_t d) {
  BFSData *dataHeader =  getBFSDataHeader(t);
  Message *m = new BFSTraversalSPDeleteMessage(dataHeader,i,d);
  p->send(m);
}

void BFSTraversalSPAlgorithm::sendConvergecastBFSPGo(BFSTraversalSP* t) {
  std::set<P2PNetworkInterface*>::iterator it;

  for (it = t->tree.children.begin(); it != t->tree.children.end(); it++) {
    
    P2PNetworkInterface *p = *it;
    
    BFSData *dataHeader =  getBFSDataHeader(t);
    Message *m = new BFSTraversalSPConvergecastGoMessage(dataHeader,
							 t->tree.rootID,
							 t->tree.distance);
    t->waiting.insert(p);
    p->send(m);
  }
}

void BFSTraversalSPAlgorithm::sendConvergecastBFSPBack(BFSTraversalSP* t) {
  distance_t height = t->getHeight();
  sysSize_t size = t->getSize();
  BFSData *dataHeader =  getBFSDataHeader(t);
  BFSData *dataBack =  t->getBFSDataBack();
  
  Message *m = new BFSTraversalSPConvergecastBackMessage(dataHeader,
							 t->tree.rootID,
							 t->tree.distance-1,
							 height,
							 size,
							 dataBack);

  assert(t->waiting.empty());
  t->tree.parent->send(m);
}


/*********** TreeAlgData Class ************/
TreeAlgData::TreeAlgData(Tree &t): tree(t)  {
  getHeader = BFSData::getNULLBFSData;
}

TreeAlgData::~TreeAlgData() {}

/*********** TreeAlgData Class ************/

TreeBroadcast::TreeBroadcast(Tree &t): TreeAlgData(t) {
  visitHandler = emptyChildHandler;
}

TreeBroadcast::~TreeBroadcast() {}

void TreeBroadcast::reset() {}

/*********** TreeConvergecast Class ************/
TreeConvergecast::TreeConvergecast(Tree &t): TreeAlgData(t) {
  waiting = 0;
  getBackData = BFSData::getNULLBFSData;
  goHandler = emptyChildHandler;
  backHandler = emptyChildHandler;
}

TreeConvergecast::~TreeConvergecast() {
  
}

void TreeConvergecast::reset() {
  waiting = 0;
}

/*********** TreeBroadcastAlgorithm Class ************/
TreeBroadcastAlgorithm::TreeBroadcastAlgorithm(BaseSimulator::BuildingBlock *m): module(m) {
  
}
  
TreeBroadcastAlgorithm::~TreeBroadcastAlgorithm() {}

bool TreeBroadcastAlgorithm::start(TreeBroadcast* t) {
  if (t->tree.isALeaf()) {
    return true;
  }
  
  TreeBroadcastMessage m(NULL);
  t->tree.broadcast(&m);
  return false;
}
  
void TreeBroadcastAlgorithm::reset(TreeBroadcast* t) {

}

bool TreeBroadcastAlgorithm::handle(MessagePtr m, TreeBroadcast* t) {
  assert(hasToHandle(m));

  t->visitHandler(m);
  
  t->tree.broadcast(m.get());

  return false;
}
  
bool TreeBroadcastAlgorithm::hasToHandle(MessagePtr m) {
  return (m->type == TREE_BROADCAST);
}

/*********** TreeBroadcastAlgorithm Class ************/
TreeConvergecastAlgorithm::TreeConvergecastAlgorithm(BaseSimulator::BuildingBlock *m): module(m) {

}

TreeConvergecastAlgorithm::~TreeConvergecastAlgorithm() {}


bool TreeConvergecastAlgorithm::startBroadcast(TreeConvergecast* t) {

  if (t->tree.isALeaf()) {
    return true;
  }
  
  TreeConvergecastGoMessage m(t->getHeader());
  setWaiting(t);
  t->tree.broadcast(&m);
  return false;
}

bool TreeConvergecastAlgorithm::startConvergecast(TreeConvergecast *t) {

  if (t->waiting == 0) {
    if (t->tree.isRoot()) {
      return true;
    }
    TreeConvergecastBackMessage *reply = new TreeConvergecastBackMessage(NULL);
    t->tree.sendParent(reply);
  }
  return false;
}
  
void TreeConvergecastAlgorithm::reset(TreeConvergecast* t) {
  t->reset();
}

bool TreeConvergecastAlgorithm::handle(MessagePtr m, TreeConvergecast* t) {
  assert(hasToHandle(m));

  
  switch(m->type) {
  case TREE_CONVERGECAST_GO:
    {
      TreeConvergecastGoMessagePtr recm = std::static_pointer_cast<TreeConvergecastGoMessage>(m);
      
#ifdef DEBUG_CONVERGECAST_ALGORITHM
      MY_CERR << "CONV_GO<>" << endl;
#endif
      
      t->goHandler(m);
      
      if (t->tree.isALeaf()) {
	sendBack(t,recm->header->clone());
      } else {
	setWaiting(t);
	t->tree.broadcast(m.get());
      }
      
    }
    break;
  case TREE_CONVERGECAST_BACK:
    {
      TreeConvergecastBackMessagePtr recm = std::static_pointer_cast<TreeConvergecastBackMessage>(m);

#ifdef DEBUG_CONVERGECAST_ALGORITHM
      MY_CERR << "CONV_BACK<>"
	      << " waiting: " << (uint) t->waiting << endl;
#endif
      
      t->waiting--;
      
      t->backHandler(m);

      if (t->waiting == 0) {
	if (t->tree.isRoot()) {
	  return true;
	} else {
	  sendBack(t,recm->header->clone());
	}
      }
    }
    break;
  default:
    assert(false);
  }

  return false;
}

bool TreeConvergecastAlgorithm::hasToHandle(MessagePtr m) {
  return (m->type == TREE_CONVERGECAST_GO || m->type == TREE_CONVERGECAST_BACK);
}

void TreeConvergecastAlgorithm::setWaiting(TreeConvergecast* t) {
  t->waiting = t->tree.children.size();
}

void TreeConvergecastAlgorithm::sendBack(TreeConvergecast* t, BFSData* h) {
  // sendBack
  TreeConvergecastBackMessage *reply = new TreeConvergecastBackMessage(h);
  reply->setBackData(t->getBackData());
  t->tree.sendParent(reply);
}
