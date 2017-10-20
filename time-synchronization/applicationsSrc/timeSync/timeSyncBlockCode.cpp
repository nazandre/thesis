/*
 * timeSyncBlockCode.cpp
 *
 *  Created on: 26 mars 2013
 *      Author: dom
 */

#include <iostream>
#include <sstream>

#include "timeSyncBlockCode.h"

#include "scheduler.h"
#include "network.h"
#include "messageFailure.h"
#include "configStat.h"
#include "trace.h"
#include <fstream>

#include "timeMessage.h"
#include "appMsgID.h"
#include "appEventsID.h"
#include "utils/def.h"
#include "simulation.h"

#include "neighborhood/neighborhood.h"
#include "myNeighborhood.h"

#include "mrtp/mrtp.h"
#include "ad/ad.h"
#include "ats/ats.h"
#include "ftsp/ftsp.h"
#include "wmts/wmts.h"

#include "synchronization/synchronizationEv.h"

#include "app/app.h"

//#define TEST_RETRANSMISSION_MODEL

//#define DEBUG_MESSAGE_LOSSES

//#define ENABLE_APPLICATION
//#define ENABLE_NEIGHBORHOOD_KEEPALIVE

using namespace std;
using namespace BaseSimulator;

TimeSyncBlockCode::TimeSyncBlockCode(BuildingBlock *host): BlockCode(host) {
  OUTPUT << "TimeSyncBlockCode constructor" << endl;

  msgHandler = NULL;

  layers.module = host;
  layers.neighborhood = new Neighborhood(layers);
  layers.localClock = new LocalClock(layers);
  
  switch(Simulation::Parameters::parameters.protocol) {
  case Simulation::protocol_t::MRTP:
  case Simulation::protocol_t::TPSN:
  case Simulation::protocol_t::TPSN_MLE:
  case Simulation::protocol_t::TREE:
  case Simulation::protocol_t::CONVERGECAST:
    layers.synchronization = new Synchronization::MRTPNode(layers);
    break;
  case Simulation::protocol_t::FTSP:
  case Simulation::protocol_t::PULSE_SYNC:
    layers.synchronization = new Synchronization::FTSPNode(layers);
    break;
  case Simulation::protocol_t::AD:
    layers.synchronization = new Synchronization::ADNode(layers);
    break;
  case Simulation::protocol_t::ModifiedATS:
    layers.synchronization = new Synchronization::ATSNode(layers);
    break;
  case Simulation::protocol_t::WMTS:
    layers.synchronization = new Synchronization::WMTSNode(layers);
    break;
  default:
    assert(false);
  }

#ifdef ENABLE_APPLICATION
  layers.application = new Application::Node(layers);
#endif
}

TimeSyncBlockCode::~TimeSyncBlockCode() {
  OUTPUT << "TimeSyncBlockCode destructor" << endl;
}

void TimeSyncBlockCode::init() {
  Simulation::Parameters::parameters.setParameters(layers.module);  
}

void TimeSyncBlockCode::startup() {
  stringstream info;
  static bool errorMeasurementEnabled = false;

  if (!errorMeasurementEnabled) {
    errorMeasurementEnabled = true;
    ErrorMeasurementEvent::init(); // file + schedule first error mesurement
  }
  
  info << "  Starting TimeSyncBlockCode in block " << hostBlock->blockId;
  BaseSimulator::getScheduler()->trace(info.str(),hostBlock->blockId);

  crcEscapeGenerator = utils::Random::getUniformDoubleRNG(hostBlock->blockId, 0.0, 1.0);

  msgHandler = new MyMessageHandler(layers, 0); //layers.module->getRandomUint());
  
  init();

#ifdef ENABLE_NEIGHBORHOOD_KEEPALIVE
  NeighborhoodKeepAliveEvent::init(layers);
#endif

#ifdef ENABLE_APPLICATION
  layers.application->startup();
#endif
  
#ifdef TEST_RETRANSMISSION_MODEL
  // Test for retransmissions!
  if (hostBlock->blockId == 1) {
    P2PNetworkInterface *n = NULL;
    for (int i = 0; i < 6; i++) {
      n = hostBlock->getInterface(i);
      if (n->isConnected()) {
	break;
      }
    }
    
    for (int i = 0; i < 3; i++) {
      Message *m = new TimeMessage();
      n->send(m);
    }
  }
#endif

  assert(layers.synchronization);
  layers.synchronization->startup();
}

