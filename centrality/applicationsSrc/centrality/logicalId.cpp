#include "logicalId.h"

#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

LogicalID LogicalID::singleton;

LogicalID::LogicalID() {}

LogicalID::~LogicalID() {}

struct Generator {
  mID_t current;
  std::mt19937 gen;
  mID_t maxStep;
  Generator (mID_t start, BaseSimulator::rint seed, mID_t m): current(start), gen(seed), maxStep(m) {}
  mID_t operator() () {
    mID_t previous = current;
    current = current + (gen()%maxStep) +1 ;
    assert(previous < current);
    return current;
  }
};

void LogicalID::generate(bID size, BaseSimulator::rint seed) {
  cerr << "Generating "
       << size << " logical IDs "
       << "using " << (unsigned int) seed << " seed"
       << endl;

  
  mID_t start = 0;
  mID_t maxStep = (std::numeric_limits<mID_t>::max()-start) / size; 
  Generator g (start,seed,maxStep);

  assert(size < std::numeric_limits<mID_t>::max());
   
  singleton.lIDs = vector<mID_t>(size);

  // Principle of the algorithm:

  // 1: Fill with the result of calling g() repeatedly
  // (i.e., generate strictly increasing thus unique ids with
  // random step between every id.) 
  std::generate(singleton.lIDs.begin(), singleton.lIDs.end(), g); 

  // 2: Shuffle the generated ids
  std::shuffle(begin(singleton.lIDs),
	       end(singleton.lIDs),
	       g.gen);
}

mID_t LogicalID::getID(bID id) {

  assert(id < std::numeric_limits<mID_t>::max());
  
  if (singleton.lIDs.size() == 0) {
    return id;
  } else {
    return singleton.lIDs[id-1];
  }
}
