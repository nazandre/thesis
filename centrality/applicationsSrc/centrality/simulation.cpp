#include "simulation.h"

#include <iostream>
#include <string>
#include <cassert>
#include <map>

#include "world.h"

#include "random.h"
#include "rate.h"

using namespace std;

namespace Simulation {
  
  Parameters Parameters::parameters;

  void  Parameters::parseCmdLine(int argc, char **argv) {
    
    alg = NONE;
    version = VERSION_NONE;
    probabilisticCounter = UNKNOWN_PROBABILISTIC_COUNTER;
    hashFunc = NONE_HASH;
    numHashFuncs = 0;
    randomLogicalIDs = false;
    fmBitStringSize = 0;
    controlledBroadcast = true;
    
    // intercept -C command line argument
    // warning: must be at the end of the command line!
    for (int i = 0; i < argc; i++) {
      if (argv[i][0] == '-') {
	switch(argv[i][1]) {
	case 'A':
	  {
	    alg = algorithm_t(atoi(argv[i+1]));
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'V':
	  {
	    version = version_t(atoi(argv[i+1]));
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'H':
	  {
	    hashFunc = hashFunction_t(atoi(argv[i+1]));
	    numHashFuncs = atoi(argv[i+2]);
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'I':
	  {
	    randomLogicalIDs = true;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'P':
	  {
	    probabilisticCounter =  probabilisticCounter_t(atoi(argv[i+1]));
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'B':
	  {
	    fmBitStringSize =  (atoi(argv[i+1]));
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	case 'U':
	  {
	    controlledBroadcast = false;
	    argv[i][0] = 0;
	    argv[i][1] = 0;
	    break;
	  }
	}
      }
    }

    if (alg <= NONE || alg >= NUM_ALGORITHMS) {
      cerr << "Error: provide a (valid) algorithm to execute!" << endl;
      exit(EXIT_FAILURE);
    }

    if (isVersionable(alg) && (version <= VERSION_NONE || version >= NUM_VERSIONS)) {
      cerr << "Error: provide a (valid) version of this algorithm to execute!" << endl;
      exit(EXIT_FAILURE);
    }

    if (alg == E2ACE && (probabilisticCounter <= UNKNOWN_PROBABILISTIC_COUNTER || probabilisticCounter >= NUM_PROBABILISTIC_COUNTERS)) {
      cerr << "Error: provide a valid probabilistic counter for E2ACE!" << endl;
      exit(EXIT_FAILURE);
    }

    if (alg == E2ACE && (hashFunc <= NONE_HASH || hashFunc >= NUM_HASH_FUNCS)) {
      cerr << "Error: provide a (valid) hash function to use with E2ACE/probabilistic counting!" << endl;
      exit(EXIT_FAILURE);
    }

    if (alg == E2ACE
	&& (probabilisticCounter == FLAJOLLET_MARTIN || probabilisticCounter == FLAJOLLET_MARTIN_M)
	&& fmBitStringSize != 2 && fmBitStringSize != 4) {
      cerr << "Error: provide a bitStringSize in bytes (2 bytes or 4 bytes) for E2ACE-FM" << endl;
      exit(EXIT_FAILURE);
    }
    
    printParameters();
  }

  void Parameters::setCommunicationRate(BaseSimulator::BuildingBlock *b) {
    /*
    vector<P2PNetworkInterface*>& interfaces = b->getP2PNetworkInterfaces();
    vector<P2PNetworkInterface*>::iterator it;

    for (it = interfaces.begin() ; it != interfaces.end(); ++it) {
      P2PNetworkInterface* p2p = *it;

      // data rate
      //doubleRNG g = utils::Random::getNormalDoubleRNG(b->getRandomUint(),meanRate,stdRate);    
      //doubleRNG g = utils::Random::getUniformDoubleRNG(b->getRandomUint(),minRate,maxRate);
      
      //RandomRate *rate = new RandomRate(g);
     
      //Rate *rate = new MixedLawRate(b->getRandomUint(),commModel);
      //p2p->setDataRate(rate);
      }*/
  }

  
  void Parameters::setParameters(BaseSimulator::BuildingBlock *b) {
    // comm
    setCommunicationRate(b);
  }

  std::string Parameters::toString(algorithm_t a) {
    switch(a) {
    case MINIMUM_ID:
      return "MINIMUM_ID";
    case SHORTEST_PATH_MINIMUM_ID:
      return "SHORTEST_PATH_MINIMUM_ID";
    case ABC_CENTER_V1:
      return "ABC_CENTER_V1";
    case ABC_CENTER_V2:
      return "ABC_CENTER_V2";
    case E2ACE:
      return "E2ACE";
    case TBCE:
      return "TBCE";
    case EXACT:
      return "EXACT";
    case KORACH:
      return "KORACH";
    case RAND_CENTER:
      return "RAND_CENTER";
    case EXTREMUM_CENTER:
      return "EXTREMUM_CENTER";
    case MINIMUM_ID_THEN_SP_BFS:
      return "MINIMUM_ID_THEN_SP_BFS";
    case DEE:
      return "DEE";
    case RAND_CENTER_2:
      return "RAND_CENTER_2";
    case PARALLEL_RAND_CENTER:
      return "PARALLEL_RAND_CENTER";
    case  SINGLE_SHORTEST_PATH_BFS:
      return "SINGLE_SHORTEST_PATH_BFS";
    case ALPHA_SYNCHRONIZED_SP_BFS:
      return "ALPHA_SYNCHRONIZED_SP_BFS";
    default:
      return "ERROR: UNKNOWN ALGORITHM!";
    } 
  }

  std::string Parameters::toString(version_t v) {
    switch(v) {
    case VERSION_NONE:
      return "VERSION_NONE";
    case VERSION_CENTER:
      return "VERSION_CENTER";
    case VERSION_CENTROID:
      return "VERSION_CENTROID";
    default:
      return "ERROR: UNKNOWN VERSION!";
    }
  }

  std::string Parameters::toString(probabilisticCounter_t pc) {
    switch(pc) {
    case FLAJOLLET_MARTIN:
      return "FLAJOLLET_MARTIN";
    case FLAJOLLET_MARTIN_M:
      return "FLAJOLLET_MARTIN_M";
    case HYPER_LOG_LOG:
      return "HYPER_LOG_LOG";
    case HYPER_LOG_LOG_HIP:
      return "HYPER_LOG_LOG_HIP";
    default:
      return "ERROR: UNKNOWN PROBABILISTIC COUNTER!";
    }
  }
  
  bool Parameters::isVersionable(algorithm_t a) {
    switch(a) {
    case E2ACE:
    case EXACT:
    case RAND_CENTER:
    case RAND_CENTER_2:
    case PARALLEL_RAND_CENTER:
    case EXTREMUM_CENTER:
    case DEE:
      return true;
    default:
      return false;
    }
  }

  std::string Parameters::toString(hashFunction_t h) {
    switch(h) {
    case AFFINE:
      return "AFFINE";
    case AFFINE_PRIME:
      return "AFFINE_PRIME";
    case KNUTH:
      return "KNUTH MULTIPLICATIVE";
    case MURMUR3:
      return "MURMUR3";
    case FNV1:
      return "FNV1";
    case RAND_HASH:
      return "RAND_HASH";
    default:
      return "ERROR: HASH FUNC TYPE!";
    }
  }
  
  void Parameters::printParameters() {
    cerr << "Communication parameters:" << endl;
    //cerr << "\tBitrate N(m,s)" << endl;
    //cerr << "\tMsg Processing N(m,s)" << endl;
    cerr << "Sizeof(module id and system size): " << sizeof(mID_t) << endl;
    cerr << "Random logical IDs: " << randomLogicalIDs << endl;
    cerr << "ControlledBroadcast: " << controlledBroadcast << endl;

    cerr << "TwoWayBFSSP: ";
#ifdef TWO_WAY_BFS_SP
    cerr << "1" << endl;
#else
    cerr << "0" << endl;
#endif

    cerr << "TwoWayIESP: ";
#ifdef TWO_WAY_IE_SP
    cerr << "1" << endl;
#else
    cerr << "0" << endl;
#endif

    cerr << "WaitOneNeighborInsteadOfAll: ";
#ifdef WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL
    cerr << "1" << endl;
#else
    cerr << "0" << endl;
#endif
    
    cerr << "Election algorithm:" << endl;
    cerr << "\tAlgorithm: " << toString(alg) << endl;
    if (isVersionable(alg)) {
      cerr << "\tVersion: " << toString(version) << endl;
    }
    if (alg == E2ACE) {
      cerr << "\tProbabilistic counter: " << toString(probabilisticCounter) << endl;
      if (probabilisticCounter == FLAJOLLET_MARTIN || probabilisticCounter == FLAJOLLET_MARTIN_M) {
	cerr << "\t\tByte-size of FLAJOLLET_MARTIN bitstrings: " << fmBitStringSize << endl;
      }
      cerr << "\t\tHash func: " << toString(hashFunc) << endl;
      if (hashFunc == RAND_HASH) {
	cerr << "\t\t\tNumber of random numbers: ";
      } else {
	cerr << "\t\t\tNumber of hash functions: ";
      }      
      if (numHashFuncs == 0) {
	cerr << "as much as possible";
      } else {
	cerr << numHashFuncs;
      }
      cerr << endl;
    }
  }
}
