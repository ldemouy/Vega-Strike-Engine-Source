///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple pair class.
 *	\file		IcePairs.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPAIRS_H__
#define __ICEPAIRS_H__
#include "IceContainer.h"
//! A generic couple structure
struct Pair
{
	inline Pair() {}
	inline Pair(uint32_t i0, uint32_t i1) : id0(i0), id1(i1) {}

	uint32_t id0; //!< First index of the pair
	uint32_t id1; //!< Second index of the pair
};

#endif // __ICEPAIRS_H__
