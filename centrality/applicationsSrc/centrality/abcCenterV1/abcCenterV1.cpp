/*
 * abcCenterV1.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#include <iostream>
#include <sstream>
#include <climits>

#include "scheduler.h"
#include "network.h"
#include "trace.h"

#include "messages.h"
#include "abcCenterV1.h"

//#define INHIBIT_ALGO

#define NOT_AN_ID 0
#define NB_ID_ALIVE 2

//#define SPREAD_LEADER_ELECTED_MSG

using namespace std;
using namespace BaseSimulator;

ABCCenterV1::ABCCenterV1(BuildingBlock *host): ElectionAlgorithm(host) {
  distance = 0;
  gradient = 0;
  gradient2 = 0;
  leader = false;

  waiting = new bool[module->getNbInterfaces()];
  children = new bool[module->getNbInterfaces()];
}

ABCCenterV1::~ABCCenterV1() {
  delete[] waiting;
  delete[] children;
}

size_t ABCCenterV1::size() {
  degree_t delta = module->getNbInterfaces();
  size_t s = 3*sizeof(distance_t) // gradient1, gradient2, prevGradient
    + 3*sizeof(degree_t) // step + numBlocks
    + 4*sizeof(distance_t) // distances: distance, A, B, C
    + sizeof(bool) // candidate
    + 3*sizeof(mID_t) // remainingIDs
    + 2*sizeof(uint8_t) // role + role_election
    + 2 * delta * sizeof(bool) // waiting + children
    + sizeof(mID_t) // min id
    + 3*sizeof(distance_t) // minGradient1,2 + maxDistance
    + sizeof(degree_t) // parent
    ;
  return s;
}

void ABCCenterV1::init() {}

void ABCCenterV1::initAlgoVariable(unsigned int re, unsigned int s, unsigned int vg) {
  // Algo variable init:
  //bool was_leader = leader;
  electing = false;
  minGradient = UINT_MAX;
  minGradient2 = UINT_MAX;
  minId = UINT_MAX;
  maxDistance = 0;
  parent = NULL;
  role = ROLE_N;
  application_broadcasted = false;
  step = s;
  role_election = re;
  //leader = false;
  switch(role_election) {
  case ROLE_A:
  case ROLE_N:
    nextstep = true;
    if (step == 1) {
      elected = false;				
      candidate = true;
      prev_gradient = 0;
      gradient = 0;
      //gradient2 = 0;
      //module->setColor(0);
				
      //cout << "reset prev_grad" << endl;
    } else {
      prev_gradient = gradient;
      candidate = candidate && (prev_gradient <= vg);
      //cout << "gradient: " << gradient << endl;
      //cout << "prev_gradient : " << prev_gradient << endl;
      //candidate = (prev_gradient <= vg);
    }
    if (candidate) {
      minId = module->blockId;
      minGradient = gradient;
      minGradient2 = gradient2;
    }
    distFromB = 0;
    distFromC = 0;
    //distFromA = 0;
    gradient = 0;
    //gradient2 = 0;
    break;
  case ROLE_B:
  case ROLE_C:
    nextstep = false;
    if (candidate) {
      minId = module->blockId;
      maxDistance = distance; // dist from A or B
    }
    break;
  }
	
  distance = 0;
	
  for (int i = 0; i <  NB_ID_ALIVE; i++) {
    remainingId[i] = NOT_AN_ID;
  }
	
  for (int i = 0; i < module->getNbInterfaces(); i++) {
    waiting[i] = false;
    children[i] = false;
  }
	
  //cout << "@" << module->blockId << " re-init: role_election: " << getRoleString(role_election) << " step " << step <<  " gradient: " << gradient << " prev_gradient " << prev_gradient << endl;
  //module->blockCode->addAvailabilityDelay(50);
}

int ABCCenterV1::launchAlgo(unsigned int vg, unsigned int v,unsigned int v2) {
  ST_launch_go_2(NULL, step, role_election, vg, v, v2, minId, distance);
  if(NoOneToWaitFor()) {
    return 0;
  } else {
    return 1;
  }
}

void ABCCenterV1::start() {	
  initAlgoVariable(ROLE_A, 1, 0);
  application_broadcasted = true;	
  if (!launchAlgo(0,0,0)) {
    elected = true;
    //cout << "Center distributively elected: " << module->blockId  << endl;
    //<< "(ecc: " << module->eccentricity << ")" << endl;
    //cout << "NB messages sent Election: " << 0 << endl;
    //cout << "Time to distributively elect the center: " << BaseSimulator::getScheduler()->now()/1000 << "." << (BaseSimulator::getScheduler()->now()%1000)/10 << "ms ..." << endl;
    win();
    cout << "number of steps: " << 0 << endl;

    module->setColor(WHITE);
    leader = true;
    initAlgoVariable(ROLE_N, 1, 0);
  }
}

#define ELECTION_RECV_BETTER 1
#define ELECTION_RECV_EQUAL 0
#define ELECTION_RECV_WORSE -1
#define ELECTION_RECV_ERROR 2
// 1  : better
// 0  : equal (loop)
// -1 : worse
// -2 : error
int ABCCenterV1::cmp(unsigned int id, unsigned int v, unsigned int v2) {
  unsigned int local_sec_value;
  unsigned int local_sec_value2;
			
  switch(role_election) {
  case ROLE_A:
    // Minimize the gradient
    local_sec_value = minGradient;
    local_sec_value2 = minGradient2;
    if (v < local_sec_value) {
      return ELECTION_RECV_BETTER;
    } else if ((v == local_sec_value) && (v2 == local_sec_value2)) {
      if (id < minId) {
	return ELECTION_RECV_BETTER;
      } else if (id == minId) {
	return ELECTION_RECV_EQUAL;
      } else {
	return ELECTION_RECV_WORSE;
      }
    } else if ((v == local_sec_value) && (v2 < local_sec_value2)) {
      return ELECTION_RECV_BETTER;
    } else {
      return ELECTION_RECV_WORSE;
    }
    break;
  case ROLE_B:
  case ROLE_C:
    // Maximize the distance
    local_sec_value = maxDistance;
    if (v > local_sec_value) {
      return ELECTION_RECV_BETTER;
    } else if ((v == local_sec_value) && (id < minId)) {
      return ELECTION_RECV_BETTER;
    } else if ((v == local_sec_value) && (id == minId)) {
      return ELECTION_RECV_EQUAL;
    } else {
      return ELECTION_RECV_WORSE;
    }
    break;
  default:
    return ELECTION_RECV_ERROR;
  }
}

#define CURRENT_STEP 1
#define NEXT_STEP 2
#define STEP_ERROR 3

unsigned int ABCCenterV1::stepConsistency(unsigned int re, unsigned int s, unsigned int recvre, unsigned int recvs) {
  //cout << "@ " << hostBlock->blockId << "re: " << re << " s : " << s << " recvre: " << recvre <<" recvS: " << recvs << endl;
  if (s == recvs) {
    if (re == recvre) {
      return CURRENT_STEP;
    } else if ((re+1) == recvre) {
      return NEXT_STEP;
    } 
  } else if ((s+1) == recvs) {
    if ((re == ROLE_C) && (recvre == ROLE_A)) {
      return NEXT_STEP;
    }
  }
  return STEP_ERROR;
}

void ABCCenterV1::handle(EventPtr e) {

}

void ABCCenterV1::handle(MessagePtr m) {

  stringstream info;
  
  switch (m->type) {
  case GO_MESSAGE: {
    GoMsg_ptr recv = std::static_pointer_cast<GoMsg>(m);
    int cmp_res = 0;
    unsigned int local_sec_value = 0;
	
    unsigned int consistency = 0;
	
    if (recv->role == ROLE_A) {
      consistency = stepConsistency(role_election, step, recv->role, recv->step);
    } else {
      consistency = stepConsistency(role_election, step, recv->role, recv->step);
    }
    if (consistency == STEP_ERROR) {
      goto end_process;
    } else if (consistency == NEXT_STEP) {
      initAlgoVariable(recv->role, recv->step, recv->valid_gradient);
      cmp_res = cmp(recv->id, recv->value, recv->value2);
      if (candidate) {
	if (cmp_res == ELECTION_RECV_WORSE) { // AVOID TO LAUNCH USELESS MSG
	  minId = module->blockId;
	  parent = NULL;
	  if(recv->role == ROLE_A) {
	    minGradient = prev_gradient;
	    minGradient2 = gradient2;
	    ST_launch_go_2(parent, step, role_election, minGradient, minGradient, minGradient2, minId, 0);
	  } else {
	    ST_launch_go_2(parent, step, role_election, recv->valid_gradient, maxDistance, 0, minId, 0);
	  }
	}
      }
    }
					
    consistency = stepConsistency(role_election, step, recv->role, recv->step);
    if (consistency != CURRENT_STEP) {
      cout << "@" << module->blockId <<  "STEP_ERROR2: " << recv->step << " " << getRoleString(recv->role) << " " << recv->value << " " << recv->value2 <<  " " << recv->id << " " << "valid_gradient: " << recv->valid_gradient << endl;
      goto end_process;
    }
					
    cmp_res = cmp(recv->id, recv->value, recv->value2);
    if(recv->role == ROLE_A) {
      candidate = candidate && (prev_gradient <= recv->valid_gradient);
      if(cmp_res == ELECTION_RECV_BETTER) {
	for (int i = 0; i <  NB_ID_ALIVE; i++) {
	  remainingId[i] = NOT_AN_ID;
	}
      }
    }
					
    if ((cmp_res == ELECTION_RECV_BETTER) || 
	((cmp_res == ELECTION_RECV_EQUAL) && (distance > (recv->distance+1)))) {
      if ((cmp_res == ELECTION_RECV_EQUAL) && (parent != NULL) && (distance > (recv->distance+1)) ) {
	// send delete to parent distance-1
	if (role_election == ROLE_A) {
	  local_sec_value = minGradient;
	} else {
	  local_sec_value = maxDistance;
	}
	ST_send_delete_2(parent, step, role_election,local_sec_value, minGradient2, minId, distance-1, remainingId);
      }
      parent = recv->destinationInterface;
      minId = recv->id;
      distance = recv->distance+1;
      //setColorPerValue(distance);
      switch(role_election) {
      case ROLE_A:
	minGradient = recv->value;
	minGradient2 = recv->value2;
	distFromA = distance;
	//cout << "@" << module->blockId << " distFromA: "<< distFromA << endl;
	break;
      case ROLE_B:
	maxDistance = recv->value;
	distFromB = distance;
	//cout << "@" << module->blockId << " distFromB: "<< distFromB << endl;
	break;
      case ROLE_C:
	maxDistance = recv->value;
	distFromC = distance;
	gradient = abs((int)distFromB - (int)distFromC);
	gradient2 = max(distFromB, distFromC);
	//cout << "@" << module->blockId << " distFromC: "<< distFromC << ", gradient: " << gradient << endl;
	break;
      }
      ST_launch_go_2(parent, step, role_election, recv->valid_gradient, recv->value, recv->value2, minId, distance);
      if(NoOneToWaitFor()) {
	ST_send_back_2(parent, step, role_election, recv->value, recv->value2, minId, distance-1, remainingId);
      }
    } 
    //else if ((cmp_res == ELECTION_RECV_EQUAL) && (distance <= (recv->distance+1))) {
    else if ((cmp_res == ELECTION_RECV_EQUAL)) {
      ST_send_delete_2(recv->destinationInterface, step, role_election, recv->value, recv->value2, recv->id, recv->distance, remainingId);
    } else if ((cmp_res == ELECTION_RECV_EQUAL)) {
      //cout << "@" << module->blockId << " optDist: " << distance << ", recv+1: " << distance+1 << endl;
    }
  }
    break;
  case BACK_MESSAGE: {
    BackMsg_ptr recv = std::static_pointer_cast<BackMsg>(m);
    unsigned int consistency = stepConsistency(role_election, step, recv->role, recv->step);
    if (elected) { goto end_process;}
    if (consistency != CURRENT_STEP) {
      goto end_process;
    }
    int cmp_res = cmp(recv->id, recv->value, recv->value2);
    if ((cmp_res == ELECTION_RECV_EQUAL) && (distance == recv->distance)) {
      //cout << "@" << module->blockId <<  "BACK_MSG EQUAL RECV: " << getRoleString(role_election) << " " << recv->value << " " << recv->id <<  endl;  
      waiting[module->getDirection(recv->sourceInterface->connectedInterface)] = false;
      for (unsigned int i = 0; i <  NB_ID_ALIVE; i++) {
	addIdIfNotInRemaining(recv->remaining_id[i]);
      }									
      /*if (candidate) {
	addIdIfNotInRemaining((unsigned int)hostBlock->blockId);
	}*/
      if (NoOneToWaitFor()) {
	//setColorPerValue(distance);
	if (minId == (unsigned int) module->blockId) {
	  //cout << "sp over" << endl;
	  // All the nodes have replied, I'm elected
	  role = role_election;
	  // All the nodes have their final distance to me, I can launch the next step
	  switch(role) {
	  case ROLE_A: {										
	    cout << "A: " << module->blockId <<  "(minGradient: " <<  minGradient << ", minGradient2: " << minGradient2  << ")" << endl;
	    // returns either 1, 2, or 3 (storage inside a message is limited!)
	    //cout << "numDifferentId: " << numDifferentId() << endl;
	    if (step == 1) {
	      cout << "Time to elect A: " << BaseSimulator::getScheduler()->now()/1000 << "." << (BaseSimulator::getScheduler()->now()%1000)/10 << "ms ..." << endl;
	      //cout << "A: @" << module->blockId << ", valid gradient: " << prev_gradient << endl;
	    }

	    int nAlive = numDifferentId();
	    //if (!isIdIn(module->blockId)) {
	      nAlive++;
	      //}
	    printRemainingIds();
	    if (nAlive > 2) {
	      //cout << "electing B: " << endl;
	      // Launch B election
	      initAlgoVariable(ROLE_B, step, minGradient);
	      // launchAlgo(unsigned int vg, unsigned int v);
	      launchAlgo(minGradient,0,0);
											
	      // becomes brown
	      module->setColor(BROWN);
	    } else {
	      elected = true;

	      //cout << "Center distributively elected: " << module->blockId
	      //<< endl;
	      //"(ecc: " << module->eccentricity << ")" << endl;
	      //cout << "Time to distributively elect the center: " << BaseSimulator::getScheduler()->now()/1000 << "." << (BaseSimulator::getScheduler()->now()%1000)/10 << "ms ..." << endl;
	      //cout << "number of steps: " << step-1 << endl;

	      win();
	      cout << "number of steps: " << step - 1 << endl;

#ifdef SPREAD_LEADER_ELECTED_MSG
	      spreadLeaderElectedMsg((unsigned int) module->blockId);
#endif
	      // becomes red
	      //module->setColor(0);
	      leader = true;
	      //initAlgoVariable(ROLE_N, 1, 0);
	    }
	  }
	    break;
	  case ROLE_B:
	    //cout << "electing C: " << endl;
	    cout << "B: " << module->blockId <<  " (dAB: " << distFromA  << ", prev_gradient: " << prev_gradient  << ")" << endl;
	    initAlgoVariable(ROLE_C, step, 0);
	    // launchAlgo(unsigned int vg, unsigned int v);
	    launchAlgo(0,0,0);
	    candidate = false; // B is eliminated, (not a problem: because C != B)
	    // becomes BLUE
	    module->setColor(BLUE);
	    break;
	  case ROLE_C:
	    //cout << "electing A: " << endl;
	    cout << "C: " << module->blockId <<  " (dBC: " << distFromB << ", prev_gradient: " << prev_gradient  << ")" << endl;
	    gradient = distFromB;
	    //cout << "gradient avt init: " << gradient << endl;
	    candidate = false; // C is eliminated
	    spreadNextStepMsg(step+1);
	    // becomes GREEN
	    module->setColor(BLACK);
	    //initAlgoVariable(ROLE_A, step+1, gradient);
	    break;
	  }
	} else {
	  if (parent != NULL) {
	    ST_send_back_2(parent, step, role_election, recv->value, recv->value2, minId, distance-1, remainingId);
	  }
	}
      }
    }
  }
    break;
  case DELETE_MESSAGE: {
    DeleteMsg_ptr recv = std::static_pointer_cast<DeleteMsg>(m);
					
    unsigned int consistency = stepConsistency(role_election, step, recv->role, recv->step);
    if (consistency != CURRENT_STEP) {
      goto end_process;
    }
					
    unsigned int cmp_res = cmp(recv->id, recv->value, recv->value2);
				      
    if ((cmp_res == ELECTION_RECV_EQUAL) && (distance == recv->distance)) {
      //cout << "@" << module->blockId <<  "DELETE_MSG EQUAL RECV: " << getRoleString(role_election) << " " << recv->value << " " << recv->value2 << " " << recv->id <<  endl;  
      children[module->getDirection(recv->sourceInterface->connectedInterface)] = false;
      waiting[module->getDirection(recv->sourceInterface->connectedInterface)] = false;
      if(NoOneToWaitFor()) {
	if (minId == (unsigned int) module->blockId) { // NOT FEASABLE!
	  cout << "@" << module->blockId <<  "DELETE_MSG EQUAL RECV: " << getRoleString(role_election) << " " << recv->value << " " << recv->value2 << " " << recv->id <<  endl;  
	  cout << "minId: " << minId << endl;
	  cout << "[" << BaseSimulator::getScheduler()->now()/1000 <<  "] " << module->blockId << " DELETED WHEN PARENT! " << endl;
	}
	if (parent != NULL) { // SHOULD NOT HAPPEN
	  ST_send_back_2(parent, step, role_election, recv->value, recv->value2, minId, distance-1, remainingId);
	}
      }
    }
  }
    break;
  case NEXT_STEP_MESSAGE : {
    NextStepMsg_ptr recv = std::static_pointer_cast<NextStepMsg>(m);
    if (step != recv->step) {
      //cout << "@" << module->blockId << " next step" << endl;
      if(candidate) {
	//	cout << "@" << module->blockId << " next step: grad: " << gradient << endl;
	initAlgoVariable(ROLE_A, recv->step, gradient);
	//launchAlgo(unsigned int vg, unsigned int v);
	minGradient = prev_gradient;
	minGradient2 = gradient2;
	//cout << "Next step: grad: " << prev_gradient << endl;
	launchAlgo(prev_gradient, prev_gradient, gradient2);
      } else {
	spreadNextStepMsg(recv->step);
      }
    }
  }
    break;
  case LEADER_ELECTED_MESSAGE: {
    LeaderElectedMsg_ptr recv = std::static_pointer_cast<LeaderElectedMsg>(m);
    spreadLeaderElectedMsg(recv->id);					
    initAlgoVariable(ROLE_N, 1, 0);
  }
    break;
  default:
    cout << "unknown message" << endl;
  }
  info << "message received from " << m->sourceInterface->hostBlock->blockId;
 end_process:
  if (info.str() != "" ) {
    BaseSimulator::getScheduler()->trace(info.str(),module->blockId);
  }
}

