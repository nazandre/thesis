#include "appStats.h"

#include <iostream>
#include <limits>
#include <cmath>
#include <cassert>

using namespace std;

#define UNDEFINED_VALUE -1.0

namespace AppStats {

  bool Collector::msgPrinted = false;
  
  Collector::Collector(std::string n) {
    name = n;
  }

  Collector::Collector(const Collector &c) {
    name = c.name;
  }
  
  Collector::~Collector() {}

  void Collector::checkAndPrintTitle() {
    if (!msgPrinted) {
      printTitle();
      msgPrinted = true;
    }
  }
  
  void Collector::printTitle() {
      cerr << "=== APP STATISTICS ===" << endl;
      cerr << "Format"
	   << "\t Collector: \"parameter: min mean max standard-deviation\"\n"
	   << "\t Counter: \"parameter: min mean max\"\n"
	   << "\t Boolean Occurences: \"parameter: #true #false total\""
	   << endl;
  }

  // Generic Collector

  GenericCollector::GenericCollector(std::string n): Collector(n) {
    min = UNDEFINED_VALUE;
    max = UNDEFINED_VALUE;
    mean = UNDEFINED_VALUE;
    sd = UNDEFINED_VALUE;
  }

  GenericCollector::GenericCollector(const GenericCollector &gc): Collector(gc) {
    min = gc.min;
    max = gc.max;
    mean = gc.mean;
    sd = gc.sd;
  }

  GenericCollector::~GenericCollector() {
    print();
  }
  
  void GenericCollector::print() {

    checkAndPrintTitle();
    
    cerr << name
	 << " (collector): "
	 << min << " "
	 << mean << " "
	 << max << " "
	 << sd
	 << endl;
  }
  
  // Basic Collector

  BasicCollector::BasicCollector(std::string n): GenericCollector(n) {
    numValues = 0;
    min = numeric_limits<long double>::max();
    max = numeric_limits<long double>::min();
  }
  
  BasicCollector::BasicCollector(const BasicCollector &bc): GenericCollector(bc) {
	
  }

  BasicCollector::~BasicCollector() {

  }

  void  BasicCollector::add(long double v) {
    numValues++;
    min = std::min(v,min);
    max = std::max(v,max);
    mean = ((numValues - 1) * mean + v) / numValues;
  }
  
  // Occurence/Frequency Collector
  FrequencyCollector::FrequencyCollector(std::string n): GenericCollector(n) {

  }

  FrequencyCollector::FrequencyCollector(const FrequencyCollector &fc):
    GenericCollector(fc) {
    occurences = fc.occurences;
  }

  FrequencyCollector::~FrequencyCollector() {
    setValues();
  }

  void FrequencyCollector::add(unsigned int index) {
    if (index >= occurences.size()) {
      occurences.resize(index+1,0);
      assert(occurences[index] == 0);
    }
    
    /*if (index > 1) {
      occurences[index-1]--;
      }*/
    
    occurences[index]++;
  }
  
  void FrequencyCollector::setValues() {

    long double size = 0;
    
    max = numeric_limits<long double>::min();
    min = numeric_limits<long double>::max();
    mean = 0;
    sd = 0;

    if (occurences.size() == 0) {
      return;
    }
    
    // mean computation
    for(unsigned int i = 0; i < occurences.size(); i++) {

      if (occurences[i] > 0 && i < min) {
	min = i;
      }
      if (occurences[i] > 0 && i > max) {
	max = i;
      }
    
      mean += i*occurences[i];
      size += occurences[i];
    }

    //cerr << "size: " << size << endl;
    mean /= size;

    // standard-deviation computation
    for(unsigned int i = 0; i < occurences.size(); i++) {
      long double v = i,
	n = occurences[i];
      sd += n * (v - mean) * (v - mean);
    }
    sd /= size;
    sd = sqrt(sd);
  }

  // All Values Collector
  template <class T>
  AllValuesCollector<T>::AllValuesCollector(std::string n): GenericCollector(n) {
    
  }

  template <class T>
   AllValuesCollector<T>::AllValuesCollector(const AllValuesCollector<T> &avc):
    GenericCollector(avc.name) {
    values = avc.values;
  }

  template <class T>
  AllValuesCollector<T>::~AllValuesCollector() {
    setValues();
  }
  
  template <class T>
  void AllValuesCollector<T>::setValues() {
    
    max = numeric_limits<long double>::min();
    min = numeric_limits<long double>::max();
    mean = 0;
    sd = 0;

    // mean computation
    for(unsigned int i = 0; i < values.size(); i++) {

      min = std::min(min,values[i]);
      max = std::max(max,values[i]);
      mean += values[i];
    }

    mean /= (long double) values.size();

    // standard-deviation computation
    for(unsigned int i = 0; i < values.size(); i++) {
      long double v = values[i];
      sd += (v - mean) * (v - mean);
    }
    
    sd /= (long double) values.size();
    sd = sqrt(sd);
  }

  template <class T>
  void AllValuesCollector<T>::add(T v) {
    values.push_back(v);
  }

  // Counter
  
  Counter::Counter(std::string n): Collector(n) {
    min = numeric_limits<unsigned int>::max();
    max = numeric_limits<unsigned int>::min();
  }
  
  Counter::Counter(const Counter &cc): Collector(cc) {
    
  }
  
  Counter::~Counter() {
    print();
  }

  void Counter::print() {

    checkAndPrintTitle();
    
    cerr << name
	 << " (counter): "
	 << min << " "
	 << max << " "
	 << endl;
  }

  // Basic Counter
   
  BasicCounter::BasicCounter(std::string n): Counter(n) {
    counter = 0;
  }
  
  BasicCounter::BasicCounter(const BasicCounter &cc): Counter(cc), counter(cc.counter) {
    
  }
  
  BasicCounter::~BasicCounter() {
   
  }
  
  void BasicCounter::incr() {
    counter++;
    max = std::max(counter,max); 
  }
  
  void BasicCounter::decr() {
    assert(counter>0);
    counter--;
    min = std::min(counter,min); 
  }

  // BooleanOccurencesx
  BooleanOccurences::BooleanOccurences(std::string n): Collector(n) {
    _true = 0;
    _false = 0;
  }

  BooleanOccurences::BooleanOccurences(const BooleanOccurences &bo): Collector(bo) {
    _true = bo._true;
    _false = bo._false;
  }
  
  BooleanOccurences::~BooleanOccurences() {
    print();
  }

  void BooleanOccurences::print() {

    checkAndPrintTitle();
    
    cerr << name
	 << " (boolean occurences): "
	 << _true << " "
	 << _false << " "
	 << _true + _false
	 << endl;
  }

  void BooleanOccurences::add(bool v) {
    if(v) {
      _true++;
    } else {
      _false++;
    }
  }
  
}

