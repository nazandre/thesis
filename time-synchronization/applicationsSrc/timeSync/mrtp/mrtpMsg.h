#ifndef MRTP_MSG_H_
#define MRTP_MSG_H_

#include "../myMessage.h"
#include "../utils/def.h"
#include "../synchronization/point.h"

#include <vector>

class MRTPTimeSyncMessage;
typedef std::shared_ptr<MRTPTimeSyncMessage> MRTPTimeSyncMessagePtr;

class MRTPTimeSyncMessage: public MyMessage {
public :
  Time receptionSimTime; // just for evaluation purpose!
  
  Time sendTime;
  Time receptionTime;
  hopDistance_t hops;

  MRTPTimeSyncMessage(hopDistance_t h);
  MRTPTimeSyncMessage(MRTPTimeSyncMessage *m);
  ~MRTPTimeSyncMessage();

  MRTPTimeSyncMessage* clone();
  
  Time getSendTime();
  Time getReceptionTime();

  unsigned int numEscapedBytes();
  unsigned int dataSize();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
};

// Round-trip synchronization (clock read)
class MRTPSyncRT;
typedef std::shared_ptr<MRTPSyncRT> MRTPSyncRTPtr;

#define T1_INDEX 0
#define T2_INDEX 1
#define T3_INDEX 2
#define T4_INDEX 3
#define T5_INDEX 4
#define T6_INDEX 5

class MRTPSyncRT: public MyMessage {
public :
  
  std::vector<Time> simTime;
  std::vector<Time> localTime;
  std::vector<Time> globalTime;
  unsigned int sizePrevious;
  
  MRTPSyncRT(int t);
  MRTPSyncRT(int t, std::vector<Time> &s,
	     std::vector<Time> &l,
	     std::vector<Time> &g);
  
  MRTPSyncRT(MRTPSyncRT *m);
  ~MRTPSyncRT();

  MRTPSyncRT* clone();

  unsigned int numEscapedBytes();
  unsigned int dataSize();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
  
  void timeStamp(BaseSimulator::BuildingBlock *module, int index);
};

#endif
