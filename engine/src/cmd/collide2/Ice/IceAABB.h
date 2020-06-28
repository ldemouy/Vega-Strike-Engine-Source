///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains AABB-related code. (axis-aligned bounding box)
 *	\file		IceAABB.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEAABB_H__
#define __ICEAABB_H__
#include <cmath>
#include "IcePoint.h"
#include "IceMatrix3x3.h"
#include "IceMatrix4x4.h"

class AABB
{
public:
	//! Constructor
	inline AABB() {}
	//! Destructor
	inline ~AABB() {}

	//! Type-independent methods
	AABB &Add(const AABB &aabb);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Setups an AABB from min & max vectors.
		 *	\param		min			[in] the min point
		 *	\param		max			[in] the max point
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetMinMax(const Point &min, const Point &max)
	{
		mCenter = (max + min) * 0.5f;
		mExtents = (max - min) * 0.5f;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Setups an empty AABB.
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetEmpty()
	{
		mCenter.Zero();
		mExtents.Set(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Extends the AABB.
		 *	\param		p	[in] the next point
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Extend(const Point &p)
	{
		Point Max = mCenter + mExtents;
		Point Min = mCenter - mExtents;

		if (p.x > Max.x)
		{
			Max.x = p.x;
		}
		if (p.x < Min.x)
		{
			Min.x = p.x;
		}

		if (p.y > Max.y)
		{
			Max.y = p.y;
		}
		if (p.y < Min.y)
		{
			Min.y = p.y;
		}

		if (p.z > Max.z)
		{
			Max.z = p.z;
		}
		if (p.z < Min.z)
		{
			Min.z = p.z;
		}

		SetMinMax(Min, Max);
	}
	// Data access

	//! Get min point of the box
	inline void GetMin(Point &min) const { min = mCenter - mExtents; }
	//! Get max point of the box
	inline void GetMax(Point &max) const { max = mCenter + mExtents; }

	//! Get box center
	inline void GetCenter(Point &center) const { center = mCenter; }
	//! Get box extents
	inline void GetExtents(Point &extents) const { extents = mExtents; }

	//! Get component of the box's center along a given axis
	inline float GetCenter(uint32_t axis) const { return mCenter[axis]; }

private:
	Point mCenter;	//!< AABB Center
	Point mExtents; //!< x, y and z extents
};

#endif // __ICEAABB_H__
