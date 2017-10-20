#include "mrtp.h"

#include "scheduler.h"

#include <ctime>
#include <sys/resource.h>
#include <sys/times.h>

#include "../utils/utils.h"
#include "../simulation.h"
#include "../layers.h"

#include "../appMsgID.h"
#include "../appEventsID.h"

#include "mrtpMsg.h"

#include "../synchronization/clock.h"
#include "../synchronization/mleClock.h"

#define EXPORT_MSG_SIZE

//#define GET_GLOBAL_TIME timeMaster->getTimeNow()

//#define DEBUG_MRTP_TIME_SYNC

#define MRTP_OUTPUT_FILE_NAME "mrtp.dat"
#define RTT_FILE_NAME "rtt.dat"
#define MSG_SIZE_FILE "msgSize-sim.out"

//#define TEST_CLOCK_COMP

namespace Synchronization {
  
  MRTPNode* MRTPNode::timeMasterMRTPNode = NULL;

  MRTPNode::MRTPNode(Layers &l): Node(l),
				 spanningTreeConstructor(l,true),
				 spanningTree(),
				 convergecastTimeMax(l,spanningTree.tree) {
    
    if (LINEAR_REGRESSION_ENABLED) {
      clock = new LinearClock(layers,NUM_MAX_POINTS_LINEAR_MODEL);
    } else if (Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TPSN_MLE) {
      clock = new MLEClock(layers);
    } else {
      clock  = new OffsetClock(layers);
    }
    
    round = 0;    
  }

  MRTPNode::MRTPNode(const MRTPNode &n): Node(n),
					 spanningTreeConstructor(n.spanningTreeConstructor),
					 spanningTree(n.spanningTree),
					 convergecastTimeMax(n.convergecastTimeMax)
  {
    round = n.round;
    assert(false);
  }

  MRTPNode::~MRTPNode() {
  }

  /*
  double getCPUTime() {
    clock_t cl = clock( );
    if ( cl != (clock_t)-1 )
      return (double)cl / (double)CLOCKS_PER_SEC;
    else
      return -1;
    

    
    //struct rusage rusage;
    //if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
     // return (double)rusage.ru_utime.tv_sec +
//	(double)rusage.ru_utime.tv_usec / 1000000.0;
    
 //   return -1;
  }*/
  
  void MRTPNode::startup() {
    static bool initialized = false;

    if (!initialized) {
      ofstream output;
      
      // clean output files
      output.open(MRTP_OUTPUT_FILE_NAME, fstream::trunc);
      output.close();
      
      output.open(RTT_FILE_NAME, fstream::trunc);
      output.close();

      output.open(MSG_SIZE_FILE, fstream::trunc);
      output.close();
      
      initialized = true;

#ifdef TEST_CLOCK_COMP

      Point p1(239204*1000,239347*1000);
      Point p2(244202*1000,244350*1000);
      Point p5(249205*1000, 249353*1000);
      Point p4(229202*1000, 229341*1000);
      Point p3(234202*1000, 234344*1000);
      Time now = 249307*1000;

      /*
      Point p1(464201*1000,464482*1000);
      Point p2(469199*1000,469485*1000);
      Point p3(474200*1000, 474488*1000);
      Point p4(479198*1000, 479491*1000);
      Point p5(484200*1000, 484494*1000);
      Time now = 484301*1000;
      */
      
      LinearClock *lc = (LinearClock*)clock;
      lc->synchronize(p1);
      lc->synchronize(p2);
      lc->synchronize(p3);
      lc->synchronize(p4);
      lc->synchronize(p5);
	
      cout.precision(7);
      cout << "y0: " << lc->y0 << ", x0: " << lc->x0 << endl;
      cout << "offsetHW: " << lc->offsetHW << endl;
      cout << "S: " << lc->getTimeLast(now,true) << endl;
      
      exit(0);
#endif
    }

    round = 0;
    
    if(layers.module->blockId ==
       Simulation::Parameters::parameters.timeMasterId) {
      timeMasterMRTPNode = this;
      startCentralized(true);
    } else {
      startCentralized(false);
    }
    
  }
  
