#include "qclock.h"
#include "scheduler.h"

using namespace BaseSimulator;
using namespace BaseSimulator::utils;

//#define DEBUG_CLOCK
//#define DEBUG_CLOCK_SIM_TIME

//==============================================================================
//
//          QClock  (class)
//
//==============================================================================

QClock::QClock(double _d, double _y0, double _x0): Clock(), d(_d), y0(_y0), x0(_x0) {
}

/*QClock::QClock(double mean[], double sd[], unsigned int seed): Clock() {
  double values[2];
  for (int i = 0; i < 2; i++) {
    mt19937 uGenerator(seed);
    normal_distribution<double> normalDist(mean[i],sd[i]);
    auto generator = std::bind(normalDist, uGenerator);
    values[i] = generator();
  }
  d = values[0];
  y0 = values[1];
  x0 = BaseSimulator::getScheduler()->now();
  }*/

Time QClock::getTime(Time simTime) {
  double localTime = (1.0/2.0)*d*pow((double)simTime,2) + y0*((double)simTime) + x0;
  localTime = std::max(localTime,0.0);
  
  return localTime;
}

Time QClock::getSimulationTime(Time localTime) {
  double simTime = 0;
  double delta =  pow(y0,2) - 4 * (1.0/2.0)*d * (x0-(double)localTime);
  if (delta > 0) {
    double s1 = (-y0 + sqrt(delta)) / d;
    double s2 = (-y0 - sqrt(delta)) / d;
    // we take the value closest to localTime
    if (abs(s1-localTime) < abs(s2-localTime)) {
      simTime = s1;
    } else {
      simTime = s2;
    }
  } else if (delta == 0) {
        simTime = -y0 / d;
  } else {
    cerr << "delta should be positive!" << endl;
    simTime = 0;
  }
  
  simTime = max(0.0,simTime);
  simTime = min(numeric_limits<double>::max(),simTime);

#ifdef DEBUG_CLOCK
  cerr << "LocalTime: " << localTime
       << " => simTime (double): " << simTime
       << " => simTime (Time) :" << (Time) simTime
       << endl;
#endif
  
  return (Time) simTime;
}

//==============================================================================
//
//          GNoiseQClock  (class)
//
//==============================================================================

GNoiseQClock::GNoiseQClock(double _d, double _y0, double _x0,
                           double _sigma, ruint _seed): QClock(_d,_y0,_x0),
                                                               noise(new GClockNoise(_seed,0,_sigma)) {
}

/*GNoiseQClock::GNoiseQClock(double mean[], double sd[], unsigned int _seed): QClock(mean,sd,_seed) {
  mt19937 uGenerator(_seed);
  normal_distribution<double> normalDist(mean[2],sd[2]);
  auto generator = std::bind(normalDist, uGenerator);
  double sigma = generator();
  noise = new GClockNoise(_seed,0,sigma);
  }*/

GNoiseQClock::~GNoiseQClock () {
  delete noise;
}

Time GNoiseQClock::getTime(Time simTime) {
  double localTime = 0;
  double noise_SimTime = 0;
  
  double minL = 0;
  double maxL = numeric_limits<double>::max();
  list<ReferencePoint>::iterator it;
  
  cleanReferencePoints();
  for (it = referencePoints.begin(); it != referencePoints.end(); it++) {
    if (it->simulation == simTime) {
      return it->local;
    }	  
    if (it->simulation < simTime) {
      minL = max(minL,(double)it->local);
    }	  
    if (it->simulation > simTime) {
      maxL = min(maxL,(double)it->local);
      break; //sorted list
    }
  }

  noise_SimTime = noise->getNoise(simTime);
  
  localTime = (1.0/2.0)*d*pow((double)simTime,2) + y0*((double)simTime) + x0 + noise_SimTime;
  localTime = max(minL,localTime);
  localTime = min(maxL,localTime);

  insertReferencePoint((Time)localTime,simTime,it);

  return (Time)localTime;
}

