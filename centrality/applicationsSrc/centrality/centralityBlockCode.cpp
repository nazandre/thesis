
/*
 * @file CentralityBlockCode.cpp
 * @brief centrality-based leader election algorithm
 * @autor Andre Naz
 * @date 30/08/2016
 */

#include <iostream>
#include <sstream>

#include "scheduler.h"
#include "network.h"
#include "lattice.h"
#include "trace.h"

#include "centralityBlockCode.h"
#include "eaMsg.h"

#include "e2ace/e2ace.h"
#include "tbce/tbce.h"
#include "exact/exact.h"
#include "abcCenterV1/abcCenterV1.h"
#include "abcCenterV2/abcCenterV2.h"
#include "simple/simple.h"
#include "simple/simpleSP.h"
#include "extremumCenter/extremumCenter.h"
#include "randCenter/randCenter.h"
#include "dee/dee.h"
#include "randCenter2/randCenter2.h"
#include "parallelRandCenter/parallelRandCenter.h"
#include "simple/simpleSingleSP.h"

#include "simulation.h"
#include "logicalId.h"
#include "initiatorMsg.h"

#include "bfsTree.h"
#include "alphaSynchronizedBFS.h"

//#define PRINT_NEIGHBORS

using namespace std;

ModuleType CentralityBlockCode::moduleType;

CentralityBlockCode::CentralityBlockCode(BuildingBlock *host): BlockCode(host) {
  int d = BaseSimulator::getWorld()->lattice->getMaxNumNeighbors();

  nbNeighbors = 0;
  algorithm = NULL;
  idPrevMsg.resize(d,0);
  
  switch(THIS_ALGORITHM) {    
  case ALGORITHM(MINIMUM_ID):
    algorithm = new SimpleEA(host,false);
    break;
  case ALGORITHM(MINIMUM_ID_THEN_SP_BFS):
    algorithm = new SimpleEA(host,true);
    break;
  case ALGORITHM(SHORTEST_PATH_MINIMUM_ID):
    algorithm = new SimpleSPEA(host);
    break;
  case ALGORITHM(SINGLE_SHORTEST_PATH_BFS):
    algorithm = new SimpleSingleSP<BFSTraversalSPAlgorithm,BFSTraversalSP>(host);
    break;
  case ALGORITHM(ALPHA_SYNCHRONIZED_SP_BFS):
    algorithm = new SimpleSingleSP<AlphaSynchronizedBFSAlgorithm,
				   AlphaSynchronizedBFS::Traversal>(host);
      break;
  case ALGORITHM(ABC_CENTER_V1):
    algorithm = new ABCCenterV1(host);
    break;
  case ALGORITHM(ABC_CENTER_V2):
    algorithm = new ABCCenterV2(host);
    break;
  case ALGORITHM(E2ACE):
    algorithm = new E2ACE(host);
    break;
  case ALGORITHM(TBCE):
    algorithm = new TBCE(host);
    break;
  case ALGORITHM(EXACT):
    algorithm = new Exact(host);
    break;
  case ALGORITHM(EXTREMUM_CENTER):
    algorithm = new ExtremumCenter(host);
    break;
  case ALGORITHM(RAND_CENTER):
    algorithm = new RandCenter(host);
    break;
  case ALGORITHM(DEE):
    algorithm = new DEE(host);
    break;
  case ALGORITHM(RAND_CENTER_2):
    algorithm = new RandCenter2(host);
    break;
  case ALGORITHM(PARALLEL_RAND_CENTER):
    algorithm = new ParallelRandCenter(host);
    break;
  case ALGORITHM(KORACH):
  default:
    assert(false);
  }
}

CentralityBlockCode::~CentralityBlockCode() {
  OUTPUT << "CentralityBlockCode destructor" << endl;
  delete algorithm;
}

void CentralityBlockCode::init() {
  static bool logicalIdInitialized = false;
  static bool memoryUsagePrinted = false;
  stringstream info;
 
  // Random IDs if required
  if (IS_RANDOM_LOGICAL_IDS_ENABLED && !logicalIdInitialized) {
    unsigned int size = getWorld()->getSize();
    BaseSimulator::rint seed = hostBlock->getRandomUint();
    LogicalID::generate(size,seed);
    logicalIdInitialized = true;
  }

#ifdef PRINT_NEIGHBORS
  cerr << "@" << hostBlock->blockId << ": neighbors = ";
  hostBlock->printNeighborIds();
#endif
  
  nbNeighbors = hostBlock->getNbNeighbors();
  
  // Set network interface data bitrate
  EAMessage::setBitRate(hostBlock);

  addAvailabilityDelay(hostBlock->getRandomUint()%MS_TO_US(20));
  
  // Initialize algorithm
  algorithm->init(); // should instantiante and create all needed
                     // spanning-trees!

  if(!memoryUsagePrinted) {
    cerr << "Algorithm-specific variable memory usage per node: " << algorithm->dataSize() << " bytes." << endl;
    memoryUsagePrinted = true;
  }
}

void CentralityBlockCode::startup() {
  stringstream info;
  info << "  Starting CentralityBlockCode in block " << hostBlock->blockId;
  getScheduler()->trace(info.str(),hostBlock->blockId);

  init();
  algorithm->start();
  
  /* if (hostBlock->blockId == 1) {
  for (degree_t i = 0; i < hostBlock->getNbInterfaces(); i++) {
    P2PNetworkInterface *p = hostBlock->getInterface(i);
    if (p->isConnected()) {
      Message *m = new IEGoMsg(i);
      p->send(m);
    }
  }
  }*/

  /*
  if (hostBlock->blockId == 1)
    hostBlock->setColor(BROWN);
  
  if (hostBlock->blockId == 148)
    hostBlock->setColor(BLUE);
  */
}

void CentralityBlockCode::processLocalEvent(EventPtr pev) {
  stringstream info;
	
  info.str("");

  OUTPUT << hostBlock->blockId << " processLocalEvent: date: "
	 << BaseSimulator::getScheduler()->now()
	 << " process event " << pev->getEventName()
	 << "(" << pev->eventType << ")" << endl;

  switch (pev->eventType) {
  case EVENT_STOP:
    {
      info << " stopped";
    }
    break;
  case EVENT_ADD_NEIGHBOR:
    {
    }
    break;
  case EVENT_REMOVE_NEIGHBOR:
    {
    }
    break;
  case EVENT_RECEIVE_MESSAGE: /*EVENT_NI_RECEIVE: */
    {
      MessagePtr mes = (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;
      EAMessagePtr eaMes = std::static_pointer_cast<EAMessage>(mes);

      P2PNetworkInterface *from = mes->destinationInterface;
      degree_t fromIndex = hostBlock->getDirection(from);
      uint64_t idPrev = idPrevMsg[fromIndex];

      // assert fifo channel!
      if (mes->id != 0 && idPrev >= mes->id) {
        cerr << "@" << hostBlock->blockId << ": id prev=" << idPrev << " >= " << mes->id << endl;
	assert(false);
      }
      idPrevMsg[fromIndex] = mes->id;

      addAvailabilityDelay(eaMes->getProcessingDelays());
      algorithm->handle(mes);
    }
    break;
  default: {
    ERRPUT << "*** ERROR *** : unknown local event";
  }
    break;
  }
	
  if(info.str() != "") {
    getScheduler()->trace(info.str(),hostBlock->blockId);
  }
}

BlockCode* CentralityBlockCode::buildNewBlockCode(BuildingBlock *host) {
  return(new CentralityBlockCode(host));
}
