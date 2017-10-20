#ifndef SIMPLE_SP_EA_H_
#define SIMPLE_SP_EA_H_

#include "../ea.h"
#include "initiator.h" // IE SP algorithm

#include "../alphaSynchronizedBFS.h"
#include "../simulation.h"

class SimpleSPEA : public ElectionAlgorithm {
 public:
  
#ifdef ALPHA_SYNCHRONIZED_BFS
  AlphaSynchronizedBFSAlgorithm *algorithm;
  AlphaSynchronizedBFS::Traversal *traversal;
#else
  InitiatorElectionSP *algorithm;
#endif
  
  SimpleSPEA(BaseSimulator::BuildingBlock *m);
  ~SimpleSPEA();

  size_t size();
  
  /* Election Algorithm Mandatory Methods */
  void start();
  void init();

  void handle(EventPtr e);
  void handle(MessagePtr m);
};

#endif
