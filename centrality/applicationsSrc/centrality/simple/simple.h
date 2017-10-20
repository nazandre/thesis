#ifndef SIMPLE_EA_H_
#define SIMPLE_EA_H_

#include "../ea.h"
#include "initiator.h" // IE algorithm
#include "bfsTree.h" // SP algorithm

class SimpleEA : public ElectionAlgorithm {
 public:
  SimpleEA(BaseSimulator::BuildingBlock *m, bool t = false);
  ~SimpleEA();

  InitiatorElection *algorithm;

  BFSTraversalSPAlgorithm *traversalAlgorithm;
  BFSTraversalSP *traversal;
  

  size_t size();
  
  /* Election Algorithm Mandatory Methods */
  void start();
  void init();

  void handle(EventPtr e);
  void handle(MessagePtr m);
};

#endif
