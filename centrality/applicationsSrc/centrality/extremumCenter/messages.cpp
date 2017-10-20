#include "messages.h"
#include "../msgID.h"

ExtremumCenterMessageNext::ExtremumCenterMessageNext(longDistance_t v, sysSize_t s) {
  type = EXTREMUM_CENTER_NEXT;
  value = v;
  sysSize = s;

  dataSize = sizeof(longDistance_t);
#ifndef TWO_WAY_BFS_SP
  dataSize += sizeof(sysSize_t);
#endif
}

ExtremumCenterMessageNext::~ExtremumCenterMessageNext() { }
