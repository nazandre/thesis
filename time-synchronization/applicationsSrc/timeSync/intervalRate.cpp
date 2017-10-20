#include "intervalRate.h"

#include <iostream>

using namespace std;
using namespace BaseSimulator;

ProbabilisticInterval::ProbabilisticInterval(double _a, double _b, double _p, ruint seed) {
  min = _a;
  max = _b;
  probability = _p;
  generator =  utils::Random::getUniformDoubleRNG(seed,min,max);
}
  
ProbabilisticInterval::ProbabilisticInterval(const ProbabilisticInterval &i) {
  min = i.min;
  max = i.max;
  probability = i.probability;
  generator = i.generator;
}

ProbabilisticInterval::~ProbabilisticInterval() {
  
}

IntervalRandomRate::IntervalRandomRate(BaseSimulator::ruint seed, std::vector<ProbabilisticInterval> &i): Rate() {
  intervals = i;
  probability = utils::Random::getUniformDoubleRNG(seed,0,1);
}

IntervalRandomRate::IntervalRandomRate(const IntervalRandomRate &r): Rate(), intervals(r.intervals), probability(r.probability) {
  
}

IntervalRandomRate::~IntervalRandomRate() {
  
}

double IntervalRandomRate::get() {
  double p = probability();
  size_t i = 0;
  
  while (i < intervals.size() && intervals[i].probability < p) {
    i++;
  }

  if (i == intervals.size()) {
    i--;
  }
  
  double rate = intervals[i].generator();

  //cout << rate << endl;
  
  return rate;
}

IntervalRandomRate* IntervalRandomRate::getBBModel(ruint seed) {
  uintRNG gen(seed);


  ProbabilisticInterval i1(24507.536605752, 26957.5340942629, 0.11508425811755/100.0, gen());
  ProbabilisticInterval i2(22465.7670351006, 24507.536605752, i1.probability + 14.5252774352651/100.0, gen());
  ProbabilisticInterval i3(20738.0413006186, 22465.7670351006, i2.probability + 55.1171393341554/100.0, gen());
  ProbabilisticInterval i4(19257.0791270677, 20738.0413006186, i3.probability + 27.1160432936019/100.0, gen());
  ProbabilisticInterval i5(17973.5379550517, 19257.0791270677, i4.probability + 3.05795314426634/100.0, gen());
  ProbabilisticInterval i6(16850.41, 17973.5379550517, 1.0, gen()); //i5.probability + 0.06850253459378/100.0);
  
  std::vector<ProbabilisticInterval> v;
  v.push_back(i1);
  v.push_back(i2);
  v.push_back(i3);
  v.push_back(i4);
  v.push_back(i5);
  v.push_back(i6);

  return new IntervalRandomRate(gen(),v);
}
