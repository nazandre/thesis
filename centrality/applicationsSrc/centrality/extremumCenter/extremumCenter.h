/*
 * extremumCenter.h
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#ifndef EXTREMUM_CENTER_H_
#define EXTREMUM_CENTER_H_

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm
#include "../bfsTreeMsg.h"

#include <vector>

class ExtremumCenter : public ElectionAlgorithm {
  
 public:
  static unsigned int numBFS;
  
  InitiatorElectionSP *initiatorElection;
  BFSTraversalSP *bfs;
  BFSTraversalSPAlgorithm *bfsAlg;
  
  uint8_t currIteration;
  uint8_t maxIteration;
  longDistance_t estimatedEcc;
  longDistance_t sum;
  bool activated;
  
  std::vector<longDistance_t> childrenMaxSumDistances;
  std::vector<longDistance_t> childrenMinEcc;
  
  ExtremumCenter(BaseSimulator::BuildingBlock *host);
  ~ExtremumCenter();

  size_t size();

  // ElectionAlgorithm Abstract Methods
  void reset();
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);
  
  // ExtremumCenter specific methods
  void updateEstimatedEcc();
  
  void resetHandler();
  void addChildHandler(MessagePtr m);
  void removeChildHandler(MessagePtr m);
  BFSData* getBFSDataHeader();
  BFSData* getBFSDataBack();

  void initFirstIteration();
  void setHandlers();
  void initNextIteration(mID_t rootID);

  void forwardCenter();
  void forwardMaxSumDistances();
  void forwardFarthest();
};

#endif /* EXTREMUM_CENTER_H_ */
