#ifndef MIXED_LAW_RATE_H_
#define MIXED_LAW_RATE_H_

#include "rate.h"
#include "random.h"
#include "simulation.h"

class MixedLawRate: public BaseSimulator::Rate {
public:
  BaseSimulator::doubleRNG g1;
  BaseSimulator::doubleRNG g2;
  BaseSimulator::doubleRNG g3;
  
  MixedLawRate(BaseSimulator::ruint seed, Simulation::commModel_t model);
  MixedLawRate(const MixedLawRate &r);
  ~MixedLawRate();

  double get();
};


#endif
