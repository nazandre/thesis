#include "neighbor.h"

Neighbor::Neighbor(P2PNetworkInterface *i){
  interface = i;
}

Neighbor::Neighbor(const Neighbor &n) {
  interface = n.interface;
}

Neighbor::~Neighbor() {

}

size_t Neighbor::size() {
  return sizeof(P2PNetworkInterface*);
}

/*
Neighborhood::Neighborhood() {

}

Neighborhood::Neighborhood(const Neighborhood& n) {
  neighbors = n.neighbors;
}

Neighborhood::~Neighborhood() {

}
 
void Neighborhood::update() {
  neighbors.clear();

  vector<P2PNetworkInterface*>& interfaces = module->getP2PNetworkInterfaces();
  vector<P2PNetworkInterface*>::iterator it;
  int i = 0;
  for (it = interfaces.begin(); it != interfaces.end(); ++it) {
    P2PNetworkInterface *p = *it;
    if (p->isConnected()) {
      Neighbor n(p);
      neighbors.push_back(n);
    }
  }
}

Neighbor* Neighborhood::get(P2PNetworkInterface *p) {
  Neighbor *n = NULL;
  int i = 0;
  for (i =0; i< neighbors.size(); i++) {
    n = &neighbors[i];
    if (n->interface == p) {
      break;
    }
  }
  return n;
}
*/
