#ifndef NEIGHBOR_H_
#define NEIGHBOR_H_

#include <vector>

#include "buildingBlock.h"
#include "network.h"

class Neighbor {
public:
  P2PNetworkInterface *interface;
  
  Neighbor(P2PNetworkInterface *i);
  Neighbor(const Neighbor &n);
  virtual ~Neighbor();

  virtual size_t size();
};

/*
class Neighborhood {
public:
  std::vector<Neighbor> neighbors;

  Neighborhood();
  Neighborhood(const Neighborhood& n);
  ~Neighborhood();
  
  void update();
  
  Neighbor* get(P2PNetworkInterface *p);
  
  };*/

#endif
