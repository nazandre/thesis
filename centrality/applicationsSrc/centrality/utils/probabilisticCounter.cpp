#include "probabilisticCounter.h"

#include "simulator.h"

std::vector<hashFunc_t> ProbabilisticCounter::hashFuncs;

ProbabilisticCounter::ProbabilisticCounter(BaseSimulator::BuildingBlock* b) {
  module = b;
}

ProbabilisticCounter::ProbabilisticCounter(const ProbabilisticCounter &pc) {
  module = pc.module;
}

ProbabilisticCounter::~ProbabilisticCounter() {

}

unsigned int ProbabilisticCounter::getNumHashFuncs() {
  return hashFuncs.size();
}

void ProbabilisticCounter::initHashFuncs(unsigned int n) {

  cerr << n << " hash funcs:" << endl;

  switch(THIS_HASH_FUNCTION) {
  case HASH_FUNCTION(AFFINE): {
    hash_t m = 1;
    hash_t k = 1;
    for (unsigned int i = 0; i < n; i++) {
      cerr << "\th(x) = " << k << " * x + " << m << endl;
      hashFuncs.push_back(std::bind(&ProbabilisticCounter::affineHash,std::placeholders::_1,k,m));
      k = 3*(k+2);
      m = 5*(m+2);
    }
  }
    break;
  case HASH_FUNCTION(AFFINE_PRIME): {
    std::vector<hash_t> primes;
    hash_t lb = (1 << 16);
    hash_t ub = (1 << 31);
    hash_t m,k,r;
    for (unsigned int i = 0; i < 2*n; i++) {
      do {
	r = BaseSimulator::Simulator::getSimulator()->getRandomUint();
	k = lb + r % (ub - lb);
      } while(!isPrime(k));
      primes.push_back(k);
    }
    for (unsigned int i = 0; i < n; i++) {
      k = primes[2*i];
      m = primes[2*i+1];
      cerr << "\th(x) = " << k << " * x + " << m << endl;    
      hashFuncs.push_back(std::bind(&ProbabilisticCounter::affineHash,std::placeholders::_1,k,m));
    }
  }
    break;
  case HASH_FUNCTION(KNUTH): {
    assert(n == 1);
    cerr << "\tKnuth hash func" << endl;
    hashFuncs.push_back(std::bind(&ProbabilisticCounter::knuthHash,std::placeholders::_1));
  }
    break;
  case HASH_FUNCTION(MURMUR3): {
    hash_t seed = 0;
    for (unsigned int i = 0; i < n; i++) {
      seed = BaseSimulator::Simulator::getSimulator()->getRandomUint();
      cerr << "\th(x) = murmur3Hash(x," << seed << ")" << endl;
      hashFuncs.push_back(std::bind(&ProbabilisticCounter::murmur3Hash,std::placeholders::_1,seed));
    }
  }
    break;
  case HASH_FUNCTION(FNV1): {
    assert(n == 1);
    cerr << "\tFNV1 hash func" << endl;
    hashFuncs.push_back(std::bind(&ProbabilisticCounter::fnv1Hash,std::placeholders::_1));
  }
    break;
  case HASH_FUNCTION(RAND_HASH): {
    // fill with useless funcs (hashFuns.size() is used ...)
    cerr << "\thash(x) = rand()" << endl;
    for (unsigned int i = 0; i < n; i++) {
      hashFuncs.push_back(std::bind(&ProbabilisticCounter::affineHash,std::placeholders::_1,1,1));
    }
  }
    break;
  default:
    assert(false);
  }
}

hash_t ProbabilisticCounter::hash(unsigned int n,hash_t e,unsigned int s) {
  hash_t h = 0;
  assert(hashFuncs.size() > n);

  if (IS_RUNNING_HASH_FUNCTION(RAND_HASH)) {
    h = module->getRandomUint() % std::numeric_limits<hash_t>::max();
  } else {
    h = hashFuncs[n](e);
  }
  
  if (s == 2) {
    if (IS_RUNNING_HASH_FUNCTION(AFFINE)) {
      h = h & MASK_16_BITS;
    } else {
      // If you need an x-bit hash where x is not a power of 2, then we recommend that you compute the FNV hash that is just larger than x-bits and xor-fold the result down to x-bits. By xor-folding we mean shift the excess high order bits down and xor them with the lower x-bits.
      h = (h>>16) ^ (h & MASK_16_BITS);
    }
  }
  
  return h;
}
    
hash_t ProbabilisticCounter::affineHash(hash_t e,hash_t n, hash_t m) {
  hash_t h = (e*n + m)  % std::numeric_limits<hash_t>::max();
  return h;
}

hash_t ProbabilisticCounter::knuthHash(hash_t e) {
  hash_t h = (e*2654435761) % std::numeric_limits<hash_t>::max();
  return h;
}

// Hash funcs
hash_t ProbabilisticCounter::murmur3Hash(const hash_t myKey,hash_t seed) {
  hash_t h = seed;
  size_t len = 1; // 32bits
  const uint8_t *key = (uint8_t*) (&myKey);
  
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

hash_t ProbabilisticCounter::fnv1Hash(hash_t e) {
#define FNV1_UINT32_T_PRIME 16777619
#define FNV1_UINT32_T_OFFSET_BASIS 2166136261

  hash_t hash = FNV1_UINT32_T_OFFSET_BASIS;
  uint8_t* key = (uint8_t*) &e;
  for (unsigned int i = 0; i < sizeof(hash_t); i++) {
    hash ^= (hash_t) (*key);
    hash *= FNV1_UINT32_T_PRIME;
  }
  return hash;
}


// source: https://math.stackexchange.com/questions/424238/random-primes-between-4000000000-and-4294967291-c
// remark: not time optimal (see Miller-Rabin pretesting and Lucas test for faster version)
/*
bool ProbabilisticCounter::isPrime(hash_t n) {
  hash_t p = 5;
 
  if(n%3 == 0)
    return (n == 3);

  while (p*p <= n) {
    
    if (n%p == 0)
      return false;

    p += 2;
    if (n%p==0)
      return false;
    
    p += 4;
  }
  return true;
  }*/

// https://stackoverflow.com/questions/4424374/determining-if-a-number-is-prime
bool ProbabilisticCounter::isPrime(hash_t n){
  if (n != 2){
    if (n < 2 || n % 2 == 0) {
      return false;
    }
    for(hash_t i=3; (i*i)<=n; i+=2){
      if(n % i == 0 ){
	return false;
      }
    }
  }
  return true;
}