bool ABCCenterV1::NoOneToWaitFor() {
  for (int i=0; i<module->getNbInterfaces(); i++) {
    if (waiting[i]) {
      return false;
    }
  }
  return true;
}

void ABCCenterV1::addIdIfNotInRemaining(unsigned int id) {
  int k = -1;
  bool isIn = false;
	
  for (int i = 0; i <  NB_ID_ALIVE; i++) {
    if (remainingId[i] == NOT_AN_ID) {
      k = i;
      break;
    }
    if (remainingId[i] == id) {
      isIn = true;
      break;
    }
  }
  if (!isIn && (k != -1)) {
    remainingId[k] = id;
  }
}

void ABCCenterV1::printRemainingIds() {
  cout << "remaining ids: ";
  for (int i = 0; i <  NB_ID_ALIVE; i++) {
    if (remainingId[i] != NOT_AN_ID) {
      cout << remainingId[i] << ",";
    }
  }
  cout << endl;
}

unsigned int ABCCenterV1::numDifferentId() {
  unsigned int res = 0;
  for (unsigned int i = 0; i < NB_ID_ALIVE; i++) {
    if (remainingId[i] != NOT_AN_ID) {
      res++;
    }
  }
  return res;
}

bool ABCCenterV1::isIdIn(unsigned int id) {
  for (unsigned int i = 0; i < NB_ID_ALIVE; i++) {
    if (remainingId[i] == id) {
      return true;
    }
  }
  return false;
}

