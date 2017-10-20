/*
 * timeSyncBlockCode.h
 *
 *  Created on: 28/01/2017
 *      Author: Andre
 */

#ifndef TIMESYNCBLOCKCODE_H_
#define TIMESYNCBLOCKCODE_H_

#include "buildingBlock.h"
#include "blockCode.h"
#include "simulator.h"

#include "myMessageHandler.h"
#include "layers.h"
#include "random.h"

class TimeSyncBlockCode : public BlockCode { 
public:
  Layers layers;
  MyMessageHandler *msgHandler;
  doubleRNG crcEscapeGenerator;
  
  TimeSyncBlockCode(BaseSimulator::BuildingBlock *host);
  ~TimeSyncBlockCode();

  void startup();
  void init();
  void processLocalEvent(EventPtr pev);

  //void scheduleHandleMsgEvent(MessagePtr &m);
  
  static BlockCode *buildNewBlockCode(BuildingBlock *host);
};

#endif /* TIMESYNCBLOCKCODE_H_ */
