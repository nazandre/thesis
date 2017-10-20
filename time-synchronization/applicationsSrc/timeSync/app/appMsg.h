#ifndef APP_MSG_H_
#define APP_MSG_H_

#include "myMessage.h"
#include "utils/def.h"

class AppMessage;
typedef std::shared_ptr<AppMessage> AppMessagePtr;

class AppMessage: public MyMessage {
public :
  
  AppMessage();
  AppMessage(AppMessage *m);
  ~AppMessage();

  AppMessage* clone();
  
  unsigned int numEscapedBytes();
  unsigned int dataSize();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
};

#endif