unsigned int ABCCenterV1::ST_launch_go_2(P2PNetworkInterface *p, unsigned int s, unsigned int r, unsigned int vg, unsigned int v, unsigned int v2, unsigned int id, unsigned int d) {
  P2PNetworkInterface *p2 = NULL;
  unsigned int sent = 0;
  
  for (int i=0; i<module->getNbInterfaces(); i++) {
    p2 = module->getInterface(i);
    if ((p2->isConnected()) && (p2 != p)) {
      // GoMsg(bool e, unsigned int i, unsigned int l, unsigned int g)
      GoMsg *message = new GoMsg(s,r,vg,v,v2,id,d);
      p2->send(message);
      waiting[i] = true;
      children[i] = true;
      sent++;
    } else {
      waiting[i] = false;
      children[i] = false;
    }
  }
  return sent;
}


//void ST_send_back_2(P2PNetworkInterface *p, unsigned int v, unsigned int id, unsigned int d, unsigned int *rid);
//void ST_send_delete_2(P2PNetworkInterface *p, unsigned int v, unsigned int id, unsigned int d, unsigned int *rid);

void ABCCenterV1::ST_send_back_2(P2PNetworkInterface *p, unsigned int s, unsigned int r, unsigned int v, unsigned int v2, unsigned id, unsigned int d, unsigned int *rid) {
					
  if (candidate) {
    addIdIfNotInRemaining((unsigned int)module->blockId);
  }
  
  BackMsg * message = new BackMsg(s,r,v,v2,id,d,rid);
  p->send(message);
}

