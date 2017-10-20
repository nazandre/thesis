#include "point.h"

namespace Synchronization {

  Point::Point() {
    local = 0;
    global = 0;
  }
  
  Point::Point(Time _local, Time _global) {
    local = _local;
    global = _global;
  }

  Point::Point(const Point &p) {
    local = p.local;
    global = p.global;
  }

  Point::~Point() {

  }

}
