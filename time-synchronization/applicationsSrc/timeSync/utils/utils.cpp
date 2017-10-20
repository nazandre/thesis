#include "utils.h"

#include <random>

#include "scheduler.h"
#include "../myMessage.h"

using namespace std;
using namespace BaseSimulator;

P2PNetworkInterface* Utils::getRandomConnectedInterface(Layers &layers, P2PNetworkInterface *ignore) {
  BuildingBlock *m = layers.module;
  degree_t n = layers.neighborhood->getNbNeighbors();
  degree_t pn = m->getNbInterfaces();
  P2PNetworkInterface *p2p = NULL;

  if (n < 1) {
    return NULL;
  }
  
  if (ignore && (n == 1)) {
    return ignore;
  } else {
    std::mt19937 gen(m->getRandomUint());
    std::uniform_int_distribution<> dis(1, n);
    degree_t i = dis(gen);
    degree_t j = pn;
    while (i > 0) {
      j--;
      p2p = m->getInterface(j);
      if (layers.neighborhood->isConnected(j)) {
	i--;
      }
    }
  }
  assert(p2p);
  return p2p;
}

degree_t Utils::broadcast(Layers &layers, Message& msg, P2PNetworkInterface *ignore) {
  BuildingBlock *module = layers.module;
  Neighborhood *neighborhood = layers.neighborhood;
  degree_t sent = 0;
  degree_t pn = module->getNbInterfaces();
  P2PNetworkInterface *p2p = NULL;

  Time sendTime = layers.synchronization->getSendingTime(NULL);
  
  for (degree_t i = 0; i < pn; i++) {
    p2p = module->getInterface(i);
    if (p2p->isConnected() && (p2p != ignore)) {
      Message *message = msg.clone();
      //p2p->send(message);
      p2p->send(sendTime,message);
      //MyMessage::send(layers,p2p,message);
      sent++;
    }
  }
  return sent;
}

degree_t Utils::directBroadcast(Layers &layers, Message& msg, P2PNetworkInterface *ignore, Time t) {
  BuildingBlock *module = layers.module;
  Neighborhood *neighborhood = layers.neighborhood;
  degree_t sent = 0;
  degree_t pn = module->getNbInterfaces();
  P2PNetworkInterface *p2p = NULL;

  for (degree_t i = 0; i < pn; i++) {
    p2p = module->getInterface(i);
    if (p2p->isConnected() && (p2p != ignore)) {
      Message *message = msg.clone();
      p2p->send(t,message);
      sent++;
    }
  }
  return sent;
}

/*degree_t Utils::broadcast(Layers &layers, Message& msg, P2PNetworkInterface *ignore, Time time) {
  BuildingBlock *module = layers.module;
  Neighborhood *neighborhood = layers.neighborhood;
  degree_t sent = 0;
  degree_t pn = module->getNbInterfaces();
  P2PNetworkInterface *p2p = NULL;
  for (degree_t i = 0; i < pn; i++) {
    p2p = module->getInterface(i);
    if (p2p->isConnected() && (p2p != ignore)) {
      Message *message = msg.clone();
      p2p->send(time,message);
      sent++;
    }
  }
  return sent;
  }*/

uint Utils::getRandom(Layers &layers, uint min, uint max) {
  return layers.module->getRandomUint()%(max-min) + min;
}
