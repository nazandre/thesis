#ifndef MY_MESSAGE_HANDLER_H_
#define MY_MESSAGE_HANDLER_H_

#include "random.h"
#include "network.h"

class Layers;

class MyMessageHandler {
public:
  Layers &layers;
  
  MyMessageHandler(Layers &l, unsigned int seed);
  ~MyMessageHandler();

  void scheduleHandleMsgEvent(MessagePtr &m);
};

#endif
