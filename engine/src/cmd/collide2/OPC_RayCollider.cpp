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
 *	\file		OPC_RayCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a ray-vs-tree collider.
 *	This class performs a stabbing query on an AABB tree, i.e. does a ray-mesh collision.
 *
 *	HIGHER DISTANCE BOUND:
 *
 *		If P0 and P1 are two 3D points, let's define:
 *		- d = distance between P0 and P1
 *		- Origin	= P0
 *		- Direction	= (P1 - P0) / d = normalized direction vector
 *		- A parameter t such as a point P on the line (P0,P1) is P = Origin + t * Direction
 *		- t = 0  -->  P = P0
 *		- t = d  -->  P = P1
 *
 *		Then we can define a general "ray" as:
 *
 *			struct Ray
 *			{
 *				Point	Origin;
 *				Point	Direction;
 *			};
 *
 *		But it actually maps three different things:
 *		- a segment,   when 0 <= t <= d
 *		- a half-line, when 0 <= t < +infinity, or -infinity < t <= d
 *		- a line,      when -infinity < t < +infinity
 *
 *		In Opcode, we support segment queries, which yield half-line queries by setting d = +infinity.
 *		We don't support line-queries. If you need them, shift the origin along the ray by an appropriate margin.
 *
 *		In short, the lower bound is always 0, and you can setup the higher bound "d" with RayCollider::SetMaxDist().
 *
 *		Query	|segment			|half-line		|line
 *		--------|-------------------|---------------|----------------
 *		Usages	|-shadow feelers	|-raytracing	|-
 *				|-sweep tests		|-in/out tests	|
 *
 *	FIRST CONTACT:
 *
 *		- You can setup "first contact" mode or "all contacts" mode with RayCollider::SetFirstContact().
 *		- In "first contact" mode we return as soon as the ray hits one face. If can be useful e.g. for shadow feelers, where
 *		you want to know whether the path to the light is free or not (a boolean answer is enough).
 *		- In "all contacts" mode we return all faces hit by the ray.
 *
 *	TEMPORAL COHERENCE:
 *
 *		- You can enable or disable temporal coherence with RayCollider::SetTemporalCoherence().
 *		- It currently only works in "first contact" mode.
 *		- If temporal coherence is enabled, the previously hit triangle is cached during the first query. Then, next queries
 *		start by colliding the ray against the cached triangle. If they still collide, we return immediately.
 *
 *	CLOSEST HIT:
 *
 *		- You can enable or disable "closest hit" with RayCollider::SetClosestHit().
 *		- It currently only works in "all contacts" mode.
 *		- If closest hit is enabled, faces are sorted by distance on-the-fly and the closest one only is reported.
 *
 *	BACKFACE CULLING:
 *
 *		- You can enable or disable backface culling with RayCollider::SetCulling().
 *		- If culling is enabled, ray will not hit back faces (only front faces).
 *		
 *
 *
 *	\class		RayCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		June, 2, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This class describes a face hit by a ray or segment.
 *	This is a particular class dedicated to stabbing queries.
 *
 *	\class		CollisionFace
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This class is a dedicated collection of CollisionFace.
 *
 *	\class		CollisionFaces
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "OPC_RayCollider.h"

#define SET_CONTACT(prim_index, flag)                                          \
	mNbIntersections++;                                                        \
	/* Set contact status */                                                   \
	mFlags |= flag;                                                            \
	/* In any case the contact has been found and recorded in mStabbedFace  */ \
	mStabbedFace.mFaceID = prim_index;

#define HANDLE_CONTACT(prim_index, flag)         \
	SET_CONTACT(prim_index, flag)                \
                                                 \
	if (mHitCallback)                            \
		(mHitCallback)(mStabbedFace, mUserData); \
	else                                         \
	{                                            \
	}

#define UPDATE_CACHE                   \
	if (cache && GetContactStatus())   \
	{                                  \
		*cache = mStabbedFace.mFaceID; \
	}                                  \
	else                               \
	{                                  \
	}

#define SEGMENT_PRIM(prim_index, flag)                                                       \
	/* Request vertices from the app */                                                      \
	VertexPointers VP;                                                                       \
	mIMesh->GetTriangle(VP, prim_index);                                                     \
                                                                                             \
	/* Perform ray-tri overlap test and return */                                            \
	if (RayTriOverlap(*VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))                          \
	{                                                                                        \
		/* Intersection point is valid if dist < segment's length */                         \
		/* We know dist>0 so we can use integers */                                          \
		if (static_cast<uint32_t>(mStabbedFace.mDistance) < static_cast<uint32_t>(mMaxDist)) \
		{                                                                                    \
			HANDLE_CONTACT(prim_index, flag)                                                 \
		}                                                                                    \
	}                                                                                        \
	else                                                                                     \
	{                                                                                        \
	}

