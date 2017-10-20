#ifndef TREE_ELECTION_H_
#define TREE_ELECTION_H_

#include <set>

#include "network.h"
#include "bfsTree.h"
#include "utils/def.h"

template <typename T>
class TreeElection {
public:
  P2PNetworkInterface *path;
  mID_t id;
  T value;
  Tree &tree;
  std::set<P2PNetworkInterface*> received;
  
  TreeElection(Tree &t): tree(t) {
    path = NULL;
    id = NOT_A_VALID_MODULE_ID;
    value = T();
  }

  TreeElection(Tree &t, P2PNetworkInterface *p, mID_t i, T &v): tree(t) {
    path = p;
    id = i;
    value = v;
  }

  ~TreeElection() {}
};

#endif
