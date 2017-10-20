#ifndef EXTREMUM_CENTER_BFS_DATA
#define EXTREMUM_CENTER_BFS_DATA

#include "../bfsData.h"
#include "../utils/def.h"

class ExtremumCenterBFSHeader: public BFSData {
public:
  uint8_t iteration;
  
  ExtremumCenterBFSHeader(uint8_t i): iteration(i) {}
  ExtremumCenterBFSHeader(ExtremumCenterBFSHeader &d): BFSData(d), iteration(d.iteration) {}
  ~ExtremumCenterBFSHeader() {}

  ExtremumCenterBFSHeader* clone() { return new ExtremumCenterBFSHeader(*this);}
  unsigned int size() {return sizeof(uint8_t);}
};

class ExtremumCenterBFSBackData: public BFSData {
public:
  longDistance_t maxSum;
  longDistance_t minEcc;
  
  ExtremumCenterBFSBackData(longDistance_t s, longDistance_t m): maxSum(s), minEcc(m) {}
  ExtremumCenterBFSBackData(ExtremumCenterBFSBackData &d): BFSData(d),
							   maxSum(d.maxSum),
							   minEcc(d.minEcc) {}
  ~ExtremumCenterBFSBackData() {}
  
  ExtremumCenterBFSBackData* clone() { return new ExtremumCenterBFSBackData(*this);}
  unsigned int size() {
    return sizeof(longDistance_t)
      + sizeof(longDistance_t);}
};

#endif
