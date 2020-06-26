///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains custom types.
 *	\file		IceTypes.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICETYPES_H__
#define __ICETYPES_H__

#include <cfloat>

#ifndef OPASSERT
#define OPASSERT(exp) \
	{                 \
	}
#endif

// Constants
#ifndef PI
#define PI 3.1415926535897932384626433832795028841971693993751f //!< PI
#endif
#define INV3 0.33333333333333333333f //!< 1/3
#define INVSQRT2 0.707106781188f	 //!< 1 / sqrt(2)
#define INVSQRT3 0.577350269189f	 //!< 1 / sqrt(3)

#define null 0 //!< our own nullptr pointer

// Custom types used in ICE
typedef signed char sbyte;	  //!< sizeof(sbyte)	must be 1
typedef unsigned char ubyte;  //!< sizeof(ubyte)	must be 1
typedef signed short sword;	  //!< sizeof(sword)	must be 2
typedef unsigned short uword; //!< sizeof(uword)	must be 2
typedef signed int sdword;	  //!< sizeof(sdword)	must be 4
typedef unsigned int udword;  //!< sizeof(udword)	must be 4
typedef int64_t sqword;		  //!< sizeof(sqword)	must be 8
typedef uint64_t uqword;	  //!< sizeof(uqword)	must be 8
typedef float float32;		  //!< sizeof(float32)	must be 4
typedef double float64;		  //!< sizeof(float64)	must be 4

#define INVALID_ID 0xffffffff //!< Invalid dword ID (counterpart of null pointers)

// Type ranges
#define IEEE_1_0 0x3f800000		  //!< integer representation of 1.0
#define IEEE_MAX_FLOAT 0x7f7fffff //!< integer representation of MAX_FLOAT

#define ONE_OVER_RAND_MAX (1.0f / float(RAND_MAX)) //!< Inverse of the max possible value returned by rand()

#endif // __ICETYPES_H__
