#ifndef ALPHA_SYNCHRONIZED_BFS_H_
#define ALPHA_SYNCHRONIZED_BFS_H_

#include <vector>
#include "utils/def.h"
#include "bfsTree.h"
#include "msgID.h"
#include "eaMsg.h"

// Source: http://www.cs.yale.edu/homes/aspnes/pinewiki/DistributedBreadthFirstSearch.html

namespace AlphaSynchronizedBFS {
  enum state_t {EXACTLY = 0, MORE_THAN = 1, NUM_STATES};

  class State {
  public:
    int32_t distance;
    state_t state;
    
    State(): distance(-1), state(MORE_THAN) {};
    State(distance_t d, state_t s): distance(d), state(s) {};
    State(const State &s): distance(s.distance), state(s.state){};
    ~State() {};

    std::string toString();

    static size_t getSize() {return sizeof(int32_t) + sizeof(uint8_t); }
  };
    
  class Traversal: public BFSTraversalSP {
  public:

    State state;
    std::vector<State> neighborStates;
    bool started;
    sysSize_t sysSize = 0;
    distance_t height = 0;
    degree_t numBackReceived = 0;
    bool backSent = false;
    bool election = false;
    
    Traversal();
    Traversal(Traversal &t);
    ~Traversal();

    size_t size();

    void reset();
    void subscribe(mID_t id);
    void setElection(bool b) {election = b;}

    distance_t getHeight();
    sysSize_t getSize();
  };
}

class AlphaSynchronizedBFSAlgorithm {
  public:

  BaseSimulator::BuildingBlock *module;
  
  AlphaSynchronizedBFSAlgorithm(BaseSimulator::BuildingBlock *m);
  ~AlphaSynchronizedBFSAlgorithm();
  
  bool start(AlphaSynchronizedBFS::Traversal *t);
  void reset(AlphaSynchronizedBFS::Traversal *t);
  
  bool handle(MessagePtr m, AlphaSynchronizedBFS::Traversal *t);
  bool hasToHandle(MessagePtr m);

  void sendGo(AlphaSynchronizedBFS::Traversal *t);
  void sendStart(AlphaSynchronizedBFS::Traversal *t,P2PNetworkInterface *ignore);

  void setSysSize(size_t s) {}; // useless
};


// messages
class AlphaSynchronizedBFSGoMessage;
typedef std::shared_ptr<AlphaSynchronizedBFSGoMessage>  AlphaSynchronizedBFSGoMessagePtr;

class AlphaSynchronizedBFSGoMessage : public EAMessage {
 public:
  mID_t rootID;
  AlphaSynchronizedBFS::State state;
  
  //BFSData* header;
  //BFSData* backData; // data to send back up to the root
  
  AlphaSynchronizedBFSGoMessage(mID_t rid, AlphaSynchronizedBFS::State s): EAMessage(), rootID(rid), state(s)  {
    type = ALPHA_SYNCHRONIZED_BFS_GO_MSG;
    dataSize += sizeof(mID_t) + AlphaSynchronizedBFS::State::getSize();
  };
  
  ~AlphaSynchronizedBFSGoMessage() {};
};

class AlphaSynchronizedBFSBackMessage;
typedef std::shared_ptr<AlphaSynchronizedBFSBackMessage>  AlphaSynchronizedBFSBackMessagePtr;

class AlphaSynchronizedBFSBackMessage : public EAMessage {
 public:
  mID_t rootID;
  sysSize_t sysSize;
  distance_t height;
  bool child;
  
  AlphaSynchronizedBFSBackMessage(mID_t rid, sysSize_t s, distance_t h, bool c): EAMessage(), rootID(rid), sysSize(s), height(h), child(c)  {
    type = ALPHA_SYNCHRONIZED_BFS_BACK_MSG;
    dataSize += sizeof(mID_t) + sizeof(sysSize_t) + sizeof(distance_t) + sizeof(bool);
  };
  
  ~AlphaSynchronizedBFSBackMessage() {};
};

class AlphaSynchronizedBFSStartMessage;
typedef std::shared_ptr<AlphaSynchronizedBFSStartMessage>  AlphaSynchronizedBFSStartMessagePtr;

class AlphaSynchronizedBFSStartMessage : public EAMessage {
 public:
  mID_t rootID;
  
  AlphaSynchronizedBFSStartMessage(mID_t rid): EAMessage(), rootID(rid) {
    type = ALPHA_SYNCHRONIZED_BFS_START_MSG;
    dataSize += sizeof(mID_t);
  };
  
  ~AlphaSynchronizedBFSStartMessage() {};
};

#endif
