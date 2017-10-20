#ifndef CTM_H_
#define CTM_H_

#include "network.h"

#include "../utils/def.h"
#include "../spanningTree/bfsTree.h"
#include "../synchronization/clock.h"

class Layers;

class ConvergecastTimeMaxAlgorithm {
public:
  Layers &layers;

  Tree &tree;
  Synchronization::OffsetClock maxClock;
  degree_t waiting;
  hopDistance_t distanceMaxClock;
  
  ConvergecastTimeMaxAlgorithm(Layers &l, Tree &t);
  ConvergecastTimeMaxAlgorithm(const ConvergecastTimeMaxAlgorithm &c);
  ~ConvergecastTimeMaxAlgorithm();

  void init();
  void start();
  
  void handle(MessagePtr m);

  void scheduleCTMDone();
};

#endif
