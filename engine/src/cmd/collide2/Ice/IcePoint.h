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
#include <cmath>
// Forward declarations
class HPoint;
class Plane;
class Matrix3x3;
class Matrix4x4;

#define CROSS2D(a, b) (a.x * b.y - b.x * a.y)

class Point
{
public:
	//! Empty constructor
	inline Point() {}
	//! Constructor from a single float
	//		inline					Point(float val) : x(val), y(val), z(val)					{}
	// Removed since it introduced the nasty "Point T = *Matrix4x4.GetTrans();" bug.......
	//! Constructor from floats
	inline Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	//! Constructor from array
	inline Point(const float f[3]) : x(f[_X]), y(f[_Y]), z(f[_Z]) {}
	//! Copy constructor
	inline Point(const Point &p) : x(p.x), y(p.y), z(p.z) {}
	//! Destructor
	inline ~Point() {}

	//! Clears the vector
	inline Point &Zero()
	{
		x = y = z = 0.0f;
		return *this;
	}

	//! + infinity
	inline Point &SetPlusInfinity()
	{
		x = y = z = std::numeric_limits<float>::max();
		return *this;
	}
	//! - infinity
	inline Point &SetMinusInfinity()
	{
		x = y = z = std::numeric_limits<float>::min();
		return *this;
	}

	//! Sets positive unit random vector
	Point &PositiveUnitRandomVector();
	//! Sets unit random vector
	Point &UnitRandomVector();

	//! Assignment from values
	inline Point &Set(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
		return *this;
	}
	//! Assignment from array
	inline Point &Set(const float f[3])
	{
		x = f[_X];
		y = f[_Y];
		z = f[_Z];
		return *this;
	}
	//! Assignment from another point
	inline Point &Set(const Point &src)
	{
		x = src.x;
		y = src.y;
		z = src.z;
		return *this;
	}

	//! Adds a vector
	inline Point &Add(const Point &p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}
	//! Adds a vector
	inline Point &Add(float _x, float _y, float _z)
	{
		x += _x;
		y += _y;
		z += _z;
		return *this;
	}
	//! Adds a vector
	inline Point &Add(const float f[3])
	{
		x += f[_X];
		y += f[_Y];
		z += f[_Z];
		return *this;
	}
	//! Adds vectors
	inline Point &Add(const Point &p, const Point &q)
	{
		x = p.x + q.x;
		y = p.y + q.y;
		z = p.z + q.z;
		return *this;
	}

	//! Subtracts a vector
	inline Point &Sub(const Point &p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}
	//! Subtracts a vector
	inline Point &Sub(float _x, float _y, float _z)
	{
		x -= _x;
		y -= _y;
		z -= _z;
		return *this;
	}
	//! Subtracts a vector
	inline Point &Sub(const float f[3])
	{
		x -= f[_X];
		y -= f[_Y];
		z -= f[_Z];
		return *this;
	}
	//! Subtracts vectors
	inline Point &Sub(const Point &p, const Point &q)
	{
		x = p.x - q.x;
		y = p.y - q.y;
		z = p.z - q.z;
		return *this;
	}

