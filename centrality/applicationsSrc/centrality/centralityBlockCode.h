#ifndef CENTRALITYBLOCKCODE_H_
#define CENTRALITYBLOCKCODE_H_

#include "blockCode.h"
#include "simulator.h"

#include "ea.h" // ElectionAlgorithm

class CentralityBlockCode : public BlockCode {
 public:  
  static ModuleType moduleType;
  unsigned int nbNeighbors;

  std::vector<uint64_t> idPrevMsg;
  
  CentralityBlockCode(BaseSimulator::BuildingBlock *host);
  ~CentralityBlockCode();
  
  ElectionAlgorithm *algorithm;
  
  void startup();
  void init();
  
  void processLocalEvent(EventPtr pev);
  static BlockCode *buildNewBlockCode(BuildingBlock *host);
  
};

#endif /* CENTRALITYBLOCKCODE_H_ */
