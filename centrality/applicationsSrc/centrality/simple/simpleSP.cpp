#include "simpleSP.h"

using namespace std;
using namespace BaseSimulator;

SimpleSPEA::SimpleSPEA(BuildingBlock *m) : ElectionAlgorithm(m) {
#ifdef ALPHA_SYNCHRONIZED_BFS
  algorithm = new AlphaSynchronizedBFSAlgorithm(m);
  traversal = new AlphaSynchronizedBFS::Traversal();
  traversal->setElection(true);
#else
  algorithm = new InitiatorElectionSP(m);
#endif
}

size_t SimpleSPEA::size() {
#ifdef ALPHA_SYNCHRONIZED_BFS
  return traversal->size();
#else
  return algorithm->traversal.size();
#endif
}

SimpleSPEA::~SimpleSPEA() {

}

void SimpleSPEA::init() {
  
}

void SimpleSPEA::start() {
#ifdef ALPHA_SYNCHRONIZED_BFS
  algorithm->start(traversal);
#else
  algorithm->start();
#endif
}

void SimpleSPEA::handle(MessagePtr m) {
  //assert();
#ifdef ALPHA_SYNCHRONIZED_BFS
  bool converged = algorithm->handle(m,traversal);
#else
  bool converged = algorithm->handle(m);
#endif
  
  if (converged) {
    win();
  }
}

void SimpleSPEA::handle(EventPtr e) {

}