#define RAY_PRIM(prim_index, flag)                                  \
	/* Request vertices from the app */                             \
	VertexPointers VP;                                              \
	mIMesh->GetTriangle(VP, prim_index);                            \
                                                                    \
	/* Perform ray-tri overlap test and return */                   \
	if (RayTriOverlap(*VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2])) \
	{                                                               \
		HANDLE_CONTACT(prim_index, flag)                            \
	}                                                               \
	else                                                            \
	{                                                               \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RayCollider::RayCollider() : mHitCallback(nullptr),
							 mUserData(0),
							 mNbRayBVTests(0),
							 mNbRayPrimTests(0),
							 mNbIntersections(0),
							 mMaxDist(std::numeric_limits<float>::max()),
							 mCulling(true)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RayCollider::~RayCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings and callbacks have been defined.
 *	\return		null if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char *RayCollider::ValidateSettings()
{
	if (mMaxDist < 0.0f)
		return "Higher distance bound must be positive!";
	if (TemporalCoherenceEnabled() && !FirstContactEnabled())
		return "Temporal coherence only works with "
			   "First contact"
			   " mode!";
	if (SkipPrimitiveTests())
		return "SkipPrimitiveTests not possible for RayCollider ! (not implemented)";
	return nullptr;
}

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
bool RayCollider::Collide(const Ray &world_ray, const Model &model, const Matrix4x4 *world, uint32_t *cache)
{
	// Checkings
	if (!Setup(&model))
		return false;

	// Init collision query
	if (InitQuery(world_ray, world, cache))
		return true;

	if (!model.HasLeafNodes())
	{
		if (model.IsQuantized())
		{
			const AABBQuantizedNoLeafTree *Tree = (const AABBQuantizedNoLeafTree *)model.GetTree();

			// Setup dequantization coeffs
			mCenterCoeff = Tree->mCenterCoeff;
			mExtentsCoeff = Tree->mExtentsCoeff;

			// Perform stabbing query
			if (static_cast<uint32_t>(mMaxDist) != IEEE_MAX_FLOAT)
			{
				_SegmentStab(Tree->GetNodes());
			}
			else
			{
				_RayStab(Tree->GetNodes());
			}
		}
		else
		{
			const AABBNoLeafTree *Tree = (const AABBNoLeafTree *)model.GetTree();

			// Perform stabbing query
			if (static_cast<uint32_t>(mMaxDist) != IEEE_MAX_FLOAT)
			{
				_SegmentStab(Tree->GetNodes());
			}
			else
			{
				_RayStab(Tree->GetNodes());
			}
		}
	}
	else
	{
		if (model.IsQuantized())
		{
			const AABBQuantizedTree *Tree = (const AABBQuantizedTree *)model.GetTree();

			// Setup dequantization coeffs
			mCenterCoeff = Tree->mCenterCoeff;
			mExtentsCoeff = Tree->mExtentsCoeff;

			// Perform stabbing query
			if (static_cast<uint32_t>(mMaxDist) != IEEE_MAX_FLOAT)
			{
				_SegmentStab(Tree->GetNodes());
			}
			else
			{
				_RayStab(Tree->GetNodes());
			}
		}
		else
		{
			const AABBCollisionTree *Tree = (const AABBCollisionTree *)model.GetTree();

			// Perform stabbing query
			if (static_cast<uint32_t>(mMaxDist) != IEEE_MAX_FLOAT)
			{
				_SegmentStab(Tree->GetNodes());
			}
			else
			{
				_RayStab(Tree->GetNodes());
			}
		}
	}

	// Update cache if needed
	UPDATE_CACHE
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a stabbing query :
 *	- reset stats & contact status
 *	- compute ray in local space
 *	- check temporal coherence
 *
 *	\param		world_ray	[in] stabbing ray in world space
 *	\param		world		[in] object's world matrix, or null
 *	\param		face_id		[in] index of previously stabbed triangle
 *	\return		true if we can return immediately
 *	\warning	SCALE NOT SUPPORTED. The matrix must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RayCollider::InitQuery(const Ray &world_ray, const Matrix4x4 *world, uint32_t *face_id)
{
	// Reset stats & contact status
	Collider::InitQuery();
	mNbRayBVTests = 0;
	mNbRayPrimTests = 0;
	mNbIntersections = 0;

	// Compute ray in local space
	// The (Origin/Dir) form is needed for the ray-triangle test anyway (even for segment tests)
	if (world)
	{
		Matrix3x3 InvWorld = *world;
		mDir = InvWorld * world_ray.mDir;

		Matrix4x4 World;
		InvertPRMatrix(World, *world);
		mOrigin = world_ray.mOrig * World;
	}
	else
	{
		mDir = world_ray.mDir;
		mOrigin = world_ray.mOrig;
	}

	// 4) Special case: 1-triangle meshes [Opcode 1.3]
	if (mCurrentModel && mCurrentModel->HasSingleNode())
	{
		// We simply perform the BV-Prim overlap test each time. We assume single triangle has index 0.
		if (!SkipPrimitiveTests())
		{
			// Perform overlap test between the unique triangle and the ray (and set contact status if needed)
			SEGMENT_PRIM(uint32_t(0), OPC_CONTACT)

			// Return immediately regardless of status
			return true;
		}
	}

	// Check temporal coherence :

	// Test previously colliding primitives first
	if (TemporalCoherenceEnabled() && FirstContactEnabled() && face_id && *face_id != INVALID_ID)
	{
		// New code
		// We handle both Segment/ray queries with the same segment code, and a possible infinite limit
		SEGMENT_PRIM(*face_id, OPC_TEMPORAL_CONTACT)

		// Return immediately if possible
		if (GetContactStatus())
			return true;
	}

	// Precompute data (moved after temporal coherence since only needed for ray-AABB)
	if (static_cast<uint32_t>(mMaxDist) != IEEE_MAX_FLOAT)
	{
		// For Segment-AABB overlap
		mData = 0.5f * mDir * mMaxDist;
		mData2 = mOrigin + mData;

		// Precompute mFDir;
		mFDir.x = fabsf(mData.x);
		mFDir.y = fabsf(mData.y);
		mFDir.z = fabsf(mData.z);
	}
	else
	{
		// For Ray-AABB overlap
		//		uint32_t x = SIR(mDir.x)-1;
		//		uint32_t y = SIR(mDir.y)-1;
		//		uint32_t z = SIR(mDir.z)-1;
		//		mData.x = FR(x);
		//		mData.y = FR(y);
		//		mData.z = FR(z);

		// Precompute mFDir;
		mFDir.x = fabsf(mDir.x);
		mFDir.y = fabsf(mDir.y);
		mFDir.z = fabsf(mDir.z);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBCollisionNode *node)
{
	// Perform Segment-AABB overlap test
	if (!SegmentAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))
		return;

	if (node->IsLeaf())
	{
		SEGMENT_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_SegmentStab(node->GetPos());

		if (ContactFound())
			return;

		_SegmentStab(node->GetNeg());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBQuantizedNode *node)
{
	// Dequantize box
	const QuantizedAABB &Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z);
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z);

	// Perform Segment-AABB overlap test
	if (!SegmentAABBOverlap(Center, Extents))
		return;

	if (node->IsLeaf())
	{
		SEGMENT_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_SegmentStab(node->GetPos());

		if (ContactFound())
			return;

		_SegmentStab(node->GetNeg());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBNoLeafNode *node)
{
	// Perform Segment-AABB overlap test
	if (!SegmentAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))
		return;

	if (node->HasPosLeaf())
	{
		SEGMENT_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else
		_SegmentStab(node->GetPos());

	if (ContactFound())
		return;

	if (node->HasNegLeaf())
	{
		SEGMENT_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else
		_SegmentStab(node->GetNeg());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBQuantizedNoLeafNode *node)
{
	// Dequantize box
	const QuantizedAABB &Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z);
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z);

	// Perform Segment-AABB overlap test
	if (!SegmentAABBOverlap(Center, Extents))
		return;

	if (node->HasPosLeaf())
	{
		SEGMENT_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else
		_SegmentStab(node->GetPos());

	if (ContactFound())
		return;

	if (node->HasNegLeaf())
	{
		SEGMENT_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else
		_SegmentStab(node->GetNeg());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for vanilla AABB trees.
 *	\param		node		[in] current collision node
 *	\param		box_indices	[out] indices of stabbed boxes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBTreeNode *node, Container &box_indices)
{
	// Test the box against the segment
	Point Center, Extents;
	node->GetAABB()->GetCenter(Center);
	node->GetAABB()->GetExtents(Extents);
	if (!SegmentAABBOverlap(Center, Extents))
		return;

	if (node->IsLeaf())
	{
		box_indices.Add(node->GetPrimitives(), node->GetNbPrimitives());
	}
	else
	{
		_SegmentStab(node->GetPos(), box_indices);
		_SegmentStab(node->GetNeg(), box_indices);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBCollisionNode *node)
{
	// Perform Ray-AABB overlap test
	if (!RayAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))
		return;

	if (node->IsLeaf())
	{
		RAY_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_RayStab(node->GetPos());

		if (ContactFound())
			return;

		_RayStab(node->GetNeg());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBQuantizedNode *node)
{
	// Dequantize box
	const QuantizedAABB &Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z);
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z);

	// Perform Ray-AABB overlap test
	if (!RayAABBOverlap(Center, Extents))
		return;

	if (node->IsLeaf())
	{
		RAY_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_RayStab(node->GetPos());

		if (ContactFound())
			return;

		_RayStab(node->GetNeg());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBNoLeafNode *node)
{
	// Perform Ray-AABB overlap test
	if (!RayAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))
		return;

	if (node->HasPosLeaf())
	{
		RAY_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else
		_RayStab(node->GetPos());

	if (ContactFound())
		return;

	if (node->HasNegLeaf())
	{
		RAY_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else
		_RayStab(node->GetNeg());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBQuantizedNoLeafNode *node)
{
	// Dequantize box
	const QuantizedAABB &Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z);
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z);

	// Perform Ray-AABB overlap test
	if (!RayAABBOverlap(Center, Extents))
		return;

	if (node->HasPosLeaf())
	{
		RAY_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else
		_RayStab(node->GetPos());

	if (ContactFound())
		return;

	if (node->HasNegLeaf())
	{
		RAY_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else
		_RayStab(node->GetNeg());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for vanilla AABB trees.
 *	\param		node		[in] current collision node
 *	\param		box_indices	[out] indices of stabbed boxes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBTreeNode *node, Container &box_indices)
{
	// Test the box against the ray
	Point Center, Extents;
	node->GetAABB()->GetCenter(Center);
	node->GetAABB()->GetExtents(Extents);
	if (!RayAABBOverlap(Center, Extents))
		return;
	if (node->IsLeaf())
	{
		mFlags |= OPC_CONTACT;
		box_indices.Add(node->GetPrimitives(), node->GetNbPrimitives());
	}
	else
	{
		_RayStab(node->GetPos(), box_indices);
		_RayStab(node->GetNeg(), box_indices);
	}
}

inline bool RayCollider::SegmentAABBOverlap(const Point &center, const Point &extents)
{
	// Stats
	mNbRayBVTests++;

	float Dx = mData2.x - center.x;
	if (fabsf(Dx) > extents.x + mFDir.x)
		return false;
	float Dy = mData2.y - center.y;
	if (fabsf(Dy) > extents.y + mFDir.y)
		return false;
	float Dz = mData2.z - center.z;
	if (fabsf(Dz) > extents.z + mFDir.z)
		return false;

	float f;
	f = mData.y * Dz - mData.z * Dy;
	if (fabsf(f) > extents.y * mFDir.z + extents.z * mFDir.y)
		return false;
	f = mData.z * Dx - mData.x * Dz;
	if (fabsf(f) > extents.x * mFDir.z + extents.z * mFDir.x)
		return false;
	f = mData.x * Dy - mData.y * Dx;
	if (fabsf(f) > extents.x * mFDir.y + extents.y * mFDir.x)
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes a ray-AABB overlap test using the separating axis theorem. Ray is cached within the class.
 *	\param		center	[in] AABB center
 *	\param		extents	[in] AABB extents
 *	\return		true on overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool RayCollider::RayAABBOverlap(const Point &center, const Point &extents)
{
	// Stats
	mNbRayBVTests++;

	//	float Dx = mOrigin.x - center.x;	if(fabsf(Dx) > extents.x && Dx*mDir.x>=0.0f)	return false;
	//	float Dy = mOrigin.y - center.y;	if(fabsf(Dy) > extents.y && Dy*mDir.y>=0.0f)	return false;
	//	float Dz = mOrigin.z - center.z;	if(fabsf(Dz) > extents.z && Dz*mDir.z>=0.0f)	return false;

	float Dx = mOrigin.x - center.x;
	if (GREATER(Dx, extents.x) && Dx * mDir.x >= 0.0f)
		return false;
	float Dy = mOrigin.y - center.y;
	if (GREATER(Dy, extents.y) && Dy * mDir.y >= 0.0f)
		return false;
	float Dz = mOrigin.z - center.z;
	if (GREATER(Dz, extents.z) && Dz * mDir.z >= 0.0f)
		return false;

	//	float Dx = mOrigin.x - center.x;	if(GREATER(Dx, extents.x) && ((SIR(Dx)-1)^SIR(mDir.x))>=0.0f)	return false;
	//	float Dy = mOrigin.y - center.y;	if(GREATER(Dy, extents.y) && ((SIR(Dy)-1)^SIR(mDir.y))>=0.0f)	return false;
	//	float Dz = mOrigin.z - center.z;	if(GREATER(Dz, extents.z) && ((SIR(Dz)-1)^SIR(mDir.z))>=0.0f)	return false;

	float f;
	f = mDir.y * Dz - mDir.z * Dy;
	if (fabsf(f) > extents.y * mFDir.z + extents.z * mFDir.y)
		return false;
	f = mDir.z * Dx - mDir.x * Dz;
	if (fabsf(f) > extents.x * mFDir.z + extents.z * mFDir.x)
		return false;
	f = mDir.x * Dy - mDir.y * Dx;
	if (fabsf(f) > extents.x * mFDir.y + extents.y * mFDir.x)
		return false;

	return true;
}

inline bool RayCollider::RayTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2)
{
	// Stats
	mNbRayPrimTests++;

	// Find vectors for two edges sharing vert0
	Point edge1 = vert1 - vert0;
	Point edge2 = vert2 - vert0;

	// Begin calculating determinant - also used to calculate U parameter
	Point pvec = mDir ^ edge2;

	// If determinant is near zero, ray lies in plane of triangle
	float det = edge1 | pvec;

	if (mCulling)
	{
		if (det < std::numeric_limits<float>::epsilon())
			return false;
		// From here, det is > 0. So we can use integer cmp.

		// Calculate distance from vert0 to ray origin
		Point tvec = mOrigin - vert0;

		// Calculate U parameter and test bounds
		mStabbedFace.mU = tvec | pvec;
		//		if(IR(u)&0x80000000 || u>det)					return false;
		if ((mStabbedFace.mU < 0.0f) || static_cast<uint32_t>(mStabbedFace.mU) > static_cast<uint32_t>(det))
		{
			return false;
		}

		// Prepare to test V parameter
		Point qvec = tvec ^ edge1;

		// Calculate V parameter and test bounds
		mStabbedFace.mV = mDir | qvec;
		if ((mStabbedFace.mV < 0.0f) || mStabbedFace.mU + mStabbedFace.mV > det)
		{
			return false;
		}

		// Calculate t, scale parameters, ray intersects triangle
		mStabbedFace.mDistance = edge2 | qvec;
		// Det > 0 so we can early exit here
		// Intersection point is valid if distance is positive (else it can just be a face behind the orig point)
		if ((mStabbedFace.mDistance < 0.0f))
		{
			return false;
		}
		// Else go on
		float OneOverDet = 1.0f / det;
		mStabbedFace.mDistance *= OneOverDet;
		mStabbedFace.mU *= OneOverDet;
		mStabbedFace.mV *= OneOverDet;
	}
	else
	{
		// the non-culling branch
		if (det > -std::numeric_limits<float>::epsilon() && det < std::numeric_limits<float>::epsilon())
			return false;
		float OneOverDet = 1.0f / det;

		// Calculate distance from vert0 to ray origin
		Point tvec = mOrigin - vert0;

		// Calculate U parameter and test bounds
		mStabbedFace.mU = (tvec | pvec) * OneOverDet;
		//		if(IR(u)&0x80000000 || u>1.0f)					return false;
		if ((mStabbedFace.mU < 0.0f) || (mStabbedFace.mU > 1.0f))
			return false;

		// prepare to test V parameter
		Point qvec = tvec ^ edge1;

		// Calculate V parameter and test bounds
		mStabbedFace.mV = (mDir | qvec) * OneOverDet;
		if ((mStabbedFace.mV < 0.0f) || mStabbedFace.mU + mStabbedFace.mV > 1.0f)
			return false;

		// Calculate t, ray intersects triangle
		mStabbedFace.mDistance = (edge2 | qvec) * OneOverDet;
		// Intersection point is valid if distance is positive (else it can just be a face behind the orig point)
		if ((mStabbedFace.mDistance < 0.0f))
			return false;
	}
	return true;
}