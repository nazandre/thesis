/*
 * tbce.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef TBCE_H_
#define TBCE_H_

#include "buildingBlock.h"

#include "../ea.h"
#include "../initiator.h" // InitiatorElectionSP algorithm
#include "../treeElection.h"

class TBCE : public ElectionAlgorithm {
	
 public:
  InitiatorElectionSP *initiatorElection;
  BFSTraversalSP *bfs;

  longDistance_t initiatorFarness;
  distance_t height; // avoid to recompute each time. do not account in memory usage.
  
  TreeElection<mID_t> *treeElection;
  
  TBCE(BaseSimulator::BuildingBlock *host);
  ~TBCE();

  size_t size();
  
  // ElectionAlgorithm abstract methods:
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);

  // compute averaged farness
  void setHandlers();
  void resetHandler();
  void addChildHandler(MessagePtr mes);
  BFSData* getBFSDataBack();

  // Tree election
  void spreadTBCentralityGoLeaderMsg(distance_t tbc);
  void spreadTBCentralityBackLeaderMsg();
  void spreadTBCentralityLeaderMsg();
};

#endif /* TBCE_H_ */