  void MRTPNode::handle(MessagePtr m) {

    //double startCPU = getCPUTime();
    
    switch(m->type) {
    case BFS_TRAVERSAL_SP_GO:
    case BFS_TRAVERSAL_SP_GO_ACK:
    case BFS_TRAVERSAL_SP_BACK:
    case BFS_TRAVERSAL_SP_DELETE:
      {
	spanningTreeConstructor.handle(m,&spanningTree);
      }
      break;
    case CTM_GO:
    case CTM_BACK:
      {
	convergecastTimeMax.handle(m);
      }
      break;
    case MRTP_TIME_SYNC_1_MSG:
    case MRTP_TIME_SYNC_2_MSG:
    case MRTP_TIME_SYNC_3_MSG:
      {
	handleRoundTripSync(m);
      }
      break;
    case MRTP_TIME_SYNC_MSG:
      {
	MRTPTimeSyncMessagePtr msg = std::static_pointer_cast<MRTPTimeSyncMessage>(m);
	
	Time nowSimTime = BaseSimulator::getScheduler()->now();
	
	hopDistance_t level = spanningTree.tree.distance;

	Time receptionLocalTime = msg->receptionTime;

	Time receptionEstimatedGlobalTime =
	  Synchronization::Node::compensateForCommDelays(msg->sendTime, msg->size(), level);
	
	Point p(receptionLocalTime,receptionEstimatedGlobalTime);
	
	//Time receptionSimTime = layers.localClock->getSimTime(receptionLocalTime); // US
	Time receptionSimTime = msg->receptionSimTime;

#ifdef EXPORT_MSG_SIZE
	ofstream output;
	output.open(MSG_SIZE_FILE, fstream::app);
	output << receptionEstimatedGlobalTime << " " << msg->size() << endl;
	output.close();
#endif
	
	round++;	

	if(EXPORT_PROTOCOL_DATA_ENABLED) {
	  if (round > 2) {
	    exportStat(receptionSimTime, receptionLocalTime, receptionEstimatedGlobalTime);
	  }
	}
	
	// update clock
	clock->synchronize(p);
	
	if (LINEAR_REGRESSION_ENABLED) {
	  setHighProcessingDelays();
	}
	
	// Sync children
	if (nowSimTime < SIMULATION_END) {
	  synchronizeChildren();
	}

        setLowProcessingDelays();

      }
      break;
    default:
      assert(false);
    }

    /*
    double endCPU = getCPUTime();
    double durationCPU = endCPU - startCPU;
    cout << "duration: " << durationCPU << endl;
    cout << "CPU freq: " << CLOCKS_PER_SEC << endl;

    double cpuFreq = CLOCKS_PER_SEC;
    double ucFreq = 32*pow(10,6);
    double durationUC = ucFreq/cpuFreq * durationCPU;
    cout << "Computation:" << durationUC << endl;
    */
  }

  void MRTPNode::handle(EventPtr e) {
    
    switch(e->eventType) {
    case EVENT_TIME_MASTER_ELECTED:
      {
	MY_CERR << "Time Master elected!" << endl;
	BFSTraversalSP *sp = spanningTreeConstructor.create();
	spanningTree = *sp;
	delete sp;
	spanningTreeConstructor.start(&spanningTree);
      }
      break;
    case EVENT_SPANNING_TREE_CONSTRUCTED:
      {
	MY_CERR << "Spanning-tree constructed!" << endl;

	if (Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TREE) {
	  return;
	}

	if (Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TPSN ||
	    Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TPSN_MLE)
	  {
	    //Simulation::Parameters::parameters.enableTransmissionFailures();
	  Time now = BaseSimulator::getScheduler()->now();
	  scheduleTimeSync(now);
	  return;
	}
	
	// launch convergecastMaxTime
	convergecastTimeMax.start();
      }
      break;
    case EVENT_CONVERGECAST_TIME_MAX_DONE:
      {
	// update global clock
	MY_CERR << "Convergecast time max done!" << endl;
	
	// first synchronization wave
	// now
	Time now = BaseSimulator::getScheduler()->now();
	Time localTime = layers.localClock->getLocalTime(now,MS_RESOLUTION_CLOCK);
	Time localGlobal = clock->getTime(localTime);
	Time maxNow = convergecastTimeMax.maxClock.getTime(now);

	Point p(localGlobal,maxNow);

	clock->setTime(p);
#ifdef DEBUG_MRTP_TIME_SYNC
	cerr << "\t MaxCTM: " << maxNow
	     << " => global: " << clock->getTime(localTime)
	     << endl;
#endif

	if (Simulation::Parameters::parameters.protocol == Simulation::protocol_t::CONVERGECAST) {
	  return;
	}
	scheduleTimeSync(now);
      }
      break;
    case EVENT_TIME_SYNC:
      {
	Time simTime = BaseSimulator::getScheduler()->now();

	// Should be 
	// But this is faster (although a little bit wrong)
	//Time nextSync = getSimTime(getLocalTime(false)+SYNC_PERIOD_US);
	Time localTime = layers.localClock->getLocalTime(simTime,MS_RESOLUTION_CLOCK);	
	Time globalTime = clock->getTime(localTime,MS_RESOLUTION_CLOCK);
	Point p(localTime,globalTime);

	if (Simulation::Parameters::parameters.protocol !=
	    Simulation::protocol_t::MRTP || 
	    round < NUM_CALIBRATION_ROUNDS) {
	  scheduleNextTimeSync(SYNC_PERIOD_1_US);
	} else {
	  scheduleNextTimeSync(SYNC_PERIOD_2_US);
	}	
	
	round++;

	synchronizeChildren();
      }
      break;
    default:
      MY_CERR << " unknown event type: " << e->getEventName() << endl;
      assert(false);
    }
  }

