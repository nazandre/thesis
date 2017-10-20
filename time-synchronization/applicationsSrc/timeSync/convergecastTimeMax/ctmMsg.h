#ifndef CTM_MESSAGE_H_
#define CTM_MESSAGE_H_

#include "../utils/def.h"
#include "../myMessage.h"
#include "../appMsgID.h"

/******** CTMGoMessage Class ********/
class CTMGoMessage;
typedef std::shared_ptr<CTMGoMessage>  CTMGoMessagePtr;

class CTMGoMessage: public MyMessage {
 public:
  CTMGoMessage();
  CTMGoMessage(CTMGoMessage *m);
  ~CTMGoMessage();

  CTMGoMessage* clone();
  
  unsigned int dataSize();
  unsigned int numEscapedBytes(); // always returns 0, don't really care, important only for precision in sync. msgs
};

/******** CTMBackMessage Class ********/
class CTMBackMessage;
typedef std::shared_ptr<CTMBackMessage>  CTMBackMessagePtr;

class CTMBackMessage: public MyMessage {
 public:
  hopDistance_t hops;
  Time sendTime;
  Time receptionTime;
  
  CTMBackMessage(hopDistance_t h);
  CTMBackMessage(CTMBackMessage *m);
  ~CTMBackMessage();

  CTMBackMessage* clone();
  
  unsigned int dataSize();
  unsigned int numEscapedBytes(); // always returns 0, don't really care, important only for precision in sync. msgs

  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();

};

#endif
