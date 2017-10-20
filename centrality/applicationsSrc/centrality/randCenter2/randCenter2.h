/*
 * randCenter22.h
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#ifndef RAND_CENTER_2_2_H_
#define RAND_CENTER_2_2_H_

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm
#include "../bfsTreeMsg.h"

#include "bfsData.h"

#include "../treeElection.h"

#include <vector>

class RandCenter2 : public ElectionAlgorithm {
  
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

  // next random node
  distance_t maxRandomNumber;
  mID_t maxRandomNumberId;
  
  // tree election
  TreeElection<longDistance_t> *treeElection;
  
  RandCenter2(BaseSimulator::BuildingBlock *host);
  ~RandCenter2();

  size_t size();
  
  // ElectionAlgorithm Abstract Methods
  void reset();
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);
  
  // RandCenter2 specific methods
  void updateEstimatedEcc();
  
  void resetHandler();
  void addChildHandler(MessagePtr m);
  void removeChildHandler(MessagePtr m);
  BFSData* getBFSDataHeader();
  BFSData* getBFSDataBack();
  
  void initFirstIteration();
  void setHandlers();
  void initNextIteration(mID_t rootID);

  void broadcastNext(distance_t r, mID_t i);

  void broadcastLeaderGoMsg();
  void sendLeaderBackMsg();
  void sendLeaderMsg();
};

#endif /* EXTREMUM_CENTER_H_ */
