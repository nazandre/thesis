#ifndef TIME_MESSAGE_H
#define TIME_MESSAGE_H

#include "utils/def.h"

#include "myMessage.h"

class TimeMessage;
typedef std::shared_ptr<TimeMessage> TimeMessagePtr;

class TimeMessage: public MyMessage {
  Time sendTime;
  Time receptionTime;
  
public :
  TimeMessage();
  TimeMessage(TimeMessage *m);
  ~TimeMessage();
  
  Time getSendTime();
  Time getReceptionTime();

  unsigned int dataSize();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
};


#endif
