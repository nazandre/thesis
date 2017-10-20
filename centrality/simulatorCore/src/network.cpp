/*
 * network.cpp
 *
 *  Created on: 24 mars 2013
 *      Author: dom
 */

#include <iostream>
#include <sstream>

#include "scheduler.h"
#include "network.h"
#include "trace.h"
#include "statsIndividual.h"
#include "utils.h"
#include "messageFailure.h"

//#define TRANSMISSION_TIME_DEBUG

using namespace std;
using namespace BaseSimulator;
using namespace BaseSimulator::utils;

uint64_t Message::nextId = 0;
uint64_t Message::nbMessages = 0;

unsigned int P2PNetworkInterface::nextId = 0;
int P2PNetworkInterface::defaultDataRate = 1000000;

//===========================================================================================================
//
//          Message  (class)
//
//===========================================================================================================

Message::Message() {
	id = nextId;
	nextId++;
	nbMessages++;
	accountForInStats = true;
	MESSAGE_CONSTRUCTOR_INFO();
}

Message::~Message() {
	MESSAGE_DESTRUCTOR_INFO();
	nbMessages--;
}

uint64_t Message::getNbMessages() {
	return(nbMessages);
}

string Message::getMessageName() {
	return("generic message");
}

Message* Message::clone() {
    Message* ptr = new Message();
    ptr->sourceInterface = sourceInterface;
    ptr->destinationInterface = destinationInterface;
    ptr->type = type;
    ptr->accountForInStats = accountForInStats;
    return ptr;
}

//===========================================================================================================
//
//          P2PNetworkInterface  (class)
//
//===========================================================================================================

P2PNetworkInterface::P2PNetworkInterface(BaseSimulator::BuildingBlock *b) {
#ifndef NDEBUG
	OUTPUT << "P2PNetworkInterface constructor" << endl;
#endif
	hostBlock = b;
	connectedInterface = NULL;
	availabilityDate = 0;
	globalId = nextId;
	nextId++;
	dataRate = new StaticRate(defaultDataRate);
	transmissionFailure = new ZeroTransmissionFailure();
}

void P2PNetworkInterface::setDataRate(Rate *r) {
	assert(r != NULL);
	delete dataRate;
	dataRate = r;
}

void P2PNetworkInterface::setTransmissionFailure(TransmissionFailure *tf) {
	assert(tf != NULL);
	delete transmissionFailure;
	transmissionFailure = tf;
}

P2PNetworkInterface::~P2PNetworkInterface() {
#ifndef NDEBUG
	OUTPUT << "P2PNetworkInterface destructor" << endl;
#endif
	delete dataRate;
	delete transmissionFailure;
}

void P2PNetworkInterface::send(Message *m) {
  BlockCode *bc = hostBlock->blockCode;
  
  if (bc->availabilityDate < getScheduler()->now()) {
    bc->availabilityDate = getScheduler()->now();
  }

  bc->availabilityDate += 25;
  
  send(bc->availabilityDate,m);
}

void P2PNetworkInterface::send(Time t, Message *m) {
  getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(t, m, this));
}

bool P2PNetworkInterface::addToOutgoingBuffer(MessagePtr msg) {
	stringstream info;

	if (connectedInterface != NULL) {
		outgoingQueue.push_back(msg);

		if (msg->accountForInStats) {
		  BaseSimulator::utils::StatsIndividual::incOutgoingMessageQueueSize(hostBlock->stats);
		}
		
		if (availabilityDate < BaseSimulator::getScheduler()->now()) {
		  availabilityDate = BaseSimulator::getScheduler()->now();
		}
		
		if (outgoingQueue.size() == 1 && messageBeingTransmitted == NULL) { //TODO
		  BaseSimulator::getScheduler()->schedule(new NetworkInterfaceStartTransmittingEvent(availabilityDate,this));
		}
		
		return(true);
	} else {
		info.str("");
		info << "*** WARNING *** [block " << hostBlock->blockId << ",interface " << globalId <<"] : trying to enqueue a Message but no interface connected";
		BaseSimulator::getScheduler()->trace(info.str());

		cerr << "Message "
		     << "(" << msg->getMessageName() << ")"
		     << " from " << hostBlock->blockId
		     << " enqueue into the buffer of an unconnected interface!"
		     << endl;
		cerr << "@" <<  hostBlock->blockId << " num neighbors: " << hostBlock->getNbNeighbors() << endl;
		hostBlock->printNeighborIds();
		getchar();

		return(false);
	}
}

