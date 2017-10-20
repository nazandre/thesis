#ifndef PARALLEL_BFS_SP_H_
#define PARALLEL_BFS_SP_H_

#include "../bfsTree.h"
#include "../utils/def.h"

class ParallelBFSTraversalSPKey {
public:
  mID_t id;
  distance_t randomNumber;
  
  ParallelBFSTraversalSPKey(mID_t i, distance_t r);
  ParallelBFSTraversalSPKey(const ParallelBFSTraversalSPKey &k);
  ~ParallelBFSTraversalSPKey();
  bool operator<(const ParallelBFSTraversalSPKey& other) const; 
};

class ParallelBFSTraversalSP: public BFSTraversalSP {
public:
  
  ParallelBFSTraversalSP();
  ParallelBFSTraversalSP(const ParallelBFSTraversalSP &p);
  ~ParallelBFSTraversalSP();
};

#endif
