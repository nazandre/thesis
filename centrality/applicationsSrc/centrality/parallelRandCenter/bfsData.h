#ifndef PARALLEL_RAND_CENTER_BFS_DATA_H_
#define PARALLEL_RAND_CENTER_BFS_DATA_H_

#include <cassert>

#include "parallelBFSSP.h"

#include "../bfsData.h"
#include "../utils/def.h"

class ParallelRandCenterBFSHeader: public BFSData {
public:
  ParallelBFSTraversalSPKey key;
  
  ParallelRandCenterBFSHeader(ParallelBFSTraversalSPKey& k): key(k) {}
  ParallelRandCenterBFSHeader(ParallelRandCenterBFSHeader &d): BFSData(d), key(d.key) {}
  ~ParallelRandCenterBFSHeader() {}

  ParallelRandCenterBFSHeader* clone() { return new ParallelRandCenterBFSHeader(*this);}
  unsigned int size() {return sizeof(ParallelBFSTraversalSPKey);}
};
#endif
