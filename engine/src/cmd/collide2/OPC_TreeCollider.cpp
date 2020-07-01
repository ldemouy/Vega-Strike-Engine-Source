///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a tree collider.
 *	\file		OPC_TreeCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an AABB tree collider.
 *	This class performs a collision test between two AABB trees.
 *
 *	\class		AABBTreeCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "OPC_TreeCollider.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeCollider::AABBTreeCollider()
    : mIMesh0(nullptr), mIMesh1(nullptr), mNbBVBVTests(0), mNbPrimPrimTests(0), mNbBVPrimTests(0),
      mFullBoxBoxTest(true), mFullPrimBoxTest(true)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeCollider::~AABBTreeCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic collision query for generic OPCODE models. After the call, access the results with:
 *	- GetContactStatus()
 *	- GetNbPairs()
 *	- GetPairs()
 *
 *	\param		cache			[in] collision cache for model pointers and a colliding pair of primitives
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(BVTCache &cache, const Matrix4x4 *world0, const Matrix4x4 *world1)
{
    // Checkings
    if (!cache.Model0 || !cache.Model1)
        return false;
    if (cache.Model0->HasLeafNodes() != cache.Model1->HasLeafNodes())
        return false;
    if (cache.Model0->IsQuantized() != cache.Model1->IsQuantized())
        return false;

    /*

      Rules:
        - perform hull test
        - when hulls collide, disable hull test
        - if meshes overlap, reset countdown
        - if countdown reaches 0, enable hull test

    */

    // Checkings
    if (!Setup(cache.Model0->GetMeshInterface(), cache.Model1->GetMeshInterface()))
        return false;

    // Simple double-dispatch
    bool Status;

    const AABBQuantizedNoLeafTree *T0 = (const AABBQuantizedNoLeafTree *)cache.Model0->GetTree();
    const AABBQuantizedNoLeafTree *T1 = (const AABBQuantizedNoLeafTree *)cache.Model1->GetTree();
    Status = Collide(T0, T1, world0, world1, &cache);

    return Status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a collision query :
 *	- reset stats & contact status
 *	- setup matrices
 *
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::InitQuery(const Matrix4x4 *world0, const Matrix4x4 *world1)
{
    // Reset stats & contact status
    Collider::InitQuery();
    mNbBVBVTests = 0;
    mNbPrimPrimTests = 0;
    mNbBVPrimTests = 0;
    mPairs.Reset();

    // Setup matrices
    Matrix4x4 InvWorld0, InvWorld1;
    if (world0)
        InvertPRMatrix(InvWorld0, *world0);
    else
        InvWorld0.Identity();

    if (world1)
        InvertPRMatrix(InvWorld1, *world1);
    else
        InvWorld1.Identity();

    Matrix4x4 World0to1 = world0 ? (*world0 * InvWorld1) : InvWorld1;
    Matrix4x4 World1to0 = world1 ? (*world1 * InvWorld0) : InvWorld0;

    mR0to1 = World0to1;
    World0to1.GetTrans(mT0to1);
    mR1to0 = World1to0;
    World1to0.GetTrans(mT1to0);

    // Precompute absolute 1-to-0 rotation matrix
    for (uint32_t i = 0; i < 3; i++)
    {
        for (uint32_t j = 0; j < 3; j++)
        {
            // Epsilon value prevents floating-point inaccuracies (strategy borrowed from RAPID)
            mAR.m[i][j] = 1e-6f + fabsf(mR1to0.m[i][j]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Takes advantage of temporal coherence.
 *	\param		cache	[in] cache for a pair of previously colliding primitives
 *	\return		true if we can return immediately
 *	\warning	only works for "First Contact" mode
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::CheckTemporalCoherence(BVTCache *cache)
{
    // Checkings
    if (!cache)
        return false;

    // Test previously colliding primitives first
    if (TemporalCoherenceEnabled() && FirstContactEnabled())
    {
        PrimTest(cache->id0, cache->id1);
        if (GetContactStatus())
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for quantized no-leaf AABB trees.
 *	\param		tree0			[in] AABB tree from first object
 *	\param		tree1			[in] AABB tree from second object
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\param		cache			[in/out] cache for a pair of previously colliding primitives
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(const AABBQuantizedNoLeafTree *tree0, const AABBQuantizedNoLeafTree *tree1,
                               const Matrix4x4 *world0, const Matrix4x4 *world1, BVTCache *cache)
{
    // Init collision query
    InitQuery(world0, world1);

    // Check previous state
    if (CheckTemporalCoherence(cache))
        return true;

    // Setup dequantization coeffs
    mCenterCoeff0 = tree0->mCenterCoeff;
    mExtentsCoeff0 = tree0->mExtentsCoeff;
    mCenterCoeff1 = tree1->mCenterCoeff;
    mExtentsCoeff1 = tree1->mExtentsCoeff;

    // Perform collision query
    _Collide(tree0->GetNodes(), tree1->GetNodes());

    if (cache && GetContactStatus())
    {
        cache->id0 = mPairs.GetEntry(0);
        cache->id1 = mPairs.GetEntry(1);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// No-leaf trees
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Leaf-leaf test for two primitive indices.
 *	\param		id0		[in] index from first leaf-triangle
 *	\param		id1		[in] index from second leaf-triangle
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::PrimTest(uint32_t id0, uint32_t id1)
{
    // Request vertices from the app
    VertexPointers VP0;
    VertexPointers VP1;
    mIMesh0->GetTriangle(VP0, id0);
    mIMesh1->GetTriangle(VP1, id1);

    // Transform from space 1 to space 0
    Point u0, u1, u2;
    TransformPoint(u0, *VP1.Vertex[0], mR1to0, mT1to0);
    TransformPoint(u1, *VP1.Vertex[1], mR1to0, mT1to0);
    TransformPoint(u2, *VP1.Vertex[2], mR1to0, mT1to0);

    // Perform triangle-triangle overlap test
    if (TriTriOverlap(*VP0.Vertex[0], *VP0.Vertex[1], *VP0.Vertex[2], u0, u1, u2))
    {
        // Keep track of colliding pairs
        mPairs.Add(id0).Add(id1);
        // Set contact status
        mFlags |= OPC_CONTACT;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Leaf-leaf test for a previously fetched triangle from tree A (in B's space) and a new leaf from B.
 *	\param		id1		[in] leaf-triangle index from tree B
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void AABBTreeCollider::PrimTestTriIndex(uint32_t id1)
{
    // Request vertices from the app
    VertexPointers VP;
    mIMesh1->GetTriangle(VP, id1);

    // Perform triangle-triangle overlap test
    if (TriTriOverlap(mLeafVerts[0], mLeafVerts[1], mLeafVerts[2], *VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))
    {
        // Keep track of colliding pairs
        mPairs.Add(mLeafIndex).Add(id1);
        // Set contact status
        mFlags |= OPC_CONTACT;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Leaf-leaf test for a previously fetched triangle from tree B (in A's space) and a new leaf from A.
 *	\param		id0		[in] leaf-triangle index from tree A
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void AABBTreeCollider::PrimTestIndexTri(uint32_t id0)
{
    // Request vertices from the app
    VertexPointers VP;
    mIMesh0->GetTriangle(VP, id0);

    // Perform triangle-triangle overlap test
    if (TriTriOverlap(mLeafVerts[0], mLeafVerts[1], mLeafVerts[2], *VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))
    {
        // Keep track of colliding pairs
        mPairs.Add(id0).Add(mLeafIndex);
        // Set contact status
        mFlags |= OPC_CONTACT;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantized no-leaf trees
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision of a leaf node from A and a quantized branch from B.
 *	\param		leaf	[in] leaf triangle from first tree
 *	\param		b		[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_CollideTriBox(const AABBQuantizedNoLeafNode *b)
{
    // Dequantize box
    const QuantizedAABB *bb = &b->mAABB;
    const Point Pb(float(bb->mCenter[0]) * mCenterCoeff1.x, float(bb->mCenter[1]) * mCenterCoeff1.y,
                   float(bb->mCenter[2]) * mCenterCoeff1.z);
    const Point eb(float(bb->mExtents[0]) * mExtentsCoeff1.x, float(bb->mExtents[1]) * mExtentsCoeff1.y,
                   float(bb->mExtents[2]) * mExtentsCoeff1.z);

    // Perform triangle-box overlap test
    if (!TriBoxOverlap(Pb, eb))
        return;

    if (b->HasPosLeaf())
        PrimTestTriIndex(b->GetPosPrimitive());
    else
        _CollideTriBox(b->GetPos());

    if (ContactFound())
        return;

    if (b->HasNegLeaf())
        PrimTestTriIndex(b->GetNegPrimitive());
    else
        _CollideTriBox(b->GetNeg());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision of a leaf node from B and a quantized branch from A.
 *	\param		b		[in] collision node from first tree
 *	\param		leaf	[in] leaf triangle from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_CollideBoxTri(const AABBQuantizedNoLeafNode *b)
{
    // Dequantize box
    const QuantizedAABB *bb = &b->mAABB;
    const Point Pa(float(bb->mCenter[0]) * mCenterCoeff0.x, float(bb->mCenter[1]) * mCenterCoeff0.y,
                   float(bb->mCenter[2]) * mCenterCoeff0.z);
    const Point ea(float(bb->mExtents[0]) * mExtentsCoeff0.x, float(bb->mExtents[1]) * mExtentsCoeff0.y,
                   float(bb->mExtents[2]) * mExtentsCoeff0.z);

    // Perform triangle-box overlap test
    if (!TriBoxOverlap(Pa, ea))
        return;

    if (b->HasPosLeaf())
        PrimTestIndexTri(b->GetPosPrimitive());
    else
        _CollideBoxTri(b->GetPos());

    if (ContactFound())
        return;

    if (b->HasNegLeaf())
        PrimTestIndexTri(b->GetNegPrimitive());
    else
        _CollideBoxTri(b->GetNeg());
}

//! Request triangle vertices from the app and transform them
#define FETCH_LEAF(prim_index, imesh, rot, trans)                                                                      \
    mLeafIndex = prim_index;                                                                                           \
    /* Request vertices from the app */                                                                                \
    VertexPointers VP;                                                                                                 \
    imesh->GetTriangle(VP, prim_index);                                                                                \
    /* Transform them in a common space */                                                                             \
    TransformPoint(mLeafVerts[0], *VP.Vertex[0], rot, trans);                                                          \
    TransformPoint(mLeafVerts[1], *VP.Vertex[1], rot, trans);                                                          \
    TransformPoint(mLeafVerts[2], *VP.Vertex[2], rot, trans);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees.
 *	\param		a	[in] collision node from first tree
 *	\param		b	[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_Collide(const AABBQuantizedNoLeafNode *a, const AABBQuantizedNoLeafNode *b)
{
    // Dequantize box A
    const QuantizedAABB *ab = &a->mAABB;
    const Point Pa(float(ab->mCenter[0]) * mCenterCoeff0.x, float(ab->mCenter[1]) * mCenterCoeff0.y,
                   float(ab->mCenter[2]) * mCenterCoeff0.z);
    const Point ea(float(ab->mExtents[0]) * mExtentsCoeff0.x, float(ab->mExtents[1]) * mExtentsCoeff0.y,
                   float(ab->mExtents[2]) * mExtentsCoeff0.z);
    // Dequantize box B
    const QuantizedAABB *bb = &b->mAABB;
    const Point Pb(float(bb->mCenter[0]) * mCenterCoeff1.x, float(bb->mCenter[1]) * mCenterCoeff1.y,
                   float(bb->mCenter[2]) * mCenterCoeff1.z);
    const Point eb(float(bb->mExtents[0]) * mExtentsCoeff1.x, float(bb->mExtents[1]) * mExtentsCoeff1.y,
                   float(bb->mExtents[2]) * mExtentsCoeff1.z);

    // Perform BV-BV overlap test
    if (!BoxBoxOverlap(ea, Pa, eb, Pb))
    {
        return;
    }

    // Catch leaf status
    bool BHasPosLeaf = b->HasPosLeaf();
    bool BHasNegLeaf = b->HasNegLeaf();

    if (a->HasPosLeaf())
    {
        FETCH_LEAF(a->GetPosPrimitive(), mIMesh0, mR0to1, mT0to1)

        if (BHasPosLeaf)
        {
            PrimTestTriIndex(b->GetPosPrimitive());
        }
        else
        {
            _CollideTriBox(b->GetPos());
        }

        if (ContactFound())
        {
            return;
        }

        if (BHasNegLeaf)
        {
            PrimTestTriIndex(b->GetNegPrimitive());
        }
        else
        {
            _CollideTriBox(b->GetNeg());
        }
    }
    else
    {
        if (BHasPosLeaf)
        {
            FETCH_LEAF(b->GetPosPrimitive(), mIMesh1, mR1to0, mT1to0)

            _CollideBoxTri(a->GetPos());
        }
        else
        {
            _Collide(a->GetPos(), b->GetPos());
        }

        if (ContactFound())
        {
            return;
        }

        if (BHasNegLeaf)
        {
            FETCH_LEAF(b->GetNegPrimitive(), mIMesh1, mR1to0, mT1to0)

            _CollideBoxTri(a->GetPos());
        }
        else
        {
            _Collide(a->GetPos(), b->GetNeg());
        }
    }

    if (ContactFound())
    {
        return;
    }

    if (a->HasNegLeaf())
    {
        FETCH_LEAF(a->GetNegPrimitive(), mIMesh0, mR0to1, mT0to1)

        if (BHasPosLeaf)
        {
            PrimTestTriIndex(b->GetPosPrimitive());
        }
        else
        {
            _CollideTriBox(b->GetPos());
        }

        if (ContactFound())
        {
            return;
        }

        if (BHasNegLeaf)
        {
            PrimTestTriIndex(b->GetNegPrimitive());
        }
        else
        {
            _CollideTriBox(b->GetNeg());
        }
    }
    else
    {
        if (BHasPosLeaf)
        {
            // ### That leaf has possibly already been fetched
            FETCH_LEAF(b->GetPosPrimitive(), mIMesh1, mR1to0, mT1to0)

            _CollideBoxTri(a->GetNeg());
        }
        else
        {
            _Collide(a->GetNeg(), b->GetPos());
        }

        if (ContactFound())
        {
            return;
        }

        if (BHasNegLeaf)
        {
            // ### That leaf has possibly already been fetched
            FETCH_LEAF(b->GetNegPrimitive(), mIMesh1, mR1to0, mT1to0)

            _CollideBoxTri(a->GetNeg());
        }
        else
        {
            _Collide(a->GetNeg(), b->GetNeg());
        }
    }
}

inline bool AABBTreeCollider::BoxBoxOverlap(const Point &ea, const Point &ca, const Point &eb, const Point &cb)
{
    // Stats
    mNbBVBVTests++;

    float t, t2;

    // Class I : A's basis vectors
    float Tx = (mR1to0.m[0][0] * cb.x + mR1to0.m[1][0] * cb.y + mR1to0.m[2][0] * cb.z) + mT1to0.x - ca.x;
    t = ea.x + eb.x * mAR.m[0][0] + eb.y * mAR.m[1][0] + eb.z * mAR.m[2][0];
    if (GREATER(Tx, t))
    {
        return false;
    }

    float Ty = (mR1to0.m[0][1] * cb.x + mR1to0.m[1][1] * cb.y + mR1to0.m[2][1] * cb.z) + mT1to0.y - ca.y;
    t = ea.y + eb.x * mAR.m[0][1] + eb.y * mAR.m[1][1] + eb.z * mAR.m[2][1];
    if (GREATER(Ty, t))
    {
        return false;
    }

    float Tz = (mR1to0.m[0][2] * cb.x + mR1to0.m[1][2] * cb.y + mR1to0.m[2][2] * cb.z) + mT1to0.z - ca.z;
    t = ea.z + eb.x * mAR.m[0][2] + eb.y * mAR.m[1][2] + eb.z * mAR.m[2][2];
    if (GREATER(Tz, t))
    {
        return false;
    }

    // Class II : B's basis vectors
    t = Tx * mR1to0.m[0][0] + Ty * mR1to0.m[0][1] + Tz * mR1to0.m[0][2];
    t2 = ea.x * mAR.m[0][0] + ea.y * mAR.m[0][1] + ea.z * mAR.m[0][2] + eb.x;
    if (GREATER(t, t2))
    {
        return false;
    }

    t = Tx * mR1to0.m[1][0] + Ty * mR1to0.m[1][1] + Tz * mR1to0.m[1][2];
    t2 = ea.x * mAR.m[1][0] + ea.y * mAR.m[1][1] + ea.z * mAR.m[1][2] + eb.y;
    if (GREATER(t, t2))
    {
        return false;
    }

    t = Tx * mR1to0.m[2][0] + Ty * mR1to0.m[2][1] + Tz * mR1to0.m[2][2];
    t2 = ea.x * mAR.m[2][0] + ea.y * mAR.m[2][1] + ea.z * mAR.m[2][2] + eb.z;
    if (GREATER(t, t2))
    {
        return false;
    }

    // Class III : 9 cross products
    // Cool trick: always perform the full test for first level, regardless of settings.
    // That way pathological cases (such as the pencils scene) are quickly rejected anyway !
    if (mFullBoxBoxTest || mNbBVBVTests == 1)
    {
        t = Tz * mR1to0.m[0][1] - Ty * mR1to0.m[0][2];
        t2 = ea.y * mAR.m[0][2] + ea.z * mAR.m[0][1] + eb.y * mAR.m[2][0] + eb.z * mAR.m[1][0];
        if (GREATER(t, t2))
        {
            return false; // L = A0 x B0
        }
        t = Tz * mR1to0.m[1][1] - Ty * mR1to0.m[1][2];
        t2 = ea.y * mAR.m[1][2] + ea.z * mAR.m[1][1] + eb.x * mAR.m[2][0] + eb.z * mAR.m[0][0];
        if (GREATER(t, t2))
        {
            return false; // L = A0 x B1
        }
        t = Tz * mR1to0.m[2][1] - Ty * mR1to0.m[2][2];
        t2 = ea.y * mAR.m[2][2] + ea.z * mAR.m[2][1] + eb.x * mAR.m[1][0] + eb.y * mAR.m[0][0];
        if (GREATER(t, t2))
        {
            return false; // L = A0 x B2
        }
        t = Tx * mR1to0.m[0][2] - Tz * mR1to0.m[0][0];
        t2 = ea.x * mAR.m[0][2] + ea.z * mAR.m[0][0] + eb.y * mAR.m[2][1] + eb.z * mAR.m[1][1];
        if (GREATER(t, t2))
        {
            return false; // L = A1 x B0
        }
        t = Tx * mR1to0.m[1][2] - Tz * mR1to0.m[1][0];
        t2 = ea.x * mAR.m[1][2] + ea.z * mAR.m[1][0] + eb.x * mAR.m[2][1] + eb.z * mAR.m[0][1];
        if (GREATER(t, t2))
        {
            return false; // L = A1 x B1
        }
        t = Tx * mR1to0.m[2][2] - Tz * mR1to0.m[2][0];
        t2 = ea.x * mAR.m[2][2] + ea.z * mAR.m[2][0] + eb.x * mAR.m[1][1] + eb.y * mAR.m[0][1];
        if (GREATER(t, t2))
        {
            return false; // L = A1 x B2
        }
        t = Ty * mR1to0.m[0][0] - Tx * mR1to0.m[0][1];
        t2 = ea.x * mAR.m[0][1] + ea.y * mAR.m[0][0] + eb.y * mAR.m[2][2] + eb.z * mAR.m[1][2];
        if (GREATER(t, t2))
        {
            return false; // L = A2 x B0
        }
        t = Ty * mR1to0.m[1][0] - Tx * mR1to0.m[1][1];
        t2 = ea.x * mAR.m[1][1] + ea.y * mAR.m[1][0] + eb.x * mAR.m[2][2] + eb.z * mAR.m[0][2];
        if (GREATER(t, t2))
        {
            return false; // L = A2 x B1
        }
        t = Ty * mR1to0.m[2][0] - Tx * mR1to0.m[2][1];
        t2 = ea.x * mAR.m[2][1] + ea.y * mAR.m[2][0] + eb.x * mAR.m[1][2] + eb.y * mAR.m[0][2];
        if (GREATER(t, t2))
        {
            return false; // L = A2 x B2
        }
    }
    return true;
}

//! This macro quickly finds the min & max values among 3 variables
#define FINDMINMAX(x0, x1, x2, min, max)                                                                               \
    min = max = x0;                                                                                                    \
    if (x1 < min)                                                                                                      \
    {                                                                                                                  \
        min = x1;                                                                                                      \
    }                                                                                                                  \
    if (x1 > max)                                                                                                      \
    {                                                                                                                  \
        max = x1;                                                                                                      \
    }                                                                                                                  \
    if (x2 < min)                                                                                                      \
    {                                                                                                                  \
        min = x2;                                                                                                      \
    }                                                                                                                  \
    if (x2 > max)                                                                                                      \
    {                                                                                                                  \
        max = x2;                                                                                                      \
    }

//! TO BE DOCUMENTED
inline bool planeBoxOverlap(const Point &normal, const float d, const Point &maxbox)
{
    Point vmin, vmax;
    for (uint32_t q = 0; q <= 2; q++)
    {
        if (normal[q] > 0.0f)
        {
            vmin[q] = -maxbox[q];
            vmax[q] = maxbox[q];
        }
        else
        {
            vmin[q] = maxbox[q];
            vmax[q] = -maxbox[q];
        }
    }
    if ((normal | vmin) + d > 0.0f)
    {
        return false;
    }
    if ((normal | vmax) + d >= 0.0f)
    {
        return true;
    }

    return false;
}

//! TO BE DOCUMENTED
#define AXISTEST_X01(a, b, fa, fb)                                                                                     \
    min = a * v0.y - b * v0.z;                                                                                         \
    max = a * v2.y - b * v2.z;                                                                                         \
    if (min > max)                                                                                                     \
    {                                                                                                                  \
        const float tmp = max;                                                                                         \
        max = min;                                                                                                     \
        min = tmp;                                                                                                     \
    }                                                                                                                  \
    rad = fa * extents.y + fb * extents.z;                                                                             \
    if (min > rad || max < -rad)                                                                                       \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

//! TO BE DOCUMENTED
#define AXISTEST_X2(a, b, fa, fb)                                                                                      \
    min = a * v0.y - b * v0.z;                                                                                         \
    max = a * v1.y - b * v1.z;                                                                                         \
    if (min > max)                                                                                                     \
    {                                                                                                                  \
        const float tmp = max;                                                                                         \
        max = min;                                                                                                     \
        min = tmp;                                                                                                     \
    }                                                                                                                  \
    rad = fa * extents.y + fb * extents.z;                                                                             \
    if (min > rad || max < -rad)                                                                                       \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

//! TO BE DOCUMENTED
#define AXISTEST_Y02(a, b, fa, fb)                                                                                     \
    min = b * v0.z - a * v0.x;                                                                                         \
    max = b * v2.z - a * v2.x;                                                                                         \
    if (min > max)                                                                                                     \
    {                                                                                                                  \
        const float tmp = max;                                                                                         \
        max = min;                                                                                                     \
        min = tmp;                                                                                                     \
    }                                                                                                                  \
    rad = fa * extents.x + fb * extents.z;                                                                             \
    if (min > rad || max < -rad)                                                                                       \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

//! TO BE DOCUMENTED
#define AXISTEST_Y1(a, b, fa, fb)                                                                                      \
    min = b * v0.z - a * v0.x;                                                                                         \
    max = b * v1.z - a * v1.x;                                                                                         \
    if (min > max)                                                                                                     \
    {                                                                                                                  \
        const float tmp = max;                                                                                         \
        max = min;                                                                                                     \
        min = tmp;                                                                                                     \
    }                                                                                                                  \
    rad = fa * extents.x + fb * extents.z;                                                                             \
    if (min > rad || max < -rad)                                                                                       \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

//! TO BE DOCUMENTED
#define AXISTEST_Z12(a, b, fa, fb)                                                                                     \
    min = a * v1.x - b * v1.y;                                                                                         \
    max = a * v2.x - b * v2.y;                                                                                         \
    if (min > max)                                                                                                     \
    {                                                                                                                  \
        const float tmp = max;                                                                                         \
        max = min;                                                                                                     \
        min = tmp;                                                                                                     \
    }                                                                                                                  \
    rad = fa * extents.x + fb * extents.y;                                                                             \
    if (min > rad || max < -rad)                                                                                       \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

//! TO BE DOCUMENTED
#define AXISTEST_Z0(a, b, fa, fb)                                                                                      \
    min = a * v0.x - b * v0.y;                                                                                         \
    max = a * v1.x - b * v1.y;                                                                                         \
    if (min > max)                                                                                                     \
    {                                                                                                                  \
        const float tmp = max;                                                                                         \
        max = min;                                                                                                     \
        min = tmp;                                                                                                     \
    }                                                                                                                  \
    rad = fa * extents.x + fb * extents.y;                                                                             \
    if (min > rad || max < -rad)                                                                                       \
    {                                                                                                                  \
        return false;                                                                                                  \
    }

// compute triangle edges
// - edges lazy evaluated to take advantage of early exits
// - fabs precomputed (half less work, possible since extents are always >0)
// - customized macros to take advantage of the null component
// - axis vector discarded, possibly saves useless movs
#define IMPLEMENT_CLASS3_TESTS                                                                                         \
    float rad;                                                                                                         \
    float min, max;                                                                                                    \
                                                                                                                       \
    const float fey0 = fabsf(e0.y);                                                                                    \
    const float fez0 = fabsf(e0.z);                                                                                    \
    AXISTEST_X01(e0.z, e0.y, fez0, fey0);                                                                              \
    const float fex0 = fabsf(e0.x);                                                                                    \
    AXISTEST_Y02(e0.z, e0.x, fez0, fex0);                                                                              \
    AXISTEST_Z12(e0.y, e0.x, fey0, fex0);                                                                              \
                                                                                                                       \
    const float fey1 = fabsf(e1.y);                                                                                    \
    const float fez1 = fabsf(e1.z);                                                                                    \
    AXISTEST_X01(e1.z, e1.y, fez1, fey1);                                                                              \
    const float fex1 = fabsf(e1.x);                                                                                    \
    AXISTEST_Y02(e1.z, e1.x, fez1, fex1);                                                                              \
    AXISTEST_Z0(e1.y, e1.x, fey1, fex1);                                                                               \
                                                                                                                       \
    const Point e2 = mLeafVerts[0] - mLeafVerts[2];                                                                    \
    const float fey2 = fabsf(e2.y);                                                                                    \
    const float fez2 = fabsf(e2.z);                                                                                    \
    AXISTEST_X2(e2.z, e2.y, fez2, fey2);                                                                               \
    const float fex2 = fabsf(e2.x);                                                                                    \
    AXISTEST_Y1(e2.z, e2.x, fez2, fex2);                                                                               \
    AXISTEST_Z12(e2.y, e2.x, fey2, fex2);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Triangle-Box overlap test using the separating axis theorem.
 *	This is the code from Tomas M�ller, a bit optimized:
 *	- with some more lazy evaluation (faster path on PC)
 *	- with a tiny bit of assembly
 *	- with "SAT-lite" applied if needed
 *	- and perhaps with some more minor modifs...
 *
 *	\param		center		[in] box center
 *	\param		extents		[in] box extents
 *	\return		true if triangle & box overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AABBTreeCollider::TriBoxOverlap(const Point &center, const Point &extents)
{
    // Stats
    mNbBVPrimTests++;

    // use separating axis theorem to test overlap between triangle and box
    // need to test for overlap in these directions:
    // 1) the {x,y,z}-directions (actually, since we use the AABB of the triangle
    //    we do not even need to test these)
    // 2) normal of the triangle
    // 3) crossproduct(edge from tri, {x,y,z}-directin)
    //    this gives 3x3=9 more tests

    // move everything so that the boxcenter is in (0,0,0)
    Point v0, v1, v2;
    v0.x = mLeafVerts[0].x - center.x;
    v1.x = mLeafVerts[1].x - center.x;
    v2.x = mLeafVerts[2].x - center.x;

    // First, test overlap in the {x,y,z}-directions

    float min, max;
    // Find min, max of the triangle in x-direction, and test for overlap in X
    FINDMINMAX(v0.x, v1.x, v2.x, min, max);
    if (min > extents.x || max < -extents.x)
    {
        return false;
    }

    // Same for Y
    v0.y = mLeafVerts[0].y - center.y;
    v1.y = mLeafVerts[1].y - center.y;
    v2.y = mLeafVerts[2].y - center.y;

    FINDMINMAX(v0.y, v1.y, v2.y, min, max);
    if (min > extents.y || max < -extents.y)
    {
        return false;
    }

    // Same for Z
    v0.z = mLeafVerts[0].z - center.z;
    v1.z = mLeafVerts[1].z - center.z;
    v2.z = mLeafVerts[2].z - center.z;

    FINDMINMAX(v0.z, v1.z, v2.z, min, max);
    if (min > extents.z || max < -extents.z)
    {
        return false;
    }

    // 2) Test if the box intersects the plane of the triangle
    // compute plane equation of triangle: normal*x+d=0
    // ### could be precomputed since we use the same leaf triangle several times
    const Point e0 = v1 - v0;
    const Point e1 = v2 - v1;
    const Point normal = e0 ^ e1;
    const float d = -normal | v0;
    if (!planeBoxOverlap(normal, d, extents))
    {
        return false;
    }

    // 3) "Class III" tests
    if (mFullPrimBoxTest)
    {
        IMPLEMENT_CLASS3_TESTS
    }
    return true;
}
//! if OPC_TRITRI_EPSILON_TEST is true then we do a check (if |dv|<EPSILON then dv=0.0;) else no check is done (which is
//! less robust, but faster)

//! sort so that a<=b
#define SORT(a, b)                                                                                                     \
    if (a > b)                                                                                                         \
    {                                                                                                                  \
        const float c = a;                                                                                             \
        a = b;                                                                                                         \
        b = c;                                                                                                         \
    }

//! Edge to edge test based on Franlin Antonio's gem: "Faster Line Segment Intersection", in Graphics Gems III, pp.
//! 199-202
#define EDGE_EDGE_TEST(V0, U0, U1)                                                                                     \
    Bx = U0[i0] - U1[i0];                                                                                              \
    By = U0[i1] - U1[i1];                                                                                              \
    Cx = V0[i0] - U0[i0];                                                                                              \
    Cy = V0[i1] - U0[i1];                                                                                              \
    f = Ay * Bx - Ax * By;                                                                                             \
    d = By * Cx - Bx * Cy;                                                                                             \
    if ((f > 0.0f && d >= 0.0f && d <= f) || (f < 0.0f && d <= 0.0f && d >= f))                                        \
    {                                                                                                                  \
        const float e = Ax * Cy - Ay * Cx;                                                                             \
        if (f > 0.0f)                                                                                                  \
        {                                                                                                              \
            if (e >= 0.0f && e <= f)                                                                                   \
            {                                                                                                          \
                return true;                                                                                           \
            }                                                                                                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            if (e <= 0.0f && e >= f)                                                                                   \
            {                                                                                                          \
                return true;                                                                                           \
            }                                                                                                          \
        }                                                                                                              \
    }

//! TO BE DOCUMENTED
#define EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2)                                                                     \
    {                                                                                                                  \
        float Bx, By, Cx, Cy, d, f;                                                                                    \
        const float Ax = V1[i0] - V0[i0];                                                                              \
        const float Ay = V1[i1] - V0[i1];                                                                              \
        /* test edge U0,U1 against V0,V1 */                                                                            \
        EDGE_EDGE_TEST(V0, U0, U1);                                                                                    \
        /* test edge U1,U2 against V0,V1 */                                                                            \
        EDGE_EDGE_TEST(V0, U1, U2);                                                                                    \
        /* test edge U2,U1 against V0,V1 */                                                                            \
        EDGE_EDGE_TEST(V0, U2, U0);                                                                                    \
    }

//! TO BE DOCUMENTED
#define POINT_IN_TRI(V0, U0, U1, U2)                                                                                   \
    {                                                                                                                  \
        /* is T1 completly inside T2? */                                                                               \
        /* check if V0 is inside tri(U0,U1,U2) */                                                                      \
        float a = U1[i1] - U0[i1];                                                                                     \
        float b = -(U1[i0] - U0[i0]);                                                                                  \
        float c = -a * U0[i0] - b * U0[i1];                                                                            \
        float d0 = a * V0[i0] + b * V0[i1] + c;                                                                        \
                                                                                                                       \
        a = U2[i1] - U1[i1];                                                                                           \
        b = -(U2[i0] - U1[i0]);                                                                                        \
        c = -a * U1[i0] - b * U1[i1];                                                                                  \
        const float d1 = a * V0[i0] + b * V0[i1] + c;                                                                  \
                                                                                                                       \
        a = U0[i1] - U2[i1];                                                                                           \
        b = -(U0[i0] - U2[i0]);                                                                                        \
        c = -a * U2[i0] - b * U2[i1];                                                                                  \
        const float d2 = a * V0[i0] + b * V0[i1] + c;                                                                  \
        if (d0 * d1 > 0.0f)                                                                                            \
        {                                                                                                              \
            if (d0 * d2 > 0.0f)                                                                                        \
            {                                                                                                          \
                return true;                                                                                           \
            }                                                                                                          \
        }                                                                                                              \
    }

//! TO BE DOCUMENTED
bool CoplanarTriTri(const Point &n, const Point &v0, const Point &v1, const Point &v2, const Point &u0, const Point &u1,
                    const Point &u2)
{
    float A[3];
    short i0, i1;
    /* first project onto an axis-aligned plane, that maximizes the area */
    /* of the triangles, compute indices: i0,i1. */
    A[0] = fabsf(n[0]);
    A[1] = fabsf(n[1]);
    A[2] = fabsf(n[2]);
    if (A[0] > A[1])
    {
        if (A[0] > A[2])
        {
            i0 = 1; /* A[0] is greatest */
            i1 = 2;
        }
        else
        {
            i0 = 0; /* A[2] is greatest */
            i1 = 1;
        }
    }
    else /* A[0]<=A[1] */
    {
        if (A[2] > A[1])
        {
            i0 = 0; /* A[2] is greatest */
            i1 = 1;
        }
        else
        {
            i0 = 0; /* A[1] is greatest */
            i1 = 2;
        }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(v0, v1, u0, u1, u2);
    EDGE_AGAINST_TRI_EDGES(v1, v2, u0, u1, u2);
    EDGE_AGAINST_TRI_EDGES(v2, v0, u0, u1, u2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(v0, u0, u1, u2);
    POINT_IN_TRI(u0, v0, v1, v2);

    return false;
}

//! TO BE DOCUMENTED
#define NEWCOMPUTE_INTERVALS(VV0, VV1, VV2, D0, D1, D2, D0D1, D0D2, A, B, C, X0, X1)                                   \
    {                                                                                                                  \
        if (D0D1 > 0.0f)                                                                                               \
        {                                                                                                              \
            /* here we know that D0D2<=0.0 */                                                                          \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */                                 \
            A = VV2;                                                                                                   \
            B = (VV0 - VV2) * D2;                                                                                      \
            C = (VV1 - VV2) * D2;                                                                                      \
            X0 = D2 - D0;                                                                                              \
            X1 = D2 - D1;                                                                                              \
        }                                                                                                              \
        else if (D0D2 > 0.0f)                                                                                          \
        {                                                                                                              \
            /* here we know that d0d1<=0.0 */                                                                          \
            A = VV1;                                                                                                   \
            B = (VV0 - VV1) * D1;                                                                                      \
            C = (VV2 - VV1) * D1;                                                                                      \
            X0 = D1 - D0;                                                                                              \
            X1 = D1 - D2;                                                                                              \
        }                                                                                                              \
        else if (D1 * D2 > 0.0f || D0 != 0.0f)                                                                         \
        {                                                                                                              \
            /* here we know that d0d1<=0.0 or that D0!=0.0 */                                                          \
            A = VV0;                                                                                                   \
            B = (VV1 - VV0) * D0;                                                                                      \
            C = (VV2 - VV0) * D0;                                                                                      \
            X0 = D0 - D1;                                                                                              \
            X1 = D0 - D2;                                                                                              \
        }                                                                                                              \
        else if (D1 != 0.0f)                                                                                           \
        {                                                                                                              \
            A = VV1;                                                                                                   \
            B = (VV0 - VV1) * D1;                                                                                      \
            C = (VV2 - VV1) * D1;                                                                                      \
            X0 = D1 - D0;                                                                                              \
            X1 = D1 - D2;                                                                                              \
        }                                                                                                              \
        else if (D2 != 0.0f)                                                                                           \
        {                                                                                                              \
            A = VV2;                                                                                                   \
            B = (VV0 - VV2) * D2;                                                                                      \
            C = (VV1 - VV2) * D2;                                                                                      \
            X0 = D2 - D0;                                                                                              \
            X1 = D2 - D1;                                                                                              \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            /* triangles are coplanar */                                                                               \
            return CoplanarTriTri(N1, V0, V1, V2, U0, U1, U2);                                                         \
        }                                                                                                              \
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Triangle/triangle intersection test routine,
 *	by Tomas Moller, 1997.
 *	See article "A Fast Triangle-Triangle Intersection Test",
 *	Journal of Graphics Tools, 2(2), 1997
 *
 *	Updated June 1999: removed the divisions -- a little faster now!
 *	Updated October 1999: added {} to CROSS and SUB macros
 *
 *	int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
 *                      float U0[3],float U1[3],float U2[3])
 *
 *	\param		V0		[in] triangle 0, vertex 0
 *	\param		V1		[in] triangle 0, vertex 1
 *	\param		V2		[in] triangle 0, vertex 2
 *	\param		U0		[in] triangle 1, vertex 0
 *	\param		U1		[in] triangle 1, vertex 1
 *	\param		U2		[in] triangle 1, vertex 2
 *	\return		true if triangles overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AABBTreeCollider::TriTriOverlap(const Point &V0, const Point &V1, const Point &V2, const Point &U0,
                                            const Point &U1, const Point &U2)
{
    // Stats
    mNbPrimPrimTests++;

    // Compute plane equation of triangle(V0,V1,V2)
    Point E1 = V1 - V0;
    Point E2 = V2 - V0;
    const Point N1 = E1 ^ E2;
    const float d1 = -N1 | V0;
    // Plane equation 1: N1.X+d1=0

    // Put U0,U1,U2 into plane equation 1 to compute signed distances to the plane
    float du0 = (N1 | U0) + d1;
    float du1 = (N1 | U1) + d1;
    float du2 = (N1 | U2) + d1;

    // Coplanarity robustness check

    if (fabsf(du0) < std::numeric_limits<float>::epsilon())
    {
        du0 = 0.0f;
    }
    if (fabsf(du1) < std::numeric_limits<float>::epsilon())
    {
        du1 = 0.0f;
    }
    if (fabsf(du2) < std::numeric_limits<float>::epsilon())
    {
        du2 = 0.0f;
    }
    const float du0du1 = du0 * du1;
    const float du0du2 = du0 * du2;

    if (du0du1 > 0.0f && du0du2 > 0.0f) // same sign on all of them + not equal 0 ?
    {
        return false; // no intersection occurs
    }

    // Compute plane of triangle (U0,U1,U2)
    E1 = U1 - U0;
    E2 = U2 - U0;
    const Point N2 = E1 ^ E2;
    const float d2 = -N2 | U0;
    // plane equation 2: N2.X+d2=0

    // put V0,V1,V2 into plane equation 2
    float dv0 = (N2 | V0) + d2;
    float dv1 = (N2 | V1) + d2;
    float dv2 = (N2 | V2) + d2;

    if (fabsf(dv0) < std::numeric_limits<float>::epsilon())
    {
        dv0 = 0.0f;
    }
    if (fabsf(dv1) < std::numeric_limits<float>::epsilon())
    {
        dv1 = 0.0f;
    }
    if (fabsf(dv2) < std::numeric_limits<float>::epsilon())
    {
        dv2 = 0.0f;
    }

    const float dv0dv1 = dv0 * dv1;
    const float dv0dv2 = dv0 * dv2;

    if (dv0dv1 > 0.0f && dv0dv2 > 0.0f) // same sign on all of them + not equal 0 ?
    {
        return false; // no intersection occurs
    }

    // Compute direction of intersection line
    const Point D = N1 ^ N2;

    // Compute and index to the largest component of D
    float max = fabsf(D[0]);
    short index = 0;
    float bb = fabsf(D[1]);
    float cc = fabsf(D[2]);
    if (bb > max)
    {
        max = bb, index = 1;
    }
    if (cc > max)
    {
        max = cc, index = 2;
    }

    // This is the simplified projection onto L
    const float vp0 = V0[index];
    const float vp1 = V1[index];
    const float vp2 = V2[index];

    const float up0 = U0[index];
    const float up1 = U1[index];
    const float up2 = U2[index];

    // Compute interval for triangle 1
    float a, b, c, x0, x1;
    NEWCOMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c, x0, x1);

    // Compute interval for triangle 2
    float d, e, f, y0, y1;
    NEWCOMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f, y0, y1);

    const float xx = x0 * x1;
    const float yy = y0 * y1;
    const float xxyy = xx * yy;

    float isect1[2], isect2[2];

    float tmp = a * xxyy;
    isect1[0] = tmp + b * x1 * yy;
    isect1[1] = tmp + c * x0 * yy;

    tmp = d * xxyy;
    isect2[0] = tmp + e * xx * y1;
    isect2[1] = tmp + f * xx * y0;

    SORT(isect1[0], isect1[1]);
    SORT(isect2[0], isect2[1]);

    if (isect1[1] < isect2[0] || isect2[1] < isect1[0])
    {
        return false;
    }
    return true;
}
