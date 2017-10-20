#ifndef NEIGHBORHOOD_H_
#define NEIGHBORHOOD_H_

#include "network.h"

#include "../utils/def.h"

class Layers;

class Neighborhood {
private:
  enum neighborState_t {DISCONNECTED = 0, CONNECTED = 1};
  Layers &layers;
  std::vector<neighborState_t> states;
  
public:
  Neighborhood(Layers &l);
  Neighborhood(const Neighborhood &n);

  ~Neighborhood();

  void init();
  
  bool isConnected(P2PNetworkInterface *p);
  inline bool isConnected(degree_t i) {return states[i] == CONNECTED;}

  void connect(P2PNetworkInterface *p);
  void disconnect(P2PNetworkInterface *p);
  inline void connect(degree_t i) {states[i] = CONNECTED;}
  inline void disconnect(degree_t i) {states[i] = DISCONNECTED;}
  
  degree_t getNbNeighbors();
};

#endif
