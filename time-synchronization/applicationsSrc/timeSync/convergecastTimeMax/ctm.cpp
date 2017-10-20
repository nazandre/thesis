#include "ctm.h"

#include "scheduler.h"

#include "../layers.h"

#include "ctmEv.h"
#include "ctmMsg.h"

#include "../simulation.h"

//#define DEBUG_CTM

ConvergecastTimeMaxAlgorithm::ConvergecastTimeMaxAlgorithm(Layers &l, Tree &t): layers(l), tree(t), maxClock(layers) {

}

ConvergecastTimeMaxAlgorithm::ConvergecastTimeMaxAlgorithm(const ConvergecastTimeMaxAlgorithm &c): layers(c.layers), tree(c.tree), maxClock(c.maxClock) {

}

ConvergecastTimeMaxAlgorithm::~ConvergecastTimeMaxAlgorithm() {

}

void ConvergecastTimeMaxAlgorithm::init() {
  Time now = BaseSimulator::getScheduler()->now();
  Time localTime = layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
  Time syncGlobalTime = layers.synchronization->clock->getTime(localTime,MS_RESOLUTION_CLOCK);

#ifdef DEBUG_CTM
  cerr << "\t CTM init: "
       << "schedulerNow: " << now << " "
       << "localTime: " << localTime << " "
       << "globalTime: " << syncGlobalTime
       << endl;
  getchar();
#endif  
  Synchronization::Point p(localTime,syncGlobalTime);
  maxClock.synchronize(p);

  distanceMaxClock = 0;
}

void ConvergecastTimeMaxAlgorithm::start() {

#ifdef DEBUG_CTM
  MY_CERR << "CTM Start: " << endl;
#endif
  
  init();

  // broadcast go on the tree
  waiting = tree.numChildren();
  if (waiting > 0) {
    CTMGoMessage m;
    tree.broadcast(m);
  } else { // no child (single-node network)
    scheduleCTMDone();
  }
}

void ConvergecastTimeMaxAlgorithm::handle(MessagePtr m) {
  switch(m->type) {
  case CTM_GO:
    {
#ifdef DEBUG_CTM
      MY_CERR << "CTM_GO: ";
#endif

      init();
      
      if (tree.numChildren() == 0) { // if leaf (no child), send back to parent
#ifdef DEBUG_CTM
	cerr << "\t reaches a leaf!" << endl;
#endif
	CTMBackMessage *mBack = new CTMBackMessage(distanceMaxClock);
	tree.sendParent(mBack);

      } else { // else continue to broadcast go:
#ifdef DEBUG_CTM	
	cerr << "\t continues to broadcast!" << endl;
#endif
	waiting = tree.numChildren();
	CTMGoMessage mGo;
	tree.broadcast(mGo);
      }
    }
    break;
  case CTM_BACK:
    {
      CTMBackMessagePtr msg = std::static_pointer_cast<CTMBackMessage>(m);

#ifdef DEBUG_CTM
      MY_CERR << "CTM_BACK: " << endl;
#endif
      
      hopDistance_t hopDistance = msg->hops + 1;
      
      Time localTime = msg->receptionTime;

      Time maxTimeSender = layers.synchronization->compensateForCommDelays(msg->sendTime,
									   msg->size(),
									   hopDistance);

      Time maxTimeHere = maxClock.getTime(localTime,MS_RESOLUTION_CLOCK);
      
#ifdef DEBUG_CTM
      cerr << "\t MaxTimeSender: " << maxTimeSender
	   << "MaxTimeHere: " << maxTimeHere
	   << endl;
#endif
      
      if (maxTimeSender > maxTimeHere) {
	Synchronization::Point p(localTime,maxTimeSender);
	// offset = maxTimeSender - local
	maxClock.synchronize(p);
	distanceMaxClock = hopDistance;

#ifdef DEBUG_CTM
	// maxClock = local + offset;
	cerr << "\t\t" << "Update max clock: "
	     << maxClock.getTime(localTime,MS_RESOLUTION_CLOCK)
	     << endl;
#endif
      }

      waiting--;
      if (waiting == 0) {
	if (tree.isRoot()) {
#ifdef DEBUG_CTM
	  cerr << "\t reaches the root!" << endl;
#endif
	  scheduleCTMDone();
	} else {	  
#ifdef DEBUG_CTM
	  cerr << "\t continues to go back up!" << endl;
#endif
	  // TODO: distance ?
	  CTMBackMessage *mBack = new CTMBackMessage(distanceMaxClock);
	  tree.sendParent(mBack);
	}
      }
    }
    break;
  default:
    assert(false);
  }
}

void ConvergecastTimeMaxAlgorithm::scheduleCTMDone() {
  BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
  sched->schedule(new ConvergecastTimeMaxDone(sched->now(),layers.module)); 
}
