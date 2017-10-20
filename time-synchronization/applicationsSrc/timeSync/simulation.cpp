#include "simulation.h"

#include <iostream>
#include <string>
#include <cassert>
#include <map>

#include "world.h"

#include "random.h"
#include "rate.h"
#include "messageFailure.h"
#include "qclock.h"
#include "clockNoise.h"

#include "intervalRate.h"
#include "measuresRate.h"
#include "mixedLawRate.h"

//#define CLOCK_TYPE_DEBUG

using namespace std;

namespace Simulation {
  
  Parameters Parameters::parameters;

  void  Parameters::parseCmdLine(int argc, char **argv) {
    
    timeToElectTimeMaster = 0;
    timeMasterId = 0;
    networkLoad = false;
    linearRegression = false;
    linearRegressionWindowSize = 0;
    roundTrip = false;
    maxReachedEnabled = false;
    protocol = NONE;
    synchronizationPeriod1 = 0;
    synchronizationPeriod2 = 0;
    exportProtocolData = false;
    exportRttData = false;
    
    synchronizationStart =  std::numeric_limits<Time>::max();
    
    minProcessingDelayUS = 0;
    maxProcessingDelayUS = 0;

    commModel = UNKNOWN_COMM_MODEL;
    
    // intercept -C command line argument
    // warning: must be at the end of the command line!
    for (int i = 0; i < argc; i++) {
      if (argv[i][0] == '-') {
	switch(argv[i][1]) {
	case 'B':
	  {
	    stringstream(argv[i+1]) >> synchronizationStart;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'P':
	  {
	    protocol = protocol_t(atoi(argv[i+1]));
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'M':
	  {
	    timeMasterId = atoi(argv[i+1]);
	    timeToElectTimeMaster = atoi(argv[i+2]);
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'T':     
	  {
	    roundTrip = true;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'L':     
	  {
	    linearRegression = true;
	    linearRegressionWindowSize = atoi(argv[i+1]);
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'S':
	  {
	    synchronizationPeriod1 =  atoi(argv[i+1]);
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'Q':
	  {
	    synchronizationPeriod2 =  atoi(argv[i+1]);
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'F':
	  {
	    maxReachedEnabled = true;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'E':
	  {
	    exportProtocolData = true;
	    exportRttData = true;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	  }
	  break;
	case 'C':
	  {
	    int n = atoi(argv[i+1]);
	    for (int j = 0; j < n && (i+2+j) < argc; j++) {
	      sameClockModelAsMaster.insert(atoi(argv[i+2+j]));
	    }
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	  }
	  break;
	case 'Z':
	  {
	    commModel = commModel_t(atoi(argv[i+1]));
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	  }
	  break;
	case 'N':     
	  {
	    networkLoad = true;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	}
      }
    }

    if (commModel == UNKNOWN_COMM_MODEL) {
      cerr << "Error: provide a communication model (Sparse or dense systems)!" << endl;
      exit(EXIT_FAILURE);
    }
    
    if (protocol <= NONE || protocol >= NUM_PROTOCOLS) {
      cerr << "Error: provide a (valid) synchronization protocol (MRTP/TPSN/AD/ATS/FTSP/...)!" << endl;
      exit(EXIT_FAILURE);
    }

    if (synchronizationStart ==  std::numeric_limits<Time>::max()) {
      cerr << "Error: simulation have to start before the end of the universe!" << endl;
      exit(EXIT_FAILURE);
    }
    
    if (synchronizationPeriod1 == 0) {
      cerr << "Error: provide a synchronization period!" << endl;
      exit(EXIT_FAILURE);
    }
    
    if (protocol == MRTP && synchronizationPeriod2 == 0) {
      cerr << "Error: provide a synchronization period 2 for MRTP!" << endl;
      exit(EXIT_FAILURE);
    }
    
    if (protocol == MRTP || protocol == TPSN || protocol == TPSN_MLE || protocol == TREE || protocol == CONVERGECAST) {
      if (timeMasterId == 0) {
	cerr << "Error: TPSN/MRTP/TREE/CONVERGECAST needs a time master!" << endl;
	exit(EXIT_FAILURE);
      }
      
      if(timeToElectTimeMaster == 0) {
	cerr << "Error: TPSN/MRTP/TREE/CONVERGECAST needs a time to elect the time master!" << endl;
	exit(EXIT_FAILURE);
      }
    }

    printParameters();
    
  }

  void Parameters::setCommunicationRate(BaseSimulator::BuildingBlock *b) {
    vector<P2PNetworkInterface*>& interfaces = b->getP2PNetworkInterfaces();
    vector<P2PNetworkInterface*>::iterator it;

    // new experiments:
    //double meanRate =  27712.98;
    //double stdRate = 1047.217;
    
    //double maxRate = (17.0*8.0) / (5.5*pow(10,-3)) ;
    //double minRate = (17.0*8.0) / (7.1*pow(10,-3)) ;
    //double staticRate = (17.0*8.0) / (6.1*pow(10,-3));
    
    for (it = interfaces.begin() ; it != interfaces.end(); ++it) {
      P2PNetworkInterface* p2p = *it;

      // data rate
      //doubleRNG g = utils::Random::getNormalDoubleRNG(b->getRandomUint(),meanRate,stdRate);    
      //doubleRNG g = utils::Random::getUniformDoubleRNG(b->getRandomUint(),minRate,maxRate);
      
      //RandomRate *rate = new RandomRate(g);
      //Rate *rate = new MeasuresBasedRandomRate(b->getRandomUint());
      //Rate *rate = new StaticRate(staticRate);
      //IntervalRandomRate* rate = IntervalRandomRate::getBBModel(b->getRandomUint());

      Rate *rate = new MixedLawRate(b->getRandomUint(),commModel);
      p2p->setDataRate(rate);

      /*
      // Transmission failure model
      DataTransmissionFailure *transmissionFailure =
      new DataTransmissionFailure(TRANSMISSION_FAILURE_DETECTION_DELAY_US,
      transmissionFailureDistribution,
      b->getRandomUint());
      p2p->setTransmissionFailure(transmissionFailure);
      */
    }
  }

  void Parameters::setClock(BaseSimulator::BuildingBlock *b) {
#ifdef BB_CLOCK
#ifdef CLOCK_TYPE_DEBUG
    cerr << "@" << b->blockId << " BlinkyBlock clock: " << endl;
#endif

    unsigned int seed = 0;
    bool masterClockModel = false;

    if (sameClockModelAsMaster.find(b->blockId) != sameClockModelAsMaster.end()) {
      masterClockModel = true;
    }
    
    if (b->blockId == timeMasterId || masterClockModel) {
      seed = MASTER_CLOCK_SEED;
    } else {
      seed = b->getRandomUint();
    }
    
    // set BB clock
    DNoiseQClock* localClock  = DNoiseQClock::createXMEGA_RTC_OSC1K_CRC(seed);
    
    b->setClock(localClock);

    if (b->blockId == timeMasterId) {
      cout << "Master clock parameter: "
	   << "\n\tModule id: " << b->blockId
	   << "\n\tClockseed: " << seed
	   << "\n\tNoise id: " << localClock->getNoiseId()
	   << endl;
    }

    if (b->blockId != timeMasterId && masterClockModel) {
      cout << "Same parameters as the master clock: "
	   << "\n\tModule id: " << b->blockId
	   << "\n\tClockseed: " << seed
	   << "\n\tNoise id: " << localClock->getNoiseId()
	   << endl;
    }
    
#endif
  }
  
  void Parameters::setParameters(BaseSimulator::BuildingBlock *b) {
    // comm
    setCommunicationRate(b);
    
    // clock type
    setClock(b);
  }

  void Parameters::enableTransmissionFailures() {
    assert(false);
#if 0
    World *world = getWorld(); 
    map<bID, BuildingBlock*> &map = world->getMap();
    std::map<bID, BuildingBlock*>::iterator it1;
    BuildingBlock *b = NULL;

    std::vector<double> transmissionFailureDistribution = TRANSMISSION_FAILURE_MODEL;

    for(it1 = map.begin(); it1 != map.end(); ++it1) {
      b = it1->second;
      
      vector<P2PNetworkInterface*>& interfaces = b->getP2PNetworkInterfaces();
      vector<P2PNetworkInterface*>::iterator it2;
      
      for (it2 = interfaces.begin() ; it2 != interfaces.end(); ++it2) {
	P2PNetworkInterface* p2p = *it2;
	DataTransmissionFailure *transmissionFailure =
	  new DataTransmissionFailure(TRANSMISSION_FAILURE_DETECTION_DELAY_US,
				      transmissionFailureDistribution,
				      b->getRandomUint());
	p2p->setTransmissionFailure(transmissionFailure);
      }
    }
#endif
  }


  std::string Parameters::toString(protocol_t p) {
    switch(p) {
    case NONE:
      return "none";
    case MRTP:
      return "MRTP";
    case TPSN:
      return "Modified-TPSN";
    case TPSN_MLE:
      return "Modified-TPSN-MLE";
    case FTSP:
      return "Modified-FTSP";
    case AD:
      return "AD";
    case ModifiedATS:
      return "Modified-ATS";
    case WMTS:
      return "WMTS";
    case TREE:
      return "Tree";
    case CONVERGECAST:
      return "Convergecast";
    case PULSE_SYNC:
      return "PulseSync";
    case GTSP:
      return "GTSP";
    default:
      return "ERROR: UNKNOWN PROTOCOL!";
    } 
  }

  std::string Parameters::toString(commModel_t m) {
    switch(m) {
    case UNKNOWN_COMM_MODEL:
      return "UNKNOWN_COMM_MODEL";
    case SPARSE_COMM_MODEL:
      return "SPARSE_COMM_MODEL";
    case INTERMEDIATE_COMM_MODEL:
      return "INTERMEDIATE_COMM_MODEL";
    case COMPACT_COMM_MODEL:
      return "COMPACT_COMM_MODEL";
    default:
      return "ERROR: UNKNOWN PROTOCOL!";
    } 
  }


  void Parameters::printParameters() {
    cerr << "Time-Sync:" << endl;
    cerr << "\tProtocol: " << toString(protocol) << endl;
    cerr << "\tMaster id: " << timeMasterId << endl;
    cerr << "\tTimeToElectTimeMaster: " << timeToElectTimeMaster << endl;
    cerr << "\tRoundTrip: " << roundTrip << endl;
    cerr << "\tLinearRegression: " << linearRegression << endl;
    cerr << "\tLinearRegressionWindowSize: " << linearRegressionWindowSize << endl;
    cerr << "\tMaxReachedEnabled: " << maxReachedEnabled << endl;
    cerr << "\tSynchronizationPeriod1: " << SYNC_PERIOD_1_US << endl;
    cerr << "\tSynchronizationPeriod2: " << SYNC_PERIOD_2_US << endl;
    cerr << "\tCommunication model: " << toString(commModel) << endl;
  }
}
