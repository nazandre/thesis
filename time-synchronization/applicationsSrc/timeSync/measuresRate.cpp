#include "measuresRate.h"

#include "utils/utils.h"

#include <iostream>

using namespace std;
using namespace BaseSimulator;

Measure::Measure(double v, double p) {
  value = v;
  probability = p;
}

Measure::Measure(const Measure &m) {
  value = m.value;
  probability = m.probability;
}

Measure::~Measure() {

}

std::vector<Measure> MeasuresBasedRandomRate::measures = setBBModel();
  
MeasuresBasedRandomRate::MeasuresBasedRandomRate(BaseSimulator::ruint seed): Rate() {
  probability = utils::Random::getUniformDoubleRNG(seed,0,1);
  gen = utils::Random::getUniformDoubleRNG(seed,0,0.500);
}

MeasuresBasedRandomRate::MeasuresBasedRandomRate(const MeasuresBasedRandomRate &r): Rate(), probability(r.probability), gen(r.gen) {
  
}

MeasuresBasedRandomRate::~MeasuresBasedRandomRate() {
  
}

double MeasuresBasedRandomRate::get() {
  double p = probability();
  size_t i = 0;
  double pCumul = 0;
  while (i < measures.size()) {

    pCumul += measures[i].probability;

    if (p < pCumul) {
      break;
    }
    
    i++;
  }

  if (i == measures.size()) {
    i--;
  }

  double time = measures[i].value;
  //time += gen();
  
  // L(t) = a*S(t) + b
  // b = 0 => L(t) = a*S(t)
  // S(t) = L(t) / a;
  
  //time = time / 0.991386;

  //cout << "value: " << time << endl;
  double rate = TIME_US_TO_RATE(21.0,time*pow(10,-3));
  //cout << "my rate: " << (17.0*8.0)/(time*pow(10,-3)) << endl;
  //cout << rate << endl;
  return rate;
}

std::vector<Measure> MeasuresBasedRandomRate::setBBModel() {

  /*
  Measure m1(5.0, 0.0/100.0);
  Measure m2(5.5, 12.0/100.0);
  Measure m3(6.0, 55.0/100.0);
  Measure m4(6.5, 22.0/100.0);
  Measure m5(7, 6.0/100.0);
  Measure m6(7.5, 4.0/100.0);
  */

  /*
  Measure m1(5.0, 0.11508425811755/100.0);
  Measure m2(5.5, 14.5252774352651/100.0);
  Measure m3(6.0, 53.1171393341554/100.0);
  Measure m4(6.5, 18.1160432936019/100.0);
  Measure m5(7, 0.05795314426634/100.0);
  Measure m6(7.5, 14.06850253459378/100.0);
  */
  
  /*
    // Bon en moyenne avec blocktick: 
    //Time p =  ONE_MILLISECOND; //500;
    //Time r = localTime%p;
    //Time res = 0;
    //Time us = 125 + layers.module->getRandomUint()%10;
    // delay: 0->0
    */
  /*
  Measure m1(5.0, 0.11508425811755/100.0);
  Measure m2(5.5, 14.5252774352651/100.0);
  Measure m3(6.0, 55.1171393341554/100.0);
  Measure m4(6.5, 20.1160432936019/100.0);
  Measure m5(7, 7.05795314426634/100.0);
  Measure m6(7.5, 3.06850253459378/100.0);
  */
  
  /* Original */
  Measure m1(5.0, 0.11508425811755/100.0);
  Measure m2(5.5, 14.5252774352651/100.0);
  Measure m3(6.0, 55.1171393341554/100.0);
  Measure m4(6.5, 27.1160432936019/100.0);
  Measure m5(7, 3.05795314426634/100.0);
  Measure m6(7.5, 0.06850253459378/100.0);
  
  std::vector<Measure> v;
  v.push_back(m1);
  v.push_back(m2);
  v.push_back(m3);
  v.push_back(m4);
  v.push_back(m5);
  v.push_back(m6);

  return v;
}
