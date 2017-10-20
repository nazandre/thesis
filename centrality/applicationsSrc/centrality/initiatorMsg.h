#ifndef INITIATOR_ELECTION_MESSAGE_H_
#define INITIATOR_ELECTION_MESSAGE_H_

#include "eaMsg.h"
#include "utils/def.h"
#include "bfsData.h"

#define IE_SP_GO_MESSAGE 37001
#define IE_SP_GO_ACK_MESSAGE 37002
#define IE_SP_BACK_MESSAGE 37003
#define IE_SP_DELETE_MESSAGE 37004
#define IE_SP_CONVERGECAST_GO_MESSAGE 37005
#define IE_SP_CONVERGECAST_BACK_MESSAGE 37006

#define IE_GO_MESSAGE 37005
#define IE_GO_ACK_MESSAGE 37006
#define IE_BACK_MESSAGE 37007

/********** IESPGoMsg Class **********/
class IESPGoMsg;
typedef std::shared_ptr<IESPGoMsg>  IESPGoMsgPtr;

class IESPGoMsg : public EAMessage {
 public :
  mID_t rootID;
  distance_t distance;

  BFSData* header;
  
  IESPGoMsg(BFSData* h, mID_t i, distance_t d);
  ~IESPGoMsg();
};

/********** IESPGoAckMsg Class **********/
class IESPGoAckMsg;
typedef std::shared_ptr<IESPGoAckMsg>  IESPGoAckMsgPtr;

class IESPGoAckMsg : public EAMessage {
 public :
  mID_t rootID;
  distance_t distance;

  BFSData* header;
  
  IESPGoAckMsg(BFSData* h, mID_t i, distance_t d);
  ~IESPGoAckMsg();
};

/********** IESPBackMsg Class **********/
class IESPBackMsg;
typedef std::shared_ptr<IESPBackMsg> IESPBackMsgPtr;

class IESPBackMsg : public EAMessage {
 public :  
  mID_t rootID;
  distance_t distance;
  distance_t height;
  sysSize_t bSize;

  BFSData* header;
  BFSData* backData; // data to send back up to the root
  
  IESPBackMsg(BFSData* he, mID_t i, distance_t d, distance_t h, sysSize_t s,BFSData* b);
  ~IESPBackMsg();
};

/********** IESPDeleteMsg Class **********/
class IESPDeleteMsg;
typedef std::shared_ptr<IESPDeleteMsg>  IESPDeleteMsgPtr;

class IESPDeleteMsg : public EAMessage {
 public :
  mID_t rootID;
  distance_t distance;
  BFSData* header;

  IESPDeleteMsg(BFSData* h, mID_t i, distance_t d);
  ~IESPDeleteMsg();
};


/********** IESPConvergecastGoMsg Class **********/
class IESPConvergecastGoMsg;
typedef std::shared_ptr<IESPConvergecastGoMsg>  IESPConvergecastGoMsgPtr;

class IESPConvergecastGoMsg : public IESPGoMsg {
 public :
  IESPConvergecastGoMsg(BFSData* h, mID_t i, distance_t d);
  ~IESPConvergecastGoMsg();
};

/********** IESPBackMsg Class **********/
class IESPConvergecastBackMsg;
typedef std::shared_ptr<IESPConvergecastBackMsg> IESPConvergecastBackMsgPtr;

class IESPConvergecastBackMsg : public IESPBackMsg {
 public:
  IESPConvergecastBackMsg(BFSData* he, mID_t i, distance_t d, distance_t h, sysSize_t s,BFSData* b);
  ~IESPConvergecastBackMsg();
};

/********** IEGoMsg Class **********/
class IEGoMsg;
typedef std::shared_ptr<IEGoMsg>  IEGoMsgPtr;

class IEGoMsg : public EAMessage {
 public :
  mID_t rootID;
  IEGoMsg(mID_t i);
  ~IEGoMsg();
};

/********** IEGoMsg Class **********/
class IEGoAckMsg;
typedef std::shared_ptr<IEGoAckMsg>  IEGoAckMsgPtr;

class IEGoAckMsg : public EAMessage {
 public :
  mID_t rootID;
  IEGoAckMsg(mID_t i);
  ~IEGoAckMsg();
};

/********** IEBackMsg Class **********/
class IEBackMsg;
typedef std::shared_ptr<IEBackMsg> IEBackMsgPtr;

class IEBackMsg : public EAMessage {
 public :
  mID_t rootID;
	
  IEBackMsg(mID_t i);
  ~IEBackMsg();
};

#endif
