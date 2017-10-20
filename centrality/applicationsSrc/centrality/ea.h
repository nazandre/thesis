#ifndef ELECTION_ALGORITHM_H_
#define ELECTION_ALGORITHM_H_

#include "buildingBlock.h"
#include "network.h"
#include "events.h"

#include "bfsTree.h"

class ElectionAlgorithm {
 protected:
  BaseSimulator::BuildingBlock *module;

  ElectionAlgorithm(BaseSimulator::BuildingBlock *m);
  ElectionAlgorithm(const ElectionAlgorithm &ea);
  
 public:

  virtual ~ElectionAlgorithm();
  
  virtual void init() = 0;
  virtual void start() = 0;
  virtual void handle(MessagePtr m) = 0; // msg
  virtual void handle(EventPtr e) = 0; // event

  void win(bool printDate = true);

  virtual size_t size() = 0;
  size_t dataSize(); 
};


#endif // ELECTION_ALGORITHM_H_
