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
#include "IceBoundingSphere.h"
#include "IcePlane.h"
#include "IceMatrix3x3.h"
#include "IceMatrix4x4.h"

//! Declarations of type-independent methods (most of them implemented in the .cpp)

enum AABBType
{
	AABB_RENDER = 0, //!< AABB used for rendering. Not visible == not rendered.
	AABB_UPDATE = 1, //!< AABB used for dynamic updates. Not visible == not updated.

	AABB_FORCE_DWORD = 0x7fffffff
};

class AABB
{
public:
	//! Constructor
	inline AABB() {}
	//! Destructor
	inline ~AABB() {}

	//! Type-independent methods
	AABB &Add(const AABB &aabb);
	float MakeCube(AABB &cube) const;
	void MakeSphere(Sphere &sphere) const;
	const int8_t *ComputeOutline(const Point &local_eye, int32_t &num) const;
	bool IsInside(const AABB &box) const;
	bool ComputePlanes(Plane *planes) const;
	bool ComputePoints(Point *pts) const;
	const Point *GetVertexNormals() const;
	const uint32_t *GetEdges() const;
	const Point *GetEdgeNormals() const;
	inline bool ContainsPoint(const Point &p) const
	{
		return !(p.x > GetMax(0) || p.x < GetMin(0)) &&
			   !(p.y > GetMax(1) || p.y < GetMin(1)) &&
			   !(p.z > GetMax(2) || p.z < GetMin(2));
	}

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
		 *	Setups an AABB from center & extents vectors.
		 *	\param		c			[in] the center point
		 *	\param		e			[in] the extents vector
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetCenterExtents(const Point &c, const Point &e)
	{
		mCenter = c;
		mExtents = e;
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
		 *	Setups a point AABB.
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetPoint(const Point &pt)
	{
		mCenter = pt;
		mExtents.Zero();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Gets the size of the AABB. The size is defined as the longest extent.
		 *	\return		the size of the AABB
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float GetSize() const { return mExtents.Max(); }

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

	//! Get component of the box's min point along a given axis
	inline float GetMin(uint32_t axis) const { return mCenter[axis] - mExtents[axis]; }
	//! Get component of the box's max point along a given axis
	inline float GetMax(uint32_t axis) const { return mCenter[axis] + mExtents[axis]; }

	//! Get box center
	inline void GetCenter(Point &center) const { center = mCenter; }
	//! Get box extents
	inline void GetExtents(Point &extents) const { extents = mExtents; }

	//! Get component of the box's center along a given axis
	inline float GetCenter(uint32_t axis) const { return mCenter[axis]; }
	//! Get component of the box's extents along a given axis
	inline float GetExtents(uint32_t axis) const { return mExtents[axis]; }

	//! Get box diagonal
	inline void GetDiagonal(Point &diagonal) const { diagonal = mExtents * 2.0f; }
	inline float GetWidth() const { return mExtents.x * 2.0f; }
	inline float GetHeight() const { return mExtents.y * 2.0f; }
	inline float GetDepth() const { return mExtents.z * 2.0f; }

	//! Volume
	inline float GetVolume() const { return mExtents.x * mExtents.y * mExtents.z * 8.0f; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Computes the intersection between two AABBs.
		 *	\param		a		[in] the other AABB
		 *	\return		true on intersection
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool Intersect(const AABB &a) const
	{
		float tx = mCenter.x - a.mCenter.x;
		float ex = a.mExtents.x + mExtents.x;
		if (static_cast<uint32_t>(std::abs(tx)) > static_cast<uint32_t>(ex))
		{
			return false;
		}
		float ty = mCenter.y - a.mCenter.y;
		float ey = a.mExtents.y + mExtents.y;
		if (static_cast<uint32_t>(std::abs(ty)) > static_cast<uint32_t>(ey))
		{
			return false;
		}
		float tz = mCenter.z - a.mCenter.z;
		float ez = a.mExtents.z + mExtents.z;
		if (static_cast<uint32_t>(std::abs(tz)) > static_cast<uint32_t>(ez))
		{
			return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	The standard intersection method from Gamasutra. Just here to check its speed against the one above.
		 *	\param		a		[in] the other AABB
		 *	\return		true on intersection
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool GomezIntersect(const AABB &a)
	{
		Point T = mCenter - a.mCenter; // Vector from A to B
		return ((fabsf(T.x) <= (a.mExtents.x + mExtents.x)) && (fabsf(T.y) <= (a.mExtents.y + mExtents.y)) && (fabsf(T.z) <= (a.mExtents.z + mExtents.z)));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Computes the 1D-intersection between two AABBs, on a given axis.
		 *	\param		a		[in] the other AABB
		 *	\param		axis	[in] the axis (0, 1, 2)
		 *	\return		true on intersection
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool Intersect(const AABB &a, uint32_t axis) const
	{
		float t = mCenter[axis] - a.mCenter[axis];
		float e = a.mExtents[axis] + mExtents[axis];
		return !(static_cast<uint32_t>(std::abs(t)) > static_cast<uint32_t>(e));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Recomputes the AABB after an arbitrary transform by a 4x4 matrix.
		 *	\param		mtx			[in] the transform matrix
		 *	\param		aabb		[out] the transformed AABB [can be *this]
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void Rotate(const Matrix4x4 &mtx, AABB &aabb) const
	{
		// Compute new center
		aabb.mCenter = mCenter * mtx;

		// Compute new extents.
		Point Ex(mtx.m[0][0] * mExtents.x, mtx.m[0][1] * mExtents.x, mtx.m[0][2] * mExtents.x);
		Ex.x = std::abs(Ex.x);
		Ex.y = std::abs(Ex.y);
		Ex.z = std::abs(Ex.z);

		Point Ey(mtx.m[1][0] * mExtents.y, mtx.m[1][1] * mExtents.y, mtx.m[1][2] * mExtents.y);
		Ey.x = std::abs(Ey.x);
		Ey.y = std::abs(Ey.y);
		Ey.z = std::abs(Ey.z);

		Point Ez(mtx.m[2][0] * mExtents.z, mtx.m[2][1] * mExtents.z, mtx.m[2][2] * mExtents.z);
		Ez.x = std::abs(Ez.x);
		Ez.y = std::abs(Ez.y);
		Ez.z = std::abs(Ez.z);

		aabb.mExtents.x = Ex.x + Ey.x + Ez.x;
		aabb.mExtents.y = Ex.y + Ey.y + Ez.y;
		aabb.mExtents.z = Ex.z + Ey.z + Ez.z;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Checks the AABB is valid.
		 *	\return		true if the box is valid
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool IsValid() const
	{
		// Consistency condition for (Center, Extents) boxes: Extents >= 0
		return !(mExtents.x < 0.0f) &&
			   !(mExtents.y < 0.0f) &&
			   !(mExtents.z < 0.0f);
	}

	//! Operator for AABB *= float. Scales the extents, keeps same center.
	inline AABB &operator*=(float s)
	{
		mExtents *= s;
		return *this;
	}

	//! Operator for AABB /= float. Scales the extents, keeps same center.
	inline AABB &operator/=(float s)
	{
		mExtents /= s;
		return *this;
	}

	//! Operator for AABB += Point. Translates the box.
	inline AABB &operator+=(const Point &trans)
	{
		mCenter += trans;
		return *this;
	}

private:
	Point mCenter;	//!< AABB Center
	Point mExtents; //!< x, y and z extents
};

inline void ComputeMinMax(const Point &p, Point &min, Point &max)
{
	if (p.x > max.x)
	{
		max.x = p.x;
	}
	if (p.x < min.x)
	{
		min.x = p.x;
	}

	if (p.y > max.y)
	{
		max.y = p.y;
	}
	if (p.y < min.y)
	{
		min.y = p.y;
	}

	if (p.z > max.z)
	{
		max.z = p.z;
	}
	if (p.z < min.z)
	{
		min.z = p.z;
	}
}

inline void ComputeAABB(AABB &aabb, const Point *list, uint32_t nb_pts)
{
	if (list)
	{
		Point Maxi(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
		Point Mini(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		while (nb_pts--)
		{
			//				_prefetch(list+1);	// off by one ?
			ComputeMinMax(*list++, Mini, Maxi);
		}
		aabb.SetMinMax(Mini, Maxi);
	}
}

#endif // __ICEAABB_H__
