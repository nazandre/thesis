/*
 * randCenter.h
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#ifndef RAND_CENTER_H_
#define RAND_CENTER_H_

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm
#include "../bfsTreeMsg.h"

#include "bfsData.h"

#include <vector>

class RandCenter : public ElectionAlgorithm {
  
 public:
  static unsigned int numBFS;
  InitiatorElectionSP *initiatorElection;
  BFSTraversalSP *bfs;
  BFSTraversalSPAlgorithm *bfsAlg;

  //TreeConvergecastAlgorithm* convergecastAlg;
  //TreeConvergecast* convergecast;
  
  uint8_t currIteration;
  uint8_t maxIteration;
  longDistance_t estimatedEcc;
  longDistance_t sum;
  distance_t randomNumber;
  bool activated;

  std::vector<distance_t> childrenMaxActivatedRandomNumber;
  std::vector<longDistance_t> childrenMinEcc;
  
  RandCenter(BaseSimulator::BuildingBlock *host);
  ~RandCenter();

  size_t size();
  
  // ElectionAlgorithm Abstract Methods
  void reset();
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);
  
  // RandCenter specific methods
  void updateEstimatedEcc();
  
  void resetHandler();
  void addChildHandler(MessagePtr m);
  void removeChildHandler(MessagePtr m);
  BFSData* getBFSDataHeader();
  BFSData* getBFSDataBack();
  
  //void convergecastGoHandler(MessagePtr msg);
  //void convergecastBackHandler(MessagePtr msg);
  //BFSData* getConvergecastDataHeader();
  //BFSData* getConvergecastDataBack();
  
  void initFirstIteration();
  void setHandlers();
  void initNextIteration(mID_t rootID);

  void forwardCenter();
  void forwardMaxActivatedRandomNumber();
};

#endif /* EXTREMUM_CENTER_H_ */
