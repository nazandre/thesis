#ifndef ABCCENTERV2_BFS_DATA
#define ABCCENTERV2_BFS_DATA

#include "../bfsData.h"
#include "../utils/def.h"

class ABCCenterV2BFSHeader: public BFSData {
public:
  uint8_t iteration;
  
  ABCCenterV2BFSHeader(uint8_t i): iteration(i) {}
  ABCCenterV2BFSHeader(ABCCenterV2BFSHeader &d): BFSData(d), iteration(d.iteration) {}
  ~ABCCenterV2BFSHeader() {}
  
  ABCCenterV2BFSHeader* clone() {return new ABCCenterV2BFSHeader(*this);}
  unsigned int size() {return sizeof(uint8_t);}
};

class ABCCenterV2BFSBackData: public BFSData {
public:
  distance_t minCandidateGradient;
  distance_t maxCandidateDistance;
  uint8_t numCandidates;
  
  ABCCenterV2BFSBackData(distance_t g, distance_t f, uint8_t c): minCandidateGradient(g), maxCandidateDistance(f), numCandidates(c) {}
  ABCCenterV2BFSBackData(ABCCenterV2BFSBackData &d): BFSData(d),
						     minCandidateGradient(d.minCandidateGradient),
						     maxCandidateDistance(d.maxCandidateDistance),
						     numCandidates(d.numCandidates) {}
  ~ABCCenterV2BFSBackData() {}

  
  ABCCenterV2BFSBackData* clone() {return new ABCCenterV2BFSBackData(*this);}
  unsigned int size() {return 2*sizeof(distance_t) + sizeof(uint8_t);}
};

class ABCCenterV2ConvergecastGoHeader: public BFSData {
public:
  distance_t minCandidateGradient;
  
  ABCCenterV2ConvergecastGoHeader(distance_t g): minCandidateGradient(g) {};
  ABCCenterV2ConvergecastGoHeader(ABCCenterV2ConvergecastGoHeader &d): BFSData(d),
								       minCandidateGradient(d.minCandidateGradient) {}
  ~ABCCenterV2ConvergecastGoHeader() {};
  
  ABCCenterV2ConvergecastGoHeader* clone(){return new ABCCenterV2ConvergecastGoHeader(*this);}
  unsigned int size() {return sizeof(distance_t);}
};

class ABCCenterV2ConvergecastBackData: public BFSData {
public:
  distance_t minCandidateGradient;
  distance_t maxCandidateDistance;
  uint8_t numCandidates;
  
  ABCCenterV2ConvergecastBackData(distance_t g, distance_t f, uint8_t c): minCandidateGradient(g), maxCandidateDistance(f), numCandidates(c) {}
  ABCCenterV2ConvergecastBackData(ABCCenterV2ConvergecastBackData &d): BFSData(d),
						     minCandidateGradient(d.minCandidateGradient),
						     maxCandidateDistance(d.maxCandidateDistance),
						     numCandidates(d.numCandidates) {}
  ~ABCCenterV2ConvergecastBackData() {}

  ABCCenterV2ConvergecastBackData* clone() { return new ABCCenterV2ConvergecastBackData(*this); }
  unsigned int size() {return 2*sizeof(distance_t) + sizeof(uint8_t);}
};

#endif
