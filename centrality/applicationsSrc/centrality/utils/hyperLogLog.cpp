#include "hyperLogLog.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <algorithm>
#include <limits>
#include <iostream>
#include <cassert>

#define HLL_ELEM_T_SIZE_IN_BITS (sizeof(hll_elem_t)*8)
#define _GET_CLZ(x, b) (std::min((hll_elem_t)b, (hll_elem_t)::__builtin_clz(x)) + 1)

//#define LSB_VERSION

using namespace std;

//https://blog.demofox.org/2015/03/09/hyperloglog-estimate-unique-value-counts-like-the-pros/
ProbabilisticCounter::elem_t HyperLogLog::b_ = 4; // # bits to identify register
ProbabilisticCounter::elem_t HyperLogLog::s_ = 5;  // register size
ProbabilisticCounter::elem_t HyperLogLog::m_ = 1 << b_; // # of registers

myDouble_t HyperLogLog::alpha_ = 0.673; // b = 4
//double HyperLogLog::alpha_ = 0.697; // b = 5

myDouble_t HyperLogLog::alphaMM_ = alpha_*pow(m_,2);

static const myDouble_t pow_2_32 = 4294967296.0; ///< 2^32
static const myDouble_t neg_pow_2_32 = -4294967296.0; ///< -(2^32)

// b = 4
// m = 2^4 = 16
// s = 5
// total in memory+msg = m*s = 16*5 = 80 bits = 10 bytes

HyperLogLog::HyperLogLog(BaseSimulator::BuildingBlock *mo): ProbabilisticCounter(mo), M_(m_,0) {
  //  M_.resize(m_,0);
}

HyperLogLog::HyperLogLog(const HyperLogLog& hll):
  ProbabilisticCounter(hll),
  M_(hll.M_)
{
  //assert(false);
}

HyperLogLog::~HyperLogLog() {

}

void HyperLogLog::init(size_t availableSpace) {
  // init data structure ? 
  //
  initHashFuncs(1);

  cerr << "HyperLogLog: " << m_ << " registers of " << s_ << " bits " << endl;
  //cerr << "Size of registers: " << size() << endl;
}

HyperLogLog* HyperLogLog::clone() {
  return new HyperLogLog(*this);
}

bool HyperLogLog::isEqual(ProbabilisticCounter *p) const {
  HyperLogLog *hll = (HyperLogLog*) p;
  unsigned int i =  0;
  
  if (M_.size() != hll->M_.size()) {
    return false;
  }

  for (i = 0; i < M_.size(); i++) {
    if (M_[i] != hll->M_[i]) {
      return false;
    }
  }
  return true;
}

void HyperLogLog::add(elem_t e) {
  hll_elem_t s = HLL_ELEM_T_SIZE_IN_BITS;
  hll_elem_t h = hash(0,e);

#ifdef LSB_VERSION
  hll_elem_t indexMask = m_ - 1;
  hll_elem_t index = h & indexMask; // register index
  hll_elem_t v = h >> b_;
  hll_elem_t k = (s - b_);
  hll_elem_t rank = 1+std::min((hll_elem_t)__builtin_ctz(v),k);
#else
  hll_elem_t k = (s - b_);
  hll_elem_t index = h >> k;
  hll_elem_t rank = _GET_CLZ((h << b_), k);
#endif
  //cerr << "h: " << h << endl;
  //cerr << "i: " << index << endl;
  //cerr << "v: " << v << endl;
  //cerr << "rank: " << rank << endl;

  hll_elem_t rankMax = 1 << s_;
  assert(rank < rankMax);
  assert(index < m_);
  if (rank > M_[index]) {
    M_[index] = rank;
  }
}

longSysSize_t HyperLogLog::getSizeEstimation() {
  myDouble_t estimate;
  myDouble_t sum = 0.0;

  for (uint8_t i = 0; i < m_; i++) {
    //cout << "M[" << i << "]" << " = " << (unsigned int) M_[i] << endl;
    //sum += 1.0 / (1 << M_[i]);
    sum += pow(2, -1* M_[i]);
  }

  estimate = alphaMM_ / sum; // E in the original paper
  
  if (estimate <= 2.5 * m_) {
    uint32_t zeros = 0;
    for (uint32_t i = 0; i < m_; i++) {
      if (M_[i] == 0) {
	zeros++;
      }
    }
    if (zeros != 0) {
      estimate = m_ * std::log(static_cast<myDouble_t>(m_)/ zeros);
    }
  } else if (estimate > (1.0 / 30.0) * pow_2_32) {
    estimate = neg_pow_2_32 * log(1.0 - (estimate / pow_2_32));
  }
  return (longSysSize_t)estimate;
}

