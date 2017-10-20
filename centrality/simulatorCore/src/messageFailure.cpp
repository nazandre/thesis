#include "messageFailure.h"

#include <vector>

#include "network.h"
#include "events.h"
#include "scheduler.h"

//#define TRANSMISSION_FAILURE_DEBUG

namespace BaseSimulator {

  TransmissionFailure::TransmissionFailure(Time d, unsigned int m):
    detectionDelay(d),
    numFailures(0),
    maxNumFailures(m)
  {
  }
  
  TransmissionFailure::TransmissionFailure(const TransmissionFailure &tf):
    detectionDelay(tf.detectionDelay),
    numFailures(tf.numFailures),
    maxNumFailures(tf.maxNumFailures)
  {
  }
    
  TransmissionFailure::~TransmissionFailure() {

  }

  unsigned int TransmissionFailure::getNumFailures() {
    return numFailures;
  }

  bool TransmissionFailure::isDefinitelyLost() {
    return numFailures > maxNumFailures;
  }
  
  void TransmissionFailure::reset() {
    numFailures = 0;
  }
    
  Time TransmissionFailure::getDetectionDelay() {
    return detectionDelay;
  }

  // ZeroTransmissionFailure class
  
  ZeroTransmissionFailure::ZeroTransmissionFailure(): TransmissionFailure() {

  }

  ZeroTransmissionFailure::ZeroTransmissionFailure(const ZeroTransmissionFailure &ztf):
    TransmissionFailure(ztf) {
    
  }

  ZeroTransmissionFailure::~ZeroTransmissionFailure() {

  }

  void ZeroTransmissionFailure::drawNumFailures() {
    numFailures = 0;
  }
  
  // DataTransmissionFailure class
  
  DataTransmissionFailure::DataTransmissionFailure(Time d,
						   std::vector<double>& dis,
						   int seed
						   ): TransmissionFailure(d,dis.size()-1) {
    distribution = dis;
    generator = utils::Random::getUniformDoubleRNG(seed,0.0,1.0);
  }
  
  DataTransmissionFailure::DataTransmissionFailure(const DataTransmissionFailure &dtf):
    TransmissionFailure(dtf),
    distribution(dtf.distribution),
    generator(dtf.generator) {
    
  }
    
  DataTransmissionFailure::~DataTransmissionFailure() {

  }
    
  void DataTransmissionFailure::drawNumFailures() {
      double p = generator();

      numFailures = 0;
      while (numFailures < distribution.size()  && p > distribution[numFailures]) {
	numFailures++;
      }
#ifdef TRANSMISSION_FAILURE_DEBUG
      cerr << "Num transmission failures: " << numFailures << " "
	   << "(randow draw: " << p << ")"
	   << " at " << BaseSimulator::getScheduler()->now()
	   << endl;
#endif
  }
  
  // NetworkInterfaceMessageLossEvent class

  NetworkInterfaceMessageLossEvent::NetworkInterfaceMessageLossEvent(Time t,
					   BaseSimulator::BuildingBlock *b,
					   MessagePtr m,
					   P2PNetworkInterface *i): BlockEvent(t,b),
								    message(m),
								    interface(i) {
    eventType = EVENT_NI_MESSAGE_LOSS;
    
      
  }
    
  NetworkInterfaceMessageLossEvent::NetworkInterfaceMessageLossEvent(NetworkInterfaceMessageLossEvent *ev): BlockEvent(ev),
								     message(ev->message),
								     interface(ev->interface) {
    
  }
    
  NetworkInterfaceMessageLossEvent::~NetworkInterfaceMessageLossEvent() {

  }
    
  void NetworkInterfaceMessageLossEvent::consumeBlockEvent() {
    interface->transmissionFailure->reset();
    interface->dequeueOutgoingQueue();
    if (interface->outgoingQueue.size() > 0) {
      interface->send();
    }
    concernedBlock->scheduleLocalEvent(EventPtr(new NetworkInterfaceMessageLossEvent(this)));
  }
    
  const string NetworkInterfaceMessageLossEvent::getEventName() {
    return("NetworkInterfaceMessageLoss Event");
  }
}
