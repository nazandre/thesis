#ifndef HYPER_LOGLOG_H_
#define HYPER_LOGLOG_H_

#include "probabilisticCounter.h"

//https://github.com/hideo55/cpp-HyperLogLog/blob/master/include/hyperloglog.hpp#L10

#include "def.h"

typedef uint32_t hll_elem_t;

class HyperLogLog: public ProbabilisticCounter {
public:
  static elem_t b_;
  static elem_t m_;
  static elem_t s_;
  static myDouble_t alpha_;
  static myDouble_t alphaMM_; ///< alpha * m^2
  
  std::vector<uint8_t> M_; ///< registers
  
  HyperLogLog(BaseSimulator::BuildingBlock *mo);
  HyperLogLog(const HyperLogLog &h);
  virtual ~HyperLogLog();

  virtual HyperLogLog* clone();
  virtual bool isEqual(ProbabilisticCounter *p) const; 

  static void init(size_t availableSpace);
  
  virtual void add(elem_t e);
  virtual longSysSize_t getSizeEstimation();
  virtual void merge(ProbabilisticCounter &p);

  size_t sizeOfRegisters();
  virtual size_t size();
  size_t sizeDataToSendInMsg();
};

class HyperLogLogHIP: public HyperLogLog {
public:
  static uint8_t register_limit_;
  myDouble_t c_;
  myDouble_t p_;

  HyperLogLogHIP(BaseSimulator::BuildingBlock *mo);
  HyperLogLogHIP(const HyperLogLogHIP &h);
  ~HyperLogLogHIP();
  virtual HyperLogLogHIP* clone();
  virtual bool isEqual(ProbabilisticCounter *p) const;
  
  void add(elem_t e);
  longSysSize_t getSizeEstimation();
  void merge(ProbabilisticCounter &p);
  size_t size();
};

#endif
