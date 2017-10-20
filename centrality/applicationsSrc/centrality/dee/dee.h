/*
 * e2ace.h
 *
 *  Created on: 31/08/2016
 *      Author: Andre Naz
 */

#ifndef DEE_H_
#define DEE_H_

#include <set>
#include <utility>

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm

#include "../utils/probabilisticCounter.h"
#include "../treeElection.h"

class DEERoundData {
public:
  std::vector<myShortDouble_t> neighborhoods;
  degree_t received;

  DEERoundData();
  DEERoundData(const DEERoundData &rd); 
  ~DEERoundData();

  void updateNeighborhoods(std::vector<myShortDouble_t> &x);
  size_t size();
};

class DEE : public ElectionAlgorithm {
	
 public:
  DEE(BaseSimulator::BuildingBlock *m);
  ~DEE();

  InitiatorElectionSP *initiatorElection;
  BFSTraversalSP *traversal;

  distance_t bound; // diameter upperbound
  distance_t round;

  std::vector<myShortDouble_t> local;
  DEERoundData previous;
  DEERoundData current;

  longDistance_t eccentricity;
  TreeElection<longDistance_t> *minEccentricity;

  size_t size();
 
  /* Election Algorithm Mandatory Methods */
  void start();
  void init();
  void handle(EventPtr e);
  void handle(MessagePtr m);

  void initNextRound();
  void broadcastStart();
  
  degree_t sendEccentricityUpdate(longDistance_t r,
				  std::vector<myShortDouble_t>& x,
				  longDistance_t b);
  
  void sendEccentricityFeedbackMsg(P2PNetworkInterface *p, longDistance_t e, mID_t i);
  void spreadEccentricityLeaderMsg(P2PNetworkInterface *p, mID_t i);

  void updateEccentricity();
  longDistance_t meanEccentricity();
  longDistance_t maxEccentricity();
  void checkMinEccentricityElection(longDistance_t value, mID_t id, P2PNetworkInterface *path);

  static void mergeInto(std::vector<myShortDouble_t> &x, std::vector<myShortDouble_t> &y);
  static myDouble_t getSize(std::vector<myShortDouble_t> &x);
};

#endif /* DEE_H_ */
