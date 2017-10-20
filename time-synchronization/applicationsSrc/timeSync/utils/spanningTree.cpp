#include "spanningTree.h"

// Convergescast class

Convergecast::Convergecast() {
  waiting = 0;
}

Convergecast::Convergecast(const Convergecast &c): waiting(c.waiting) {

}

Convergecast::~Convergecast() {

}

// SpanningTree class

SpanningTree::SpanningTree(): visited(false), waiting(0), parent(NULL) {
  
}

SpanningTree::SpanningTree(const SpanningTree &sp): visited(sp.visited), waiting(sp.waiting), parent(sp.parent) {

}

SpanningTree::~SpanningTree() {

}

bool SpanningTree::hasFinished() {
  return visited && (waiting == 0);
}

bool SpanningTree::hasBeenVisited() {
  return visited;
}

P2PNetworkInterface* SpanningTree::getParent() {
  return parent;
}

void SpanningTree::visit(P2PNetworkInterface *p) {
  visited = true;
  parent = p;
}

void SpanningTree::setWaiting(degree_t w) {
  waiting = w;
}

void SpanningTree::back(P2PNetworkInterface *p, bool c) {
  waiting--;
  if (c) {
    children.push_back(p);
  }
}

void SpanningTree::broadcast(Message &m) {
  std::vector<P2PNetworkInterface*>::iterator it;
  for(it = children.begin(); it != children.end(); ++it) {
    P2PNetworkInterface *p = *it;
    Message *msg = m.clone();
    p->send(msg);
  }
}

void SpanningTree::broadcast(MessagePtr &m) {
  std::vector<P2PNetworkInterface*>::iterator it;
  for(it = children.begin(); it != children.end(); ++it) {
    P2PNetworkInterface *p = *it;
    Message *msg = m->clone();
    p->send(msg);
  }
}
