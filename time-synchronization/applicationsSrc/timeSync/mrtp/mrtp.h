#ifndef MRTP_H_
#define MRTP_H_

#include <fstream>
#include <string>

#include "random.h"

#include "../synchronization/synchronization.h"
#include "../utils/def.h"

#include "../spanningTree/bfsTree.h"

#include "../convergecastTimeMax/ctm.h"

#include "../simulation.h"

class Layers;

namespace Synchronization {

  class MRTPNode: public Node {
  public:
    
    static MRTPNode* timeMasterMRTPNode;

    // spanning tree
    BFSTraversalSP spanningTree;
    BFSTraversalSPAlgorithm spanningTreeConstructor;
    
    // convergescast
    ConvergecastTimeMaxAlgorithm convergecastTimeMax;

    // Stats
    std::ofstream output;
    
    MRTPNode(Layers &l);
    MRTPNode(const MRTPNode &n);
    ~MRTPNode();

    void startup();
    void handle(MessagePtr m);
    void handle(EventPtr e);

    void scheduleSynchronizeChildren();
    void synchronizeChildren();

    void handleRoundTripSync(MessagePtr m);

    void exportStat(Time receptionSimTime,
		    Time receptionLocalTime,
		    Time receptionEstimatedGlobalTime);

    //Time simTimeNextBlockTick(Time localTime);
    Time simTimeNextBlockTick1(Time localTime);
    
  };
  
  
}

#endif
