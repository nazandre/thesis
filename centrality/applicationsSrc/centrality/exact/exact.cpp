/*
 * exact.cpp
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>

#include "scheduler.h"
#include "network.h"
#include "trace.h"

#include "exact.h"
#include "events.h"
#include "messages.h"

#include "../simulation.h"

using namespace std;
using namespace BaseSimulator;

// 50ms
#define ELECTION_TIMEOUT (5*10*1000)

#define FAST_COMP_ATTEMPT

Exact::Exact(BuildingBlock *host): ElectionAlgorithm(host) {
  electionTimeOutEvent = NULL;

  farness = 0;
  eccentricity = 0;
  eccHasChanged = false;
  electedTime = 0;
}

Exact::~Exact(){
  unsigned int value = 0;
  bool elected = isLocallyElected(value);

  if (elected) { // LEADER!
    win(false);
    cerr << "@" << module->blockId
	 << ": elected at time " << electedTime
	 << " with value = " << value
	 << endl;
    cerr << "\n\n";
  }
  
}

size_t Exact::size() {
  degree_t d = module->getNbInterfaces();
  size_t s = distances.size() * sizeof(distance_t) + d * sizeof(longDistance_t) + sizeof(longDistance_t)
    // timeout
    + sizeof(myTime_t) // date
    + sizeof(bbPointer_t) // handler pointer
    ;
  return s;
}

void Exact::init(){
  distances.clear();
  distances.resize(World::getWorld()->getSize(), 0);
  sums.clear();
	
  if (module->getNbNeighbors() == 0) {
    tryToElect();
  } else {
    broadcastDistanceMessage(module->blockId, 1, NULL);
    electionTimeOutEvent =
      new ElectionTimeOutEvent(getScheduler()->now() + ELECTION_TIMEOUT,
			       module);
    getScheduler()->schedule(electionTimeOutEvent);
  }
}

void Exact::start() {

}

void Exact::handle(MessagePtr m) {
  switch(m->type) {
  case DISTANCE_MESSAGE: {
    DistanceMsg_ptr recv = std::static_pointer_cast<DistanceMsg>(m);
    if (recv->id != (mID_t) module->blockId) {
      if ((distances[recv->id-1] == 0) || (recv->distance < distances[recv->id-1])) {

	farness -= distances[recv->id-1];
	farness += recv->distance;

	if(eccentricity == distances[recv->id-1]) {
	  eccHasChanged = true;
	}

	eccentricity = std::max((distance_t)recv->distance,eccentricity);
	
	distances[recv->id-1] = recv->distance;
	broadcastDistanceMessage(recv->id, recv->distance + 1,
				 recv->destinationInterface);

	if (electionTimeOutEvent != NULL) {
	  electionTimeOutEvent->cancel();
	}
	electionTimeOutEvent =
	  new ElectionTimeOutEvent(getScheduler()->now() + ELECTION_TIMEOUT,
				   module);
	getScheduler()->schedule(electionTimeOutEvent);
      }
    }
    break;
  }
  case SUM_REQUEST_MESSAGE: {
    /*unsigned int sum = 0;
					
      for (int i = 0; i < distances.size(); i++) {
      if ((distances[i] == 0) && (bb->blockId != i)) {
      cerr << "Warning Barycentric Leader Election may be wrong..." << endl;
      }
      sum += distances[i];
      }
      BlinkyBlocksLEBarSumInfoMsg *sumMsg = new BlinkyBlocksLEBarSumInfoMsg(sum); 
      getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + processingTime, sumMsg, recv->destinationInterface));*/
    break;
  }
  case SUM_INFO_MESSAGE: {
    SumInfoMsg_ptr recv = std::static_pointer_cast<SumInfoMsg>(m);
    list<pair<mID_t, longDistance_t> >::iterator it;
    // cheating, ID should be sent! (lazy to change the code)
    mID_t id = m->sourceInterface->hostBlock->blockId;
					
    for ( it = sums.begin(); it != sums.end(); it++) {
      if (id  == it->first) {
	it->second = recv->sum;
      }
    }
    if (it == sums.end()) {
      sums.push_back(make_pair(id ,recv->sum));
    }
    tryToElect();
    break;
  }
  }
}

void Exact::handle(EventPtr e) {
}

degree_t Exact::broadcastDistanceMessage(mID_t id,
					     longDistance_t distance,
					     P2PNetworkInterface *excluded) {
  P2PNetworkInterface *p = NULL;
  degree_t sent = 0;
	
  for (degree_t i=0; i<module->getNbInterfaces(); i++) {
    p = module->getInterface(i);
    if (!p->isConnected() || p == excluded) {
      continue;
    }
    Message *m = new DistanceMsg(id, distance);
    p->send(m);
    sent++;
  }
  return sent;
}

void Exact::electionTimeOut() {
  // Assume that the timeout event has not been cancelled
  P2PNetworkInterface *p = NULL;
  longDistance_t v = 0;

  getCentralityValue(v);
  
  for (degree_t i = 0; i<module->getNbInterfaces(); i++) {
    p = module->getInterface(i);
    if (p->isConnected()) {
      Message *m = new SumInfoMsg(v);
      p->send(m);
    }
  }
	
  tryToElect();
	
  electionTimeOutEvent = NULL;
}

void Exact::tryToElect() {
  longDistance_t value = 0;
  bool elected = isLocallyElected(value);
  
  if (elected) { // LEADER!
    electedTime = getScheduler()->now();
    cout << "@" << module->blockId
	 << " at time " << electedTime
	 << " elected with value = " << value
	 << endl;
  }
}

void Exact::getCentralityValue(longDistance_t &v) {

  #ifdef FAST_COMP_ATTEMPT

  if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
    v = farness; //getDistanceSum();
  } else if (IS_RUNNING_VERSION(VERSION_CENTER)) {
    if (eccHasChanged) {
      eccHasChanged = false;
      eccentricity = getDistanceMax();
    }
    v = eccentricity; //getDistanceMax();
  }
  
  #else

  if (IS_RUNNING_VERSION(VERSION_CENTROID)) {
    v = getDistanceSum();
  } else if (IS_RUNNING_VERSION(VERSION_CENTER)) {
    v = getDistanceMax();
  }
  
  #endif
}

bool Exact::isLocallyElected(unsigned int &v) {

  getCentralityValue(v);
  
  if (sums.size() >= module->getNbNeighbors()) {
    for (std::list<pair<mID_t, longDistance_t> >::iterator it =
	   sums.begin(); it != sums.end(); it++) {
      if ((v > it->second)
	  || ( (v == it->second) &&
	       ((mID_t)module->blockId > it->first) )) {
	return false;
      }
    }
  }
  
  return true;
}
  
longDistance_t Exact::getDistanceSum() {
  longDistance_t sum = 0;
  
  for (sysSize_t i = 0; i < distances.size(); i++) {
    if ((distances[i] == 0) && ((mID_t) (module->blockId-1) != i)) {
      cerr << "Warning Barycentric Leader Election may be wrong..." << endl;
    }
    sum += distances[i];
  }
  return sum;
}

distance_t Exact::getDistanceMax() {
  distance_t max = 0;
  
  for (sysSize_t i = 0; i < distances.size(); i++) {
    if ((distances[i] == 0) && ((mID_t) (module->blockId-1) != i)) {
      cerr << "Warning Barycentric Leader Election may be wrong..." << endl;
    }
    max = std::max(max,distances[i]);
  }
  return max;
}
