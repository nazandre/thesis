#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <cassert>

#include "clockNoise.h"

using namespace std;
using namespace BaseSimulator;
using namespace BaseSimulator::utils;

//#define DEBUG_CLOCKNOISE
//#define EXHAUSTIVE_SEARCH

//==============================================================================
//
//          ClockNoise  (class)
//
//==============================================================================

ClockNoise::ClockNoise() {}

ClockNoise::~ClockNoise() {}

//==============================================================================
//
//          GClockNoise  (class)
//
//==============================================================================

GClockNoise::GClockNoise(ruint seed, double mean, double sd) {
  generator = Random::getNormalDoubleRNG(seed,mean,sd);
}

GClockNoise::~GClockNoise() {}

clockNoise_t GClockNoise::getNoise(Time simTime) {
  return (clockNoise_t)generator();
}

//==============================================================================
//
//          DNoiseQClock  (class)
//
//==============================================================================

vector<DClockNoise::noiseSignal_t> DClockNoise::noiseSignals;
vector<DClockNoise::noiseSignal_t::iterator> DClockNoise::caches;

DClockNoise::DClockNoise(unsigned int _seed) {
  id = _seed % noiseSignals.size();
}

DClockNoise::~DClockNoise() {}

void DClockNoise::loadData(vector<string> &files) {
  Time time;
  clockNoise_t noise;

  noiseSignals.clear();
  for (unsigned int i = 0; i < files.size(); i++) {
    ifstream file (files[i].c_str());
    noiseSignal_t signal;
    if (file.is_open()) {
      while(file >> time >> noise) {
	referencePt_t p = make_pair(time,noise);
	signal.push_back(p);
      }
      file.close();
      noiseSignals.push_back(signal);
      caches.push_back(noiseSignals.back().begin());
    } else {
      cerr << "Unable to open file" << endl;
    }
  }
}

void DClockNoise::print() {
  int i = 0;
  for (vector<noiseSignal_t>::iterator it1 = noiseSignals.begin();
       it1 != noiseSignals.end(); it1++) {
    noiseSignal_t &signal = *it1;
    cout << "noise: " << i << endl;
    i++;
    for (vector<referencePt_t>::iterator it2 = signal.begin();
	 it2 != signal.end(); it2++) {
      referencePt_t &p = *it2;
      Time t = p.first;
      clockNoise_t n = p.second;
      cout << "\t" << t << " " << n << endl;
    }
  }
}

clockNoise_t DClockNoise::getNoise(Time simTime, referencePt_t p1, referencePt_t p2) {
  // assume linear noise between interval points [p1;p2]

  double t1,n1,t2,n2;
  t1 = p1.first;
  n1 = p1.second;
  t2 = p2.first;
  n2 = p2.second;
  double a = (n2-n1)/(t2-t1); 
  double b = n2 - a*t2;
  double n = round(a*(double)simTime + b);
  return n;
}


struct LessInterval {
  bool operator()(const DClockNoise::referencePt_t& fst,
		  const DClockNoise::referencePt_t& snd) {
    return fst.first < snd.first;
  }
};

clockNoise_t DClockNoise::getNoise(Time simTime) {
  
  if (id >= noiseSignals.size()) {
    cerr << "ERROR: wrong noise id (" << id << ")" << endl;
    return 0;
  }
  noiseSignal_t &signal = noiseSignals[id];
  noiseSignal_t::iterator &cache = caches[id];
  
  // identify interval "time" belongs to.

  if (signal.size() == 0) {
    cerr << "No signal for this noise id" << endl;
    return 0;
  }

  referencePt_t p1;
  referencePt_t p2;

  vector<referencePt_t>::iterator it;  
  
  // cache hit ?!
  if (cache->first >= simTime &&
      (cache == signal.begin() ||  std::prev(cache)->first < simTime)) {

    it = cache;

  } else {
#ifdef EXHAUSTIVE_SEARCH
    // dichotomic search in O(N)
    for (it = signal.begin();
	 it != signal.end(); it++) {
      referencePt_t &p = *it;
      Time t = p.first;
      if(t >= simTime) {
	break;
      }
    }
#else    
    // dichotomic search in O(log_2(N))
    it = std::lower_bound(signal.begin(), signal.end(),
			  std::make_pair(simTime,simTime),
			  LessInterval());
#endif
  }

#ifdef DEBUG_CLOCKNOISE  
  int cas = 0;
#endif
  if (it == signal.begin()) {
    p1 = make_pair(0,0);
    p2 = signal.front();
#ifdef DEBUG_CLOCKNOISE 
    cas = 1;
#endif
  } else if (it == signal.end()) {
    cerr << "Warning node signal shorter than the simulation" << endl;
    p1 = signal.back();
    p2 = make_pair(UINT64_MAX,0);
#ifdef DEBUG_CLOCKNOISE 
    cas = 2;
#endif    
  } else if (it->first == simTime) {
    p1 = *it;
    p2 = *it;
    //return it->second;
  } else {
    p2 = *it;
    it--;
    p1 = *it;
#ifdef DEBUG_CLOCKNOISE 
    cas = 4;
#endif
  }
  
  clockNoise_t noise = 0;
  if (p1 != p2) {
    noise = getNoise(simTime,p1,p2);
  } else {
    noise = p1.second;
  }

  /*doubleRNG white = utils::Random::getNormalDoubleRNG(id*simTime,0,500);
  double highFreqNoise = white();
  noise += highFreqNoise;
  */
  
  // add sinusoidal noise
  /*
  double A = 200000.0; // us
  double freq = 200.0;// hz
  double omega = 2.0*M_PI*freq;
  double time = simTime;
  //double phase = 90*(2.0*M_PI)/360.0;
  double v = omega*time;
  double highFreqNoise = A*std::sin(v);
  */
  //double highFreqNoise = noise/100;

  /*cout << "highFreqNoise ="
       << highFreqNoise
       << " simTime = "
       << simTime
       << endl;
       noise += highFreqNoise;*/
  
#ifdef DEBUG_CLOCKNOISE   
  cout << "getNoise(" << id << "," << simTime << ") = "  << noise 
       << ", time in [" << p1.first << "," << p2.first << "]"
       << ", noise in [" << p1.second << "," << p2.second << "]"
       << "(cas:" << cas
       << endl;
#endif
  return noise;
}

unsigned int DClockNoise::getNoiseId() {
  return id;
}
