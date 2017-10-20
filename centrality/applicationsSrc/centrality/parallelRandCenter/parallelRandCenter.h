/*
 * parallelRandCenter2.h
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#ifndef PARALLEL_RAND_CENTER_H_
#define PARALLEL_RAND_CENTER_H_

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm
#include "../bfsTreeMsg.h"

#include "bfsData.h"
#include "parallelBFSSP.h"
#include "../treeElection.h"

#include <vector>
#include <map>

class ParallelRandCenter : public ElectionAlgorithm {
  
 public:
  static unsigned int numBFS;

  BFSTraversalSP* bfs; // useless in theory
  std::map<ParallelBFSTraversalSPKey,ParallelBFSTraversalSP*> bfss;
  BFSTraversalSPAlgorithm *bfsAlg;

  //TreeConvergecastAlgorithm* convergecastAlg;
  //TreeConvergecast* convergecast;
  
  longDistance_t estimatedEcc;
  distance_t randomNumber;
  uint8_t finishedBFSes;
  
  // tree election
  TreeElection<longDistance_t> *treeElection;
  
  ParallelRandCenter(BaseSimulator::BuildingBlock *host);
  ~ParallelRandCenter();

  size_t size();
  
  // ElectionAlgorithm Abstract Methods
  void reset();
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);
  
  // ParallelRandCenter specific methods
  
  void resetHandler();
  void addChildHandler(MessagePtr m);
  void removeChildHandler(MessagePtr m);
  BFSData* getBFSDataHeader(ParallelBFSTraversalSPKey k);
  BFSData* getBFSDataBack();
  
  void setHandlers(BFSTraversalSP* t, ParallelBFSTraversalSPKey k);
  
  void sendLeaderBackMsg();
  void sendLeaderMsg();

  // BFS management
  ParallelBFSTraversalSP* find(const ParallelBFSTraversalSPKey &k);
  void printBFSS();

  void aBFSHasFinished();
  void setElectionTree();
  void computeEccentricity();
};

#endif /* EXTREMUM_CENTER_H_ */
