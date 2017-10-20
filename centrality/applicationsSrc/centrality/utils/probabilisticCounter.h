#ifndef PROBABILISTIC_COUNTER_H_
#define PROBABILISTIC_COUNTER_H_

#include <cstdint>
#include <functional>
#include <vector>

#include "buildingBlock.h"
#include "def.h"
#include "simulation.h"

#define ELEM_T_SIZE_IN_BITS (sizeof(elem_t)*8)

// https://blog.demofox.org/2015/03/09/hyperloglog-estimate-unique-value-counts-like-the-pros/
// https://github.com/hideo55/cpp-HyperLogLog/blob/master/include/hyperloglog.hpp#L10
// https://fr.wikipedia.org/wiki/Algorithme_de_Flajolet%E2%80%93Martin
//https://github.com/svengato/FlajoletMartin/blob/master/FlajoletMartin.cpp

class ProbabilisticCounter {
public:
  typedef sysSize_t elem_t;
  
  static std::vector<hashFunc_t> hashFuncs;

  BaseSimulator::BuildingBlock *module;
  
  ProbabilisticCounter(BaseSimulator::BuildingBlock *b);
  ProbabilisticCounter(const ProbabilisticCounter &pc);
  virtual ~ProbabilisticCounter();
  
  virtual void add(elem_t e) = 0;
  virtual longSysSize_t getSizeEstimation() = 0;
  virtual void merge(ProbabilisticCounter &p) = 0;
  virtual ProbabilisticCounter* clone() = 0;
  virtual bool isEqual(ProbabilisticCounter *p) const = 0;

  virtual size_t size() = 0;
  virtual size_t sizeDataToSendInMsg() = 0;

  hash_t hash(unsigned int n,hash_t e,unsigned int s = 4);

  static hash_t affineHash(hash_t e,hash_t n, hash_t m);
  static hash_t murmur3Hash(const hash_t key,hash_t seed);
  static hash_t knuthHash(hash_t e);
  static hash_t fnv1Hash(hash_t e);
  static bool isPrime(hash_t n);
  static void initHashFuncs(unsigned int n);
  static unsigned int getNumHashFuncs();

};

#endif
