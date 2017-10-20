#include <math.h>
#include <iostream>
#include <cassert>

#include "fm.h"

#define PHI ((myDouble_t)0.77351)

using namespace std;

FlajolletMartin::FlajolletMartin(BaseSimulator::BuildingBlock *b): ProbabilisticCounter(b) {
  unsigned int n = getNumHashFuncs();
  bitStrings.resize(n,0);
}


FlajolletMartin::FlajolletMartin(const FlajolletMartin &fm): ProbabilisticCounter(fm) {
  bitStrings = fm.bitStrings;
}

void FlajolletMartin::init(size_t availableSpace) {
  unsigned int n = availableSpace/FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE;
  //cerr << "availablespace:" << availableSpace << endl;
  initHashFuncs(n);
}

FlajolletMartin* FlajolletMartin::clone() {
  return new FlajolletMartin(*this);
}

bool FlajolletMartin::isEqual(ProbabilisticCounter *p) const {
  FlajolletMartin *fm = (FlajolletMartin*) p;
  unsigned int i =  0;
  
  if (bitStrings.size() != fm->bitStrings.size()) {
    return false;
  }

  for (i = 0; i < bitStrings.size(); i++) {
    if (bitStrings[i] != fm->bitStrings[i]) {
      return false;
    }
  }
  return true;
}

FlajolletMartin::~FlajolletMartin() {

}
	
longSysSize_t FlajolletMartin::getSizeEstimation(byte_t i) {
  assert(i >= 0 && i < bitStrings.size());
  
  return pow(2, r(bitStrings[i]))/PHI;
}

void FlajolletMartin::updateBitmap(byte_t i, bitString_t &b) {
  bitString_t x = 1 << i;

  if (FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE == 2) {
    x = x & MASK_16_BITS;
  }
  
  b = b | x;
}

void FlajolletMartin::add(elem_t e) {
  for (unsigned int i = 0; i < bitStrings.size(); i++) {
    bitString_t h = hash(i,e,FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE);
    if (FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE == 2) {
      assert(h <= std::numeric_limits<uint16_t>::max());
    }
    byte_t index = rho(h);
    updateBitmap(index,bitStrings[i]);
  }
  
}

void FlajolletMartin::merge(ProbabilisticCounter &pc) {
  FlajolletMartin &fm = static_cast<FlajolletMartin&>(pc);

  assert(bitStrings.size() == fm.bitStrings.size());
  for (byte_t i = 0; i < bitStrings.size(); i++) {
    merge(fm,i);
  }
}

void FlajolletMartin::merge(FlajolletMartin &fm, byte_t i) {
  assert(bitStrings.size() > i && bitStrings.size() == fm.bitStrings.size());
  bitStrings[i] = bitStrings[i] | fm.bitStrings[i];
}

longSysSize_t FlajolletMartin::getSizeEstimation() {
  vector<bitString_t>::iterator it;
  myDouble_t mr = 0;

  for (it = bitStrings.begin(); it != bitStrings.end(); it++) {
    mr += r(*it);
  }

  //mr = round(mr / (myDouble_t) bitStrings.size());
  mr = mr / ((myDouble_t) bitStrings.size());
  return (longSysSize_t) (pow(2, mr)/PHI);
}


// returns the index of the first bit equals to one (from index 0)
sysSize_t FlajolletMartin::rho(elem_t e) { // rho
  sysSize_t i = 0;
  elem_t v = e;
  for (i=0; i<ELEM_T_SIZE_IN_BITS; i++) {
    if ((v & 0x01) == 1) {
      break;
    }
    v = v >> 1;
  }

  if (i != ELEM_T_SIZE_IN_BITS) {
    sysSize_t verif = std::min((long unsigned int)__builtin_ctz(e),ELEM_T_SIZE_IN_BITS);
    if (i != verif) {
      cerr << "Error in rho(): " << i << " != " << verif << endl;
      assert(false);
    }
  }

  return i;
}

// returns the index of the first bit equals to zero (from index 0)
sysSize_t FlajolletMartin::r(elem_t e) {
  sysSize_t i = 0;
  elem_t v = 0;
	  
  for (i = 0; i < ELEM_T_SIZE_IN_BITS; i++) {
    v = e >> i;
    if ((v & 0x01) == 0) {
      break;
    }
  }
  
  if (i != ELEM_T_SIZE_IN_BITS) {  
    sysSize_t verif = std::min((long unsigned int)__builtin_ctz(~e),ELEM_T_SIZE_IN_BITS);
    
    if (i != verif) {
      cerr << "Error in r(): " << i << " != " << verif << endl;
      assert(false);
    }
  }
  
  return i;
}

size_t FlajolletMartin::size() {
  return bitStrings.size() * FLAJOLLET_MARTIN_BITSTRING_BYTE_SIZE;
}

size_t FlajolletMartin::sizeDataToSendInMsg() {
  return size();
}
