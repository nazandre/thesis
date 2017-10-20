/*
 * abcCenterV1.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef ABCCENTER_V1_H_
#define ABCCENTER_V1_H_

#include "../ea.h" // ElectionAlgorithm

class ABCCenterV1 : public ElectionAlgorithm {
  
 public:

  ABCCenterV1(BaseSimulator::BuildingBlock *host);
  ~ABCCenterV1();
  
  unsigned int gradient;
  unsigned int gradient2;
  unsigned int step;
  unsigned int numBlock;
  unsigned int numBlockPreviousStep;
  unsigned int distance;
  unsigned int distFromA;
  unsigned int distFromB;
  unsigned int distFromC;
  bool candidate;
  unsigned int prev_gradient;
  unsigned int remainingId[3];
  unsigned int role;
  unsigned int role_election;
  bool application_broadcasted;
  bool nextstep;
  unsigned int A;
  unsigned int prev_A;
  bool leader;
  bool elected;
	
  bool electing;
  
  bool *waiting;
  bool *children;
  
  // MIN ID:
  unsigned int minId;
  unsigned int minGradient;
  unsigned int minGradient2;
  unsigned int maxDistance;
  P2PNetworkInterface *parent;

  size_t size();
  
  // ElectionAlgorithm Abstract Methods
  void init();
  void start();
  void handle(MessagePtr m);
  void handle(EventPtr e);
  
  int launchAlgo(unsigned int vg, unsigned int v, unsigned int v2);
  void initAlgoVariable(unsigned int re, unsigned int s, unsigned int vg);
  int cmp(unsigned int id, unsigned int v, unsigned int v2);
	
  void addIdIfNotInRemaining(unsigned int id);
  unsigned int numDifferentId();
  bool isIdIn(unsigned int id);
  void printRemainingIds();
  
  string getRoleString(unsigned int r);
  string getMsgEvalString(int r);
  
  unsigned int ST_launch_go_2(P2PNetworkInterface *p, unsigned int s, unsigned int r, unsigned int vg, unsigned int v, unsigned int v2, unsigned int id, unsigned int d);
  void ST_send_back_2(P2PNetworkInterface *p, unsigned int s, unsigned int r, unsigned int v, unsigned int v2, unsigned int id, unsigned int d, unsigned int *rid);
  void ST_send_delete_2(P2PNetworkInterface *p, unsigned int s, unsigned int r, unsigned int v, unsigned int v2, unsigned int id, unsigned int d, unsigned int *rid);
  bool NoOneToWaitFor();
  void setColorPerValue(unsigned int v);
  unsigned int stepConsistency(unsigned int re, unsigned int s, unsigned int recvre, unsigned int recvs);
  string getConsistencyString(unsigned int c);
  void spreadNextStepMsg(unsigned int s);
  void spreadLeaderElectedMsg(unsigned int id);	
  void ST_send_elect(P2PNetworkInterface *p, unsigned int r);

};

#endif /* ABCCENTER_V1_H_ */
