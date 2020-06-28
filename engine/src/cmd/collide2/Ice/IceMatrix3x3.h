///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for 3x3 matrices.
 *	\file		IceMatrix3x3.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEMATRIX3X3_H__
#define __ICEMATRIX3X3_H__

#include "IceMemoryMacros.h"
#include "IcePoint.h"
#include <stdint.h>

class Matrix3x3
{
public:
	//! Empty constructor
	inline Matrix3x3() {}
	//! Constructor from 9 values
	inline Matrix3x3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
	}
	//! Copy constructor
	inline Matrix3x3(const Matrix3x3 &mat) { CopyMemory(m, &mat.m, 9 * sizeof(float)); }
	//! Destructor
	inline ~Matrix3x3() {}

	// Row-column access

	//! Returns a row.
	inline const Point &GetRow(const uint32_t r) const { return *(const Point *)&m[r][0]; }
	//! Returns a row.
	inline Point &GetRow(const uint32_t r) { return *(Point *)&m[r][0]; }

	// Arithmetic operators

	//! Operator for Point Mul = Matrix3x3 * Point;
	inline Point operator*(const Point &v) const { return Point(GetRow(0) | v, GetRow(1) | v, GetRow(2) | v); }

public:
	float m[3][3];
};

#endif // __ICEMATRIX3X3_H__
