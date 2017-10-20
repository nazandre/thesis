#ifndef MEASURES_BASED_RANDOM_RATE_H_
#define MEASURES_BASED_RANDOM_RATE_H_

#include "rate.h"
#include "random.h"

class Measure { //[a,b[
public:
  //double min;
  //double max;
  double value;
  double probability;
    
  Measure(double v, double p);
  Measure(const Measure &m);
  ~Measure();
};
  
class MeasuresBasedRandomRate: public BaseSimulator::Rate {
public:
  static std::vector<Measure> measures;
  BaseSimulator::doubleRNG probability;
  BaseSimulator::doubleRNG gen;
  
  MeasuresBasedRandomRate(BaseSimulator::ruint seed);
  MeasuresBasedRandomRate(const MeasuresBasedRandomRate &r);
  ~MeasuresBasedRandomRate();

  double get();

  static std::vector<Measure> setBBModel();
};


#endif
