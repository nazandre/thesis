#ifndef MESSAGE_FAILURE_H_
#define MESSAGE_FAILURE_H_

#include <vector>

#include "network.h"
#include "events.h"

namespace BaseSimulator {

  class MessageFailureEngine {
  protected:
    Time delta; // time between message re-transmissions
  public:
    MessageFailureEngine(Time d);
    MessageFailureEngine(const MessageFailureEngine &fe);
    
    virtual ~MessageFailureEngine();
    virtual Time getTimeDueToReTransmissions(MessagePtr m, bool &l) = 0;
  };

  class DataMessageFailureEngine : public MessageFailureEngine {
  protected:
    std::vector<double>& distribution;
    doubleRNG generator;
    bool loss;
  public:
    DataMessageFailureEngine(Time d, std::vector<double>& dis, doubleRNG &g,
		      bool l = false);
    DataMessageFailureEngine(const DataMessageFailureEngine &dfe);
    
    virtual ~DataMessageFailureEngine();
    virtual Time getTimeDueToReTransmissions(MessagePtr m, bool &l);
  };

  class RandomMessageFailureEngine : public MessageFailureEngine {
  protected:
    doubleRNG distribution;
    bool loss;
  public:
    RandomMessageFailureEngine(Time d, doubleRNG &g, bool l = false);
    RandomMessageFailureEngine(const RandomMessageFailureEngine &rfe);

    virtual ~RandomMessageFailureEngine();
    virtual Time getTimeDueToReTransmissions(MessagePtr m, bool &l);
  };

  class MessageFailureEvent : public BlockEvent {
  protected:
    MessagePtr message;
    P2PNetworkInterface *interface;
  public:
    MessageFailureEvent(Time t, BaseSimulator::BuildingBlock *b, MessagePtr m,P2PNetworkInterface *i);
    MessageFailureEvent(MessageFailureEvent *ev);
    ~MessageFailureEvent();
    void consumeBlockEvent();
    const virtual string getEventName();
  };
  
  /* if (MessageFailureEngine->isLost(message)) {
   *    event -> messageLost(message)
   * }
   * Time d = MessageFailureEngine->getTimeDueToReTransmissions(message,lost);
   * if (lost) {
   *   event lost;
   * }
   * 
   */

  /* MessageFailureEngine:
   *   distribution : uniform<int>/normal<int>/table<int>+uniform<double>/static<int> (0!)
   *   time between re-transmission
   *   int lost (nb à partir du quel lost?)  
   *
   */
}

#endif
