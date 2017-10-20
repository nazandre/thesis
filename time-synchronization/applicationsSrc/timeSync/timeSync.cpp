#include <iostream>
#include <trace.h>

#include "timeSyncBlockCode.h"

#include "robotBlocksSimulator.h"
#include "robotBlocksBlockCode.h"
#include "smartBlocksSimulator.h"
#include "smartBlocksBlockCode.h"
#include "blinkyBlocksSimulator.h"
#include "blinkyBlocksBlockCode.h"
#include "catoms2DSimulator.h"
#include "catoms2DBlockCode.h"
#include "catoms3DSimulator.h"
#include "catoms3DBlockCode.h"

#include "commandLine.h"
#include "configStat.h"
#include "simulation.h"
#include "mrtp/mrtp.h"
#include "ad/ad.h"

using namespace std;

#define APP_BLOCK_CODE TimeSyncBlockCode::buildNewBlockCode

int main(int argc, char **argv) {
	
  OUTPUT << "\033[1;33m"
	 << "Starting Blinky Blocks simulation (main) ..."
	 << "\033[0m"
	 << endl;
  
  Simulation::Parameters::parameters.parseCmdLine(argc,argv);

  /*
  Synchronization::Node::_init("");
  
#ifdef MRTP
  Synchronization::MRTPNode::init(Simulation::Parameters::parameters.protocolConfig);
#endif
  
#ifdef AD
  Synchronization::ADNode::init(Simulation::Parameters::parameters.protocolConfig);
#endif
  */
  
  ModuleType moduleType = CommandLine::readModuleType(argc, argv);

  switch (moduleType) {
  case BB:
    BlinkyBlocks::createSimulator(argc, argv, APP_BLOCK_CODE);
    break;
  case RB:
    RobotBlocks::createSimulator(argc, argv, APP_BLOCK_CODE);
    break;
  case SB:
    SmartBlocks::createSimulator(argc, argv, APP_BLOCK_CODE);
    break;
  case C2D:
    Catoms2D::createSimulator(argc, argv, APP_BLOCK_CODE);
    break;
  case C3D:
    Catoms3D::createSimulator(argc, argv, APP_BLOCK_CODE);
    break;
  default:
    cerr << "error: generic timeSync for this module is not yet implemented..." << endl;
    break;
  }
  
  {
    using namespace BaseSimulator;

    Simulator *s = Simulator::getSimulator();
    s->printInfo();
  }

  Simulator::getSimulator()->printInfo();
  BaseSimulator::getWorld()->printInfo();
		
  deleteSimulator();
	
  OUTPUT << "\033[1;33m" << "end (main)" << "\033[0m" << endl;
  return(0);
}
