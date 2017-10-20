/*
 * e2ace.h
 *
 *  Created on: 31/08/2016
 *      Author: Andre Naz
 */

#ifndef E2ACE_H_
#define E2ACE_H_

#include <set>
#include <utility>

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm

#include "../utils/probabilisticCounter.h"
#include "../treeElection.h"

class RoundData {
public:
  ProbabilisticCounter* neighborhoods;
  degree_t received;

  RoundData();
  RoundData(const RoundData &rd); 
  ~RoundData();

  void mergeNeighborhood(ProbabilisticCounter &neighbor);
  
  size_t size();
};


class E2ACE : public ElectionAlgorithm {
	
 public:
  E2ACE(BaseSimulator::BuildingBlock *m);
  ~E2ACE();

  InitiatorElectionSP *initiatorElection;
  BFSTraversalSP *traversal;

  distance_t bound; // diameter upperbound
  distance_t round;

  RoundData previous;
  RoundData current;
  ProbabilisticCounter* local;

  std::vector<longDistance_t> farnesses;
  longDistance_t farness;
  
  TreeElection<longDistance_t> *minFarness;

  size_t size();
  void setHashFuncs(unsigned int nh);

  /* Election Algorithm Mandatory Methods */
  void start();
  void init();
  void handle(EventPtr e);
  void handle(MessagePtr m);

  void broadcastStart();
  
  //void setPreviousToCurrent();
  void initNextRound();
  
  degree_t sendFarnessUpdate(longDistance_t r, ProbabilisticCounter *p,longDistance_t b);
  void sendFarnessFeedbackMsg(P2PNetworkInterface *p, longDistance_t e, mID_t i);
  void spreadFarnessLeaderMsg(P2PNetworkInterface *p, mID_t i);

  void updateFarnesses();
  longDistance_t meanFarness();
  longDistance_t maxFarness();
  void checkMinFarnessElection(longDistance_t value, mID_t id, P2PNetworkInterface *path);
};

#endif /* E2ACE_H_ */