void ABCCenterV1::ST_send_delete_2(P2PNetworkInterface *p, unsigned int s, unsigned int r, unsigned int v, unsigned int v2, unsigned id, unsigned int d, unsigned int *rid) {
  DeleteMsg * message = new DeleteMsg(s,r,v,v2,id,d,rid);
  p->send(message);
}

void ABCCenterV1::ST_send_elect(P2PNetworkInterface *p, unsigned int r) {
  //ElectMsg * message = new ElectMsg(r);
  //BaseSimulator::getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(BaseSimulator::getScheduler()->now(), message, bbi));
}

string ABCCenterV1::getRoleString(unsigned int r) {
  string s[5] = {"ROLE_N", "ROLE_A", "ROLE_B", "ROLE_C", "ROLE_BARYCENTER"};
  if ((r < 5) && (r >= 0)) {
    return s[r];
  } else {
    return "UNKNOWN";
  }
}

string ABCCenterV1::getMsgEvalString(int r) {
  switch (r) {
  case ELECTION_RECV_BETTER:
    return "ELECTION_RECV_BETTER";
  case ELECTION_RECV_EQUAL:
    return "ELECTION_RECV_EQUAL";
  case ELECTION_RECV_WORSE:
    return "ELECTION_RECV_WORSE";
  case ELECTION_RECV_ERROR:
    return "ELECTION_RECV_ERROR";
  default:
    return "SWITCH_ERROR";
  }
}

void ABCCenterV1::spreadNextStepMsg(unsigned int s) {
  P2PNetworkInterface *p = NULL;
  
  for (int i=0; i<module->getNbInterfaces(); i++) {
    if (children[i]) {
      p = module->getInterface(i);
      // GoMsg(bool e, unsigned int i, unsigned int l, unsigned int g)
      NextStepMsg *message = new NextStepMsg(s);
      p->send(message);
    }
  }
}

void ABCCenterV1::spreadLeaderElectedMsg(unsigned int id) {
  P2PNetworkInterface *p = NULL;
  for (int i=0; i<module->getNbInterfaces(); i++) {
    if (children[i]) {
      p = module->getInterface(i);
      LeaderElectedMsg *message = new LeaderElectedMsg(id);
      p->send(message);
    }
  }
}

string ABCCenterV1::getConsistencyString(unsigned int c) {
  string s[4] = {"UNKNOWN", "CURRENT_STEP", "NEXT_STEP", "STEP_ERROR"};
  if ((c < 4) && (c > 0)) {
    return s[c];
  } else {
    return "UNKNOWN";
  }
}
