///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains common classes & defs used in OPCODE.
 *	\file		OPC_Common.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_COMMON_H__
#define __OPC_COMMON_H__

#include "Ice/IcePoint.h"

// [GOTTFRIED]: Just a small change for readability.
#define GREATER(x, y) fabsf(x) > (y)

class CollisionAABB
{
public:
	//! Constructor
	inline CollisionAABB() {}
	//! Constructor
	inline CollisionAABB(const AABB &b)
	{
		b.GetCenter(mCenter);
		b.GetExtents(mExtents);
	}
	//! Destructor
	inline ~CollisionAABB() {}

	//! Get min point of the box
	inline void GetMin(Point &min) const { min = mCenter - mExtents; }
	//! Get max point of the box
	inline void GetMax(Point &max) const { max = mCenter + mExtents; }

	//! Get component of the box's min point along a given axis
	inline float GetMin(uint32_t axis) const { return mCenter[axis] - mExtents[axis]; }
	//! Get component of the box's max point along a given axis
	inline float GetMax(uint32_t axis) const { return mCenter[axis] + mExtents[axis]; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Setups an AABB from min & max vectors.
		 *	\param		min			[in] the min point
		 *	\param		max			[in] the max point
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void SetMinMax(const Point &min, const Point &max)
	{
		mCenter = (max + min) * 0.5f;
		mExtents = (max - min) * 0.5f;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Checks a box is inside another box.
		 *	\param		box		[in] the other box
		 *	\return		true if current box is inside input box
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool IsInside(const CollisionAABB &box) const
	{
		return !(box.GetMin(0) > GetMin(0)) &&
			   !(box.GetMin(1) > GetMin(1)) &&
			   !(box.GetMin(2) > GetMin(2)) &&
			   !(box.GetMax(0) < GetMax(0)) &&
			   !(box.GetMax(1) < GetMax(1)) &&
			   !(box.GetMax(2) < GetMax(2));
	}

	Point mCenter;	//!< Box center
	Point mExtents; //!< Box extents
};

class QuantizedAABB
{
public:
	//! Constructor
	inline QuantizedAABB() {}
	//! Destructor
	inline ~QuantizedAABB() {}

	int16_t mCenter[3];	  //!< Quantized center
	uint16_t mExtents[3]; //!< Quantized extents
};

//! Quickly rotates & translates a vector
inline void TransformPoint(Point &dest, const Point &source, const Matrix3x3 &rot, const Point &trans)
{
	dest.x = trans.x + source.x * rot.m[0][0] + source.y * rot.m[1][0] + source.z * rot.m[2][0];
	dest.y = trans.y + source.x * rot.m[0][1] + source.y * rot.m[1][1] + source.z * rot.m[2][1];
	dest.z = trans.z + source.x * rot.m[0][2] + source.y * rot.m[1][2] + source.z * rot.m[2][2];
}

#endif //__OPC_COMMON_H__
