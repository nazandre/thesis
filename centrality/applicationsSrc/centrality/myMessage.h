#ifndef MY_MESSAGE_H_
#define MY_MESSAGE_H_

#include "network.h"
#include "events.h"

#include "random.h"

#include "utils/def.h"

#define SIZE_NOT_COMPUTED 0

class Layers;

class MyMessage: public Message {
public:
  unsigned int savedSize = SIZE_NOT_COMPUTED;
  byte_t crc = 0;
  
  MyMessage();
  virtual ~MyMessage();

  void updateCrc(byte_t val);
  bool isToEscape(byte_t b);
  
  unsigned int numEscapedBytes(Time t);
  unsigned int numEscapedBytes(myDouble_t d);
  
  unsigned int numEscapedBytes(byte_t *field,unsigned int size);
  virtual unsigned int dataSize() = 0;
  virtual unsigned int numEscapedBytes() = 0;
  unsigned int size();

  static Time getNextBlockTickTime(Layers &layers, Time d);
  static void scheduleHandleMsgEvent(Layers &layers, MessagePtr &m);
  
  static void send(Layers &layers, P2PNetworkInterface *p2p, Message *m);  
};


class HandleMsgEvent;
typedef std::shared_ptr<HandleMsgEvent> HandleMsgEventPtr;

class HandleMsgEvent : public BlockEvent {
public:
  MessagePtr message;
  
  HandleMsgEvent(Time t, BaseSimulator::BuildingBlock *conBlock, MessagePtr &m);
  HandleMsgEvent(HandleMsgEvent *ev);
  ~HandleMsgEvent();
  void consumeBlockEvent();
  const virtual string getEventName();
};

#endif
