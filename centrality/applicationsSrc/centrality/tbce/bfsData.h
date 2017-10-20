#ifndef TBCE_BFS_DATA_H_
#define TBCE_BFS_DATA_H_

#include "../bfsData.h"
#include "../utils/def.h"

class TBCEBFSBackData: public BFSData {
public:
  longDistance_t farness;
  
  TBCEBFSBackData(longDistance_t f): farness(f) {}
  
  TBCEBFSBackData(TBCEBFSBackData &d): BFSData(d),
						 farness(d.farness) {}
  ~TBCEBFSBackData() {}
  
  TBCEBFSBackData* clone() { return new TBCEBFSBackData(*this);}
  
  unsigned int size() {
    return sizeof(longDistance_t);
  }
};

#endif
