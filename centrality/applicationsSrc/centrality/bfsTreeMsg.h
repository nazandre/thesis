#ifndef BFSTREE_MESSAGE_H_
#define BFSTREE_MESSAGE_H_

#include "utils/def.h"
#include "eaMsg.h"
#include "bfsData.h"

#define BFS_TRAVERSAL_SP_GO 36001
#define BFS_TRAVERSAL_SP_GO_ACK 36002
#define BFS_TRAVERSAL_SP_BACK 36003
#define BFS_TRAVERSAL_SP_DELETE 36004
#define BFS_TRAVERSAL_SP_CONVERGECAST_GO 36005
#define BFS_TRAVERSAL_SP_CONVERGECAST_BACK 36006

#define TREE_BROADCAST 36011
#define TREE_CONVERGECAST_GO 36012
#define TREE_CONVERGECAST_BACK 36013

/******** BFSTraversalSPMessage Class ********/
class BFSTraversalSPMessage;
typedef std::shared_ptr<BFSTraversalSPMessage>  BFSTraversalSPMessagePtr;

class BFSTraversalSPMessage : public EAMessage {
 public:
  mID_t rootID;
  distance_t distance;

  BFSData* header;
  BFSData* backData; // data to send back up to the root
  
  BFSTraversalSPMessage(BFSData* h, mID_t rid, distance_t d);
  virtual ~BFSTraversalSPMessage();
  
  static bool isABFSTraversalSPMessage(MessagePtr m); 
};

/******** BFSTraversalSPGoMessage Class ********/
class BFSTraversalSPGoMessage;
typedef std::shared_ptr<BFSTraversalSPGoMessage>  BFSTraversalSPGoMessagePtr;

class BFSTraversalSPGoMessage : public BFSTraversalSPMessage {
 public:
  BFSTraversalSPGoMessage(BFSData* h, mID_t rid, distance_t d);
  ~BFSTraversalSPGoMessage();  
};

/********** BFSTraversalSPGoAckMessage Class **********/
class BFSTraversalSPGoAckMessage;
typedef std::shared_ptr<BFSTraversalSPGoAckMessage>  BFSTraversalSPGoAckMessagePtr;

class BFSTraversalSPGoAckMessage : public BFSTraversalSPMessage {
 public :  
  BFSTraversalSPGoAckMessage(BFSData* h, mID_t i, distance_t d);
  ~BFSTraversalSPGoAckMessage();
};

/******** BFSTraversalSPBackMessage Class ********/
class BFSTraversalSPBackMessage;
typedef std::shared_ptr<BFSTraversalSPBackMessage>  BFSTraversalSPBackMessagePtr;

class BFSTraversalSPBackMessage : public BFSTraversalSPMessage {
 public:
  distance_t height;
  sysSize_t bSize;

  BFSTraversalSPBackMessage(BFSData* head, mID_t rid, distance_t d,distance_t h, sysSize_t s, BFSData* b);
  ~BFSTraversalSPBackMessage();  
};

/******** BFSTraversalSPDeleteMessage Class ********/
class BFSTraversalSPDeleteMessage;
typedef std::shared_ptr<BFSTraversalSPDeleteMessage>  BFSTraversalSPDeleteMessagePtr;

class BFSTraversalSPDeleteMessage : public BFSTraversalSPMessage {
 public:
  
  BFSTraversalSPDeleteMessage(BFSData* h, mID_t rid, distance_t d);
  ~BFSTraversalSPDeleteMessage();  
};

/********** BFSTraversalSPConvergecastGoMessage Class **********/
class BFSTraversalSPConvergecastGoMessage;
typedef std::shared_ptr<BFSTraversalSPConvergecastGoMessage>  BFSTraversalSPConvergecastGoMessagePtr;

class BFSTraversalSPConvergecastGoMessage: public BFSTraversalSPGoMessage {
 public :  
  BFSTraversalSPConvergecastGoMessage(BFSData* h, mID_t i, distance_t d);
  ~BFSTraversalSPConvergecastGoMessage();
};

/******** BFSTraversalSPBackMessage Class ********/
class BFSTraversalSPConvergecastBackMessage;
typedef std::shared_ptr<BFSTraversalSPConvergecastBackMessage>  BFSTraversalSPConvergecastBackMessagePtr;

class  BFSTraversalSPConvergecastBackMessage: public BFSTraversalSPBackMessage {
 public:
  BFSTraversalSPConvergecastBackMessage(BFSData* head, mID_t rid, distance_t d,distance_t h, sysSize_t s, BFSData* b);
  ~BFSTraversalSPConvergecastBackMessage();  
};

/*** TreeMessage class ***/
class TreeMessage;
typedef std::shared_ptr<TreeMessage>  TreeMessagePtr;

class TreeMessage : public EAMessage {
 public:

  BFSData* header;
  BFSData* backData; // data to send back up to the root
  
  TreeMessage(BFSData* h);
  TreeMessage(TreeMessage &m);
  virtual ~TreeMessage();
  
  virtual TreeMessage* clone();
  static bool isATreeMessage(MessagePtr m); 
};


/*** TreeBroadcastMessage class ***/
class TreeBroadcastMessage;
typedef std::shared_ptr<TreeBroadcastMessage>  TreeBroadcastMessagePtr;

class TreeBroadcastMessage : public TreeMessage {
 public:
  
  TreeBroadcastMessage(BFSData* h);
  TreeBroadcastMessage(TreeBroadcastMessage& m);
  ~TreeBroadcastMessage();
  
  virtual TreeBroadcastMessage* clone();
};

/*** TreeConvergecastGoMessage class ***/
class TreeConvergecastGoMessage;
typedef std::shared_ptr<TreeConvergecastGoMessage> TreeConvergecastGoMessagePtr;

class TreeConvergecastGoMessage : public TreeMessage {
 public:
  
  TreeConvergecastGoMessage(BFSData* h);
  ~TreeConvergecastGoMessage();  
  TreeConvergecastGoMessage(TreeConvergecastGoMessage& m);

  virtual TreeConvergecastGoMessage* clone();
};

/*** TreeConvergecastBackMessage class ***/
class TreeConvergecastBackMessage;
typedef std::shared_ptr<TreeConvergecastBackMessage> TreeConvergecastBackMessagePtr;

class TreeConvergecastBackMessage : public TreeMessage {
 public:
  TreeConvergecastBackMessage(BFSData* h);
  TreeConvergecastBackMessage(TreeConvergecastBackMessage& m);
  ~TreeConvergecastBackMessage();

  void setBackData(BFSData *b);
  
  virtual TreeConvergecastBackMessage* clone();
};

#endif
