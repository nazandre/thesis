#include "ea.h"
#include "scheduler.h"

using namespace std;
using namespace BaseSimulator;

ElectionAlgorithm::ElectionAlgorithm(BuildingBlock *m) {
  module = m;
}

ElectionAlgorithm::ElectionAlgorithm(const ElectionAlgorithm &ea) {
  module = ea.module;
}

ElectionAlgorithm::~ElectionAlgorithm() {

}

void ElectionAlgorithm::win(bool printDate) {
  module->setColor(RED);
  
  MY_CERR <<  "has won the election";

  if (printDate) {
    cerr << " at "
	 << max(getScheduler()->now(),module->blockCode->availabilityDate);
  }
  
  cerr << "!" << endl;
}

size_t ElectionAlgorithm::dataSize() {
  size_t s = 2*sizeof(uint8_t) // leader?, iteration number,
    + sizeof(mID_t) // id node initiates election
    + size(); // algorithm specific data

  return s;
}
