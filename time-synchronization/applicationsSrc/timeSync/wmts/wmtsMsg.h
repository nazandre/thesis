#ifndef WMTS_MSG_H_
#define WMTS_MSG_H_

#include "../myMessage.h"
#include "tDefs.h"

// Round-trip synchronization (clock read)
class WMTSTimeMsg;
typedef std::shared_ptr<WMTSTimeMsg> WMTSTimeMsgPtr;

class WMTSTimeMsg: public MyMessage {
public :
  int64_t offset;
  wmtsDouble_t skew;
  uint32_t omega;
  bID r;
  wmtsDouble_t mu;
  wmtsDouble_t nu;
  
  Time localSendTime;
  Time globalSendTime;

  Time localReceiveTime;
  Time globalReceiveTime;
  
  WMTSTimeMsg(int64_t _offset,
	      wmtsDouble_t _skew,
	      uint32_t _omega,
	      bID _r,
	      wmtsDouble_t _mu,
	      wmtsDouble_t _nu);
  
  WMTSTimeMsg(WMTSTimeMsg *m);
  ~WMTSTimeMsg();

  WMTSTimeMsg* clone();
  
  unsigned int dataSize();
  
  unsigned int numEscapedBytes();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
};

#endif
