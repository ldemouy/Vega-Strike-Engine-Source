///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for 4x4 matrices.
 *	\file		IceMatrix4x4.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEMATRIX4X4_H__
#define __ICEMATRIX4X4_H__

// Forward declarations
class PRS;
class PR;

class Matrix4x4
{
	//				void	LUBackwardSubstitution( sdword *indx, float* b );
	//				void	LUDecomposition( sdword* indx, float* d );

public:
	//! Empty constructor.
	inline Matrix4x4() {}
	//! Constructor from 16 values
	inline Matrix4x4(float m00, float m01, float m02, float m03,
					 float m10, float m11, float m12, float m13,
					 float m20, float m21, float m22, float m23,
					 float m30, float m31, float m32, float m33)
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;
		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;
		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;
		m[3][0] = m30;
		m[3][1] = m31;
		m[3][2] = m32;
		m[3][3] = m33;
	}
	//! Copy constructor
	inline Matrix4x4(const Matrix4x4 &mat) { CopyMemory(m, &mat.m, 16 * sizeof(float)); }
	//! Destructor.
	inline ~Matrix4x4() {}

	//! Assign values (rotation only)
	inline Matrix4x4 &Set(float m00, float m01, float m02,
						  float m10, float m11, float m12,
						  float m20, float m21, float m22)
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
		return *this;
	}
	//! Assign values
	inline Matrix4x4 &Set(float m00, float m01, float m02, float m03,
						  float m10, float m11, float m12, float m13,
						  float m20, float m21, float m22, float m23,
						  float m30, float m31, float m32, float m33)
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;
		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;
		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;
		m[3][0] = m30;
		m[3][1] = m31;
		m[3][2] = m32;
		m[3][3] = m33;
		return *this;
	}

	//! Copy from a Matrix4x4
	inline void Copy(const Matrix4x4 &source) { CopyMemory(m, source.m, 16 * sizeof(float)); }

	// Row-column access
	//! Returns a row.
	inline void GetRow(const udword r, HPoint &p) const
	{
		p.x = m[r][0];
		p.y = m[r][1];
		p.z = m[r][2];
		p.w = m[r][3];
	}
	//! Returns a row.
	inline void GetRow(const udword r, Point &p) const
	{
		p.x = m[r][0];
		p.y = m[r][1];
		p.z = m[r][2];
	}
	//! Returns a row.
	inline const HPoint &GetRow(const udword r) const { return *(const HPoint *)&m[r][0]; }
	//! Returns a row.
	inline HPoint &GetRow(const udword r) { return *(HPoint *)&m[r][0]; }
	//! Sets a row.
	inline void SetRow(const udword r, const HPoint &p)
	{
		m[r][0] = p.x;
		m[r][1] = p.y;
		m[r][2] = p.z;
		m[r][3] = p.w;
	}
	//! Sets a row.
	inline void SetRow(const udword r, const Point &p)
	{
		m[r][0] = p.x;
		m[r][1] = p.y;
		m[r][2] = p.z;
		m[r][3] = (r != 3) ? 0.0f : 1.0f;
	}
	//! Returns a column.
	inline void GetCol(const udword c, HPoint &p) const
	{
		p.x = m[0][c];
		p.y = m[1][c];
		p.z = m[2][c];
		p.w = m[3][c];
	}
	//! Returns a column.
	inline void GetCol(const udword c, Point &p) const
	{
		p.x = m[0][c];
		p.y = m[1][c];
		p.z = m[2][c];
	}
	//! Sets a column.
	inline void SetCol(const udword c, const HPoint &p)
	{
		m[0][c] = p.x;
		m[1][c] = p.y;
		m[2][c] = p.z;
		m[3][c] = p.w;
	}
	//! Sets a column.
	inline void SetCol(const udword c, const Point &p)
	{
		m[0][c] = p.x;
		m[1][c] = p.y;
		m[2][c] = p.z;
		m[3][c] = (c != 3) ? 0.0f : 1.0f;
	}

	// Translation
	//! Returns the translation part of the matrix.
	inline const HPoint &GetTrans() const { return GetRow(3); }
	//! Gets the translation part of the matrix
	inline void GetTrans(Point &p) const
	{
		p.x = m[3][0];
		p.y = m[3][1];
		p.z = m[3][2];
	}
	//! Sets the translation part of the matrix, from a Point.
	inline void SetTrans(const Point &p)
	{
		m[3][0] = p.x;
		m[3][1] = p.y;
		m[3][2] = p.z;
	}
	//! Sets the translation part of the matrix, from a HPoint.
	inline void SetTrans(const HPoint &p)
	{
		m[3][0] = p.x;
		m[3][1] = p.y;
		m[3][2] = p.z;
		m[3][3] = p.w;
	}
	//! Sets the translation part of the matrix, from floats.
	inline void SetTrans(float tx, float ty, float tz)
	{
		m[3][0] = tx;
		m[3][1] = ty;
		m[3][2] = tz;
	}

	// Scale
	//! Sets the scale from a Point. The point is put on the diagonal.
	inline void SetScale(const Point &p)
	{
		m[0][0] = p.x;
		m[1][1] = p.y;
		m[2][2] = p.z;
	}
	//! Sets the scale from floats. Values are put on the diagonal.
	inline void SetScale(float sx, float sy, float sz)
	{
		m[0][0] = sx;
		m[1][1] = sy;
		m[2][2] = sz;
	}
	//! Scales from a Point. Each row is multiplied by a component.
	void Scale(const Point &p)
	{
		m[0][0] *= p.x;
		m[1][0] *= p.y;
		m[2][0] *= p.z;
		m[0][1] *= p.x;
		m[1][1] *= p.y;
		m[2][1] *= p.z;
		m[0][2] *= p.x;
		m[1][2] *= p.y;
		m[2][2] *= p.z;
	}
	//! Scales from floats. Each row is multiplied by a value.
	void Scale(float sx, float sy, float sz)
	{
		m[0][0] *= sx;
		m[1][0] *= sy;
		m[2][0] *= sz;
		m[0][1] *= sx;
		m[1][1] *= sy;
		m[2][1] *= sz;
		m[0][2] *= sx;
		m[1][2] *= sy;
		m[2][2] *= sz;
	}

	//! Computes the trace. The trace is the sum of the 4 diagonal components.
	inline float Trace() const { return m[0][0] + m[1][1] + m[2][2] + m[3][3]; }
	//! Computes the trace of the upper 3x3 matrix.
	inline float Trace3x3() const { return m[0][0] + m[1][1] + m[2][2]; }
	//! Clears the matrix.
	inline void Zero() { ZeroMemory(&m, sizeof(m)); }
	//! Sets the identity matrix.
	inline void Identity()
	{
		Zero();
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}
	//! Checks for identity
	inline bool IsIdentity() const
	{
		return !(static_cast<uint32_t>(m[0][0]) != IEEE_1_0) &&
			   !(static_cast<uint32_t>(m[0][1]) != 0) &&
			   !(static_cast<uint32_t>(m[0][2]) != 0) &&
			   !(static_cast<uint32_t>(m[0][3]) != 0) &&
			   !(static_cast<uint32_t>(m[1][0]) != 0) &&
			   !(static_cast<uint32_t>(m[1][1]) != IEEE_1_0) &&
			   !(static_cast<uint32_t>(m[1][2]) != 0) &&
			   !(static_cast<uint32_t>(m[1][3]) != 0) &&
			   !(static_cast<uint32_t>(m[2][0]) != 0) &&
			   !(static_cast<uint32_t>(m[2][1]) != 0) &&
			   !(static_cast<uint32_t>(m[2][2]) != IEEE_1_0) &&
			   !(static_cast<uint32_t>(m[2][3]) != 0) &&
			   !(static_cast<uint32_t>(m[3][0]) != 0) &&
			   !(static_cast<uint32_t>(m[3][1]) != 0) &&
			   !(static_cast<uint32_t>(m[3][2]) != 0) &&
			   !(static_cast<uint32_t>(m[3][3]) != IEEE_1_0);
	}

	//! Checks matrix validity
	inline bool IsValid() const
	{
		for (udword j = 0; j < 4; j++)
		{
			for (udword i = 0; i < 4; i++)
			{
				if (isnanf(m[j][i]))
				{
					return false;
				}
			}
		}
		return true;
	}

	//! Sets a rotation matrix around the X axis.
	void RotX(float angle)
	{
		float Cos = cosf(angle), Sin = sinf(angle);
		Identity();
		m[1][1] = m[2][2] = Cos;
		m[2][1] = -Sin;
		m[1][2] = Sin;
	}
	//! Sets a rotation matrix around the Y axis.
	void RotY(float angle)
	{
		float Cos = cosf(angle), Sin = sinf(angle);
		Identity();
		m[0][0] = m[2][2] = Cos;
		m[2][0] = Sin;
		m[0][2] = -Sin;
	}
	//! Sets a rotation matrix around the Z axis.
	void RotZ(float angle)
	{
		float Cos = cosf(angle), Sin = sinf(angle);
		Identity();
		m[0][0] = m[1][1] = Cos;
		m[1][0] = -Sin;
		m[0][1] = Sin;
	}

	//! Makes a rotation matrix about an arbitrary axis
	Matrix4x4 &Rot(float angle, Point &p1, Point &p2);

	//! Transposes the matrix.
	void Transpose()
	{
		//TODO: Figure out why this is being casted to ints
		(uint32_t &)(m[1][0]) ^= static_cast<uint32_t>(m[0][1]);
		(uint32_t &)(m[0][1]) ^= static_cast<uint32_t>(m[1][0]);
		(uint32_t &)(m[1][0]) ^= static_cast<uint32_t>(m[0][1]);
		(uint32_t &)(m[2][0]) ^= static_cast<uint32_t>(m[0][2]);
		(uint32_t &)(m[0][2]) ^= static_cast<uint32_t>(m[2][0]);
		(uint32_t &)(m[2][0]) ^= static_cast<uint32_t>(m[0][2]);
		(uint32_t &)(m[3][0]) ^= static_cast<uint32_t>(m[0][3]);
		(uint32_t &)(m[0][3]) ^= static_cast<uint32_t>(m[3][0]);
		(uint32_t &)(m[3][0]) ^= static_cast<uint32_t>(m[0][3]);
		(uint32_t &)(m[1][2]) ^= static_cast<uint32_t>(m[2][1]);
		(uint32_t &)(m[2][1]) ^= static_cast<uint32_t>(m[1][2]);
		(uint32_t &)(m[1][2]) ^= static_cast<uint32_t>(m[2][1]);
		(uint32_t &)(m[1][3]) ^= static_cast<uint32_t>(m[3][1]);
		(uint32_t &)(m[3][1]) ^= static_cast<uint32_t>(m[1][3]);
		(uint32_t &)(m[1][3]) ^= static_cast<uint32_t>(m[3][1]);
		(uint32_t &)(m[2][3]) ^= static_cast<uint32_t>(m[3][2]);
		(uint32_t &)(m[3][2]) ^= static_cast<uint32_t>(m[2][3]);
		(uint32_t &)(m[2][3]) ^= static_cast<uint32_t>(m[3][2]);
	}

	//! Computes a cofactor. Used for matrix inversion.
	float CoFactor(udword row, udword col) const;
	//! Computes the determinant of the matrix.
	float Determinant() const;
	//! Inverts the matrix. Determinant must be different from zero, else matrix can't be inverted.
	Matrix4x4 &Invert();
	//				Matrix&	ComputeAxisMatrix(Point& axis, float angle);

	// Cast operators
	//! Casts a Matrix4x4 to a Matrix3x3.
	inline operator Matrix3x3() const
	{
		return Matrix3x3(
			m[0][0], m[0][1], m[0][2],
			m[1][0], m[1][1], m[1][2],
			m[2][0], m[2][1], m[2][2]);
	}
	//! Casts a Matrix4x4 to a Quat.
	operator Quat() const;
	//! Casts a Matrix4x4 to a PR.
	operator PR() const;

	// Arithmetic operators
	//! Operator for Matrix4x4 Plus = Matrix4x4 + Matrix4x4;
	inline Matrix4x4 operator+(const Matrix4x4 &mat) const
	{
		return Matrix4x4(
			m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2], m[0][3] + mat.m[0][3],
			m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1], m[1][2] + mat.m[1][2], m[1][3] + mat.m[1][3],
			m[2][0] + mat.m[2][0], m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2], m[2][3] + mat.m[2][3],
			m[3][0] + mat.m[3][0], m[3][1] + mat.m[3][1], m[3][2] + mat.m[3][2], m[3][3] + mat.m[3][3]);
	}

	//! Operator for Matrix4x4 Minus = Matrix4x4 - Matrix4x4;
	inline Matrix4x4 operator-(const Matrix4x4 &mat) const
	{
		return Matrix4x4(
			m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2], m[0][3] - mat.m[0][3],
			m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1], m[1][2] - mat.m[1][2], m[1][3] - mat.m[1][3],
			m[2][0] - mat.m[2][0], m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2], m[2][3] - mat.m[2][3],
			m[3][0] - mat.m[3][0], m[3][1] - mat.m[3][1], m[3][2] - mat.m[3][2], m[3][3] - mat.m[3][3]);
	}

	//! Operator for Matrix4x4 Mul = Matrix4x4 * Matrix4x4;
	inline Matrix4x4 operator*(const Matrix4x4 &mat) const
	{
		return Matrix4x4(
			m[0][0] * mat.m[0][0] + m[0][1] * mat.m[1][0] + m[0][2] * mat.m[2][0] + m[0][3] * mat.m[3][0],
			m[0][0] * mat.m[0][1] + m[0][1] * mat.m[1][1] + m[0][2] * mat.m[2][1] + m[0][3] * mat.m[3][1],
			m[0][0] * mat.m[0][2] + m[0][1] * mat.m[1][2] + m[0][2] * mat.m[2][2] + m[0][3] * mat.m[3][2],
			m[0][0] * mat.m[0][3] + m[0][1] * mat.m[1][3] + m[0][2] * mat.m[2][3] + m[0][3] * mat.m[3][3],

			m[1][0] * mat.m[0][0] + m[1][1] * mat.m[1][0] + m[1][2] * mat.m[2][0] + m[1][3] * mat.m[3][0],
			m[1][0] * mat.m[0][1] + m[1][1] * mat.m[1][1] + m[1][2] * mat.m[2][1] + m[1][3] * mat.m[3][1],
			m[1][0] * mat.m[0][2] + m[1][1] * mat.m[1][2] + m[1][2] * mat.m[2][2] + m[1][3] * mat.m[3][2],
			m[1][0] * mat.m[0][3] + m[1][1] * mat.m[1][3] + m[1][2] * mat.m[2][3] + m[1][3] * mat.m[3][3],

			m[2][0] * mat.m[0][0] + m[2][1] * mat.m[1][0] + m[2][2] * mat.m[2][0] + m[2][3] * mat.m[3][0],
			m[2][0] * mat.m[0][1] + m[2][1] * mat.m[1][1] + m[2][2] * mat.m[2][1] + m[2][3] * mat.m[3][1],
			m[2][0] * mat.m[0][2] + m[2][1] * mat.m[1][2] + m[2][2] * mat.m[2][2] + m[2][3] * mat.m[3][2],
			m[2][0] * mat.m[0][3] + m[2][1] * mat.m[1][3] + m[2][2] * mat.m[2][3] + m[2][3] * mat.m[3][3],

			m[3][0] * mat.m[0][0] + m[3][1] * mat.m[1][0] + m[3][2] * mat.m[2][0] + m[3][3] * mat.m[3][0],
			m[3][0] * mat.m[0][1] + m[3][1] * mat.m[1][1] + m[3][2] * mat.m[2][1] + m[3][3] * mat.m[3][1],
			m[3][0] * mat.m[0][2] + m[3][1] * mat.m[1][2] + m[3][2] * mat.m[2][2] + m[3][3] * mat.m[3][2],
			m[3][0] * mat.m[0][3] + m[3][1] * mat.m[1][3] + m[3][2] * mat.m[2][3] + m[3][3] * mat.m[3][3]);
	}

	//! Operator for HPoint Mul = Matrix4x4 * HPoint;
	inline HPoint operator*(const HPoint &v) const { return HPoint(GetRow(0) | v, GetRow(1) | v, GetRow(2) | v, GetRow(3) | v); }

	//! Operator for Point Mul = Matrix4x4 * Point;
	inline Point operator*(const Point &v) const
	{
		return Point(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
					 m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
					 m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
	}

	//! Operator for Matrix4x4 Scale = Matrix4x4 * float;
	inline Matrix4x4 operator*(float s) const
	{
		return Matrix4x4(
			m[0][0] * s, m[0][1] * s, m[0][2] * s, m[0][3] * s,
			m[1][0] * s, m[1][1] * s, m[1][2] * s, m[1][3] * s,
			m[2][0] * s, m[2][1] * s, m[2][2] * s, m[2][3] * s,
			m[3][0] * s, m[3][1] * s, m[3][2] * s, m[3][3] * s);
	}

	//! Operator for Matrix4x4 Scale = float * Matrix4x4;
	inline friend Matrix4x4 operator*(float s, const Matrix4x4 &mat)
	{
		return Matrix4x4(
			s * mat.m[0][0], s * mat.m[0][1], s * mat.m[0][2], s * mat.m[0][3],
			s * mat.m[1][0], s * mat.m[1][1], s * mat.m[1][2], s * mat.m[1][3],
			s * mat.m[2][0], s * mat.m[2][1], s * mat.m[2][2], s * mat.m[2][3],
			s * mat.m[3][0], s * mat.m[3][1], s * mat.m[3][2], s * mat.m[3][3]);
	}

	//! Operator for Matrix4x4 Div = Matrix4x4 / float;
	inline Matrix4x4 operator/(float s) const
	{
		if (s)
			s = 1.0f / s;

		return Matrix4x4(
			m[0][0] * s, m[0][1] * s, m[0][2] * s, m[0][3] * s,
			m[1][0] * s, m[1][1] * s, m[1][2] * s, m[1][3] * s,
			m[2][0] * s, m[2][1] * s, m[2][2] * s, m[2][3] * s,
			m[3][0] * s, m[3][1] * s, m[3][2] * s, m[3][3] * s);
	}

	//! Operator for Matrix4x4 Div = float / Matrix4x4;
	inline friend Matrix4x4 operator/(float s, const Matrix4x4 &mat)
	{
		return Matrix4x4(
			s / mat.m[0][0], s / mat.m[0][1], s / mat.m[0][2], s / mat.m[0][3],
			s / mat.m[1][0], s / mat.m[1][1], s / mat.m[1][2], s / mat.m[1][3],
			s / mat.m[2][0], s / mat.m[2][1], s / mat.m[2][2], s / mat.m[2][3],
			s / mat.m[3][0], s / mat.m[3][1], s / mat.m[3][2], s / mat.m[3][3]);
	}

	//! Operator for Matrix4x4 += Matrix4x4;
	inline Matrix4x4 &operator+=(const Matrix4x4 &mat)
	{
		m[0][0] += mat.m[0][0];
		m[0][1] += mat.m[0][1];
		m[0][2] += mat.m[0][2];
		m[0][3] += mat.m[0][3];
		m[1][0] += mat.m[1][0];
		m[1][1] += mat.m[1][1];
		m[1][2] += mat.m[1][2];
		m[1][3] += mat.m[1][3];
		m[2][0] += mat.m[2][0];
		m[2][1] += mat.m[2][1];
		m[2][2] += mat.m[2][2];
		m[2][3] += mat.m[2][3];
		m[3][0] += mat.m[3][0];
		m[3][1] += mat.m[3][1];
		m[3][2] += mat.m[3][2];
		m[3][3] += mat.m[3][3];
		return *this;
	}

	//! Operator for Matrix4x4 -= Matrix4x4;
	inline Matrix4x4 &operator-=(const Matrix4x4 &mat)
	{
		m[0][0] -= mat.m[0][0];
		m[0][1] -= mat.m[0][1];
		m[0][2] -= mat.m[0][2];
		m[0][3] -= mat.m[0][3];
		m[1][0] -= mat.m[1][0];
		m[1][1] -= mat.m[1][1];
		m[1][2] -= mat.m[1][2];
		m[1][3] -= mat.m[1][3];
		m[2][0] -= mat.m[2][0];
		m[2][1] -= mat.m[2][1];
		m[2][2] -= mat.m[2][2];
		m[2][3] -= mat.m[2][3];
		m[3][0] -= mat.m[3][0];
		m[3][1] -= mat.m[3][1];
		m[3][2] -= mat.m[3][2];
		m[3][3] -= mat.m[3][3];
		return *this;
	}

	//! Operator for Matrix4x4 *= Matrix4x4;
	Matrix4x4 &operator*=(const Matrix4x4 &mat)
	{
		HPoint TempRow;

		GetRow(0, TempRow);
		m[0][0] = TempRow.x * mat.m[0][0] + TempRow.y * mat.m[1][0] + TempRow.z * mat.m[2][0] + TempRow.w * mat.m[3][0];
		m[0][1] = TempRow.x * mat.m[0][1] + TempRow.y * mat.m[1][1] + TempRow.z * mat.m[2][1] + TempRow.w * mat.m[3][1];
		m[0][2] = TempRow.x * mat.m[0][2] + TempRow.y * mat.m[1][2] + TempRow.z * mat.m[2][2] + TempRow.w * mat.m[3][2];
		m[0][3] = TempRow.x * mat.m[0][3] + TempRow.y * mat.m[1][3] + TempRow.z * mat.m[2][3] + TempRow.w * mat.m[3][3];

		GetRow(1, TempRow);
		m[1][0] = TempRow.x * mat.m[0][0] + TempRow.y * mat.m[1][0] + TempRow.z * mat.m[2][0] + TempRow.w * mat.m[3][0];
		m[1][1] = TempRow.x * mat.m[0][1] + TempRow.y * mat.m[1][1] + TempRow.z * mat.m[2][1] + TempRow.w * mat.m[3][1];
		m[1][2] = TempRow.x * mat.m[0][2] + TempRow.y * mat.m[1][2] + TempRow.z * mat.m[2][2] + TempRow.w * mat.m[3][2];
		m[1][3] = TempRow.x * mat.m[0][3] + TempRow.y * mat.m[1][3] + TempRow.z * mat.m[2][3] + TempRow.w * mat.m[3][3];

		GetRow(2, TempRow);
		m[2][0] = TempRow.x * mat.m[0][0] + TempRow.y * mat.m[1][0] + TempRow.z * mat.m[2][0] + TempRow.w * mat.m[3][0];
		m[2][1] = TempRow.x * mat.m[0][1] + TempRow.y * mat.m[1][1] + TempRow.z * mat.m[2][1] + TempRow.w * mat.m[3][1];
		m[2][2] = TempRow.x * mat.m[0][2] + TempRow.y * mat.m[1][2] + TempRow.z * mat.m[2][2] + TempRow.w * mat.m[3][2];
		m[2][3] = TempRow.x * mat.m[0][3] + TempRow.y * mat.m[1][3] + TempRow.z * mat.m[2][3] + TempRow.w * mat.m[3][3];

		GetRow(3, TempRow);
		m[3][0] = TempRow.x * mat.m[0][0] + TempRow.y * mat.m[1][0] + TempRow.z * mat.m[2][0] + TempRow.w * mat.m[3][0];
		m[3][1] = TempRow.x * mat.m[0][1] + TempRow.y * mat.m[1][1] + TempRow.z * mat.m[2][1] + TempRow.w * mat.m[3][1];
		m[3][2] = TempRow.x * mat.m[0][2] + TempRow.y * mat.m[1][2] + TempRow.z * mat.m[2][2] + TempRow.w * mat.m[3][2];
		m[3][3] = TempRow.x * mat.m[0][3] + TempRow.y * mat.m[1][3] + TempRow.z * mat.m[2][3] + TempRow.w * mat.m[3][3];

		return *this;
	}

	//! Operator for Matrix4x4 *= float;
	inline Matrix4x4 &operator*=(float s)
	{
		m[0][0] *= s;
		m[0][1] *= s;
		m[0][2] *= s;
		m[0][3] *= s;
		m[1][0] *= s;
		m[1][1] *= s;
		m[1][2] *= s;
		m[1][3] *= s;
		m[2][0] *= s;
		m[2][1] *= s;
		m[2][2] *= s;
		m[2][3] *= s;
		m[3][0] *= s;
		m[3][1] *= s;
		m[3][2] *= s;
		m[3][3] *= s;
		return *this;
	}

	//! Operator for Matrix4x4 /= float;
	inline Matrix4x4 &operator/=(float s)
	{
		if (s)
			s = 1.0f / s;
		m[0][0] *= s;
		m[0][1] *= s;
		m[0][2] *= s;
		m[0][3] *= s;
		m[1][0] *= s;
		m[1][1] *= s;
		m[1][2] *= s;
		m[1][3] *= s;
		m[2][0] *= s;
		m[2][1] *= s;
		m[2][2] *= s;
		m[2][3] *= s;
		m[3][0] *= s;
		m[3][1] *= s;
		m[3][2] *= s;
		m[3][3] *= s;
		return *this;
	}

	inline const HPoint &operator[](int row) const { return *(const HPoint *)&m[row][0]; }
	inline HPoint &operator[](int row) { return *(HPoint *)&m[row][0]; }

public:
	float m[4][4];
};