Time GNoiseQClock::getSimulationTime(Time localTime) {
  double noise_LocalTime = 0;
  double simTime = 0;
    
  double minL = 0;
  double maxL = numeric_limits<double>::max();
  list<ReferencePoint>::iterator it;

  cleanReferencePoints();
  for (it = referencePoints.begin(); it != referencePoints.end(); it++) {
    if (it->local == localTime) {
      return it->simulation;
    }
	  
    if (it->local < localTime) {
      minL = max(minL,(double)it->simulation);
    }
	  
    if (it->local > localTime) {
      maxL = min(maxL,(double)it->simulation);
      break; //sorted list
    }
  }
 
  noise_LocalTime = noise->getNoise(localTime);
  
  double delta =  pow(y0,2) - 4 * (1.0/2.0)*d * (x0+noise_LocalTime-(double)localTime);
  if (delta > 0) {
    double s1 = (-y0 + sqrt(delta)) / d;
    double s2 = (-y0 - sqrt(delta)) / d;
    // we take the value closest to localTime
    if (abs(s1-localTime) < abs(s2-localTime)) {
      simTime = s1;
    } else {
      simTime = s2;
    }
  } else if (delta == 0) {
    simTime = -y0 / d;
  } else {
    cerr << "delta should be positive!" << endl;
    simTime = minL;
  }

  simTime = max(minL,simTime);
  simTime = min(maxL,simTime);

  insertReferencePoint(localTime,(Time)simTime,it);

  return (Time) simTime;
}

void GNoiseQClock::insertReferencePoint(Time local, Time simulation,
					list<ReferencePoint>::iterator pos) {
 ReferencePoint p = ReferencePoint(local,simulation); 
  if (pos == referencePoints.end()) {
    referencePoints.push_back(p);
  } else {
    referencePoints.insert(pos,p);
  }
}

void GNoiseQClock::cleanReferencePoints() {
  Time simTime = getScheduler()->now();
  
  for (list<ReferencePoint>::iterator it = referencePoints.begin();
       it != referencePoints.end(); it++) {
    if (it->simulation >= simTime) {
      if (it != referencePoints.begin()) {
	it--;
	referencePoints.erase(referencePoints.begin(),it);
      }
      break;
    }
  }
  for (list<ReferencePoint>::iterator it = referencePoints.begin();
       it != referencePoints.end(); it++) {
  }
}

//==============================================================================
//
//          DNoiseQClock  (class)
//
//==============================================================================

DNoiseQClock::DNoiseQClock(double _d, double _y0, double _x0, ruint _seed): QClock(_d,_y0,_x0),
                                                                                   noise(new DClockNoise(_seed)) {

}

//DNoiseQClock::DNoiseQClock(double mean[], double sd[], ruint _seed): QClock(mean,sd,_seed),
//                                                                            noise(new DClockNoise(_seed)) {
//}

DNoiseQClock::~DNoiseQClock() {
  delete noise;
}

void DNoiseQClock::loadNoiseData(vector<string> &noiseData) {
  DClockNoise::loadData(noiseData);
}

Time DNoiseQClock::getTime(Time simTime) {
  long double noise_SimTime = noise->getNoise(simTime);
  long double localTime = (1.0/2.0)*d*pow((long double)simTime,2) + y0*((long double)simTime) + x0 + noise_SimTime;
  localTime = max((long double)0.0,localTime);

#ifdef DEBUG_CLOCK
  cout << "----------" << endl;
  cout << "DNoiseQClock: simTime = " << simTime << endl;
  cout << "d = " << d << endl;
  cout << "y0 = " << y0 << endl;
  cout << "x0 = " << x0 << endl;
  cout << "noise = " << noise_SimTime << endl;
  cout << "localtime = " << (Time) localTime << endl;
  cout << "----------" << endl;
#endif
  return localTime;
}

