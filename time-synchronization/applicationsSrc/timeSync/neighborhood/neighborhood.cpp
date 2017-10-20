#include "neighborhood.h"

#include <cassert>

#include "scheduler.h"
#include "world.h"
#include "lattice.h"

#include "../utils/utils.h"
#include "../layers.h"

Neighborhood::Neighborhood(Layers &l): layers(l) {
  degree_t d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();
  states.resize(d,DISCONNECTED);
}

Neighborhood::Neighborhood(const Neighborhood &n):
  layers(n.layers), states(n.states) {
  
}

Neighborhood::~Neighborhood() {

}

void Neighborhood::init() {
  byte_t d = layers.module->getNbInterfaces();
  P2PNetworkInterface *p2p = NULL;
  for (byte_t i = 0; i < d; i++) {
   p2p = layers.module->getInterface(i);
   byte_t indexP2p = layers.module->getInterfaceIndex(p2p);
   assert (indexP2p == i);
    if (p2p->isConnected()) {
      states[i] = CONNECTED;
    } else {
      states[i] = DISCONNECTED;
    }
  }
}

bool Neighborhood::isConnected(P2PNetworkInterface *p) {
  byte_t indexRecv = layers.module->getInterfaceIndex(p);
  return states[indexRecv] == CONNECTED;
}

void Neighborhood::connect(P2PNetworkInterface *p) {
  byte_t indexRecv = layers.module->getInterfaceIndex(p);
  connect(indexRecv);
}

void Neighborhood::disconnect(P2PNetworkInterface *p) {
  byte_t indexRecv = layers.module->getInterfaceIndex(p);
  disconnect(indexRecv);
}

degree_t Neighborhood::getNbNeighbors() {
  degree_t n = 0;
  
  for (byte_t i = 0; i < states.size(); i++) {
    if (states[i] == CONNECTED) {
      n++;
    }
  }
  return n;
}
