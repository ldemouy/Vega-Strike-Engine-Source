///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for planes.
 *	\file		IcePlane.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPLANE_H__
#define __ICEPLANE_H__
#include "IcePoint.h"
#include "IceMatrix3x3.h"
#include "IceMatrix4x4.h"
class Plane
{
public:
	//! Constructor
	inline Plane() {}
	//! Constructor from a normal and a distance
	inline Plane(float nx, float ny, float nz, float d) { Set(nx, ny, nz, d); }
	//! Constructor from a point on the plane and a normal
	inline Plane(const Point &p, const Point &n) { Set(p, n); }
	//! Constructor from three points
	inline Plane(const Point &p0, const Point &p1, const Point &p2) { Set(p0, p1, p2); }
	//! Constructor from a normal and a distance
	inline Plane(const Point &_n, float _d)
	{
		n = _n;
		d = _d;
	}
	//! Copy constructor
	inline Plane(const Plane &plane) : n(plane.n), d(plane.d) {}
	//! Destructor
	inline ~Plane() {}

	inline Plane &Zero()
	{
		n.Zero();
		d = 0.0f;
		return *this;
	}
	inline Plane &Set(float nx, float ny, float nz, float _d)
	{
		n.Set(nx, ny, nz);
		d = _d;
		return *this;
	}
	inline Plane &Set(const Point &p, const Point &_n)
	{
		n = _n;
		d = -p | _n;
		return *this;
	}
	Plane &Set(const Point &p0, const Point &p1, const Point &p2);

	inline float Distance(const Point &p) const { return (p | n) + d; }
	inline bool Belongs(const Point &p) const { return fabsf(Distance(p)) < std::numeric_limits<float>::epsilon(); }

	inline void Normalize()
	{
		float Denom = 1.0f / n.Magnitude();
		n.x *= Denom;
		n.y *= Denom;
		n.z *= Denom;
		d *= Denom;
	}

public:
	// Members
	Point n; //!< The normal to the plane
	float d; //!< The distance from the origin

	// Cast operators
	inline operator Point() const { return n; }
};

#endif // __ICEPLANE_H__
