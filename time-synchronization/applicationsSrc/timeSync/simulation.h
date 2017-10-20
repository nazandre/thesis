#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "buildingBlock.h"

#include "utils/def.h"

#include <string>
#include <set>

#define THIS_PROTOCOL Simulation::Parameters::parameters.protocol
#define PROTOCOL(p) Simulation::protocol_t::p
#define IS_RUNNING_PROTOCOL(p) (THIS_PROTOCOL == PROTOCOL(p))

// Clock type
#define BB_CLOCK

// Message size parameters:
#define CONSTANT_MESSAGE_SIZE
// Maximum Transmission Unit (octets)
#define BB_MTU 17
#define MTU BB_MTU

#define TRANSMISSION_FAILURE_DETECTION_DELAY_US (50448)

//#define TRANSMISSION_FAILURE_MODEL_3 std::vector<double> {0.98361061071668256154, 0.99816325809755925257, 0.99816325809755925257, 0.99816325809755925257}
//#define TRANSMISSION_FAILURE_MODEL_4 std::vector<double> {0.98361061071668256154, 1.0}

#define PERFECT_TRANSMSSION_FAILURE_MODEL std::vector<double> {1.0}
#define TRANSMISSION_FAILURE_MODEL PERFECT_TRANSMSSION_FAILURE_MODEL

#define SYNCHRONIZATION_START (Simulation::Parameters::parameters.synchronizationStart)

#define SIMULATION_DURATION_USEC (1*ONE_HOUR)
//#define SIMULATION_DURATION_USEC (5*ONE_MINUTE)

#define SIMULATION_END (SYNCHRONIZATION_START + SIMULATION_DURATION_USEC)

#define NUM_CALIBRATION_ROUNDS Simulation::Parameters::parameters.linearRegressionWindowSize
#define SYNC_PERIOD_1_US Simulation::Parameters::parameters.synchronizationPeriod1
#define SYNC_PERIOD_2_US Simulation::Parameters::parameters.synchronizationPeriod2

//#define RANDOM_DELAY_COMPENSATION
//#define STATIC_HOP_DELAY_COMPENSATION
//#define ROUND_TRIP_DELAY_COMPENSATION

//#define STATIC_DELAY_COMPENSATION
#define BITRATE_DELAY_COMPENSATION

#define ROUND_TRIP_DELAY_COMPENSATION_ENABLED Simulation::Parameters::parameters.roundTrip

#define COMM_DELAY_US 6000
//#define MEAN_BITRATE_BIT_S ((myDouble_t)27953.77)
//#define MEAN_BITRATE_BIT_S ((myDouble_t)28117.15)
#define MEAN_BITRATE_BIT_S ((myDouble_t)28000.0)

//#define PROCESSING_DELAY_MIN_US 50
//#define PROCESSING_DELAY_MAX_US 250

#define PROCESSING_DELAY_MIN_US Simulation::Parameters::parameters.minProcessingDelayUS
#define PROCESSING_DELAY_MAX_US Simulation::Parameters::parameters.maxProcessingDelayUS

#define PROCESSING_DELAY_SD_US 0
#define PROCESSING_DELAY_MEAN_US 0

#define MS_RESOLUTION_CLOCK true

//#define ATS 0.3 ; 0.95
#define ATS_PROBA 0.5
#define ATS_PROBA_SKEW_UPDATE ATS_PROBA 
#define ATS_PROBA_OFFSET_UPDATE ATS_PROBA 
#define ATS_RELATIVE_SKEW_UPDATE 0.2
#define ATS_POINT_NUM 3
//ATS_PROBA

//#ifdef MRTP
#define MAX_REACHED_ENABLED Simulation::Parameters::parameters.maxReachedEnabled
//#endif

//#define AD_MAX
#define AD_AVG

#define NETWORK_LOAD_ENABLED Simulation::Parameters::parameters.networkLoad

#define LINEAR_REGRESSION_ENABLED Simulation::Parameters::parameters.linearRegression

#define NUM_MAX_POINTS_LINEAR_MODEL Simulation::Parameters::parameters.linearRegressionWindowSize

#define ERROR_MEASUREMENT_PERIOD (3*ONE_SECOND)
#define ERROR_MEASUREMENT_FILE_NAME "error.dat"

#define EXPORT_PROTOCOL_DATA_ENABLED Simulation::Parameters::parameters.exportProtocolData
#define EXPORT_RTT_DATA_ENABLED Simulation::Parameters::parameters.exportRttData

//#define FAST_APPROX_LOCAL_TIME_TO_SIM_TIME_CONV

#define MASTER_CLOCK_SEED 1
//#define MASTER_CLOCK_SEED 5
//#define MASTER_CLOCK_SEED b->getRandomUint()

namespace Simulation {

  enum protocol_t {NONE = 0,
		   MRTP = 1,
		   TPSN = 2,
		   TPSN_MLE = 3,
		   FTSP = 4,
		   AD = 5,
		   ModifiedATS = 6,
		   WMTS = 7,
		   TREE = 8,
		   CONVERGECAST = 9,
		   PULSE_SYNC = 10,
		   GTSP = 11,
		   NUM_PROTOCOLS,
  };

  enum commModel_t {
    UNKNOWN_COMM_MODEL = -1,
    SPARSE_COMM_MODEL = 0,
    INTERMEDIATE_COMM_MODEL = 1,
    COMPACT_COMM_MODEL = 2
  };
  
  class Parameters {
  public:
    static Parameters parameters;
    
    Time timeToElectTimeMaster;
    mID_t timeMasterId;
    std::set<mID_t> sameClockModelAsMaster;
    bool linearRegression;
    unsigned int linearRegressionWindowSize;
    bool roundTrip;
    bool maxReachedEnabled;
    protocol_t protocol;
    Time synchronizationPeriod1;
    Time synchronizationPeriod2;
    bool networkLoad;
    
    bool exportProtocolData;
    bool exportRttData;

    Time synchronizationStart;

    Time minProcessingDelayUS;
    Time maxProcessingDelayUS;

    commModel_t commModel;
    
    void parseCmdLine(int argc, char **argv);
    void setParameters(BaseSimulator::BuildingBlock *b);

    void enableTransmissionFailures(); // all modules!
    
    void setCommunicationRate(BaseSimulator::BuildingBlock *b);
    void setClock(BaseSimulator::BuildingBlock *b);

    std::string toString(protocol_t p);
    std::string toString(commModel_t m);
    
    void printParameters();
  }; 
}
#endif
