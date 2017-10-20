#include "initiatorMsg.h"

/********** IESPGoMsg Class **********/
IESPGoMsg::IESPGoMsg(BFSData* h, mID_t i, distance_t d): EAMessage() {
  type = IE_SP_GO_MESSAGE;
  rootID = i;
  distance = d;
  header = h;

  dataSize = sizeof(mID_t)
    + sizeof(distance_t);
  
  if (header != NULL) {
    dataSize += header->size();
  }
}

IESPGoMsg::~IESPGoMsg() {
  delete header;
}

/********** IESPGoAckMsg Class **********/
IESPGoAckMsg::IESPGoAckMsg(BFSData* h, mID_t i, distance_t d): EAMessage() {
  type = IE_SP_GO_ACK_MESSAGE;

  rootID = i;
  distance = d;
  header = h;

  dataSize = sizeof(mID_t)
    + sizeof(distance_t);
  if (header != NULL) {
    dataSize += header->size();
  }
}

IESPGoAckMsg::~IESPGoAckMsg() {
  delete header;
}

/********** IESPBackMsg Class **********/
	
IESPBackMsg::IESPBackMsg(BFSData* he, mID_t i, distance_t d, distance_t h, sysSize_t s, BFSData* b): EAMessage() {
  type = IE_SP_BACK_MESSAGE;

  rootID = i;
  distance = d;
  height = h;
  bSize = s;
  header = he;
  backData = b;

  dataSize = sizeof(mID_t) // id
    + sizeof(distance_t) // distance
    + sizeof(distance_t) // height
    // size: depends on the app. No implemented app that needs it!
    ;
  
  if (header != NULL) {
    dataSize += header->size();
  }

  if (backData != NULL) {
    dataSize += backData->size();
  }

}

IESPBackMsg::~IESPBackMsg() {
  delete header;
  delete backData;
}

/********** IESPDeleteMsg Class **********/
IESPDeleteMsg::IESPDeleteMsg(BFSData* h, mID_t i, distance_t d): EAMessage() {
  type = IE_SP_DELETE_MESSAGE;
  rootID = i;
  distance = d;
  header = h;

  dataSize = sizeof(mID_t) // id
    + sizeof(distance_t) // distance  
    ;
  
  if (header != NULL) {
    dataSize += header->size();
  }

}	

IESPDeleteMsg::~IESPDeleteMsg() {
  delete header;
  
}
/********** IESPConvergecastGoMsg Class **********/  
IESPConvergecastGoMsg::IESPConvergecastGoMsg(BFSData* h, mID_t i, distance_t d): IESPGoMsg(h,i,d) {
  type = IE_SP_CONVERGECAST_GO_MESSAGE;
}
  
IESPConvergecastGoMsg::~IESPConvergecastGoMsg() {}

/********** IESPBackMsg Class **********/
IESPConvergecastBackMsg::IESPConvergecastBackMsg(BFSData* he, mID_t i, distance_t d, distance_t h, sysSize_t s,BFSData* b): IESPBackMsg(he,i,d,h,s,b) {
  type = IE_SP_CONVERGECAST_BACK_MESSAGE;
}
  
IESPConvergecastBackMsg::~IESPConvergecastBackMsg() {}

/********** IEGoMsg Class **********/

IEGoMsg::IEGoMsg(mID_t i): EAMessage() {
  type = IE_GO_MESSAGE;
  dataSize = sizeof(mID_t);
  rootID = i;
}

IEGoMsg::~IEGoMsg() {}

/********** IEGoAckMsg Class **********/

IEGoAckMsg::IEGoAckMsg(mID_t i): EAMessage() {
  type = IE_GO_ACK_MESSAGE;
  dataSize = sizeof(mID_t);
  rootID = i;
}

IEGoAckMsg::~IEGoAckMsg() {
}

/********** IEBackMsg Class **********/
IEBackMsg::IEBackMsg(mID_t i): EAMessage() {
  type = IE_BACK_MESSAGE;
  dataSize = sizeof(mID_t);
  rootID = i;
}

IEBackMsg::~IEBackMsg() {};
