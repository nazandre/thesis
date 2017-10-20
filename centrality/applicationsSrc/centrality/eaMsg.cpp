#include "eaMsg.h"
#include "rate.h"
#include "random.h"
#include "utils/def.h"
#include "simulation.h"
#include "initiatorMsg.h"
#include "centralityBlockCode.h"

//#define MESSAGE_SIZE_DEBUG

#define CHUNK_SIZE 17
// Chunk on real Blinky Blocks:
// data : 17 bytes
// Frame delimiter (FD) : 1 byte
// Pointer message handler: 2 bytes
// Checkum : 1 byte

#define MS_TIME_TO_BITRATE(t) (((double)17*8)/(t* pow(10,-3)))
#define USEFUL_BITRATE_TO_DATALINK_BITRATE(r) ((21.0/17.0)* r)
using namespace std;
using namespace BaseSimulator;
using namespace BaseSimulator::utils;

size_t EAMessage::headerSize() {
   return
      // fault tolerance and mobility management:
      + sizeof(uint16_t) // seq number
      + sizeof(mID_t) // module that initiates the election
      ;
}

size_t EAMessage::availableDataSize() {
  return CHUNK_SIZE - headerSize();
}

unsigned int EAMessage::size() {
  static bool moreThanAChunkDetected = false;
  
  unsigned int s = 0;
  unsigned int d = dataSize;
  
  if (THIS_ALGORITHM == ALGORITHM(RAND_CENTER)) {
    if (type == IE_SP_BACK_MESSAGE) {
      d -= sizeof(longDistance_t); // do not need to send minEcc for the first
    }
  }

  s = d + headerSize();

#ifdef MESSAGE_SIZE_DEBUG
  cerr << "dataSize: " << dataSize  << " (msg id: " << type << ")" << endl;
#endif
  
  if (!moreThanAChunkDetected && (s > CHUNK_SIZE)) {
    cerr << "WARNING:"
	 << " more than one chunk would be necessary to store a "
	 << s << " byte-long message"
	 << " (msg id: " << type << ")"
	 << endl; 
    moreThanAChunkDetected = true;
  }
  
  if (THIS_ALGORITHM != ALGORITHM(ABC_CENTER_V1)) {
    assert(s <= CHUNK_SIZE);
  }

  
  return CHUNK_SIZE;
  //unsigned int n = numChunks();
  //assert(n==1); // otherwise queue size stats are wrongly computed!
  //return CHUNK_SIZE*n;
}

unsigned int EAMessage::numChunks(unsigned int msize){
  unsigned int n = msize/CHUNK_SIZE;

  if ( (msize % CHUNK_SIZE) != 0) {
    n++;
  }

  if (n == 0) {
    n++;
  }
  
  return n;
}

void EAMessage::setBitRate(BuildingBlock *b) {
  P2PNetworkInterface *p = NULL;
  static bool parametersPrinted = false;
  //static std::vector<double> transferTimes = {5.925,6.3,6.5};
  //static std::vector<double> sds = {1000,2000,2000};

  // Validated!
  static std::vector<double> transferTimes = {5.93,6.3,6.5};
  static std::vector<double> sds = {900,2000,2000};

  double transferTime = 0;
  double sd = 0;

  unsigned int n = ((CentralityBlockCode*) (b->blockCode))->nbNeighbors;
  unsigned int index = 0;


  if (n <= 2) {
    index = 0;
  } else if (n <= 4) {
    // mid
    index = 1;
  } else {
    // compact
    index = 2;
  }
  
  transferTime = transferTimes[index];
  sd = sds[index];
  
  double rate = MS_TIME_TO_BITRATE(transferTime);

  if (!parametersPrinted) {
    cerr << "Communication parameters: " << endl;
    cerr << "\tBitrate:" << endl;
    cerr << "\t\t|neighbors| : "<< endl;
    for (uint i = 0; i < 3; i++) 
      cerr << "\t\t<=" << (i+1)*2 << " : N("<< USEFUL_BITRATE_TO_DATALINK_BITRATE(MS_TIME_TO_BITRATE(transferTimes[i])) << "," << USEFUL_BITRATE_TO_DATALINK_BITRATE(sds[i]) << ")" << endl;
    parametersPrinted = true;
  }

  //cerr << "@" << b->blockId << ": " << endl;
  for(int i = 0; i < b->getNbInterfaces(); i++) {
    p = b->getInterface(i);
    BaseSimulator::rint seed = b->getRandomUint();
    //cerr << "\t seed interface " << (uint64_t)p << ": " << seed << endl;
    doubleRNG g = Random::getNormalDoubleRNG(seed,
					     rate,sd);

    RandomRate *r = new RandomRate(g);
    p->setDataRate(r);
  }
}

Time EAMessage::getProcessingDelays() {
  BaseSimulator::BuildingBlock* b = destinationInterface->hostBlock;
  Time min,max,spread;
  
  unsigned int n = ((CentralityBlockCode*) (b->blockCode))->nbNeighbors;
  
  min = 25;
  max = 125;
    
  spread = max-min;

  return min + b->getRandomUint()%(spread);
}
