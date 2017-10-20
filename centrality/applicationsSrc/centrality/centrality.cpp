#include <iostream>
#include <trace.h>

#include "centralityBlockCode.h"

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
#include "simulation.h"

#include "logicalId.h"

using namespace std;

void parseCmdLine(int argc, char **argv);

int main(int argc, char **argv) {

  OUTPUT << "\033[1;33m" << "Starting Meld simulation (main) ..." << "\033[0m" << endl;
  
  Simulation::Parameters::parameters.parseCmdLine(argc,argv);
  
  CentralityBlockCode::moduleType = CommandLine::readModuleType(argc, argv);
  
  switch (CentralityBlockCode::moduleType) {
  case BB:
    BlinkyBlocks::createSimulator(argc, argv, CentralityBlockCode::buildNewBlockCode);
    break;
  case RB:
    RobotBlocks::createSimulator(argc, argv, CentralityBlockCode::buildNewBlockCode);
    break;
  case SB:
    SmartBlocks::createSimulator(argc, argv, CentralityBlockCode::buildNewBlockCode);
    break;
  case C2D:
    Catoms2D::createSimulator(argc, argv, CentralityBlockCode::buildNewBlockCode);
    break;
  case C3D:
    Catoms3D::createSimulator(argc, argv, CentralityBlockCode::buildNewBlockCode);
    break;

  default:
    cerr << "error: generic Meld for this module is not yet implemented..." << endl;
    break;
  }
  
  Simulator::getSimulator()->printInfo();

  deleteSimulator();

  OUTPUT << "\033[1;33m" << "end (main)" << "\033[0m" << endl;
  return(0);
}
