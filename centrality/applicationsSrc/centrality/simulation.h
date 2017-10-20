#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "buildingBlock.h"

#include "utils/def.h"

#include <string>
#include <set>


#define WAIT_ONE_NEIGHBOR_INSTEAD_OF_ALL

#define TWO_WAY_IE_SP
//#define TWO_WAY_BFS_SP

#define THIS_ALGORITHM Simulation::Parameters::parameters.alg
#define ALGORITHM(a) Simulation::algorithm_t::a
#define IS_RUNNING_ALGORITHM(a) (THIS_ALGORITHM == ALGORITHM(a))

#define THIS_VERSION Simulation::Parameters::parameters.version
#define VERSION(v) Simulation::version_t::v
#define IS_RUNNING_VERSION(v) (THIS_VERSION == VERSION(v))

#define THIS_PROBABILISTIC_COUNTER Simulation::Parameters::parameters.probabilisticCounter
#define PROBABILISTIC_COUNTER(pc) Simulation::probabilisticCounter_t::pc
#define IS_RUNNING_PROBABILISTIC_COUNTER(pc) (THIS_PROBABILISTIC_COUNTER == PROBABILISTIC_COUNTER(pc))

#define THIS_HASH_FUNCTION Simulation::Parameters::parameters.hashFunc
#define HASH_FUNCTION(h) Simulation::hashFunction_t::h
#define IS_RUNNING_HASH_FUNCTION(h) (THIS_HASH_FUNCTION == HASH_FUNCTION(h))  
  
#define NUMBER_OF_HASH_FUNCTIONS Simulation::Parameters::parameters.numHashFuncs

#define IS_RANDOM_LOGICAL_IDS_ENABLED Simulation::Parameters::parameters.randomLogicalIDs

#define IS_CONTROLLED_BROADCAST_ENABLED Simulation::Parameters::parameters.controlledBroadcast

// Message size parameters:
#define CONSTANT_MESSAGE_SIZE
// Maximum Transmission Unit (octets)
#define MTU 17

// us
#define SEC_TO_US(t) (t*1000000)
#define SIMULATION_START SEC_TO_US(2)

#define MSG_PROCESSING_DELAY 100

#define FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE Simulation::Parameters::parameters.fmBitStringSize

//#define RAND_NUMBER_INSTEAD_OF_HASH  
  
namespace Simulation {

  enum version_t {
    VERSION_NONE = 0,
    VERSION_CENTER, // 1
    VERSION_CENTROID, // 2
    NUM_VERSIONS
  };
  
  enum algorithm_t {NONE = 0,
		    MINIMUM_ID = 1,
		    SHORTEST_PATH_MINIMUM_ID = 2,
		    ABC_CENTER_V1 = 3,
		    ABC_CENTER_V2 = 4,
		    E2ACE = 5,
		    TBCE = 6,
		    EXACT = 7,
		    KORACH = 8,
		    RAND_CENTER = 9,
		    EXTREMUM_CENTER = 10,
		    MINIMUM_ID_THEN_SP_BFS = 11,
		    DEE = 12,
		    RAND_CENTER_2 = 13,
		    PARALLEL_RAND_CENTER = 14,
		    SINGLE_SHORTEST_PATH_BFS = 15,
		    ALPHA_SYNCHRONIZED_SP_BFS = 16,
		    NUM_ALGORITHMS
  };

  enum hashFunction_t {
    NONE_HASH = 0,
    AFFINE = 1,
    AFFINE_PRIME = 2,
    KNUTH = 3,
    MURMUR3 = 4,
    FNV1 = 5,
    RAND_HASH = 6,
    NUM_HASH_FUNCS
  };

  
  enum probabilisticCounter_t {
    UNKNOWN_PROBABILISTIC_COUNTER = 0,
    FLAJOLLET_MARTIN = 1,
    FLAJOLLET_MARTIN_M,
    HYPER_LOG_LOG,
    HYPER_LOG_LOG_HIP,
    NUM_PROBABILISTIC_COUNTERS
  };
  
  class Parameters {
  public:
    static Parameters parameters;
    
    algorithm_t alg;
    version_t version;
    unsigned int numHashFuncs;
    hashFunction_t hashFunc;
    unsigned int fmBitStringSize;
        
    bool randomLogicalIDs;
    probabilisticCounter_t probabilisticCounter;
    
    bool controlledBroadcast;
    
    void parseCmdLine(int argc, char **argv);
    void setParameters(BaseSimulator::BuildingBlock *b);

    void setCommunicationRate(BaseSimulator::BuildingBlock *b);
    
    std::string toString(algorithm_t p);
    std::string toString(version_t v);
    std::string toString(probabilisticCounter_t pc);
    std::string toString(hashFunction_t h);
    
    bool isVersionable(algorithm_t a);

    void printParameters();
  }; 
}
#endif
