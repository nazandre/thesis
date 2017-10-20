#ifndef ABCCENTERV2_MESSAGE
#define ABCCENTERV2_MESSAGE

#include "../utils/def.h"
#include "../eaMsg.h"

class ABCCenterV2MessageNext;
typedef std::shared_ptr<ABCCenterV2MessageNext>  ABCCenterV2MessageNextPtr;

class ABCCenterV2MessageNext : public EAMessage {
 public:
  longDistance_t value;
  sysSize_t sysSize;
    
  ABCCenterV2MessageNext(longDistance_t v, sysSize_t s);
  virtual ~ABCCenterV2MessageNext();
};

class ABCCenterV2MessageCenter;
typedef std::shared_ptr<ABCCenterV2MessageCenter>  ABCCenterV2MessageCenterPtr;

class ABCCenterV2MessageCenter : public EAMessage {
 public:
  distance_t gradient;
  
  ABCCenterV2MessageCenter(distance_t v);
  virtual ~ABCCenterV2MessageCenter();
};


#endif
