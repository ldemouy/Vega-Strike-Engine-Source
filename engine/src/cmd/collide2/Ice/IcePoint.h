///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for 3D vectors.
 *	\file		IcePoint.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPOINT_H__
#define __ICEPOINT_H__
#include <algorithm>
#include <cmath>
#include <stdint.h>
#include "IceAxis.h"

// Forward declarations
class Matrix4x4;

class Point
{
public:
	//! Empty constructor
	inline Point() {}

	//! Constructor from floats
	inline Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	//! Copy constructor
	inline Point(const Point &p) : x(p.x), y(p.y), z(p.z) {}
	//! Destructor
	inline ~Point() {}

	//! Assignment from values
	inline Point &Set(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
		return *this;
	}

	//! Sets each element to be componentwise minimum
	inline Point &Min(const Point &p)
	{
		x = std::min(x, p.x);
		y = std::min(y, p.y);
		z = std::min(z, p.z);
		return *this;
	}
	//! Sets each element to be componentwise maximum
	inline Point &Max(const Point &p)
	{
		x = std::max(x, p.x);
		y = std::max(y, p.y);
		z = std::max(z, p.z);
		return *this;
	}

	//! Returns largest axis
	inline PointComponent LargestAxis() const
	{
		const float *Vals = &x;
		PointComponent m = _X;
		if (Vals[_Y] > Vals[m])
		{
			m = _Y;
		}
		if (Vals[_Z] > Vals[m])
		{
			m = _Z;
		}
		return m;
	}
	//! Unary operator for Point Negate = - Point
	inline Point operator-() const { return Point(-x, -y, -z); }
	//! Operator for Point Minus = Point - Point.
	inline Point operator-(const Point &p) const { return Point(x - p.x, y - p.y, z - p.z); }
	//! Operator for Point Plus = Point + Point.
	inline Point operator+(const Point &p) const { return Point(x + p.x, y + p.y, z + p.z); }
	//! Operator for float DotProd = Point | Point.
	inline float operator|(const Point &p) const { return x * p.x + y * p.y + z * p.z; }
	//! Operator for Point Scale = Point * float.
	inline Point operator*(float s) const { return Point(x * s, y * s, z * s); }
	//! Operator for Point Scale = float * Point.
	inline friend Point operator*(float s, const Point &p) { return Point(s * p.x, s * p.y, s * p.z); }
	//! Operator for Point VecProd = Point ^ Point.
	inline Point operator^(const Point &p) const
	{
		return Point(
			y * p.z - z * p.y,
			z * p.x - x * p.z,
			x * p.y - y * p.x);
	}

	//! Operator for Point Mul = Point * Matrix4x4.
	inline Point operator*(const Matrix4x4 &mat) const
	{
		class ShadowMatrix4x4
		{
		public:
			float m[4][4];
		}; // To allow inlining
		const ShadowMatrix4x4 *Mat = (const ShadowMatrix4x4 *)&mat;

		return Point(
			x * Mat->m[0][0] + y * Mat->m[1][0] + z * Mat->m[2][0] + Mat->m[3][0],
			x * Mat->m[0][1] + y * Mat->m[1][1] + z * Mat->m[2][1] + Mat->m[3][1],
			x * Mat->m[0][2] + y * Mat->m[1][2] + z * Mat->m[2][2] + Mat->m[3][2]);
	}

	//! Operator for Point /= float.
	inline Point &operator/=(float s)
	{
		s = 1.0f / s;
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
	//! Operator for Point -= Point.
	inline Point &operator-=(const Point &p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}

	// Cast operators

	inline float operator[](int n) const { return *(&x + n); }
	inline float &operator[](int n) { return *(&x + n); }

public:
	float x, y, z;
};

#endif //__ICEPOINT_H__