  void MRTPNode::scheduleSynchronizeChildren() {

  }

  void MRTPNode::synchronizeChildren() {
    if (ROUND_TRIP_DELAY_COMPENSATION_ENABLED) {
      MRTPSyncRT m(MRTP_TIME_SYNC_1_MSG);
      spanningTree.tree.broadcast(m);
    } else {
      MRTPTimeSyncMessage m(spanningTree.tree.distance);
      spanningTree.tree.broadcast(m);
    }
  }

  // Round trip sync (test)
  void MRTPNode::handleRoundTripSync(MessagePtr m) {
    P2PNetworkInterface *recv = m->destinationInterface;

    Time now = BaseSimulator::getScheduler()->now();
    
    switch(m->type) {
    case MRTP_TIME_SYNC_1_MSG:
      {
	MRTPSyncRTPtr msg =
	  std::static_pointer_cast<MRTPSyncRT>(m);

	MRTPSyncRT* reply = new MRTPSyncRT(MRTP_TIME_SYNC_2_MSG,
					   msg->simTime,
					   msg->localTime,
					   msg->globalTime
					   );

        
	reply->sizePrevious = msg->size();

	MyMessage::send(layers, recv, reply);
      }
      break;
    case MRTP_TIME_SYNC_2_MSG:
      {
	MRTPSyncRTPtr msg =
	  std::static_pointer_cast<MRTPSyncRT>(m);
	
	MRTPSyncRT* reply = new MRTPSyncRT(MRTP_TIME_SYNC_3_MSG,
					   msg->simTime,
					   msg->localTime,
					   msg->globalTime
					   );

	reply->sizePrevious = msg->size();
        
	#if 1
	if (EXPORT_RTT_DATA_ENABLED &&
	    (msg->size() == 21) &&
	    (msg->sizePrevious == 21) &&
	    (layers.module->blockId == 2)) {
	  
	  // compute transfer delay
	  vector<Time>& localTime = msg->localTime;
	  Time processing = localTime[T3_INDEX] - localTime[T2_INDEX];
	  Time rttLocal = (localTime[T4_INDEX] - localTime[T1_INDEX]) - processing;

	  //	    (localTime[T3_INDEX] - localTime[T2_INDEX]);
	  /*
	  MY_CERR << "t1: " << localTime[T1_INDEX] << endl;
	  MY_CERR << "t2: " << localTime[T2_INDEX] << endl;
	  MY_CERR << "t3: " << localTime[T3_INDEX] << endl;
	  MY_CERR << "t4: " << localTime[T4_INDEX] << endl;
	  MY_CERR << "=======================" << endl;
	  */
	  // export transfer delay to file

	  //msg->size()
	  ofstream output;
	  output.open(RTT_FILE_NAME, fstream::app);
	  output << rttLocal << " "
		 << msg->size() << " "
		 << processing
		 << endl;
	  output.close();
	}
      #endif
	
        MyMessage::send(layers, recv, reply);
      }
      break;
    case MRTP_TIME_SYNC_3_MSG:
      {
	MRTPSyncRTPtr msg =
	  std::static_pointer_cast<MRTPSyncRT>(m);
	
	vector<Time>& simTime = msg->simTime;
	vector<Time>& localTime = msg->localTime;
	vector<Time>& globalTime = msg->globalTime;

	// compute transfer delay
	Time rttLocal = (localTime[T6_INDEX] - localTime[T3_INDEX]) - (localTime[T5_INDEX] - localTime[T4_INDEX]);
	Time oneWayLocal = rttLocal/2;

	oneWayLocal = US_TO_US_MS_RESOLUTION(oneWayLocal);
	round++;

        // Compensate for synchro
	Time syncGlobalTime = globalTime[T5_INDEX] + oneWayLocal;
	Time syncLocalTime = localTime[T6_INDEX];
	
	Point p(syncLocalTime,syncGlobalTime);

	if(EXPORT_PROTOCOL_DATA_ENABLED) {
	  // max clock not done! TODO
	  exportStat(simTime[T6_INDEX],
		     syncLocalTime, syncGlobalTime);
	}

    	/*if (EXPORT_RTT_DATA_ENABLED &&
	    (msg->size() == 21) &&
	    (msg->sizePrevious == 21) &&
	    (layers.module->blockId == 2)) {
	  
	  // export transfer delay to file
	  ofstream output;
	  output.open(RTT_FILE_NAME, fstream::app);
	  output << rttLocal << " " << msg->size() << endl;
	  output.close();
	  }*/

	if (Simulation::Parameters::parameters.protocol == Simulation::protocol_t::TPSN_MLE) {
	  clock->synchronize(localTime[T3_INDEX],globalTime[T4_INDEX],globalTime[T5_INDEX],localTime[T6_INDEX], syncGlobalTime);
	  setHighProcessingDelays();
	} else {
	  // synchronize clock
	  clock->synchronize(p);
	  
	  if (LINEAR_REGRESSION_ENABLED) {
	    setHighProcessingDelays();
	  }
	}
	
	// Sync children
	if (now < SIMULATION_END) {
	  synchronizeChildren();
	}
	
	setLowProcessingDelays();
      }
      break;
    default:
      assert(false);

    }
  }