void P2PNetworkInterface::send() {
	MessagePtr msg;
	stringstream info;
	Time now = std::max(availabilityDate,BaseSimulator::getScheduler()->now());
	Time transmissionDuration = 0;

	assert(availabilityDate <= now);
	// the message is not sent until the block is ready 
	now = max(now,hostBlock->blockCode->availabilityDate);
	
	if (!connectedInterface) {
		info << "*** WARNING *** [block " << hostBlock->blockId << ",interface " << globalId <<"] : trying to send a Message but no interface connected";
		BaseSimulator::getScheduler()->trace(info.str());

		cerr << "Message "
		     << "(" << msg->getMessageName() << ")"
		     << " from " << hostBlock->blockId
		     << " sent through an unconnected interface!"
		     << endl;
		
		return;
	}

	if (outgoingQueue.size()==0) {
		info << "*** ERROR *** [block " << hostBlock->blockId << ",interface " << globalId <<"] : The outgoing buffer of this interface should not be empty !";
		BaseSimulator::getScheduler()->trace(info.str());
		exit(EXIT_FAILURE);
	}
        
	// Number of transmissions ?
	if (transmissionFailure->getNumFailures() == 0) { // first trial
		transmissionFailure->drawNumFailures(); // deterministic random (or not) draw
		unsigned int numFailures = transmissionFailure->getNumFailures();
	  	Time detectionDelay = transmissionFailure->getDetectionDelay();
	  
	  	if (transmissionFailure->isDefinitelyLost()) {
	    		Time lossDetectionDelay = numFailures * detectionDelay;
	    		BaseSimulator::getScheduler()->schedule(new NetworkInterfaceMessageLossEvent(now +
										     lossDetectionDelay,
										     hostBlock,
										     outgoingQueue.front(),
										     this));
			return;
		}

		if (numFailures > 0) {
		  	Time successfulTransmissionStartDelay = numFailures * detectionDelay;
		  	BaseSimulator::getScheduler()->schedule(new NetworkInterfaceStartTransmittingEvent(now + successfulTransmissionStartDelay,this));
		  	return;
		}
	}
	transmissionFailure->reset();
	
	msg = outgoingQueue.front();
	messageBeingTransmitted = msg;
	messageBeingTransmitted->sourceInterface = this;
	messageBeingTransmitted->destinationInterface = connectedInterface;
	
	messageBeingTransmitted->beginningTransmissionMacLayer(); // WARNING: take into consideration "now" time variable
	
	transmissionDuration = getTransmissionDuration(messageBeingTransmitted);
	
#ifdef TRANSMISSION_TIME_DEBUG
	cerr << "Message size (bytes): " << messageBeingTransmitted->size() << endl;
	cerr << "Data rate (bit/s): " << dataRate->get() << endl;
	cerr << "Message transmission duration (us): " << transmissionDuration
	     << endl;
#endif	

	Time receptionTime = now+transmissionDuration;
	availabilityDate = receptionTime;
	
	BaseSimulator::getScheduler()->schedule(new NetworkInterfaceStopTransmittingEvent(receptionTime, this));

	if (msg->accountForInStats) {
	  StatsCollector::getInstance().incMsgCount();
	  StatsIndividual::incSentMessageCount(hostBlock->stats);
	}
}

void P2PNetworkInterface::connect(P2PNetworkInterface *ni) {
	// test ajouté par Ben, gestion du cas : connect(NULL)
	if (ni) { // Connection
		if (ni->connectedInterface != this) {
			if (ni->connectedInterface != NULL) {
				OUTPUT << "ERROR : connecting to an already connected P2PNetwork interface" << endl;
				ni->connectedInterface->hostBlock->removeNeighbor(ni->connectedInterface);
				ni->hostBlock->removeNeighbor(ni);
			}
			ni->connectedInterface = this;
			hostBlock->addNeighbor(ni->connectedInterface, ni->hostBlock);
			ni->hostBlock->addNeighbor(ni, ni->connectedInterface->hostBlock);
		}
	} else if (connectedInterface != NULL) {
		// disconnect this interface and the remote one
		hostBlock->removeNeighbor(this);
		connectedInterface->hostBlock->removeNeighbor(connectedInterface);
		connectedInterface->connectedInterface = NULL;
	}
	connectedInterface = ni;
}

Time P2PNetworkInterface::getTransmissionDuration(MessagePtr &m) {
  double rate = dataRate->get();
  Time transmissionDuration = (m->size()*8000000ULL)/rate;
  assert(rate > 0);
  return transmissionDuration;
}

bool P2PNetworkInterface::isConnected() {
  return connectedInterface != NULL;
}

void P2PNetworkInterface::dequeueOutgoingQueue() {
  if (outgoingQueue.front()->accountForInStats) {
    BaseSimulator::utils::StatsIndividual::decOutgoingMessageQueueSize(hostBlock->stats);
  }
  outgoingQueue.pop_front();
}
