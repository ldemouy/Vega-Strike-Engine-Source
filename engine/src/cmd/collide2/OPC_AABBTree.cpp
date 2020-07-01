///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a versatile AABB tree.
 *	\file		OPC_AABBTree.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a generic AABB tree node.
 *
 *	\class		AABBTreeNode
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a generic AABB tree.
 *	This is a vanilla AABB tree, without any particular optimization. It contains anonymous references to
 *	user-provided primitives, which can theoretically be anything - triangles, boxes, etc. Each primitive
 *	is surrounded by an AABB, regardless of the primitive's nature. When the primitive is a triangle, the
 *	resulting tree can be converted into an optimized tree. If the primitive is a box, the resulting tree
 *	can be used for culling - VFC or occlusion -, assuming you cull on a mesh-by-mesh basis (modern way).
 *
 *	\class		AABBTree
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "OPC_AABBTree.h"
#include "Ice/IcePoint.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeNode::AABBTreeNode() : mPos(0), mNodePrimitives(nullptr), mNbPrimitives(0)
{
#ifdef OPC_USE_TREE_COHERENCE
    mBitmask = 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeNode::~AABBTreeNode()
{
    // Opcode 1.3:
    const AABBTreeNode *Pos = GetPos();

    if (!(mPos & 1))
    {
        DELETEARRAY(Pos);
    }
    mNodePrimitives = nullptr; // This was just a shortcut to the global list => no release
    mNbPrimitives = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Splits the node along a given axis.
 *	The list of indices is reorganized according to the split values.
 *	\param		axis		[in] splitting axis index
 *	\param		builder		[in] the tree builder
 *	\return		the number of primitives assigned to the first child
 *	\warning	this method reorganizes the internal list of primitives
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t AABBTreeNode::Split(uint32_t axis, AABBTreeBuilder *builder)
{
    // Get node split value
    float SplitValue = builder->GetSplittingValue(mNodePrimitives, mNbPrimitives, mBV, axis);

    uint32_t NbPos = 0;
    // Loop through all node-related primitives. Their indices range from mNodePrimitives[0] to
    // mNodePrimitives[mNbPrimitives-1]. Those indices map the global list in the tree builder.
    for (uint32_t i = 0; i < mNbPrimitives; i++)
    {
        // Get index in global list
        uint32_t Index = mNodePrimitives[i];

        // Test against the splitting value. The primitive value is tested against the enclosing-box center.
        // [We only need an approximate partition of the enclosing box here.]
        float PrimitiveValue = builder->GetSplittingValue(Index, axis);

        // Reorganize the list of indices in this order: positive - negative.
        if (PrimitiveValue > SplitValue)
        {
            // Swap entries
            uint32_t Tmp = mNodePrimitives[i];
            mNodePrimitives[i] = mNodePrimitives[NbPos];
            mNodePrimitives[NbPos] = Tmp;
            // Count primitives assigned to positive space
            NbPos++;
        }
    }
    return NbPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Subdivides the node.
 *
 *	          N
 *	        /   \
 *	      /       \
 *	   N/2         N/2
 *	  /   \       /   \
 *	N/4   N/4   N/4   N/4
 *	(etc)
 *
 *	A well-balanced tree should have a O(log n) depth.
 *	A degenerate tree would have a O(n) depth.
 *	Note a perfectly-balanced tree is not well-suited to collision detection anyway.
 *
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeNode::Subdivide(AABBTreeBuilder *builder)
{
    // Checkings
    if (!builder)
    {
        return false;
    }

    // Stop subdividing if we reach a leaf node. This is always performed here,
    // else we could end in trouble if user overrides this.
    if (mNbPrimitives == 1)
    {
        return true;
    }

    // Let the user validate the subdivision
    if (!builder->ValidateSubdivision(mNodePrimitives, mNbPrimitives, mBV))
    {
        return true;
    }

    bool ValidSplit = true; // Optimism...
    uint32_t NbPos;
    if (builder->mSettings.mRules & SPLIT_LARGEST_AXIS)
    {
        // Find the largest axis to split along
        Point Extents;
        mBV.GetExtents(Extents);               // Box extents
        uint32_t Axis = Extents.LargestAxis(); // Index of largest axis

        // Split along the axis
        NbPos = Split(Axis, builder);

        // Check split validity
        if (!NbPos || NbPos == mNbPrimitives)
        {
            ValidSplit = false;
        }
    }
    else if (builder->mSettings.mRules & SPLIT_SPLATTER_POINTS)
    {

        // Compute the means
        Point Means(0.0f, 0.0f, 0.0f);
        for (uint32_t i = 0; i < mNbPrimitives; i++)
        {
            uint32_t Index = mNodePrimitives[i];
            Means.x += builder->GetSplittingValue(Index, 0);
            Means.y += builder->GetSplittingValue(Index, 1);
            Means.z += builder->GetSplittingValue(Index, 2);
        }
        Means /= float(mNbPrimitives);

        // Compute variances
        Point Vars(0.0f, 0.0f, 0.0f);
        for (uint32_t i = 0; i < mNbPrimitives; i++)
        {
            uint32_t Index = mNodePrimitives[i];
            float Cx = builder->GetSplittingValue(Index, 0);
            float Cy = builder->GetSplittingValue(Index, 1);
            float Cz = builder->GetSplittingValue(Index, 2);
            Vars.x += (Cx - Means.x) * (Cx - Means.x);
            Vars.y += (Cy - Means.y) * (Cy - Means.y);
            Vars.z += (Cz - Means.z) * (Cz - Means.z);
        }
        Vars /= float(mNbPrimitives - 1);

        // Choose axis with greatest variance
        uint32_t Axis = Vars.LargestAxis();

        // Split along the axis
        NbPos = Split(Axis, builder);

        // Check split validity
        if (!NbPos || NbPos == mNbPrimitives)
        {
            ValidSplit = false;
        }
    }
    else if (builder->mSettings.mRules & SPLIT_BALANCED)
    {
        // Test 3 axis, take the best
        float Results[3];
        NbPos = Split(0, builder);
        Results[0] = float(NbPos) / float(mNbPrimitives);
        NbPos = Split(1, builder);
        Results[1] = float(NbPos) / float(mNbPrimitives);
        NbPos = Split(2, builder);
        Results[2] = float(NbPos) / float(mNbPrimitives);
        Results[0] -= 0.5f;
        Results[0] *= Results[0];
        Results[1] -= 0.5f;
        Results[1] *= Results[1];
        Results[2] -= 0.5f;
        Results[2] *= Results[2];
        uint32_t Min = 0;
        if (Results[1] < Results[Min])
        {
            Min = 1;
        }
        if (Results[2] < Results[Min])
        {
            Min = 2;
        }

        // Split along the axis
        NbPos = Split(Min, builder);

        // Check split validity
        if (!NbPos || NbPos == mNbPrimitives)
        {
            ValidSplit = false;
        }
    }
    else if (builder->mSettings.mRules & SPLIT_BEST_AXIS)
    {
        // Test largest, then middle, then smallest axis...

        // Sort axis
        Point Extents;
        mBV.GetExtents(Extents); // Box extents
        uint32_t SortedAxis[] = {0, 1, 2};
        float *Keys = (float *)&Extents.x;
        for (uint32_t j = 0; j < 3; j++)
        {
            for (uint32_t i = 0; i < 2; i++)
            {
                if (Keys[SortedAxis[i]] < Keys[SortedAxis[i + 1]])
                {
                    uint32_t Tmp = SortedAxis[i];
                    SortedAxis[i] = SortedAxis[i + 1];
                    SortedAxis[i + 1] = Tmp;
                }
            }
        }

        // Find the largest axis to split along
        uint32_t CurAxis = 0;
        ValidSplit = false;
        while (!ValidSplit && CurAxis != 3)
        {
            NbPos = Split(SortedAxis[CurAxis], builder);
            // Check the subdivision has been successful
            if (!NbPos || NbPos == mNbPrimitives)
            {
                CurAxis++;
            }
            else
            {
                ValidSplit = true;
            }
        }
    }
    else if (builder->mSettings.mRules & SPLIT_FIFTY)
    {
        // Don't even bother splitting (mainly a performance test)
        NbPos = mNbPrimitives >> 1;
    }
    else
    {
        return false; // Unknown splitting rules
    }

    // Check the subdivision has been successful
    if (!ValidSplit)
    {
        // Here, all boxes lie in the same sub-space. Two strategies:
        // - if the tree *must* be complete, make an arbitrary 50-50 split
        // - else stop subdividing
        //		if(builder->mSettings.mRules&SPLIT_COMPLETE)
        if (builder->mSettings.mLimit == 1)
        {
            builder->IncreaseNbInvalidSplits();
            NbPos = mNbPrimitives >> 1;
        }
        else
        {
            return true;
        }
    }

    // Now create children and assign their pointers.
    if (builder->mNodeBase)
    {
        // We use a pre-allocated linear pool for complete trees [Opcode 1.3]
        AABBTreeNode *Pool = (AABBTreeNode *)builder->mNodeBase;
        uint32_t Count = builder->GetCount() - 1; // Count begins to 1...
        // Set last bit to tell it shouldn't be freed ### pretty ugly, find a better way. Maybe one bit in mNbPrimitives

        mPos = uintptr_t(&Pool[Count + 0]) | 1;
    }
    else
    {
        // Non-complete trees and/or Opcode 1.2 allocate nodes on-the-fly
        AABBTreeNode *PosNeg = new AABBTreeNode[2];
        CHECKALLOC(PosNeg);
        mPos = (uintptr_t)PosNeg;
    }

    // Update stats
    builder->IncreaseCount(2);

    // Assign children
    AABBTreeNode *Pos = (AABBTreeNode *)GetPos();
    AABBTreeNode *Neg = (AABBTreeNode *)GetNeg();
    Pos->mNodePrimitives = &mNodePrimitives[0];
    Pos->mNbPrimitives = NbPos;
    Neg->mNodePrimitives = &mNodePrimitives[NbPos];
    Neg->mNbPrimitives = mNbPrimitives - NbPos;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive hierarchy building in a top-down fashion.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeNode::_BuildHierarchy(AABBTreeBuilder *builder)
{
    // 1) Compute the global box for current node. The box is stored in mBV.
    builder->ComputeGlobalBox(mNodePrimitives, mNbPrimitives, mBV);

    // 2) Subdivide current node
    Subdivide(builder);

    // 3) Recurse
    AABBTreeNode *Pos = (AABBTreeNode *)GetPos();
    AABBTreeNode *Neg = (AABBTreeNode *)GetNeg();
    if (Pos)
    {
        Pos->_BuildHierarchy(builder);
    }
    if (Neg)
    {
        Neg->_BuildHierarchy(builder);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree (top-down).
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeNode::_Refit(AABBTreeBuilder *builder)
{
    // 1) Recompute the new global box for current node
    builder->ComputeGlobalBox(mNodePrimitives, mNbPrimitives, mBV);

    // 2) Recurse
    AABBTreeNode *Pos = (AABBTreeNode *)GetPos();
    AABBTreeNode *Neg = (AABBTreeNode *)GetNeg();
    if (Pos)
    {
        Pos->_Refit(builder);
    }
    if (Neg)
    {
        Neg->_Refit(builder);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::AABBTree() : mIndices(nullptr), mPool(nullptr), mTotalNbNodes(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::~AABBTree()
{
    Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases the tree.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTree::Release()
{
    DELETEARRAY(mPool);
    DELETEARRAY(mIndices);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a generic AABB tree from a tree builder.
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Build(AABBTreeBuilder *builder)
{
    // Checkings
    if (!builder || !builder->mNbPrimitives)
    {
        return false;
    }

    // Release previous tree
    Release();

    // Init stats
    builder->SetCount(1);
    builder->SetNbInvalidSplits(0);

    // Initialize indices. This list will be modified during build.
    mIndices = new uint32_t[builder->mNbPrimitives];
    CHECKALLOC(mIndices);
    // Identity permutation
    for (uint32_t i = 0; i < builder->mNbPrimitives; i++)
    {
        mIndices[i] = i;
    }

    // Setup initial node. Here we have a complete permutation of the app's primitives.
    mNodePrimitives = mIndices;
    mNbPrimitives = builder->mNbPrimitives;

    // Use a linear array for complete trees (since we can predict the final number of nodes) [Opcode 1.3]
    //	if(builder->mRules&SPLIT_COMPLETE)
    if (builder->mSettings.mLimit == 1)
    {
        // Allocate a pool of nodes
        mPool = new AABBTreeNode[builder->mNbPrimitives * 2 - 1];

        builder->mNodeBase = mPool; // ### ugly !
    }

    // Build the hierarchy
    _BuildHierarchy(builder);

    // Get back total number of nodes
    mTotalNbNodes = builder->GetCount();

    // For complete trees, check the correct number of nodes has been created [Opcode 1.3]
    if (mPool)
    {
    }

    return true;
}