void TimeSyncBlockCode::processLocalEvent(EventPtr pev) {
  stringstream info;
  info.str("");
	
  OUTPUT << hostBlock->blockId << " processLocalEvent: date: "<< BaseSimulator::getScheduler()->now() << " process event " << pev->getEventName() << "(" << pev->eventType << ")" << ", random number : " << pev->randomNumber << endl;

  switch (pev->eventType) {
  case EVENT_SET_COLOR:
    {
      Color color = (std::static_pointer_cast<SetColorEvent>(pev))->color;
      hostBlock->setColor(color);
      info << "set color "<< color << endl;
    }
    break;
  case EVENT_TIME_MASTER_ELECTED:
  case EVENT_SPANNING_TREE_CONSTRUCTED:
  case EVENT_CONVERGECAST_TIME_MAX_DONE:
  case EVENT_TIME_SYNC:
    {
      assert(layers.synchronization);

      if (pev->eventType == EVENT_TIME_SYNC) {
	layers.synchronization->timeoutCall = true;
      }
      
      layers.synchronization->handle(pev);

      if (pev->eventType == EVENT_TIME_SYNC) {
	layers.synchronization->timeoutCall = false;
      }
    }
    break;
  case EVENT_NI_MESSAGE_LOSS:
    {
      MessagePtr msg = (std::static_pointer_cast<NetworkInterfaceMessageLossEvent>(pev))->message;
#ifdef DEBUG_MESSAGE_LOSSES
      MY_CERR << " Msg " << msg->id << " lost!" << endl;
#endif
    }
    break;
  case EVENT_NI_RECEIVE:
    {
      MessagePtr message = (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;
      P2PNetworkInterface * recvInterface = message->destinationInterface;
      switch(message->type) {
      case TIME_MSG_ID : {
	TimeMessagePtr msg = std::static_pointer_cast<TimeMessage>(message);
#ifdef TEST_RETRANSMISSION_MODEL	
	MY_CERR << " receive msg " << msg->id << endl;
#endif	
      }
      	break;
      case BFS_TRAVERSAL_SP_GO:
      case BFS_TRAVERSAL_SP_GO_ACK:
      case BFS_TRAVERSAL_SP_BACK:
      case BFS_TRAVERSAL_SP_DELETE:
      case CTM_GO:
      case CTM_BACK:
      case MRTP_TIME_SYNC_MSG:
      case MRTP_TIME_SYNC_1_MSG:
      case MRTP_TIME_SYNC_2_MSG:
      case MRTP_TIME_SYNC_3_MSG:
      case AD_READCLOCK_1_MSG:
      case AD_READCLOCK_2_MSG:
      case AD_TIME_MSG:
      case ATS_TIME_MSG:
      case WMTS_TIME_MSG:
      case FTSP_TIME_SYNC_MSG:
      case GTSP_TIME_MSG:
	{
	  //MyMessage::scheduleHandleMsgEvent(layers,message);
	  msgHandler->scheduleHandleMsgEvent(message);

	  //break;
	  //assert(layers.synchronization);
	  //layers.synchronization->handle(message);
	}
	break;
      case APP_MSG:
	{
	  /*if (layers.module->blockId == 2) {
	    static Time last = 0;
	    MY_CERR << " since last: " << ((double)pev->date - last)/1000
		  << " ms with " << layers.module->getNbNeighbors() << " neighbors "
		  << endl;
	    last = pev->date;
	    }*/
	  //MY_CERR << "APP MSG" << endl;
	  layers.application->handle(message);
	}
	break;
      default: 
	ERRPUT << "*** ERROR *** : unknown message" << message->id << endl;
      }
    }
    break;
  case EVENT_APP_ROUND:
    {
      layers.application->handle(pev);
    }
    break;
  case EVENT_HANDLE_MSG:
    {
      MessagePtr message = (std::static_pointer_cast<HandleMsgEvent>(pev))->message;
      assert(layers.synchronization);
      layers.synchronization->handle(message);
      utils::StatsIndividual::decIncommingMessageQueueSize(layers.module->stats);
    }
    break;
  default:
    ERRPUT << "*** ERROR *** : unknown local event" << endl;
    break;
  }
  
  if (info.str() != "") {
    BaseSimulator::getScheduler()->trace(info.str(),hostBlock->blockId);
  }
}

BlockCode* TimeSyncBlockCode::buildNewBlockCode(BuildingBlock *host) {
  return(new TimeSyncBlockCode(host));
}
