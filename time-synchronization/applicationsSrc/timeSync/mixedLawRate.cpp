#include "mixedLawRate.h"

#include "measuresRate.h"

#include "utils/utils.h"

#include <iostream>

using namespace std;
using namespace BaseSimulator;
using namespace Simulation;

MixedLawRate::MixedLawRate(BaseSimulator::ruint seed, commModel_t model): Rate() {

  // this model until now:
  //g1 = utils::Random::getNormalDoubleRNG(seed, 5.974, 0.35);

  switch(model) {
  case SPARSE_COMM_MODEL:
    g1 = utils::Random::getNormalDoubleRNG(seed, 5.973, 0.14);
    //g1 = utils::Random::getNormalDoubleRNG(seed, 5.98, 0.1);
    //g1 = utils::Random::getNormalDoubleRNG(seed, 5.97, 0.1225);    
    break;
  case INTERMEDIATE_COMM_MODEL:
    g1 = utils::Random::getNormalDoubleRNG(seed, 5.988, 0.20);
    break;
  case COMPACT_COMM_MODEL:
    g1 = utils::Random::getNormalDoubleRNG(seed, 6.075, 0.25);
    break;
  default:
    assert(false);
  }
  
  g2 = utils::Random::getUniformDoubleRNG(seed,0,0);
  g3 = utils::Random::getUniformDoubleRNG(seed,0,0);
}

MixedLawRate::MixedLawRate(const MixedLawRate &r): Rate(), g1(r.g1), g2(r.g2), g3(r.g3) {
  
}

MixedLawRate::~MixedLawRate() {
  
}

double MixedLawRate::get() {
  double time = g1(); // + g2() + g3(); //+ g2() + g3();

  // L(t) = a*S(t) + b
  // b = 0 => L(t) = a*S(t)
  // S(t) = L(t) / a;
  
  //time = time / 0.991386;
  //time = std::max(5.5,time);
		  
  double rate = TIME_US_TO_RATE(21,time*pow(10,-3));
  //double rate = TIME_US_TO_RATE(17,time*pow(10,-3));
  return rate;
}
