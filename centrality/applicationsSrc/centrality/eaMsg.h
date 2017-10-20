#ifndef EA_MESSAGE_H
#define EA_MESSAGE_H

#include "network.h"
#include <limits>

//#include <cassert>

class EAMessage;
typedef std::shared_ptr<EAMessage> EAMessagePtr;

class EAMessage : public Message {
protected:
  unsigned int dataSize = std::numeric_limits<unsigned int>::max();
public:

  EAMessage(): Message() {};
  virtual ~EAMessage() {};

  static void setBitRate(BaseSimulator::BuildingBlock *b);

  virtual EAMessage *clone() { assert(false); }

  unsigned int size();
  static size_t headerSize();
  static size_t availableDataSize();
  
  unsigned int numChunks(unsigned int msize);

  Time getProcessingDelays();

};

#endif
