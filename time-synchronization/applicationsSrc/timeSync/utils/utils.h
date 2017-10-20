#ifndef UTILS_H_
#define UTILS_H_

#include <vector>

#include "buildingBlock.h"
#include "network.h"

#include "def.h"
#include "layers.h"

#define TIME_US_TO_RATE(s,t) ((s*8.0) / ((double)t))

class Utils {
public:

  static P2PNetworkInterface* getRandomConnectedInterface(Layers &layers, P2PNetworkInterface *ignore);
  
  static degree_t broadcast(Layers &layers, Message &msg, P2PNetworkInterface *ignore);
  static degree_t directBroadcast(Layers &layers, Message &msg, P2PNetworkInterface *ignore, Time t);
  //static degree_t broadcast(Layers &layers, Message &msg, P2PNetworkInterface *ignore, Time time);

  static uint getRandom(Layers &layers, uint min, uint max);
};


#endif
