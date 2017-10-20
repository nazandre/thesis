/*
 * exact.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef EXACT_H_
#define EXACT_H_

#include <vector>
#include <list>

#include "../ea.h"

class ElectionTimeOutEvent;

class Exact : public ElectionAlgorithm {
  
 public:
  Exact(BaseSimulator::BuildingBlock *host);
  ~Exact();

  size_t size();
  
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);

  vector<distance_t> distances;
  list<pair<mID_t, longDistance_t > > sums; // neighbor values (sum or ecc)

  distance_t eccentricity;
  bool eccHasChanged;
  longDistance_t farness;
  
  ElectionTimeOutEvent *electionTimeOutEvent;
  Time electedTime;
  
  degree_t broadcastDistanceMessage(mID_t id, longDistance_t distance,
					P2PNetworkInterface *excluded);

  void electionTimeOut();
  void tryToElect();

  void getCentralityValue(longDistance_t &v);

  bool belongsTo(list<P2PNetworkInterface*> l, P2PNetworkInterface *p);
  longDistance_t getDistanceSum();
  distance_t getDistanceMax();
  bool isLocallyElected(unsigned int &v);
};

#endif /* EXACT_H_ */
