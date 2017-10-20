#include "myMessage.h"

#include <iostream>

#include "simulator.h"
#include "scheduler.h"

#include "timeSyncBlockCode.h"
#include "simulation.h"
#include "utils/def.h"

#include "appEventsID.h"

#include "layers.h"

//#define PROBABILISTIC_CHECKSUM_ESCAPE
#define COMPUTE_CHECKSUM
//#define PROBABILISTIC_MSG_SIZE

#define DISSEMINATION_ERROR_MEASUREMENTS

//#define MESSAGE_SIZE_DEBUG

// Message size
#define POINTER_HANDLER_SIZE 2
#define CHECKSUM_SIZE 1
#define FRAME_DELIMITER_SIZE 1

// Special bytes to escape
#define FD_MASK         0xFE
#define FD              0x12
#define ACK             0x08
#define ACK_MASK        0xFE
#define NACK            0x0A
#define ESCAPE          0x7D

#define CRC_POLY        0xA6

using namespace BaseSimulator;
using namespace std;

MyMessage::MyMessage(): Message() {
  savedSize = SIZE_NOT_COMPUTED;
  crc = 0;
}

MyMessage::~MyMessage() {

}

bool MyMessage::isToEscape(byte_t val) {
#ifdef COMPUTE_CHECKSUM
  updateCrc(val);
#endif
  
  if( ((val & ACK_MASK) == ACK   ) || ((val & ACK_MASK) == NACK  ) ||
      ((val & FD_MASK)  == FD    ) || (val              == ESCAPE )   )
    {
      return true;
    }
    return false;
}

void MyMessage::updateCrc(byte_t val) {
  int i;
  
  crc ^= val;
  for (i=0; i<8; i++) {
    if (0x80 & crc) {
      crc = (crc << 1) ^ CRC_POLY;
    } else {
      crc = crc << 1;
    }
  }
}


unsigned int MyMessage::numEscapedBytes(Time t) {
  // us to ms
  myTime_t tms = t / 1000;
  return MyMessage::numEscapedBytes((byte_t*)&tms,sizeof(myTime_t));
}

unsigned int MyMessage::numEscapedBytes(myDouble_t d) {
  return MyMessage::numEscapedBytes((byte_t*)&d,sizeof(myDouble_t));
}

unsigned int MyMessage::numEscapedBytes(byte_t *field,unsigned int size) {
  unsigned int n = 0;
  for (unsigned int i = 0; i < size; i++) {
    if (isToEscape(field[i])) {
      n++;
    }
  }
  return n;
}

unsigned int MyMessage::size() {
  unsigned int total = 0;
  unsigned int data = 0;
  unsigned int typeField = sizeof(messageType_t);
  
  if (savedSize != SIZE_NOT_COMPUTED) {
    return savedSize;
  }
  
  data = dataSize();
  total += data + typeField;
  
#ifdef CONSTANT_MESSAGE_SIZE
  // message size = MTU
  // message format: type | data ...
  assert(total <= MTU);
  total = MTU;
#else
  // Celui la
  // message size: variable
  // message format: size | type | data
  unsigned int sizeField = sizeof(byte_t);
  total += data + sizeField;
  assert(total <= MTU);
#endif

  total += FRAME_DELIMITER_SIZE +
    POINTER_HANDLER_SIZE +
    CHECKSUM_SIZE +
    numEscapedBytes();

#ifdef PROBABILISTIC_CHECKSUM_ESCAPE
  double p = ((TimeSyncBlockCode*)(sourceInterface->hostBlock->blockCode))->crcEscapeGenerator();

  if (p < (7.0/255.0)) {
    total++;
  }
#endif

#ifdef COMPUTE_CHECKSUM
  // crc  
  isToEscape(0x60); // clock msg
  isToEscape(0x00); // sys msg handler
  isToEscape(0x00);

  isToEscape(0x03);

  //#define CLOCK_INFO 3
  //#define SYNC_1 200
  //#define SYNC_2 201
  //#define SYNC_3 202

  int numZeros = MTU - ((int)data + 2);
#ifdef DISSEMINATION_ERROR_MEASUREMENTS
  isToEscape(0); // way
  isToEscape(sourceInterface->hostBlock->getRandomUint() % 3 + 21); // prev size
  numZeros = MTU - ((int)data + 2 + 2);
#endif
  
  // 17 - (data + clock msg type + clock submsg type + way)
  
  for (int i = 0; i <= numZeros; i++) {
    isToEscape(0xFF);
  }
  
  if (isToEscape(crc)) {
    total++;
  }
#endif

#ifdef PROBABILISTIC_MSG_SIZE
  // probability size of a msg
  //21 79.2378124
  //22 18.5597624
  //23  2.0663202
  //24  0.1361049

  double p = ((TimeSyncBlockCode*)(sourceInterface->hostBlock->blockCode))->crcEscapeGenerator();
  
  if (p < 0.1361049/100.0) {
    total = 24;
  } else if (p < 2.0663202/100.0) {
    total = 23;
  } else if (p < 18.5597624/100.0) {
    total = 22;
  } else {
    total = 21;
  }
#endif
  
#ifdef MESSAGE_SIZE_DEBUG  
  cerr << getMessageName() << ", size (bytes) = "
       << total << endl;
#endif

  savedSize = total;
  return total;
}

void MyMessage::send(Layers &layers, P2PNetworkInterface *p2p, Message *m) {
  Time sendTime = layers.synchronization->getSendingTime(p2p);
  p2p->send(sendTime,m);
}

// NOT USED
#define BLOCKTICK_PERIOD 500
// BlockTick period: 0.5ms = 500us
Time MyMessage::getNextBlockTickTime(Layers &layers, Time d) {
  BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();
  Time simTime = sched->now();
  Time localTime = layers.localClock->getLocalTime(simTime,false);
  
  Time r = localTime % BLOCKTICK_PERIOD;
  Time res = localTime;
  if (r != 0) {
    res += BLOCKTICK_PERIOD - r;
  }
  
  Time t = layers.module->getSimulationTime(res);  
  return t;
}

void MyMessage::scheduleHandleMsgEvent(Layers &layers, MessagePtr &m) {
  assert(false);
  BaseSimulator::Scheduler *sched = BaseSimulator::getScheduler();

  Time handlingTime = sched->now();
  Time delay = 0;

  handlingTime += delay;

  sched->schedule(new HandleMsgEvent(handlingTime,layers.module,m));
}

// TimeMasterElectedEvent class

HandleMsgEvent::HandleMsgEvent(Time t, BaseSimulator::BuildingBlock *conBlock, MessagePtr &m):
  BlockEvent(t, conBlock) {
  eventType = EVENT_HANDLE_MSG;
  message = m;
}
  
HandleMsgEvent::HandleMsgEvent(HandleMsgEvent *ev): BlockEvent(ev) {
  message = ev->message;
}
  
HandleMsgEvent::~HandleMsgEvent() {

}
  
void HandleMsgEvent::consumeBlockEvent() {
  concernedBlock->scheduleLocalEvent(EventPtr(new HandleMsgEvent(this)));
}
  
const string HandleMsgEvent::getEventName() {
  return("HandleMsg Event");
}
