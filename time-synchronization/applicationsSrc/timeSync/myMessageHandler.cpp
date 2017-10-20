#include "myMessageHandler.h"

#include <iostream>

#include <functional>
#include <random>

#include "scheduler.h"
#include "layers.h"

#include "myMessage.h"

using namespace std;
using namespace BaseSimulator;

MyMessageHandler::MyMessageHandler(Layers &l, unsigned int seed): layers(l) {
}

MyMessageHandler::~MyMessageHandler() {  
}

void MyMessageHandler::scheduleHandleMsgEvent(MessagePtr &m) {
  BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
  Time handlingTime = layers.synchronization->getHandlingTime(m);
  sched->schedule(new HandleMsgEvent(handlingTime,layers.module,m));
}
