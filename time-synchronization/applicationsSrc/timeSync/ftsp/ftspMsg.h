#ifndef FTSP_MSG_H_
#define FTSP_MSG_H_

#include "../myMessage.h"
#include "../utils/def.h"
#include "../synchronization/point.h"

#include <vector>

class FTSPTimeSyncMessage;
typedef std::shared_ptr<FTSPTimeSyncMessage> FTSPTimeSyncMessagePtr;

class FTSPTimeSyncMessage: public MyMessage {
public :
  Time receptionSimTime; // just for evaluation purpose!
  
  mID_t rootID;
  uint32_t round;
  hopDistance_t hops;

  Time sendTime;
  Time receptionTime;

  FTSPTimeSyncMessage(mID_t id, uint32_t r, hopDistance_t h);
  FTSPTimeSyncMessage(FTSPTimeSyncMessage *m);
  ~FTSPTimeSyncMessage();

  FTSPTimeSyncMessage* clone();
  
  Time getSendTime();
  Time getReceptionTime();

  unsigned int dataSize();
  
  unsigned int numEscapedBytes();

  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
};

#endif
