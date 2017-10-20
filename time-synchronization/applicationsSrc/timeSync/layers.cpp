#include "layers.h"


Layers::Layers() {
  module = NULL;
  neighborhood = NULL;
  application = NULL;
  synchronization = NULL;
  localClock = NULL;
}

Layers::Layers(//Applications::Application *_application,
	       BaseSimulator::BuildingBlock *_module,
	       LocalClock *_localClock,
	       Neighborhood *_neighborhood,
	       Synchronization::Node *_synchronization,
	       Application::Node *_application) :
  application(_application),
  module(_module),
  neighborhood(_neighborhood),
  synchronization(_synchronization),
  localClock(_localClock)
{}

Layers::Layers(const Layers &l) :
  application(l.application),
  module(l.module),
  localClock(l.localClock),
  neighborhood(l.neighborhood),
  synchronization(l.synchronization)
{}

Layers::~Layers() {
  //delete application;
  //delete neighborhood;
  //delete application;
  //delete synchronization;
  //delete localClock;
}

void Layers::set(//Applications::Application *_application,
		 BaseSimulator::BuildingBlock *_module,
		 LocalClock *_localClock,
		 Neighborhood *_neighborhood,
		 Synchronization::Node *_synchronization,
		 Application::Node *_application) {
  application = _application;
  module = _module;
  localClock = _localClock,
  neighborhood = _neighborhood;
  synchronization = _synchronization;
}
