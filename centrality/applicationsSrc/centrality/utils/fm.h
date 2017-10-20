/*
 * flajolletMartin.h
 *
 *  Created on: 21 nov 2014
 *      Author: andre
 */

#ifndef FLAJOLLETMARTIN_H_
#define FLAJOLLETMARTIN_H_

#include "probabilisticCounter.h"
#include "def.h"

class FlajolletMartin: public ProbabilisticCounter {
public:
  typedef longSysSize_t bitString_t;

  std::vector<bitString_t> bitStrings;
  
  FlajolletMartin(BaseSimulator::BuildingBlock *b);
  //FlajolletMartin(bitString_t b);
  FlajolletMartin(const FlajolletMartin &fm);
  ~FlajolletMartin();
  
  longSysSize_t getSizeEstimation();
  longSysSize_t getSizeEstimation(byte_t i); // ith bitstring only
  void add(elem_t e);
  void merge(ProbabilisticCounter &pc);
  void merge(FlajolletMartin &fm,byte_t i);
  
  FlajolletMartin* clone();
  virtual bool isEqual(ProbabilisticCounter *p) const;

  size_t size();
  size_t sizeDataToSendInMsg();

  static void init(size_t availableSpace);
protected:
  void updateBitmap(byte_t i, bitString_t &b);
  sysSize_t rho(elem_t e);
  sysSize_t r(elem_t e);

};

#endif /* FLAJOLLET_MARTIN_H_ */