void HyperLogLog::merge(ProbabilisticCounter &p) {
  HyperLogLog &other = static_cast<HyperLogLog&>(p);
  for (uint8_t r = 0; r < m_; ++r) {
    if (M_[r] < other.M_[r]) {
      //M_[r] |= other.M_[r]; // order sensitive ?!

      // or max only ?
      // (in aggreement with wikipedia)
      M_[r] = other.M_[r];
    }
  }
}

size_t HyperLogLog::sizeOfRegisters() {
  size_t r = m_ * s_;
  size_t s = r/8;
  
  if (r%8 != 0) {
    s += 1;
  }
  return s;
}

size_t  HyperLogLog::size() {
  size_t s = sizeOfRegisters();
  return s;
}

size_t HyperLogLog::sizeDataToSendInMsg() {
  return sizeOfRegisters();
}

size_t  HyperLogLogHIP::size() {
  size_t s = 2 * sizeof(myDouble_t)
    + sizeOfRegisters();
  return s;
}

uint8_t HyperLogLogHIP::register_limit_ = (1<<4) - 1;

HyperLogLogHIP::HyperLogLogHIP(BaseSimulator::BuildingBlock *mo): HyperLogLog(mo),
			       c_(0.0),
			       p_(1 << b_) {
}

HyperLogLogHIP::HyperLogLogHIP(const HyperLogLogHIP& hllh): HyperLogLog(hllh) {
  c_ = hllh.c_;
  p_ = hllh.p_;
}

HyperLogLogHIP::~HyperLogLogHIP() {

}

HyperLogLogHIP* HyperLogLogHIP::clone() {
  return new HyperLogLogHIP(*this);
}

bool HyperLogLogHIP::isEqual(ProbabilisticCounter *p) const {
  HyperLogLogHIP *hllh = (HyperLogLogHIP*) p;
  return ((c_ == hllh->c_) && (p_ == hllh->p_) && HyperLogLog::isEqual(p));
}

void HyperLogLogHIP::add(elem_t e) {
  hll_elem_t s = HLL_ELEM_T_SIZE_IN_BITS;
  hll_elem_t h = hash(0,e);
  hll_elem_t index = h >> (s - b_);
  uint8_t rank = _GET_CLZ((hll_elem_t)(h << b_), (hll_elem_t)(s - b_));
  rank = rank == 0 ? register_limit_ : std::min(register_limit_, rank);
  const uint8_t old = M_[index];
  if (rank > old) {
    c_ += 1.0 / (p_/m_);
    p_ -= 1.0/(1 << old);
    M_[index] = rank;
    if(rank < (s-1)){
      p_ += 1.0/(elem_t(1) << rank);
    }
  }
}

longSysSize_t HyperLogLogHIP::getSizeEstimation() {
  return (longSysSize_t)c_;
}

void HyperLogLogHIP::merge(ProbabilisticCounter &p) {
  
  HyperLogLogHIP &other = static_cast<HyperLogLogHIP&>(p);
  
  if (m_ != other.m_) {
    /*std::stringstream ss;
    ss << "number of registers doesn't match: " << m_ << " != " << other.m_;
    throw std::invalid_argument(ss.str().c_str());*/
    cerr << "number of registers doesn't match: " << m_ << " != " << other.m_ << endl;
    assert(false);
  }
  for (uint8_t r = 0; r < m_; ++r) {
    const uint8_t b = M_[r];
    const uint8_t b_other = other.M_[r];
    if (b < b_other) {
      c_ += 1.0 / (p_/m_);
      p_ -= 1.0/(1 << b);
      M_[r] |= b_other;
      if(b_other < register_limit_){
	p_ += 1.0/(1 << b_other);
      }
    }
  }
}
