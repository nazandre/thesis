/*
 * extremumCenter.h
 *
 *  Created on: 09/09/2016
 *      Author: Andre Naz
 */

#ifndef ABCCENTERV2_H_
#define ABCCENTERV2_H_

#include "../ea.h" // ElectionAlgorithm
#include "../initiator.h" // InitiatorElectionSP algorithm
#include "../bfsTreeMsg.h"

#include <vector>

enum role_t {
  A = 0,
  B,
  C,
  center
};

struct distancesBC_t{
  distance_t B = 0;
  distance_t C = 0;
};

//typedef distance_t gradient_t;

class ABCCenterV2 : public ElectionAlgorithm {
  
 public:
  InitiatorElectionSP *initiatorElection;
  BFSTraversalSP *bfs;
  BFSTraversalSPAlgorithm *bfsAlg;

  TreeConvergecastAlgorithm* convergecastAlg;
  TreeConvergecast* convergecast;
  
  uint8_t iterations;
  bool candidate;
  distancesBC_t distancesBC;
  
  std::vector<distance_t> childrenMaxCandidateDistance;
  std::vector<distance_t> childrenMinCandidateGradient;
  std::vector<distance_t> childrenNumCandidates;
  //std::vector<distance_t> childrenGradient2;

  ABCCenterV2(BaseSimulator::BuildingBlock *host);
  ~ABCCenterV2();

  size_t size();
  
  // ElectionAlgorithm Abstract Methods
  void reset();
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);
  
  // ABCCenterV2 specific methods
  void resetHandler();
  void addChildHandler(MessagePtr m);
  void removeChildHandler(MessagePtr m);
  BFSData* getBFSDataHeader();
  BFSData* getBFSDataBack();

  void convergecastGoHandler(MessagePtr m);
  void convergecastBackHandler(MessagePtr m);
  BFSData* getConvergecastDataHeader();
  BFSData* getConvergecastDataBack();
  
  void initFirstIteration();
  void setHandlers();
  void initNextIteration(mID_t rootID);

  void forwardNext();
  void forwardCenter();
  void forwardCloseCandidate();
  void forwardFarthest(vector<distance_t>& distances);
  void forwardFarthestCandidate();
  
  uint8_t getNumStep(uint8_t i);
  role_t getRole(uint8_t i);
  
  distance_t getGradient();
  distance_t getMinGradient();
  uint8_t getNumCandidates();

  std::string toString(role_t r);
  std::string getProgressionString(uint8_t i);
};

#endif /* ABCCENTERV2_H_ */
