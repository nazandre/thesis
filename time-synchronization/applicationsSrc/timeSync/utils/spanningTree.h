#ifndef SPANNING_TREE
#define SPANNING_TREE

#include <vector>

#include "network.h"

#include "def.h"

class Convergecast {
public:
  degree_t waiting;

  Convergecast();
  Convergecast(const Convergecast &c);
  ~Convergecast();

  inline bool hasFinished() {return waiting == 0;}
};

class SpanningTree {
public:
  bool visited; // visited
  degree_t waiting;
  P2PNetworkInterface *parent;
  std::vector<P2PNetworkInterface*> children;

  SpanningTree();
  SpanningTree(const SpanningTree &sp);
  ~SpanningTree();

  bool hasFinished();
  bool hasBeenVisited();
  P2PNetworkInterface* getParent();
  void visit(P2PNetworkInterface *p);
  void setWaiting(degree_t w);
  void back(P2PNetworkInterface *p, bool c);

  void broadcast(Message &m);
  void broadcast(MessagePtr &m);

  inline degree_t numChildren() { return children.size(); }
};

#endif
