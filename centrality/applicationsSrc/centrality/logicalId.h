#ifndef LOGICAL_ID_H_
#define LOGICAL_ID_H_

#include "tDefs.h"
#include "random.h"
#include "utils/def.h"

class LogicalID {
protected:
  static LogicalID singleton;
  std::vector<mID_t> lIDs;
  
  LogicalID();
  ~LogicalID();
  
public:

  static void generate(bID size, BaseSimulator::rint seed);
  static mID_t getID(bID id);
};

#endif
