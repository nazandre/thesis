#include "messages.h"
#include "../msgID.h"

ABCCenterV2MessageNext::ABCCenterV2MessageNext(longDistance_t v, sysSize_t s) {
  type = ABCCENTERV2_NEXT;
  value = v;
  sysSize = s;
  
  dataSize = sizeof(longDistance_t);  
#ifndef TWO_WAY_BFS_SP
  dataSize += sizeof(sysSize_t);
#endif
}

ABCCenterV2MessageNext::~ABCCenterV2MessageNext() { }

ABCCenterV2MessageCenter::ABCCenterV2MessageCenter(distance_t g) {
  type = ABCCENTERV2_CENTER;
  dataSize = sizeof(distance_t);
  gradient = g;
}

ABCCenterV2MessageCenter::~ABCCenterV2MessageCenter() { }