	//! this = -this
	inline Point &Neg()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}
	//! this = -a
	inline Point &Neg(const Point &a)
	{
		x = -a.x;
		y = -a.y;
		z = -a.z;
		return *this;
	}

	//! Multiplies by a scalar
	inline Point &Mult(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	//! this = a * scalar
	inline Point &Mult(const Point &a, float scalar)
	{
		x = a.x * scalar;
		y = a.y * scalar;
		z = a.z * scalar;
		return *this;
	}

	//! this = a + b * scalar
	inline Point &Mac(const Point &a, const Point &b, float scalar)
	{
		x = a.x + b.x * scalar;
		y = a.y + b.y * scalar;
		z = a.z + b.z * scalar;
		return *this;
	}

	//! this = this + a * scalar
	inline Point &Mac(const Point &a, float scalar)
	{
		x += a.x * scalar;
		y += a.y * scalar;
		z += a.z * scalar;
		return *this;
	}

	//! this = a - b * scalar
	inline Point &Msc(const Point &a, const Point &b, float scalar)
	{
		x = a.x - b.x * scalar;
		y = a.y - b.y * scalar;
		z = a.z - b.z * scalar;
		return *this;
	}

	//! this = this - a * scalar
	inline Point &Msc(const Point &a, float scalar)
	{
		x -= a.x * scalar;
		y -= a.y * scalar;
		z -= a.z * scalar;
		return *this;
	}

	//! this = a + b * scalarb + c * scalarc
	inline Point &Mac2(const Point &a, const Point &b, float scalarb, const Point &c, float scalarc)
	{
		x = a.x + b.x * scalarb + c.x * scalarc;
		y = a.y + b.y * scalarb + c.y * scalarc;
		z = a.z + b.z * scalarb + c.z * scalarc;
		return *this;
	}

	//! this = a - b * scalarb - c * scalarc
	inline Point &Msc2(const Point &a, const Point &b, float scalarb, const Point &c, float scalarc)
	{
		x = a.x - b.x * scalarb - c.x * scalarc;
		y = a.y - b.y * scalarb - c.y * scalarc;
		z = a.z - b.z * scalarb - c.z * scalarc;
		return *this;
	}

	//! this = mat * a
	inline Point &Mult(const Matrix3x3 &mat, const Point &a);

	//! this = mat1 * a1 + mat2 * a2
	inline Point &Mult2(const Matrix3x3 &mat1, const Point &a1, const Matrix3x3 &mat2, const Point &a2);

	//! this = this + mat * a
	inline Point &Mac(const Matrix3x3 &mat, const Point &a);

	//! this = transpose(mat) * a
	inline Point &TransMult(const Matrix3x3 &mat, const Point &a);

	//! Linear interpolate between two vectors: this = a + t * (b - a)
	inline Point &Lerp(const Point &a, const Point &b, float t)
	{
		x = a.x + t * (b.x - a.x);
		y = a.y + t * (b.y - a.y);
		z = a.z + t * (b.z - a.z);
		return *this;
	}

	//! Hermite interpolate between p1 and p2. p0 and p3 are used for finding gradient at p1 and p2.
	//! this =	p0 * (2t^2 - t^3 - t)/2
	//!			+ p1 * (3t^3 - 5t^2 + 2)/2
	//!			+ p2 * (4t^2 - 3t^3 + t)/2
	//!			+ p3 * (t^3 - t^2)/2
	inline Point &Herp(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float t)
	{
		float t2 = t * t;
		float t3 = t2 * t;
		float kp0 = (2.0f * t2 - t3 - t) * 0.5f;
		float kp1 = (3.0f * t3 - 5.0f * t2 + 2.0f) * 0.5f;
		float kp2 = (4.0f * t2 - 3.0f * t3 + t) * 0.5f;
		float kp3 = (t3 - t2) * 0.5f;
		x = p0.x * kp0 + p1.x * kp1 + p2.x * kp2 + p3.x * kp3;
		y = p0.y * kp0 + p1.y * kp1 + p2.y * kp2 + p3.y * kp3;
		z = p0.z * kp0 + p1.z * kp1 + p2.z * kp2 + p3.z * kp3;
		return *this;
	}

	//! this = rotpos * r + linpos
	inline Point &Transform(const Point &r, const Matrix3x3 &rotpos, const Point &linpos);

	//! this = trans(rotpos) * (r - linpos)
	inline Point &InvTransform(const Point &r, const Matrix3x3 &rotpos, const Point &linpos);

	//! Returns MIN(x, y, z);
	inline float Min() const { return std::min({x, y, z}); }
	//! Returns MAX(x, y, z);
	inline float Max() const { return std::max({x, y, z}); }
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

	//! Clamps each element
	inline Point &Clamp(float min, float max)
	{
		if (x < min)
			x = min;
		else if (x > max)
			x = max;
		if (y < min)
			y = min;
		else if (y > max)
			y = max;
		if (z < min)
			z = min;
		else if (z > max)
			z = max;
		return *this;
	}

	//! Computes square magnitude
	inline float SquareMagnitude() const { return x * x + y * y + z * z; }
	//! Computes magnitude
	inline float Magnitude() const { return sqrtf(x * x + y * y + z * z); }
	//! Computes volume
	inline float Volume() const { return x * y * z; }

	//! Checks the point is near zero
	inline bool ApproxZero() const { return SquareMagnitude() < std::numeric_limits<float>::epsilon(); }

	//! Tests for exact zero vector
	inline bool IsZero() const
	{
		return !(static_cast<uint32_t>(x) ||
				 static_cast<uint32_t>(y) ||
				 static_cast<uint32_t>(z));
	}

	//! Checks point validity
	inline bool IsValid() const
	{
		return !(isnanf(x) || isnanf(y) || isnanf(z));
	}

	//! Slighty moves the point
	void Tweak(udword coord_mask, udword tweak_mask)
	{
		if (coord_mask & 1)
		{
			udword Dummy = static_cast<uint32_t>(x);
			Dummy ^= tweak_mask;
			x = static_cast<float>(Dummy);
		}
		if (coord_mask & 2)
		{
			udword Dummy = static_cast<uint32_t>(y);
			Dummy ^= tweak_mask;
			y = static_cast<float>(Dummy);
		}
		if (coord_mask & 4)
		{
			udword Dummy = static_cast<uint32_t>(z);
			Dummy ^= tweak_mask;
			z = static_cast<float>(Dummy);
		}
	}

#define TWEAKMASK 0x3fffff
#define TWEAKNOTMASK ~TWEAKMASK
	//! Slighty moves the point out
	inline void TweakBigger()
	{
		uint32_t Dummy = (static_cast<uint32_t>(x) & TWEAKNOTMASK);
		if (x >= 0.0f)
		{
			Dummy += TWEAKMASK + 1;
		}
		x = static_cast<float>(Dummy);
		Dummy = (static_cast<uint32_t>(y) & TWEAKNOTMASK);
		if (y >= 0.0f)
		{
			Dummy += TWEAKMASK + 1;
		}
		y = static_cast<float>(Dummy);
		Dummy = (static_cast<uint32_t>(z) & TWEAKNOTMASK);
		if (z >= 0.0f)
		{
			Dummy += TWEAKMASK + 1;
		}
		z = static_cast<float>(Dummy);
	}

	//! Slighty moves the point in
	inline void TweakSmaller()
	{
		udword Dummy = (static_cast<uint32_t>(x) & TWEAKNOTMASK);
		if (x < 0.0f)
		{
			Dummy += TWEAKMASK + 1;
		}
		x = static_cast<float>(Dummy);
		Dummy = (static_cast<uint32_t>(y) & TWEAKNOTMASK);
		if (y < 0.0f)
		{
			Dummy += TWEAKMASK + 1;
		}
		y = static_cast<float>(Dummy);
		Dummy = (static_cast<uint32_t>(z) & TWEAKNOTMASK);
		if (z < 0.0f)
		{
			Dummy += TWEAKMASK + 1;
		}
		z = static_cast<float>(Dummy);
	}

	//! Normalizes the vector
	inline Point &Normalize()
	{
		float M = x * x + y * y + z * z;
		if (M)
		{
			M = 1.0f / sqrtf(M);
			x *= M;
			y *= M;
			z *= M;
		}
		return *this;
	}

	//! Sets vector length
	inline Point &SetLength(float length)
	{
		float NewLength = length / Magnitude();
		x *= NewLength;
		y *= NewLength;
		z *= NewLength;
		return *this;
	}

	//! Clamps vector length
	inline Point &ClampLength(float limit_length)
	{
		if (limit_length >= 0.0f) // Magnitude must be positive
		{
			float CurrentSquareLength = SquareMagnitude();

			if (CurrentSquareLength > limit_length * limit_length)
			{
				float Coeff = limit_length / sqrtf(CurrentSquareLength);
				x *= Coeff;
				y *= Coeff;
				z *= Coeff;
			}
		}
		return *this;
	}

	//! Computes distance to another point
	inline float Distance(const Point &b) const
	{
		return sqrtf((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y) + (z - b.z) * (z - b.z));
	}

	//! Computes square distance to another point
	inline float SquareDistance(const Point &b) const
	{
		return ((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y) + (z - b.z) * (z - b.z));
	}

	//! Dot product dp = this|a
	inline float Dot(const Point &p) const { return p.x * x + p.y * y + p.z * z; }

	//! Cross product this = a x b
	inline Point &Cross(const Point &a, const Point &b)
	{
		x = a.y * b.z - a.z * b.y;
		y = a.z * b.x - a.x * b.z;
		z = a.x * b.y - a.y * b.x;
		return *this;
	}

	//! Vector code ( bitmask = sign(z) | sign(y) | sign(x) )
	inline uint32_t VectorCode() const
	{
		const uint32_t SIGN_BITMASK = 0x80000000;
		return (static_cast<uint32_t>(x) >> 31) | ((static_cast<uint32_t>(y) & SIGN_BITMASK) >> 30) | ((static_cast<uint32_t>(z) & SIGN_BITMASK) >> 29);
	}

	//! Returns largest axis
	inline PointComponent LargestAxis() const
	{
		const float *Vals = &x;
		PointComponent m = _X;
		if (Vals[_Y] > Vals[m])
			m = _Y;
		if (Vals[_Z] > Vals[m])
			m = _Z;
		return m;
	}

	//! Returns closest axis
	inline PointComponent ClosestAxis() const
	{
		const float *Vals = &x;
		PointComponent m = _X;
		if (static_cast<uint32_t>(std::abs(Vals[_Y])) > static_cast<uint32_t>(std::abs(Vals[m])))
		{
			m = _Y;
		}
		if (static_cast<uint32_t>(std::abs(Vals[_Z])) > static_cast<uint32_t>(std::abs(Vals[m])))
		{
			m = _Z;
		}
		return m;
	}

	//! Returns smallest axis
	inline PointComponent SmallestAxis() const
	{
		const float *Vals = &x;
		PointComponent m = _X;
		if (Vals[_Y] < Vals[m])
			m = _Y;
		if (Vals[_Z] < Vals[m])
			m = _Z;
		return m;
	}

	//! Refracts the point
	Point &Refract(const Point &eye, const Point &n, float refractindex, Point &refracted);

	//! Projects the point onto a plane
	Point &ProjectToPlane(const Plane &p);

	//! Projects the point onto the screen
	void ProjectToScreen(float halfrenderwidth, float halfrenderheight, const Matrix4x4 &mat, HPoint &projected) const;

	//! Unfolds the point onto a plane according to edge(a,b)
	Point &Unfold(Plane &p, Point &a, Point &b);

	//! Hash function from Ville Miettinen
	inline udword GetHashValue() const
	{
		const udword *h = (const udword *)(this);
		udword f = (h[0] + h[1] * 11 - (h[2] * 17)) & 0x7fffffff; // avoid problems with +-0
		return (f >> 22) ^ (f >> 12) ^ (f);
	}

	//! Stuff magic values in the point, marking it as explicitely not used.
	void SetNotUsed();
	//! Checks the point is marked as not used
	bool IsNotUsed() const;

	// Arithmetic operators

	//! Unary operator for Point Negate = - Point
	inline Point operator-() const { return Point(-x, -y, -z); }

	//! Operator for Point Plus = Point + Point.
	inline Point operator+(const Point &p) const { return Point(x + p.x, y + p.y, z + p.z); }
	//! Operator for Point Minus = Point - Point.
	inline Point operator-(const Point &p) const { return Point(x - p.x, y - p.y, z - p.z); }

	//! Operator for Point Mul   = Point * Point.
	inline Point operator*(const Point &p) const { return Point(x * p.x, y * p.y, z * p.z); }
	//! Operator for Point Scale = Point * float.
	inline Point operator*(float s) const { return Point(x * s, y * s, z * s); }
	//! Operator for Point Scale = float * Point.
	inline friend Point operator*(float s, const Point &p) { return Point(s * p.x, s * p.y, s * p.z); }

	//! Operator for Point Div   = Point / Point.
	inline Point operator/(const Point &p) const { return Point(x / p.x, y / p.y, z / p.z); }
	//! Operator for Point Scale = Point / float.
	inline Point operator/(float s) const
	{
		s = 1.0f / s;
		return Point(x * s, y * s, z * s);
	}
	//! Operator for Point Scale = float / Point.
	inline friend Point operator/(float s, const Point &p) { return Point(s / p.x, s / p.y, s / p.z); }

	//! Operator for float DotProd = Point | Point.
	inline float operator|(const Point &p) const { return x * p.x + y * p.y + z * p.z; }
	//! Operator for Point VecProd = Point ^ Point.
	inline Point operator^(const Point &p) const
	{
		return Point(
			y * p.z - z * p.y,
			z * p.x - x * p.z,
			x * p.y - y * p.x);
	}

	//! Operator for Point += Point.
	inline Point &operator+=(const Point &p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}
	//! Operator for Point += float.
	inline Point &operator+=(float s)
	{
		x += s;
		y += s;
		z += s;
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
	//! Operator for Point -= float.
	inline Point &operator-=(float s)
	{
		x -= s;
		y -= s;
		z -= s;
		return *this;
	}

	//! Operator for Point *= Point.
	inline Point &operator*=(const Point &p)
	{
		x *= p.x;
		y *= p.y;
		z *= p.z;
		return *this;
	}
	//! Operator for Point *= float.
	inline Point &operator*=(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	//! Operator for Point /= Point.
	inline Point &operator/=(const Point &p)
	{
		x /= p.x;
		y /= p.y;
		z /= p.z;
		return *this;
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

	// Logical operators

	//! Operator for "if(Point==Point)"
	inline bool operator==(const Point &p) const { return ((static_cast<uint32_t>(x) == static_cast<uint32_t>(p.x)) && (static_cast<uint32_t>(y) == static_cast<uint32_t>(p.y)) && (static_cast<uint32_t>(z) == static_cast<uint32_t>(p.z))); }
	//! Operator for "if(Point!=Point)"
	inline bool operator!=(const Point &p) const { return ((static_cast<uint32_t>(x) != static_cast<uint32_t>(p.x)) || (static_cast<uint32_t>(y) != static_cast<uint32_t>(p.y)) || (static_cast<uint32_t>(z) != static_cast<uint32_t>(p.z))); }

	// Arithmetic operators

	//! Operator for Point Mul = Point * Matrix3x3.
	inline Point operator*(const Matrix3x3 &mat) const
	{
		class ShadowMatrix3x3
		{
		public:
			float m[3][3];
		}; // To allow inlining
		const ShadowMatrix3x3 *Mat = (const ShadowMatrix3x3 *)&mat;

		return Point(
			x * Mat->m[0][0] + y * Mat->m[1][0] + z * Mat->m[2][0],
			x * Mat->m[0][1] + y * Mat->m[1][1] + z * Mat->m[2][1],
			x * Mat->m[0][2] + y * Mat->m[1][2] + z * Mat->m[2][2]);
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

	//! Operator for Point *= Matrix3x3.
	inline Point &operator*=(const Matrix3x3 &mat)
	{
		class ShadowMatrix3x3
		{
		public:
			float m[3][3];
		}; // To allow inlining
		const ShadowMatrix3x3 *Mat = (const ShadowMatrix3x3 *)&mat;

		float xp = x * Mat->m[0][0] + y * Mat->m[1][0] + z * Mat->m[2][0];
		float yp = x * Mat->m[0][1] + y * Mat->m[1][1] + z * Mat->m[2][1];
		float zp = x * Mat->m[0][2] + y * Mat->m[1][2] + z * Mat->m[2][2];

		x = xp;
		y = yp;
		z = zp;

		return *this;
	}

	//! Operator for Point *= Matrix4x4.
	inline Point &operator*=(const Matrix4x4 &mat)
	{
		class ShadowMatrix4x4
		{
		public:
			float m[4][4];
		}; // To allow inlining
		const ShadowMatrix4x4 *Mat = (const ShadowMatrix4x4 *)&mat;

		float xp = x * Mat->m[0][0] + y * Mat->m[1][0] + z * Mat->m[2][0] + Mat->m[3][0];
		float yp = x * Mat->m[0][1] + y * Mat->m[1][1] + z * Mat->m[2][1] + Mat->m[3][1];
		float zp = x * Mat->m[0][2] + y * Mat->m[1][2] + z * Mat->m[2][2] + Mat->m[3][2];

		x = xp;
		y = yp;
		z = zp;

		return *this;
	}

	// Cast operators

	//! Cast a Point to a HPoint. w is set to zero.
	operator HPoint() const;

	inline float operator[](int n) const { return *(&x + n); }
	inline float &operator[](int n) { return *(&x + n); }

public:
	float x, y, z;
};

extern "C" void Normalize1(Point &a);
extern "C" void Normalize2(Point &a);

#endif //__ICEPOINT_H__
