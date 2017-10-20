#ifndef INTERVAL_RANDOM_RATE_H_
#define INTERVAL_RANDOM_RATE_H_

#include "rate.h"
#include "random.h"

class ProbabilisticInterval { //[a,b[
public:
  double min;
  double max;
  double probability;
  
  BaseSimulator::doubleRNG generator;
  
  ProbabilisticInterval(double _a, double _b, double _p, BaseSimulator::ruint seed);
  ProbabilisticInterval(const ProbabilisticInterval &i);
  ~ProbabilisticInterval();
};
  
class IntervalRandomRate: public BaseSimulator::Rate {
public:
  std::vector<ProbabilisticInterval> intervals;
  BaseSimulator::doubleRNG probability;
  
  IntervalRandomRate(BaseSimulator::ruint seed, std::vector<ProbabilisticInterval> &i);
  IntervalRandomRate(const IntervalRandomRate &r);
  ~IntervalRandomRate();

  double get();

  static IntervalRandomRate* getBBModel(BaseSimulator::ruint seed);
};


#endif
