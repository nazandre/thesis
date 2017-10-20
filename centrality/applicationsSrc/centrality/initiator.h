#ifndef INITIATOR_H_
#define INITIATOR_H_

#include "buildingBlock.h"
#include "bfsTree.h"

class InitiatorElection {
 public:
  BaseSimulator::BuildingBlock *module;
  BFSTraversal traversal;
  
  InitiatorElection(BaseSimulator::BuildingBlock *m);
  
  void init();
  bool start();
  
  bool hasToHandle(MessagePtr m);
  bool handle(MessagePtr m);

  void sendGo(P2PNetworkInterface *p);
  void broadcastGo();
  bool checkAndBroadcastGo();
  void ackGo(P2PNetworkInterface *p, mID_t i);
  void forwardBack();
  bool checkAndForwardBack();
};

class InitiatorElectionSP {
 public:
  BaseSimulator::BuildingBlock *module;

  BFSTraversalSP traversal;
  bool elected;
  
  InitiatorElectionSP(BaseSimulator::BuildingBlock *m);

  void init();
  bool start();

  void reset();
  
  bool handle(MessagePtr m);
  bool hasToHandle(MessagePtr m);

  size_t size();

  // Message-sending methods
  void broadcastGo();
  void sendGo(P2PNetworkInterface *p);
  bool checkAndBroadcastGo();
  void ackGo(P2PNetworkInterface *p, mID_t i, distance_t d);
  void forwardBack();
  bool checkAndForwardBack();
  void sendDelete(P2PNetworkInterface *p, mID_t i, distance_t d);

  void sendConvergecastGo();
  void sendConvergecastBack();
};

#endif
