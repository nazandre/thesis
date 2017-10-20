#ifndef BFS_TREE_H_
#define BFS_TREE_H_

#include <set>

#include "bfsTreeMsg.h"
#include "buildingBlock.h"
#include "../utils/def.h"

class Layers;

class Tree {
 public:
  set<P2PNetworkInterface*> children;
  P2PNetworkInterface *parent = NULL;
  hopDistance_t distance;
  mID_t rootID;

  Tree();
  Tree(mID_t rootID);
  Tree(const Tree &t);
  ~Tree();

  void broadcast(Message &m);
  //void broadcast(Time t, Message &m);
  void sendParent(Message *m);
  
  inline P2PNetworkInterface* getParent() {return parent;}
  inline bool isRoot() {return parent == NULL;}
  inline degree_t numChildren() { return children.size();}
};

class BFSTraversal {
 public:
  Tree tree;
  degree_t waiting = 0;
  degree_t broadcastWait = 0;

  BFSTraversal();
  BFSTraversal(Tree &t);
  BFSTraversal(const BFSTraversal &t);
  ~BFSTraversal();
};

class BFSTraversalSP {
 public:
  Tree tree;
  set<P2PNetworkInterface*> waiting;
  degree_t broadcastWait = 0;
  
  BFSTraversalSP();
  BFSTraversalSP(Tree &t);
  BFSTraversalSP(const BFSTraversalSP &t);
  ~BFSTraversalSP();

};

class BFSTraversalSPAlgorithm {
 public:
  Layers &layers;
  BaseSimulator::BuildingBlock *module;
  bool controlledBroadcast = false;
  
  BFSTraversalSPAlgorithm(Layers &l, bool cb = false);
  ~BFSTraversalSPAlgorithm();
  
  bool start(BFSTraversalSP* t);
  BFSTraversalSP* create();
  BFSTraversalSP* create(MessagePtr m);
  void reset(BFSTraversalSP* t);
  
  bool handle(MessagePtr m, BFSTraversalSP *t);
  bool hasToHandle(MessagePtr m);

  bool checkAndBroadcastGo(BFSTraversalSP* t);
  void broadcastGo(BFSTraversalSP* t);
  void ackGo(P2PNetworkInterface *p, mID_t i, hopDistance_t d);
  void forwardBack(BFSTraversalSP* t);
  bool checkAndForwardBack(BFSTraversalSP* t);
  void sendDelete(P2PNetworkInterface *p, mID_t i, hopDistance_t d);

  void scheduleSpanningTreeConstructed();
};

#endif // BFS_TREE_H_
