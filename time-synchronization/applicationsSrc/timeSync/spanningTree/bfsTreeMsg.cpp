#include "bfsTreeMsg.h"

class BFSTraversalSPMessage;
typedef std::shared_ptr<BFSTraversalSPMessage>  BFSTraversalSPMessagePtr;

BFSTraversalSPMessage::BFSTraversalSPMessage(mID_t rid, hopDistance_t d): MyMessage() {
  rootID = rid;
  distance = d;
}

BFSTraversalSPMessage::~BFSTraversalSPMessage() {}

bool BFSTraversalSPMessage::isABFSTraversalSPMessage(MessagePtr m) {
  return (m->type == BFS_TRAVERSAL_SP_GO ||
	  m->type == BFS_TRAVERSAL_SP_GO_ACK ||
	  m->type == BFS_TRAVERSAL_SP_BACK ||
	  m->type == BFS_TRAVERSAL_SP_DELETE);


}

unsigned int BFSTraversalSPMessage::numEscapedBytes() {
  return 0;
}

/******** BFSTraversalSPGoMessage Class ********/

BFSTraversalSPGoMessage::BFSTraversalSPGoMessage(mID_t rid, hopDistance_t d) :
  BFSTraversalSPMessage(rid,d) {
  type =  BFS_TRAVERSAL_SP_GO;
}

BFSTraversalSPGoMessage::~BFSTraversalSPGoMessage() {}

unsigned int BFSTraversalSPGoMessage::dataSize() {
  return sizeof(mID_t) + sizeof(hopDistance_t);
}

/********** BFSTraversalSPGoAckMessage Class **********/
BFSTraversalSPGoAckMessage::BFSTraversalSPGoAckMessage(mID_t rid, hopDistance_t d) :
  BFSTraversalSPMessage(rid,d) {
  type =  BFS_TRAVERSAL_SP_GO_ACK;
}

BFSTraversalSPGoAckMessage::~BFSTraversalSPGoAckMessage() {}


unsigned int BFSTraversalSPGoAckMessage::dataSize() {
  return sizeof(mID_t) + sizeof(hopDistance_t);
}

/******** BFSTraversalSPBackMessage Class ********/

BFSTraversalSPBackMessage::BFSTraversalSPBackMessage(mID_t rid, hopDistance_t d) : BFSTraversalSPMessage(rid,d) {
  type = BFS_TRAVERSAL_SP_BACK;
}

BFSTraversalSPBackMessage::~BFSTraversalSPBackMessage() {}

unsigned int BFSTraversalSPBackMessage::dataSize() {
  return sizeof(mID_t) + sizeof(hopDistance_t);
}

/******** BFSTraversalSPDeleteMessage Class ********/
BFSTraversalSPDeleteMessage::BFSTraversalSPDeleteMessage(mID_t rid, hopDistance_t d) : BFSTraversalSPMessage(rid,d) {
  type = BFS_TRAVERSAL_SP_DELETE;
}

BFSTraversalSPDeleteMessage::~BFSTraversalSPDeleteMessage() {}


unsigned int BFSTraversalSPDeleteMessage::dataSize() {
  return sizeof(mID_t) + sizeof(hopDistance_t);
}
