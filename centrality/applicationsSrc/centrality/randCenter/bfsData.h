#ifndef RAND_CENTER_BFS_DATA
#define RAND_CENTER_BFS_DATA

#include <cassert>

#include "../bfsData.h"
#include "../utils/def.h"

class RandCenterBFSHeader: public BFSData {
public:
  uint8_t iteration;
  
  RandCenterBFSHeader(uint8_t i): iteration(i) {}
  RandCenterBFSHeader(RandCenterBFSHeader &d): BFSData(d), iteration(d.iteration) {}
  ~RandCenterBFSHeader() {}

  RandCenterBFSHeader* clone() { return new RandCenterBFSHeader(*this);}
  unsigned int size() {return sizeof(uint8_t);}
};

class RandCenterBFSBackData: public BFSData {
public:
  distance_t maxRandomNumber;
  longDistance_t minEcc;
  
  RandCenterBFSBackData(distance_t s, longDistance_t m): maxRandomNumber(s), minEcc(m) {}
  RandCenterBFSBackData(RandCenterBFSBackData &d): BFSData(d),
						   maxRandomNumber(d.maxRandomNumber),
						   minEcc(d.minEcc) { assert(false);}
  ~RandCenterBFSBackData() {}
  
  RandCenterBFSBackData* clone() { return new RandCenterBFSBackData(*this);}
  unsigned int size() {return sizeof(distance_t) + sizeof(longDistance_t);}
};

#endif
