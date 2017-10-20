#include <iostream>
#include "random.h"

using namespace std;

namespace BaseSimulator {
namespace utils {

doubleRNG Random::getUniformDoubleRNG(ruint seed, rdouble min, rdouble max) {
  uintRNG gen(seed);
  uniform_real_distribution<rdouble> dis (min,max);
  doubleRNG generator = bind(dis,gen);
  return generator;
}

doubleRNG Random::getNormalDoubleRNG(ruint seed, rdouble mean, rdouble sd) {
  uintRNG gen(seed);
  normal_distribution<rdouble> dis (mean,sd);
  doubleRNG generator = bind(dis,gen);
  return generator;
}

intRNG Random::getUniformIntRNG(ruint seed, rint min, rint max) {
  uintRNG gen(seed);
  uniform_int_distribution<rint> dis (min,max);
  intRNG generator = bind(dis,gen);
  return generator;
}

doubleRNG Random::getExponentialDoubleRNG(ruint seed, rdouble mean) {
  uintRNG gen(seed);
  double lambda = 1.0/mean;
  exponential_distribution<rdouble> dis (lambda);
  doubleRNG generator = bind(dis,gen);
  return generator;
}

intRNG Random::getPoissonIntRNG(ruint seed, rdouble mean) {
  uintRNG gen(seed);
  poisson_distribution<rint> dis (mean);
  intRNG generator = bind(dis,gen);
  return generator;
}
  
}
}
