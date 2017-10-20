#ifndef EXTREMUM_CENTER_MESSAGE
#define EXTREMUM_CENTER_MESSAGE

#include "../utils/def.h"
#include "../eaMsg.h"

class ExtremumCenterMessageNext;
typedef std::shared_ptr<ExtremumCenterMessageNext>  ExtremumCenterMessageNextPtr;

class ExtremumCenterMessageNext : public EAMessage {
 public:
  longDistance_t value;
  sysSize_t sysSize;
  
  ExtremumCenterMessageNext(longDistance_t v, sysSize_t s);
  virtual ~ExtremumCenterMessageNext();
};

#endif
