#ifndef SIMPLE_SINGLE_SP_H_
#define SIMPLE_SINGLE_SP_H_

#include "../ea.h"
#include "initiator.h" // IE SP algorithm

#include "../simulation.h"
#include "alphaSynchronizedBFS.h"

template <class Alg, class Tra> class SimpleSingleSP : public ElectionAlgorithm {
 public:
  
  Alg *algorithm;
  Tra *traversal;
  
  SimpleSingleSP(BaseSimulator::BuildingBlock *m);
  ~SimpleSingleSP();

  size_t size();
  
  /* Election Algorithm Mandatory Methods */
  void start();
  void init();

  void handle(EventPtr e);
  void handle(MessagePtr m);
};
#endif
