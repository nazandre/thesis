#include <iostream>

#include "appMsg.h"
#include "appMsgID.h"

using namespace std;
using namespace BaseSimulator;

AppMessage::AppMessage(): MyMessage() {
  type = APP_MSG;
  //accountForInStats = false;
}

AppMessage::AppMessage(AppMessage *m): MyMessage() {
  type = m->type;
  accountForInStats = m->accountForInStats;
}

AppMessage::~AppMessage() {
}

AppMessage* AppMessage::clone() {
  return new AppMessage(this);
}

unsigned int AppMessage::numEscapedBytes() {
  return 0;
}

unsigned int AppMessage::dataSize() {
  return 0;
}

void AppMessage::beginningTransmissionMacLayer() {}

void AppMessage::endReceptionMacLayer() {}
