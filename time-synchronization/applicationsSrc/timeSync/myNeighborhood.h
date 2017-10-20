#ifndef MY_NEIGHBORHOOD_H_
#define MY_NEIGHBORHOOD_H_

#include "events.h"
#include "myMessage.h"

class Layers;

// ev
class NeighborhoodKeepAliveEvent;
typedef std::shared_ptr<NeighborhoodKeepAliveEvent> NeighborhoodKeepAliveEventPtr;

class NeighborhoodKeepAliveEvent: public BlockEvent {
public:

  NeighborhoodKeepAliveEvent(Time t, BaseSimulator::BuildingBlock *conBlock);
  NeighborhoodKeepAliveEvent(NeighborhoodKeepAliveEvent *ev);
  ~NeighborhoodKeepAliveEvent();
  void consumeBlockEvent();
  const string getEventName();

  static void init(Layers &layers);
};

// msg
class NeighborKeepAliveMessage;
typedef std::shared_ptr<NeighborKeepAliveMessage> NeighborKeepAliveMessagePtr;

class NeighborKeepAliveMessage: public MyMessage {
public :
  
  NeighborKeepAliveMessage();
  NeighborKeepAliveMessage(NeighborKeepAliveMessage *m);
  ~NeighborKeepAliveMessage();

  NeighborKeepAliveMessage* clone();

  unsigned int numEscapedBytes();
  unsigned int dataSize();
  
};

#endif
