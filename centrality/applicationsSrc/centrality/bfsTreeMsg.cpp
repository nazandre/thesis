#include "bfsTreeMsg.h"

class BFSTraversalSPMessage;
typedef std::shared_ptr<BFSTraversalSPMessage>  BFSTraversalSPMessagePtr;

BFSTraversalSPMessage::BFSTraversalSPMessage(BFSData *h, mID_t rid, distance_t d) {
  rootID = rid;
  distance = d;
  header = h;
  backData = NULL;

  dataSize = //sizeof(mID_t) // rootID is not necessary in bfs execution of a single node
    + sizeof(distance_t);
  if(header != NULL) {
    dataSize += header->size(); 
  }
}

BFSTraversalSPMessage::~BFSTraversalSPMessage() {}

bool BFSTraversalSPMessage::isABFSTraversalSPMessage(MessagePtr m) {
  return (m->type == BFS_TRAVERSAL_SP_GO ||
	  m->type == BFS_TRAVERSAL_SP_GO_ACK ||
	  m->type == BFS_TRAVERSAL_SP_BACK ||
	  m->type == BFS_TRAVERSAL_SP_DELETE ||
	  m->type == BFS_TRAVERSAL_SP_CONVERGECAST_GO ||
	  m->type == BFS_TRAVERSAL_SP_CONVERGECAST_BACK
	  );
}

/******** BFSTraversalSPGoMessage Class ********/

BFSTraversalSPGoMessage::BFSTraversalSPGoMessage(BFSData *h, mID_t rid, distance_t d) :
  BFSTraversalSPMessage(h,rid,d) {
  type =  BFS_TRAVERSAL_SP_GO;
  dataSize += 0;
}

BFSTraversalSPGoMessage::~BFSTraversalSPGoMessage() {}

/********** BFSTraversalSPGoAckMessage Class **********/
BFSTraversalSPGoAckMessage::BFSTraversalSPGoAckMessage(BFSData *h,mID_t rid, distance_t d) :
  BFSTraversalSPMessage(h,rid,d) {
  type =  BFS_TRAVERSAL_SP_GO_ACK;
  dataSize += 0;
}

BFSTraversalSPGoAckMessage::~BFSTraversalSPGoAckMessage() {}

/******** BFSTraversalSPBackMessage Class ********/

BFSTraversalSPBackMessage::BFSTraversalSPBackMessage(BFSData *head, mID_t rid, distance_t d,distance_t h,sysSize_t s,BFSData *b) : BFSTraversalSPMessage(head,rid,d) {
  type = BFS_TRAVERSAL_SP_BACK;
  height = h;
  bSize = s;
  backData = b;

  if (backData != NULL) {
    dataSize += backData->size();
  }
  dataSize += sizeof(sysSize_t); // update sysSize (not required in all cases)
}

BFSTraversalSPBackMessage::~BFSTraversalSPBackMessage() {}

/******** BFSTraversalSPDeleteMessage Class ********/
BFSTraversalSPDeleteMessage::BFSTraversalSPDeleteMessage(BFSData *h,mID_t rid, distance_t d) : BFSTraversalSPMessage(h,rid,d) {
  type = BFS_TRAVERSAL_SP_DELETE;
}

BFSTraversalSPDeleteMessage::~BFSTraversalSPDeleteMessage() {}

/********** BFSTraversalSPConvergecastGoMessage Class **********/
BFSTraversalSPConvergecastGoMessage::BFSTraversalSPConvergecastGoMessage(BFSData* h, mID_t i, distance_t d): BFSTraversalSPGoMessage(h,i,d) {
    type = BFS_TRAVERSAL_SP_CONVERGECAST_GO;
  }

BFSTraversalSPConvergecastGoMessage::~BFSTraversalSPConvergecastGoMessage() {}

/******** BFSTraversalSPBackMessage Class ********/

BFSTraversalSPConvergecastBackMessage::BFSTraversalSPConvergecastBackMessage(BFSData* head, mID_t rid, distance_t d,distance_t h, sysSize_t s, BFSData* b):
  BFSTraversalSPBackMessage(head,rid,d,h,s,b) {
  type = BFS_TRAVERSAL_SP_CONVERGECAST_BACK;
}
  
BFSTraversalSPConvergecastBackMessage::~BFSTraversalSPConvergecastBackMessage() {
  
}

/*** TreeMessage class ***/
TreeMessage::TreeMessage(BFSData* h): EAMessage() {
    header = h;
    backData = NULL;
    dataSize = 0;
    
    if (header != NULL) {
      dataSize += header->size();
    }
}

TreeMessage::TreeMessage(TreeMessage &m): EAMessage(m) {
  dataSize = m.dataSize;
  type = m.type;

  header = NULL;
  backData = NULL;

  if(m.header != NULL) {
    header = m.header->clone();
  }

  if(m.backData != NULL) {
    backData = m.backData->clone();
  }
}

  
TreeMessage::~TreeMessage() {
    delete header;
    delete backData;
}
  
bool TreeMessage::isATreeMessage(MessagePtr m) {
  return (m->type == TREE_BROADCAST ||
	  m->type == TREE_CONVERGECAST_GO ||
	  m->type == TREE_CONVERGECAST_BACK);
}

TreeMessage* TreeMessage::clone() {
  return new TreeMessage(*this);
}

/*** TreeBroadcastMessage class ***/

TreeBroadcastMessage::TreeBroadcastMessage(BFSData* h): TreeMessage(h) {
  type = TREE_BROADCAST;
}

TreeBroadcastMessage::TreeBroadcastMessage(TreeBroadcastMessage &m): TreeMessage(m) {

}

TreeBroadcastMessage::~TreeBroadcastMessage() {

}

TreeBroadcastMessage* TreeBroadcastMessage::clone() {
  return new TreeBroadcastMessage(*this);
}

/*** TreeConvergecastGoMessage class ***/

TreeConvergecastGoMessage::TreeConvergecastGoMessage(BFSData* h): TreeMessage(h) {
  type = TREE_CONVERGECAST_GO;
}

TreeConvergecastGoMessage::TreeConvergecastGoMessage(TreeConvergecastGoMessage &m): TreeMessage(m) {
}

TreeConvergecastGoMessage::~TreeConvergecastGoMessage() {
  
}

TreeConvergecastGoMessage* TreeConvergecastGoMessage::clone() {
  return new TreeConvergecastGoMessage(*this);
}

/*** TreeConvergecastBackMessage class ***/

TreeConvergecastBackMessage::TreeConvergecastBackMessage(BFSData* h): TreeMessage(h) {
  type = TREE_CONVERGECAST_BACK;
}

TreeConvergecastBackMessage::TreeConvergecastBackMessage(TreeConvergecastBackMessage &m): TreeMessage(m) {
}

TreeConvergecastBackMessage::~TreeConvergecastBackMessage() {
  
}

void TreeConvergecastBackMessage::setBackData(BFSData*b) {
  backData = b;
  if (backData != NULL) {
    dataSize += backData->size();
  }
}

TreeConvergecastBackMessage* TreeConvergecastBackMessage::clone() {
  return new TreeConvergecastBackMessage(*this);
}
