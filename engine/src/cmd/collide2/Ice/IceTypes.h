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

// Constants
#ifndef PI
#define PI 3.1415926535897932384626433832795028841971693993751f //!< PI
#endif
#define INV3 0.33333333333333333333f //!< 1/3

#define INVALID_ID 0xffffffff //!< Invalid dword ID (counterpart of null pointers)

// Type ranges
#define IEEE_MAX_FLOAT 0x7f7fffff //!< integer representation of MAX_FLOAT

#endif // __ICETYPES_H__
