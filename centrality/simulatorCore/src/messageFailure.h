#ifndef MESSAGE_FAILURE_H_
#define MESSAGE_FAILURE_H_

#include <vector>

#include "events.h"
#include "network.h"

#define TRANSMISSION_FAILURE_MODEL_1 std::vector<double> {0.999, 0.9995, 0.99975, 1.0}
#define TRANSMISSION_FAILURE_MODEL_2 std::vector<double> {0.999, 0.9995, 0.99975, 0.9997}

//#define BLINKYBLOCK_TRANSMISSION_FAILURES_MODEL std::vector<double> {0.99832853815591176873, .01450883682696259761, .00086313193588162762, 0}

namespace BaseSimulator {

  class TransmissionFailure {
  protected:
    Time detectionDelay; // delay before current transmission is considered to have failed

    unsigned int numFailures; // number of transmission failures for the current msg
    unsigned int maxNumFailures;
    
  public:
    TransmissionFailure(Time d = 0, unsigned int m = 0);
    TransmissionFailure(const TransmissionFailure &tf);
    virtual ~TransmissionFailure();

    Time getDetectionDelay();
    
    unsigned int getNumFailures();
    bool isDefinitelyLost();
    
    virtual void drawNumFailures() = 0;
    
    void reset();
  };

  // Perfect model:
  // Transmissions never fail, messages are never lossed
  class ZeroTransmissionFailure: public TransmissionFailure {
  public:
    ZeroTransmissionFailure();
    ZeroTransmissionFailure(const ZeroTransmissionFailure &ztf);
    virtual ~ZeroTransmissionFailure();

    void drawNumFailures();
  };

  // Model from data
  class DataTransmissionFailure: public TransmissionFailure {
  protected:
    // Cumulative failure distribution
    // p(F <= n), F: number of failures for the currently sent message
    std::vector<double> distribution;
    doubleRNG generator; // rand between [0.0,1.0]
  public:
    DataTransmissionFailure(Time d,
			    std::vector<double>& dis,
			    int seed);
    
    DataTransmissionFailure(const DataTransmissionFailure &dtf);
    
    virtual ~DataTransmissionFailure();

    void drawNumFailures();
  };

  class NetworkInterfaceMessageLossEvent: public BlockEvent {
  public:
    MessagePtr message;
    P2PNetworkInterface *interface;
    
    NetworkInterfaceMessageLossEvent(Time t,
			BaseSimulator::BuildingBlock *b,
			MessagePtr m,
			P2PNetworkInterface *i);
    
    NetworkInterfaceMessageLossEvent(NetworkInterfaceMessageLossEvent *ev);
    ~NetworkInterfaceMessageLossEvent();
    void consumeBlockEvent();
    const virtual string getEventName();
  };
}

#endif
