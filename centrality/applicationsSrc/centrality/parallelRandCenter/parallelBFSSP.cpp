#include "parallelBFSSP.h"

ParallelBFSTraversalSPKey::ParallelBFSTraversalSPKey(mID_t i, distance_t r) {
  id = i;
  randomNumber = r;
}

ParallelBFSTraversalSPKey::ParallelBFSTraversalSPKey(const ParallelBFSTraversalSPKey &k) {
  id = k.id;
  randomNumber = k.randomNumber;
}

ParallelBFSTraversalSPKey::~ParallelBFSTraversalSPKey() {}

bool ParallelBFSTraversalSPKey::operator<(const ParallelBFSTraversalSPKey& other) const {
  return (randomNumber < other.randomNumber ||
	  (randomNumber == other.randomNumber && id < other.id));
}

ParallelBFSTraversalSP::ParallelBFSTraversalSP(): BFSTraversalSP() {
}

ParallelBFSTraversalSP::ParallelBFSTraversalSP(const ParallelBFSTraversalSP &p): BFSTraversalSP(p) {
}

ParallelBFSTraversalSP::~ParallelBFSTraversalSP() {}
