#ifndef RAND_CENTER_MESSAGE
#define RAND_CENTER_MESSAGE

#include "../utils/def.h"
#include "../eaMsg.h"

class RandCenterMessageNext;
typedef std::shared_ptr<RandCenterMessageNext>  RandCenterMessageNextPtr;

class RandCenterMessageNext : public EAMessage {
 public:
  longDistance_t value;
  sysSize_t sysSize;
  
  RandCenterMessageNext(longDistance_t v, sysSize_t s);
  virtual ~RandCenterMessageNext();
};


#endif
