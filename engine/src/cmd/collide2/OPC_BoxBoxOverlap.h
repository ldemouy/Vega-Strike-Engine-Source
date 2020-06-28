///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	OBB-OBB overlap test using the separating axis theorem.
 *	- original code by Gomez / Gamasutra (similar to Gottschalk's one in RAPID)
 *	- optimized for AABB trees by computing the rotation matrix once (SOLID-fashion)
 *	- the fabs matrix is precomputed as well and epsilon-tweaked (RAPID-style, we found this almost mandatory)
 *	- Class III axes can be disabled... (SOLID & Intel fashion)
 *	- ...or enabled to perform some profiling
 *	- CPU comparisons used when appropriate
 *	- lazy evaluation sometimes saves some work in case of early exits (unlike SOLID)
 *
 *	\param		ea	[in] extents from box A
 *	\param		ca	[in] center from box A
 *	\param		eb	[in] extents from box B
 *	\param		cb	[in] center from box B
 *	\return		true if boxes overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Ice/IcePoint.h"
#include "Ice/IceMatrix3x3.h"
#include "OPC_TreeCollider.h"

inline bool AABBTreeCollider::BoxBoxOverlap(const Point &ea, const Point &ca, const Point &eb, const Point &cb)
{
	// Stats
	mNbBVBVTests++;

	float t, t2;

	// Class I : A's basis vectors
	float Tx = (mR1to0.m[0][0] * cb.x + mR1to0.m[1][0] * cb.y + mR1to0.m[2][0] * cb.z) + mT1to0.x - ca.x;
	t = ea.x + eb.x * mAR.m[0][0] + eb.y * mAR.m[1][0] + eb.z * mAR.m[2][0];
	if (GREATER(Tx, t))
		return false;

	float Ty = (mR1to0.m[0][1] * cb.x + mR1to0.m[1][1] * cb.y + mR1to0.m[2][1] * cb.z) + mT1to0.y - ca.y;
	t = ea.y + eb.x * mAR.m[0][1] + eb.y * mAR.m[1][1] + eb.z * mAR.m[2][1];
	if (GREATER(Ty, t))
		return false;

	float Tz = (mR1to0.m[0][2] * cb.x + mR1to0.m[1][2] * cb.y + mR1to0.m[2][2] * cb.z) + mT1to0.z - ca.z;
	t = ea.z + eb.x * mAR.m[0][2] + eb.y * mAR.m[1][2] + eb.z * mAR.m[2][2];
	if (GREATER(Tz, t))
		return false;

	// Class II : B's basis vectors
	t = Tx * mR1to0.m[0][0] + Ty * mR1to0.m[0][1] + Tz * mR1to0.m[0][2];
	t2 = ea.x * mAR.m[0][0] + ea.y * mAR.m[0][1] + ea.z * mAR.m[0][2] + eb.x;
	if (GREATER(t, t2))
		return false;

	t = Tx * mR1to0.m[1][0] + Ty * mR1to0.m[1][1] + Tz * mR1to0.m[1][2];
	t2 = ea.x * mAR.m[1][0] + ea.y * mAR.m[1][1] + ea.z * mAR.m[1][2] + eb.y;
	if (GREATER(t, t2))
		return false;

	t = Tx * mR1to0.m[2][0] + Ty * mR1to0.m[2][1] + Tz * mR1to0.m[2][2];
	t2 = ea.x * mAR.m[2][0] + ea.y * mAR.m[2][1] + ea.z * mAR.m[2][2] + eb.z;
	if (GREATER(t, t2))
		return false;

	// Class III : 9 cross products
	// Cool trick: always perform the full test for first level, regardless of settings.
	// That way pathological cases (such as the pencils scene) are quickly rejected anyway !
	if (mFullBoxBoxTest || mNbBVBVTests == 1)
	{
		t = Tz * mR1to0.m[0][1] - Ty * mR1to0.m[0][2];
		t2 = ea.y * mAR.m[0][2] + ea.z * mAR.m[0][1] + eb.y * mAR.m[2][0] + eb.z * mAR.m[1][0];
		if (GREATER(t, t2))
			return false; // L = A0 x B0
		t = Tz * mR1to0.m[1][1] - Ty * mR1to0.m[1][2];
		t2 = ea.y * mAR.m[1][2] + ea.z * mAR.m[1][1] + eb.x * mAR.m[2][0] + eb.z * mAR.m[0][0];
		if (GREATER(t, t2))
			return false; // L = A0 x B1
		t = Tz * mR1to0.m[2][1] - Ty * mR1to0.m[2][2];
		t2 = ea.y * mAR.m[2][2] + ea.z * mAR.m[2][1] + eb.x * mAR.m[1][0] + eb.y * mAR.m[0][0];
		if (GREATER(t, t2))
			return false; // L = A0 x B2
		t = Tx * mR1to0.m[0][2] - Tz * mR1to0.m[0][0];
		t2 = ea.x * mAR.m[0][2] + ea.z * mAR.m[0][0] + eb.y * mAR.m[2][1] + eb.z * mAR.m[1][1];
		if (GREATER(t, t2))
			return false; // L = A1 x B0
		t = Tx * mR1to0.m[1][2] - Tz * mR1to0.m[1][0];
		t2 = ea.x * mAR.m[1][2] + ea.z * mAR.m[1][0] + eb.x * mAR.m[2][1] + eb.z * mAR.m[0][1];
		if (GREATER(t, t2))
			return false; // L = A1 x B1
		t = Tx * mR1to0.m[2][2] - Tz * mR1to0.m[2][0];
		t2 = ea.x * mAR.m[2][2] + ea.z * mAR.m[2][0] + eb.x * mAR.m[1][1] + eb.y * mAR.m[0][1];
		if (GREATER(t, t2))
			return false; // L = A1 x B2
		t = Ty * mR1to0.m[0][0] - Tx * mR1to0.m[0][1];
		t2 = ea.x * mAR.m[0][1] + ea.y * mAR.m[0][0] + eb.y * mAR.m[2][2] + eb.z * mAR.m[1][2];
		if (GREATER(t, t2))
			return false; // L = A2 x B0
		t = Ty * mR1to0.m[1][0] - Tx * mR1to0.m[1][1];
		t2 = ea.x * mAR.m[1][1] + ea.y * mAR.m[1][0] + eb.x * mAR.m[2][2] + eb.z * mAR.m[0][2];
		if (GREATER(t, t2))
			return false; // L = A2 x B1
		t = Ty * mR1to0.m[2][0] - Tx * mR1to0.m[2][1];
		t2 = ea.x * mAR.m[2][1] + ea.y * mAR.m[2][0] + eb.x * mAR.m[1][2] + eb.y * mAR.m[0][2];
		if (GREATER(t, t2))
			return false; // L = A2 x B2
	}
	return true;
}

