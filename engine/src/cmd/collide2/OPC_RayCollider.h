///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a ray collider.
 *	\file		OPC_RayCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_RAYCOLLIDER_H__
#define __OPC_RAYCOLLIDER_H__

#include "Ice/IceContainer.h"
#include "Ice/IcePoint.h"
#include "Ice/IceRay.h"
#include "OPC_Collider.h"
#include "OPC_Model.h"

class CollisionFace
{
public:
	uint32_t mFaceID; //!< Index of touched face
	float mDistance;  //!< Distance from collider to hitpoint
	float mU, mV;	  //!< Impact barycentric coordinates
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
	 *	User-callback, called by OPCODE to record a hit.
	 *	\param		hit			[in] current hit
	 *	\param		user_data	[in] user-defined data from SetCallback()
	 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*HitCallback)(const CollisionFace &hit, void *user_data);

class RayCollider : public Collider
{
public:
	// Constructor / Destructor
	RayCollider();
	virtual ~RayCollider();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Generic stabbing query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- in the user-provided destination array
		 *
		 *	\param		world_ray		[in] stabbing ray in world space
		 *	\param		model			[in] Opcode model to collide with
		 *	\param		world			[in] model's world matrix, or null
		 *	\param		cache			[in] a possibly cached face index, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Collide(const Ray &world_ray, const Model &model, const Matrix4x4 *world = nullptr, uint32_t *cache = nullptr);

	// Settings

	inline void SetHitCallback(HitCallback cb)
	{
		mHitCallback = cb;
	}
	inline void SetUserData(void *user_data) { mUserData = user_data; }

protected:
	// Ray in local space
	Point mOrigin; //!< Ray origin
	Point mDir;	   //!< Ray direction (normalized)
	Point mFDir;   //!< fabsf(mDir)
	Point mData, mData2;
	// Stabbed faces
	CollisionFace mStabbedFace; //!< Current stabbed face
	HitCallback mHitCallback;	//!< Callback used to record a hit
	void *mUserData;			//!< User-defined data

	// Stats
	uint32_t mNbRayBVTests;	   //!< Number of Ray-BV tests
	uint32_t mNbRayPrimTests;  //!< Number of Ray-Primitive tests
							   // In-out test
	uint32_t mNbIntersections; //!< Number of valid intersections
							   // Dequantization coeffs
	Point mCenterCoeff;
	Point mExtentsCoeff;
	// Settings
	float mMaxDist; //!< Valid segment on the ray
	bool mCulling;	//!< Stab culled faces or not
		// Internal methods
	void _SegmentStab(const AABBCollisionNode *node);
	void _SegmentStab(const AABBNoLeafNode *node);
	void _SegmentStab(const AABBQuantizedNode *node);
	void _SegmentStab(const AABBQuantizedNoLeafNode *node);
	void _SegmentStab(const AABBTreeNode *node, Container &box_indices);
	void _RayStab(const AABBCollisionNode *node);
	void _RayStab(const AABBNoLeafNode *node);
	void _RayStab(const AABBQuantizedNode *node);
	void _RayStab(const AABBQuantizedNoLeafNode *node);
	void _RayStab(const AABBTreeNode *node, Container &box_indices);
	// Overlap tests
	inline bool RayAABBOverlap(const Point &center, const Point &extents);
	inline bool SegmentAABBOverlap(const Point &center, const Point &extents);
	inline bool RayTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2);
	// Init methods
	bool InitQuery(const Ray &world_ray, const Matrix4x4 *world = nullptr, uint32_t *face_id = nullptr);
};

#endif // __OPC_RAYCOLLIDER_H__
