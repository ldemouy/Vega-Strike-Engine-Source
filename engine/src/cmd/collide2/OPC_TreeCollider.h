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
 *	\file		OPC_TreeCollider.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_TREECOLLIDER_H__
#define __OPC_TREECOLLIDER_H__
#include "Ice/IceContainer.h"
#include "Ice/IcePairs.h"
#include "Ice/IcePoint.h"
#include "OPC_Collider.h"
#include "OPC_Model.h"
#include "OPC_OptimizedTree.h"
//! This structure holds cached information used by the algorithm.
//! Two model pointers and two colliding primitives are cached. Model pointers are assigned
//! to their respective meshes, and the pair of colliding primitives is used for temporal
//! coherence. That is, in case temporal coherence is enabled, those two primitives are
//! tested for overlap before everything else. If they still collide, we're done before
//! even entering the recursive collision code.
struct BVTCache
{
    //! Constructor
    BVTCache()
    {
        ResetCache();
        ResetCountDown();
    }

    void ResetCache()
    {
        Model0 = nullptr;
        Model1 = nullptr;
        id0 = 0;
        id1 = 1;
    }

    void ResetCountDown()
    {
    }

    const Model *Model0; //!< Model for first object
    const Model *Model1; //!< Model for second object
    uint32_t id0;        //!< First index of the pair
    uint32_t id1;        //!< Second index of the pair
};

class AABBTreeCollider : public Collider
{
  public:
    // Constructor / Destructor
    AABBTreeCollider();
    virtual ~AABBTreeCollider();
    // Generic collision query

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Generic collision query for generic OPCODE models. After the call, access the results with:
     *	- GetContactStatus()
     *	- GetNbPairs()
     *	- GetPairs()
     *
     *	\param		cache			[in] collision cache for model pointers and a colliding pair of primitives
     *	\param		world0			[in] world matrix for first object, or null
     *	\param		world1			[in] world matrix for second object, or null
     *	\return		true if success
     *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Collide(BVTCache &cache, const Matrix4x4 *world0 = nullptr, const Matrix4x4 *world1 = nullptr);

    // Collision queries
    bool Collide(const AABBQuantizedNoLeafTree *tree0, const AABBQuantizedNoLeafTree *tree1,
                 const Matrix4x4 *world0 = nullptr, const Matrix4x4 *world1 = nullptr, BVTCache *cache = nullptr);
    // Settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: selects between full box-box tests or "SAT-lite" tests (where Class III axes are discarded)
     *	\param		flag		[in] true for full tests, false for coarse tests
     *	\see		SetFullPrimBoxTest(bool flag)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void SetFullBoxBoxTest(bool flag)
    {
        mFullBoxBoxTest = flag;
    }

    // Data access

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets the number of contacts after a collision query.
     *	\see		GetContactStatus()
     *	\see		GetPairs()
     *	\return		the number of contacts / colliding pairs.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline uint32_t GetNbPairs() const
    {
        return mPairs.GetNbEntries() >> 1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets the pairs of colliding triangles after a collision query.
     *	\see		GetContactStatus()
     *	\see		GetNbPairs()
     *	\return		the list of colliding pairs (triangle indices)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline const Pair *GetPairs() const
    {
        return (const Pair *)mPairs.GetEntries();
    }

  protected:
    // Colliding pairs
    Container mPairs;             //!< Pairs of colliding primitives
                                  // User mesh interfaces
    const MeshInterface *mIMesh0; //!< User-defined mesh interface for object0
    const MeshInterface *mIMesh1; //!< User-defined mesh interface for object1
                                  // Stats
    uint32_t mNbBVBVTests;        //!< Number of BV-BV tests
    uint32_t mNbPrimPrimTests;    //!< Number of Primitive-Primitive tests
    uint32_t mNbBVPrimTests;      //!< Number of BV-Primitive tests
                                  // Precomputed data
    Matrix3x3 mAR;                //!< Absolute rotation matrix
    Matrix3x3 mR0to1;             //!< Rotation from object0 to object1
    Matrix3x3 mR1to0;             //!< Rotation from object1 to object0
    Point mT0to1;                 //!< Translation from object0 to object1
    Point mT1to0;                 //!< Translation from object1 to object0
                                  // Dequantization coeffs
    Point mCenterCoeff0;
    Point mExtentsCoeff0;
    Point mCenterCoeff1;
    Point mExtentsCoeff1;
    // Leaf description
    Point mLeafVerts[3];   //!< Triangle vertices
    uint32_t mLeafIndex;   //!< Triangle index
                           // Settings
    bool mFullBoxBoxTest;  //!< Perform full BV-BV tests (true) or SAT-lite tests (false)
    bool mFullPrimBoxTest; //!< Perform full Primitive-BV tests (true) or SAT-lite tests (false)
                           // Internal methods

    // Quantized no-leaf AABB trees
    void _CollideTriBox(const AABBQuantizedNoLeafNode *b);
    void _CollideBoxTri(const AABBQuantizedNoLeafNode *b);
    void _Collide(const AABBQuantizedNoLeafNode *a, const AABBQuantizedNoLeafNode *b);
    // Overlap tests
    void PrimTest(uint32_t id0, uint32_t id1);
    inline void PrimTestTriIndex(uint32_t id1);
    inline void PrimTestIndexTri(uint32_t id0);

    inline bool BoxBoxOverlap(const Point &ea, const Point &ca, const Point &eb, const Point &cb);
    inline bool TriBoxOverlap(const Point &center, const Point &extents);
    inline bool TriTriOverlap(const Point &V0, const Point &V1, const Point &V2, const Point &U0, const Point &U1,
                              const Point &U2);
    // Init methods
    void InitQuery(const Matrix4x4 *world0 = nullptr, const Matrix4x4 *world1 = nullptr);
    bool CheckTemporalCoherence(BVTCache *cache);

    inline bool Setup(const MeshInterface *mi0, const MeshInterface *mi1)
    {
        mIMesh0 = mi0;
        mIMesh1 = mi1;

        if (!mIMesh0 || !mIMesh1)
        {
            return false;
        }

        return true;
    }
};

#endif // __OPC_TREECOLLIDER_H__