Time DNoiseQClock::getSimulationTime(Time localTime) {
  Time s = QClock::getSimulationTime(localTime);
  Time l = getTime(s);
  int64_t diff =  0;
  int64_t prevDiff = 0;
  
#ifdef DEBUG_CLOCK_SIM_TIME
  int i = 1;
  Time sInit = s;
  Time lInit = l;
  cerr << "##############\nin" << endl;
#endif
  
  while((l != localTime) && !(prevDiff == 1 && diff == -1)) {
    
    prevDiff = diff;
    diff = ((int64_t)l) - ((int64_t)localTime);

#ifdef DEBUG_CLOCK_SIM_TIME
    cerr << "i: " << i << endl;
    cerr << "l: " << l << endl;
    cerr << "localTime: " << localTime << endl;
    cerr << "diff: " << diff << endl;
    cerr << "s: " << s << endl;
    cerr << "noise id: " << getNoiseId() << endl;
    cerr << "noise(s)" << noise->getNoise(s) << endl;
    cerr << "Sinit: " << sInit << endl;
    cerr << "Linit: " << lInit << endl;
    cerr << "============" << endl;
    i++;
#endif
    
    if (diff > (int64_t)s) {
      return 0;
    }
    
    s -= diff;
    
    l = getTime(s);
  }
#ifdef DEBUG_CLOCK_SIM_TIME
  cerr << "FOUND!!!!" << endl;
#endif 
  return s;
}

unsigned int DNoiseQClock::getNoiseId() {
  return noise->getNoiseId();
}

#define XMEGA_RTC_OSC1K_CRC_NB_NOISE 5
#define XMEGA_RTC_OSC1K_CRC_NOISE_PATH "../../simulatorCore/resources/clockNoise/XMEGA_RTC_OSC1K_CRC_7H/"

//#define XMEGA_RTC_OSC1K_CRC_NB_NOISE 6
//#define XMEGA_RTC_OSC1K_CRC_NOISE_PATH "../../simulatorCore/resources/clockNoise/XMEGA_RTC_OSC1K_CRC_7H_6_NOISES/"

DNoiseQClock* DNoiseQClock::createXMEGA_RTC_OSC1K_CRC(ruint seed) {
  static bool noiseDataLoaded = false;
  if (!noiseDataLoaded) {
    vector<string> files;
    for (int i = 1; i <=  XMEGA_RTC_OSC1K_CRC_NB_NOISE; i++) {
      stringstream file;
      file << XMEGA_RTC_OSC1K_CRC_NOISE_PATH << i << ".dat";
      files.push_back(file.str());
    }
    loadNoiseData(files);
    noiseDataLoaded = true;
  }
  
  // us 7 hours
  // 5 noises
  static double mean[2] = {6.295527*pow(10,-14), 0.991386};
  static double sd[2] = {2.119277*pow(10,-14), 0.002077121};

  // 6 noises
  //static double mean[2] = {6.435369*pow(10,-14), 0.9911936};
  //static double sd[2] = {1.926241*pow(10,-14), 0.001916697};
  
  /*static double mean[2] = {6.435369*pow(10,-14), 0.9911936};
  static double sd[2] = {1.926241*pow(10,-14), 0.001916697};
  
  static double minV[2] = {3.485985*pow(10,-14), 0.9883691};
  static double maxV[2] = {8.638147*pow(10,-14), 0.9935956};
  */
  
  // us (4h long experiment)
  /*static double mean[2] = {7.132315*pow(10,-14), 0.9911011};
  static double sd[2] = {5.349995*pow(10,-14), 0.002114563};

  static double minV[2] = {-2.833827*pow(10,-14), 0.9879874};
  static double maxV[2] = {1.318722*pow(10,-14), 0.9935429};
  */
  
  //us (1h30 long exp)
  /*static double mean[2] = {4.438784*pow(10,-14),0.9912007};
  static double sd[2] = {2.952058*pow(10,-14), 0.001976749};

  static double minV[2] = {1.281936*pow(10,-14), 0.9882617};
  static double maxV[2] = {9.305573*pow(10,-14), 0.9935013};
  */
  
  static doubleRNG d = Random::getNormalDoubleRNG(seed,mean[0],sd[0]); 
  static doubleRNG y0 = Random::getNormalDoubleRNG(seed,mean[1],sd[1]);
  
  //static doubleRNG d = Random::getUniformDoubleRNG(seed,minV[0],maxV[0]);
  //static doubleRNG y0 = Random::getUniformDoubleRNG(seed,minV[1],maxV[1]);
  
  double x0 = 0; //- (double) getScheduler()->now();

  return new DNoiseQClock(d(),y0(),x0,seed);
}
