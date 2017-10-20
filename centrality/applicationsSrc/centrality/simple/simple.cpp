#include "simple.h"

using namespace std;
using namespace BaseSimulator;

SimpleEA::SimpleEA(BuildingBlock *m, bool t) : ElectionAlgorithm(m) {
  algorithm = new InitiatorElection(m);
  traversal = NULL;
  
  if (t) {
    traversalAlgorithm = new BFSTraversalSPAlgorithm(module);
  } else {
    traversalAlgorithm = NULL;
  }
}

size_t SimpleEA::size() {
  size_t s = 0;

  if (traversal) {
    s = traversal->size();
  }  else {
    s = algorithm->traversal.size();
    // Only leader election, do not need the distance to the root
    s -= sizeof(distance_t);
  }  
  return s;
}

SimpleEA::~SimpleEA() {
  delete algorithm;
  delete traversalAlgorithm;
  delete traversal;
}

void SimpleEA::init() {

}

void SimpleEA::start() {
  algorithm->start();
}

void SimpleEA::handle(MessagePtr m) {
  //assert();
  if (algorithm->hasToHandle(m)) {
    bool converged = algorithm->handle(m);
    if (converged) {
      win();
      if (traversalAlgorithm) {
	traversal = traversalAlgorithm->create();
	traversalAlgorithm->start(traversal);
	MY_CERR << "SP construction started!" << endl;
      }
    }
  } else if (traversalAlgorithm && traversalAlgorithm->hasToHandle(m)) {
    if (!traversal) {
      traversal = traversalAlgorithm->create(m);
    }
    bool converged = traversalAlgorithm->handle(m,traversal);
    if (converged) {
      MY_CERR << "SP constructed!" << endl;
      MY_CERR << "Eccentricity = " << traversal->getHeight() << endl;
    }
  }
}

void SimpleEA::handle(EventPtr e) {

}
