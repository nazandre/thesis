#ifndef AD_MSG_H_
#define AD_MSG_H_

#include <vector>

#include "buildingBlock.h"

#include "../myMessage.h"

// Round-trip synchronization (clock read)
class ADSyncMsg;
typedef std::shared_ptr<ADSyncMsg> ADSyncMsgPtr;

#define AD_T1_INDEX 0
#define AD_T2_INDEX 1
#define AD_T3_INDEX 2
#define AD_T4_INDEX 3

class ADSyncMsg: public MyMessage {
public :
  
  std::vector<Time> simTime;
  std::vector<Time> localTime;
  std::vector<Time> globalTime;
  
  ADSyncMsg(int t);
  ADSyncMsg(int t,
	    std::vector<Time> &s,
	    std::vector<Time> &l,
	    std::vector<Time> &g);
  
  ADSyncMsg(ADSyncMsg *m);
  ~ADSyncMsg();

  ADSyncMsg* clone();
  
  unsigned int dataSize();
  unsigned int numEscapedBytes();
  
  void beginningTransmissionMacLayer();
  void endReceptionMacLayer();
  
  void timeStamp(BaseSimulator::BuildingBlock *module,int index);
};


#endif