//! Quickly rotates & translates a vector, using the 4x3 part of a 4x4 matrix
inline void TransformPoint4x3(Point &dest, const Point &source, const Matrix4x4 &rot)
{
	dest.x = rot.m[3][0] + source.x * rot.m[0][0] + source.y * rot.m[1][0] + source.z * rot.m[2][0];
	dest.y = rot.m[3][1] + source.x * rot.m[0][1] + source.y * rot.m[1][1] + source.z * rot.m[2][1];
	dest.z = rot.m[3][2] + source.x * rot.m[0][2] + source.y * rot.m[1][2] + source.z * rot.m[2][2];
}

//! Quickly rotates a vector, using the 3x3 part of a 4x4 matrix
inline void TransformPoint3x3(Point &dest, const Point &source, const Matrix4x4 &rot)
{
	dest.x = source.x * rot.m[0][0] + source.y * rot.m[1][0] + source.z * rot.m[2][0];
	dest.y = source.x * rot.m[0][1] + source.y * rot.m[1][1] + source.z * rot.m[2][1];
	dest.z = source.x * rot.m[0][2] + source.y * rot.m[1][2] + source.z * rot.m[2][2];
}

void InvertPRMatrix(Matrix4x4 &dest, const Matrix4x4 &src);

#endif // __ICEMATRIX4X4_H__
