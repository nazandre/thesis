#ifndef BFS_TREE_H_
#define BFS_TREE_H_

#include <map>
#include <set>
#include <vector>
#include <functional>

#include "bfsTreeMsg.h"
#include "buildingBlock.h"
#include "utils/def.h"
#include "bfsData.h"

class BFSTraversalSP;

typedef std::function<BFSData* ()> bfsDataFunc_t;
typedef std::function<BFSData* (BFSTraversalSP*)> bfsDataFuncT_t;
typedef std::function<void(MessagePtr)> childHandler_t;
typedef std::function<void()> resetHandler_t;

class Tree {
 public:
  set<P2PNetworkInterface*> children;
  P2PNetworkInterface *parent = NULL;
  distance_t distance;
  mID_t rootID;

  Tree();
  Tree(mID_t rootID);
  Tree(const Tree &t);
  ~Tree();

  inline bool isALeaf() {return children.empty();}

  void broadcast(Message *m);
  //void broadcast(Time t, Message &m);
  void sendParent(Message *m);
  
  inline P2PNetworkInterface* getParent() {return parent;}
  inline bool isRoot() {return parent == NULL;}
  inline degree_t numChildren() { return children.size();}

  size_t size();
};

class BFSTraversal {
 public:
  Tree tree;
  degree_t waiting = 0;

  degree_t broadcastWait = 0;
  vector<bool> prevGoAcked;

  bfsDataFunc_t getBFSDataHeader;
  bfsDataFunc_t getBFSDataBack;
  
  childHandler_t removeChild;
  childHandler_t addChild;
  resetHandler_t resetHandler;
  
  BFSTraversal();
  BFSTraversal(Tree &t);
  BFSTraversal(const BFSTraversal &t);
  ~BFSTraversal();

  size_t size();
  
};

class BFSTraversalSP {
 public:  
  Tree tree;
  set<P2PNetworkInterface*> waiting;
  vector<distance_t> branchHeight;
  vector<sysSize_t> branchSize;
  
  degree_t broadcastWait = 0;
  vector<bool> prevGoAcked;
  
  bool enableConvergecast;
  
  bool finished;
  bool aggComputation;
  
  bool backSent = false; // just for debugging purpose
  
  bfsDataFunc_t getBFSDataHeader;  
  bfsDataFunc_t getBFSDataBack;
  
  childHandler_t removeChild;
  childHandler_t addChild;
  resetHandler_t resetHandler;

  BFSTraversalSP();
  BFSTraversalSP(Tree &t);
  BFSTraversalSP(const BFSTraversalSP &t);
  virtual ~BFSTraversalSP();

  virtual distance_t getHeight();
  virtual sysSize_t getSize();

  void subscribe(mID_t id);

  size_t size();
};

class BFSTraversalSPAlgorithm {
 public:
  
  BaseSimulator::BuildingBlock *module;
  bfsDataFuncT_t getBFSDataHeader2;

  sysSize_t sysSize; // used to compute aggregate!
  
  BFSTraversalSPAlgorithm(BaseSimulator::BuildingBlock *m);
  ~BFSTraversalSPAlgorithm();

  static BFSData* getNULLBFSData2(BFSTraversalSP *t);

  BFSData* getBFSDataHeader(BFSTraversalSP *t);

  void setSysSize(size_t s) {sysSize = s;}
  
  bool start(BFSTraversalSP* t);
  BFSTraversalSP* create();
  BFSTraversalSP* create(MessagePtr m);
  void reset(BFSTraversalSP* t);
  
  bool handle(MessagePtr m, BFSTraversalSP *t);
  bool hasToHandle(MessagePtr m);
  
  void sendGo(BFSTraversalSP *t,P2PNetworkInterface *p);
  bool checkAndBroadcastGo(BFSTraversalSP* t);
  void broadcastGo(BFSTraversalSP* t);
  void ackGo(BFSTraversalSP* t,P2PNetworkInterface *p, mID_t i, distance_t d);
  void forwardBack(BFSTraversalSP* t);
  bool checkAndForwardBack(BFSTraversalSP* t);
  void sendDelete(BFSTraversalSP* t,P2PNetworkInterface *p, mID_t i, distance_t d);

  void broadcast(BFSTraversalSP* t); // how to keep the broadcasted data?!
  void broadcastConvergecastGo(BFSTraversalSP* t, BFSData *data);
  void sendConvergecastBack(BFSTraversalSP* t);

  void sendConvergecastBFSPGo(BFSTraversalSP* t);
  void sendConvergecastBFSPBack(BFSTraversalSP* t);

  static size_t getSizeAggComp(size_t s, bool a);
  size_t size();
};


class BFSTraversalManager{
 public:
  map<mID_t,BFSTraversalSP*> traversals;

  BFSTraversalManager();
  ~BFSTraversalManager();

  BFSTraversalSP* get(mID_t rootID);
  void insert(mID_t rootID, BFSTraversalSP* traversal);
};

class TreeAlgData {
public:  
  bfsDataFunc_t getHeader;
  Tree &tree;
  
  TreeAlgData(Tree &t);
  virtual ~TreeAlgData();
  virtual void reset() = 0;
  inline void setTree(Tree &t) {tree = t;}
};

class TreeBroadcast: public TreeAlgData {
public:
  childHandler_t visitHandler;
  
  TreeBroadcast(Tree &t);
  ~TreeBroadcast();

  void reset();
};

class TreeConvergecast: public TreeAlgData {
public:
  bfsDataFunc_t getBackData;
  degree_t waiting = 0; // among children

  childHandler_t goHandler;
  childHandler_t backHandler;
  
  TreeConvergecast(Tree &t);
  ~TreeConvergecast();

  void reset();

  inline size_t size() { return sizeof(degree_t);}
};

class TreeBroadcastAlgorithm {
public:
  BaseSimulator::BuildingBlock *module;
  
  TreeBroadcastAlgorithm(BaseSimulator::BuildingBlock *m);
  ~TreeBroadcastAlgorithm();

  bool start(TreeBroadcast* t);  
  void reset(TreeBroadcast* t);

  bool handle(MessagePtr m, TreeBroadcast* t);
  bool hasToHandle(MessagePtr m);
};

class TreeConvergecastAlgorithm {
public:
  BaseSimulator::BuildingBlock *module;
  
  TreeConvergecastAlgorithm(BaseSimulator::BuildingBlock *m);
  ~TreeConvergecastAlgorithm();

  bool startBroadcast(TreeConvergecast* t);
  bool startConvergecast(TreeConvergecast *t);
  
  void reset(TreeConvergecast* t);

  bool handle(MessagePtr m, TreeConvergecast* t);
  bool hasToHandle(MessagePtr m);

  void setWaiting(TreeConvergecast* t);

  void sendBack(TreeConvergecast* t, BFSData* h);
};

#endif // BFS_TREE_H_
