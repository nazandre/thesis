#ifndef LAYER_H_
#define LAYER_H_

#include "app/app.h"
#include "localClock.h"
#include "neighborhood/neighborhood.h"
#include "synchronization/synchronization.h"

//#include "utils/spanningTree.h"

class Layers {
public:
  
  BaseSimulator::BuildingBlock *module;
  LocalClock *localClock;
  Neighborhood *neighborhood;
  Synchronization::Node *synchronization;
  Application::Node *application;
  //SpanningTree *spanningTree;
  
  Layers();

  Layers(//Applications::Application *_application,
	 BaseSimulator::BuildingBlock *_module,
	 LocalClock *_localClock,
	 Neighborhood *_neighborhood,
	 Synchronization::Node *_synchronization,
	 Application::Node *_application);
  
  Layers(const Layers &l);

  ~Layers();
  
  void set(//Applications::Application *_application,
	   BaseSimulator::BuildingBlock *_module,
	   LocalClock *_localClock,
	   Neighborhood *_neighborhood,
	   Synchronization::Node *_synchronization,
	   Application::Node *_application);
};

#endif
