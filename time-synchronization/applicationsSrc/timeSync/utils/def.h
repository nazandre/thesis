#ifndef TYPE_DEFS_H_
#define TYPE_DEFS_H

#include <cstdint>

typedef uint8_t byte_t;
typedef double distance_t; // geometrical distance

typedef uint16_t hopDistance_t; 
typedef uint32_t longHopDistance_t; // e.g., sum of distances
typedef byte_t degree_t;
typedef uint16_t bbPointer_t;

typedef uint32_t sysSize_t;
typedef uint32_t tID_t; // tree id
typedef uint32_t mID_t; // module id
#define NOT_A_VALID_ID 0

typedef uint8_t messageType_t;

typedef uint32_t myTime_t; // time_t already defined!
typedef float myDouble_t; // a double => 4 bytes
//typedef double myDouble_t;

//typedef double wmtsDouble_t;
typedef myDouble_t wmtsDouble_t;

#define ONE_MICROSECOND ((Time)1)
#define ONE_MILLISECOND ((Time)1000*ONE_MICROSECOND)
#define ONE_SECOND ((Time)1000*ONE_MILLISECOND)
#define ONE_MINUTE ((Time)60*ONE_SECOND)
#define ONE_HOUR ((Time)60*ONE_MINUTE)

#define US_TO_MS(x) (x/1000)
#define MS_TO_US(x) (x*1000)
#define US_TO_US_MS_RESOLUTION(x) ((x) - ((x)%ONE_MILLISECOND))

//#define MY_CERR cerr << "@" << hostBlock->blockId << ": "
#define MY_CERR cerr << "@" << layers.module->blockId << ": "
#define HOST_CERR cerr << "Host (@" << blockId << "): "

#define THIS_ID layers.module->blockId

#endif
