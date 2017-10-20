#include "simpleSingleSP.h"

#include "simple.h"
#include "world.h"

using namespace std;
using namespace BaseSimulator;

#define ROOT_NODE_ID 1

template <class Alg, class Tra>
SimpleSingleSP<Alg,Tra>::SimpleSingleSP(BuildingBlock *m): ElectionAlgorithm(m) {
  algorithm = new Alg(module);
  traversal = new Tra();
}

template <class Alg, class Tra>
size_t SimpleSingleSP<Alg,Tra>::size() {
  size_t s = 0;
  
  s = traversal->size();

  // Only BFS, we do not need the rootID
  s -= sizeof(mID_t);
  
  return s;
}

template <class Alg, class Tra>
SimpleSingleSP<Alg,Tra>::~SimpleSingleSP() {
  delete algorithm;
  delete traversal;
}

template <class Alg, class Tra>
void SimpleSingleSP<Alg,Tra>::init() {
  traversal->tree.rootID = ROOT_NODE_ID;
  algorithm->setSysSize((sysSize_t)getWorld()->getSize());

}

template <class Alg, class Tra>
void SimpleSingleSP<Alg,Tra>::start() {
  if (module->blockId == ROOT_NODE_ID)
    algorithm->start(traversal);
}

template <class Alg, class Tra>
void SimpleSingleSP<Alg,Tra>::handle(MessagePtr m) {
  //assert();
  if (algorithm->hasToHandle(m)) {
    bool converged = algorithm->handle(m,traversal);
    if (converged) {
      win();
      MY_CERR << "SP constructed!" << endl;
      MY_CERR << "Eccentricity = " << traversal->getHeight() << endl;
    }
  }
}

template <class Alg, class Tra>
void SimpleSingleSP<Alg,Tra>::handle(EventPtr e) {}

//The explicit instantiation part
template class SimpleSingleSP<BFSTraversalSPAlgorithm,BFSTraversalSP>; 
template class SimpleSingleSP<AlphaSynchronizedBFSAlgorithm,AlphaSynchronizedBFS::Traversal>;