  void MRTPNode::exportStat(Time receptionSimTime,
			    Time receptionLocalTime,
			    Time receptionEstimatedGlobalTime) {

    fstream output;
  
    Time receptionThisGlobalTime = clock->getTime(receptionLocalTime,MS_RESOLUTION_CLOCK); // US
  
    // Todo: MS_RESOLUTION_CLOCK ???
    Time receptionTimeMasterLocalTime =
      timeMasterMRTPNode->layers.localClock->getLocalTime(receptionSimTime,MS_RESOLUTION_CLOCK); // US
    Time receptionTimeMasterGlobalTime =
      timeMasterMRTPNode->clock->getTime(receptionTimeMasterLocalTime,MS_RESOLUTION_CLOCK); // US
  
	
#ifdef DEBUG_MRTP_TIME_SYNC	
    MY_CERR << "MRTP_SYNC_MSG: " << endl;
  
    //cerr << "\tSimNow: " << nowSimTime << endl;
    cerr << "\tReceptionLocalTime: " << receptionLocalTime << endl;
    cerr << "\tReceptionSimTime: " << receptionSimTime << endl;
    cerr << "\tReceptionEstimatedGlobalTime: "
	 << receptionEstimatedGlobalTime << endl;
    cerr << "\tReceptionThisGlobalTime: "
	 << receptionThisGlobalTime << endl;
    cerr << "\tReceptionTimeMasterLocalTime: "
	 << receptionTimeMasterLocalTime << endl;
    cerr << "\tReceptionTimeMasterGlobalTime: "
	 << receptionTimeMasterGlobalTime << endl;
	
    /*MY_CERR << "\t " << receptionSimTime << " "
      << receptionRealGlobalTime << " "
      << receptionEstimatedGlobalTime << " "
      << receptionLocalTime << " "
      << receptionThisGlobalTime
      << endl;
    */
    int64_t errorEstim =
      (int64_t) receptionTimeMasterGlobalTime - (int64_t) receptionEstimatedGlobalTime;
    cerr << "\t\t error estim (ms)" << US_TO_MS(errorEstim) << endl;
    
    int64_t error =
      (int64_t) receptionTimeMasterGlobalTime - (int64_t) receptionThisGlobalTime;
    cerr << "\t\t error sync (ms)" << US_TO_MS(error) << endl;
    //getchar();
#endif

    /*int64_t errorGlobal =  (int64_t) receptionEstimatedGlobalTime - (int64_t) receptionThisGlobalTime;
    errorGlobal /= 1000;
    if (abs(errorGlobal) > 100) {
      MY_CERR << "Wow: " << errorGlobal
	      << " at " << receptionSimTime/1000/1000 << " s "
	      << " round " << round
	      << endl;
      getchar();
      }*/
    /*MY_CERR << "time master id: "
	    << timeMasterMRTPNode->layers.module->blockId
	    	    << endl;
    */
    
    // Export to file
    output.open(MRTP_OUTPUT_FILE_NAME, fstream::app);
    output << layers.module->blockId << " "
	   << receptionSimTime << " "
	   << receptionTimeMasterGlobalTime << " "
	   << receptionEstimatedGlobalTime << " "
	   << receptionLocalTime << " "
	   << receptionThisGlobalTime << " "
	   << spanningTree.tree.distance << endl;
    output.close();
  } 
}
