#ifndef RAND_CENTER_2_BFS_DATA_H_
#define RAND_CENTER_2_BFS_DATA_H_

#include <cassert>

#include "../bfsData.h"
#include "../utils/def.h"

class RandCenter2BFSHeader: public BFSData {
public:
  uint8_t iteration;
  
  RandCenter2BFSHeader(uint8_t i): iteration(i) {}
  RandCenter2BFSHeader(RandCenter2BFSHeader &d): BFSData(d), iteration(d.iteration) {}
  ~RandCenter2BFSHeader() {}

  RandCenter2BFSHeader* clone() { return new RandCenter2BFSHeader(*this);}
  unsigned int size() {return sizeof(uint8_t);}
};

class RandCenter2BFSBackData: public BFSData {
public:
  distance_t maxRandomNumber;
  mID_t maxRandomNumberId;
  
  RandCenter2BFSBackData(distance_t m, mID_t i): maxRandomNumber(m), maxRandomNumberId(i) {}
  RandCenter2BFSBackData(RandCenter2BFSBackData &d): BFSData(d),
						   maxRandomNumber(d.maxRandomNumber),
						   maxRandomNumberId(d.maxRandomNumberId) { assert(false);}
  ~RandCenter2BFSBackData() {}
  
  RandCenter2BFSBackData* clone() { return new RandCenter2BFSBackData(*this);}
  unsigned int size() {return sizeof(distance_t) + sizeof(mID_t);}
};

#endif
