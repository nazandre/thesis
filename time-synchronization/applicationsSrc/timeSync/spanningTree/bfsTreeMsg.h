#ifndef BFSTREE_MESSAGE_H_
#define BFSTREE_MESSAGE_H_

#include "../utils/def.h"
#include "../myMessage.h"
#include "../appMsgID.h"

/******** BFSTraversalSPMessage Class ********/
class BFSTraversalSPMessage;
typedef std::shared_ptr<BFSTraversalSPMessage>  BFSTraversalSPMessagePtr;

class BFSTraversalSPMessage : public MyMessage {
 public:
  mID_t rootID;
  hopDistance_t distance;

  BFSTraversalSPMessage(mID_t rid, hopDistance_t d);
  
  virtual ~BFSTraversalSPMessage();
  
  static bool isABFSTraversalSPMessage(MessagePtr m);

  virtual unsigned int dataSize() = 0;
  unsigned int numEscapedBytes(); // always returns 0, don't really care, important only for precision in sync. msgs
};

/******** BFSTraversalSPGoMessage Class ********/
class BFSTraversalSPGoMessage;
typedef std::shared_ptr<BFSTraversalSPGoMessage>  BFSTraversalSPGoMessagePtr;

class BFSTraversalSPGoMessage : public BFSTraversalSPMessage {
 public:
  BFSTraversalSPGoMessage(mID_t rid, hopDistance_t d);
  ~BFSTraversalSPGoMessage();

  unsigned int dataSize();
};

/********** BFSTraversalSPGoAckMessage Class **********/
class BFSTraversalSPGoAckMessage;
typedef std::shared_ptr<BFSTraversalSPGoAckMessage>  BFSTraversalSPGoAckMessagePtr;

class BFSTraversalSPGoAckMessage : public BFSTraversalSPMessage {
 public :  
  BFSTraversalSPGoAckMessage(mID_t i, hopDistance_t d);
  ~BFSTraversalSPGoAckMessage();

  unsigned int dataSize();
};

/******** BFSTraversalSPBackMessage Class ********/
class BFSTraversalSPBackMessage;
typedef std::shared_ptr<BFSTraversalSPBackMessage>  BFSTraversalSPBackMessagePtr;

class BFSTraversalSPBackMessage : public BFSTraversalSPMessage {
 public:
  BFSTraversalSPBackMessage(mID_t rid, hopDistance_t d);
  ~BFSTraversalSPBackMessage();

  unsigned int dataSize();
};

/******** BFSTraversalSPDeleteMessage Class ********/
class BFSTraversalSPDeleteMessage;
typedef std::shared_ptr<BFSTraversalSPDeleteMessage>  BFSTraversalSPDeleteMessagePtr;

class BFSTraversalSPDeleteMessage : public BFSTraversalSPMessage {
 public:
  BFSTraversalSPDeleteMessage(mID_t rid, hopDistance_t d);
  ~BFSTraversalSPDeleteMessage();

  unsigned int dataSize();
};

#endif
