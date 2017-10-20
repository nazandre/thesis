#ifndef SYNCHRONIZATION_POINT
#define SYNCHRONIZATION_POINT

#include "tDefs.h"

namespace Synchronization {

  // Synchronization point (local,global)
  class Point {
  public:
    Time local;
    Time global;

    Point();
    Point(Time _local, Time _global);
    Point(const Point &p);
    ~Point();
  };
    
}

#endif
