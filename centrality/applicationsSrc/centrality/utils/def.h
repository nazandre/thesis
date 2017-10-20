#ifndef TYPE_DEFS_H_
#define TYPE_DEFS_H_

#include <cstdint>

#include "half.hpp"

typedef uint8_t byte_t;

typedef uint16_t distance_t; 
typedef uint32_t longDistance_t; // e.g., sum of distances
typedef byte_t degree_t;
typedef uint16_t bbPointer_t;

typedef uint32_t hash_t;
typedef std::function<hash_t(hash_t)> hashFunc_t;
#define MASK_16_BITS (((uint32_t)1<<16)-1) /* i.e., (uint32_t)0xffff */

// 32-bit UID and size
//typedef uint32_t sysSize_t;
// 16-bit UID and size
typedef uint16_t sysSize_t;
typedef uint32_t longSysSize_t;

typedef sysSize_t tID_t; // tree id
typedef sysSize_t mID_t; // module id

#define NOT_A_VALID_MODULE_ID 0

typedef uint8_t messageType_t;

typedef uint32_t myTime_t; // time_t already defined!

//typedef double myDouble_t; // a classical double => 8 bytes
typedef float myDouble_t; // a double => 4 bytes

typedef half_float::half myShortDouble_t; // a short double => 2 bytes
//typedef float myShortDouble_t; // a short double => 4 bytes

#define ONE_MICROSECOND ((Time)1)
#define ONE_MILLISECOND ((Time)1000*ONE_MICROSECOND)
#define ONE_SECOND ((Time)1000*ONE_MILLISECOND)
#define ONE_MINUTE ((Time)60*ONE_SECOND)
#define ONE_HOUR ((Time)60*ONE_MINUTE)

#define US_TO_MS(x) (x/1000)
#define MS_TO_US(x) (x*1000)
#define US_TO_US_MS_RESOLUTION(x) ((x) - ((x)%ONE_MILLISECOND))

#define MY_CERR cerr << "@" << module->blockId << ": "
//#define MY_CERR cerr << "@" << layers.module->blockId << ": "
#define HOST_CERR cerr << "Host (@" << blockId << "): "

//#define THIS_ID layers.module->blockId

#endif
