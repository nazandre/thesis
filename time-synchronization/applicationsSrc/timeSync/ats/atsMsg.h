#ifndef ATS_MSG_H_
#define ATS_MSG_H_

#include "../myMessage.h"

// Round-trip synchronization (clock read)
class ATSTimeMsg;
typedef std::shared_ptr<ATSTimeMsg> ATSTimeMsgPtr;

class ATSTimeMsg: public MyMessage {
public :
  myDouble_t senderSkew;
  
  Time localSendTime;
  Time globalSendTime;

  Time localReceiveTime;
  Time globalReceiveTime;
  
  ATSTimeMsg(myDouble_t s);
  
  ATSTimeMsg(ATSTimeMsg *m);
  ~ATSTimeMsg();

  ATSTimeMsg* clone();
  
  unsigned int dataSize();
  
  unsigned int numEscapedBytes();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
};

#endif
