///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for optimized trees. Implements 4 trees:
 *	- normal
 *	- no leaf
 *	- quantized
 *	- no leaf / quantized
 *
 *	\file		OPC_OptimizedTree.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A standard AABB tree.
 *
 *	\class		AABBCollisionTree
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A no-leaf AABB tree.
 *
 *	\class		AABBNoLeafTree
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A quantized AABB tree.
 *
 *	\class		AABBQuantizedTree
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A quantized no-leaf AABB tree.
 *
 *	\class		AABBQuantizedNoLeafTree
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "OPC_OptimizedTree.h"

//! Compilation flag:
//! - true to fix quantized boxes (i.e. make sure they enclose the original ones)
//! - false to see the effects of quantization errors (faster, but wrong results in some cases)
static bool gFixQuantized = true;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a "no-leaf" tree from a standard one. This is a tree whose leaf nodes have been removed.
 *
 *	Layout for no-leaf trees:
 *
 *	Node:
 *			- box
 *			- P pointer => a node (LSB=0) or a primitive (LSB=1)
 *			- N pointer => a node (LSB=0) or a primitive (LSB=1)
 *
 *	\relates	AABBNoLeafNode
 *	\fn			_BuildNoLeafTree(AABBNoLeafNode* linear, const uint32_t box_id, uint32_t& current_id, const AABBTreeNode* current_node)
 *	\param		linear			[in] base address of destination nodes
 *	\param		box_id			[in] index of destination node
 *	\param		current_id		[in] current running index
 *	\param		current_node	[in] current node from input tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void _BuildNoLeafTree(AABBNoLeafNode *linear, const uint32_t box_id, uint32_t &current_id, const AABBTreeNode *current_node)
{
	const AABBTreeNode *P = current_node->GetPos();
	const AABBTreeNode *N = current_node->GetNeg();
	// Leaf nodes here?!

	// Internal node => keep the box
	current_node->GetAABB()->GetCenter(linear[box_id].mAABB.mCenter);
	current_node->GetAABB()->GetExtents(linear[box_id].mAABB.mExtents);

	if (P->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf

		// Get the primitive index from the input tree
		uint32_t PrimitiveIndex = P->GetPrimitives()[0];
		// Setup prev box data as the primitive index, marked as leaf
		linear[box_id].mPosData = (PrimitiveIndex << 1) | 1;
	}
	else
	{
		// Get a new id for positive child
		uint32_t PosID = current_id++;
		// Setup box data
		linear[box_id].mPosData = (uintptr_t)&linear[PosID];
		// Make sure it's not marked as leaf

		// Recurse
		_BuildNoLeafTree(linear, PosID, current_id, P);
	}

	if (N->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf

		// Get the primitive index from the input tree
		uint32_t PrimitiveIndex = N->GetPrimitives()[0];
		// Setup prev box data as the primitive index, marked as leaf
		linear[box_id].mNegData = (PrimitiveIndex << 1) | 1;
	}
	else
	{
		// Get a new id for negative child
		uint32_t NegID = current_id++;
		// Setup box data
		linear[box_id].mNegData = (uintptr_t)&linear[NegID];
		// Make sure it's not marked as leaf

		// Recurse
		_BuildNoLeafTree(linear, NegID, current_id, N);
	}
}

// Quantization notes:
// - We could use the highest bits of mData to store some more quantized bits. Dequantization code
//   would be slightly more complex, but number of overlap tests would be reduced (and anyhow those
//   bits are currently wasted). Of course it's not possible if we move to 16 bits mData.
// - Something like "16 bits floats" could be tested, to bypass the int-to-float conversion.
// - A dedicated BV-BV test could be used, dequantizing while testing for overlap. (i.e. it's some
//   lazy-dequantization which may save some work in case of early exits). At the very least some
//   muls could be saved by precomputing several more matrices. But maybe not worth the pain.
// - Do we need to dequantize anyway? Not doing the extents-related muls only implies the box has
//   been scaled, for example.
// - The deeper we move into the hierarchy, the smaller the extents should be. May not need a fixed
//   number of quantization bits. Even better, could probably be best delta-encoded.

// Find max values. Some people asked why I wasn't simply using the first node. Well, I can't.
// I'm not looking for (min, max) values like in a standard AABB, I'm looking for the extremal
// centers/extents in order to quantize them. The first node would only give a single center and
// a single extents. While extents would be the biggest, the center wouldn't.
#define FIND_MAX_VALUES                                                                                                  \
	/* Get max values */                                                                                                 \
	Point CMax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()); \
	Point EMax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()); \
	for (i = 0; i < mNbNodes; i++)                                                                                       \
	{                                                                                                                    \
		if (fabsf(Nodes[i].mAABB.mCenter.x) > CMax.x)                                                                    \
		{                                                                                                                \
			CMax.x = fabsf(Nodes[i].mAABB.mCenter.x);                                                                    \
		}                                                                                                                \
		if (fabsf(Nodes[i].mAABB.mCenter.y) > CMax.y)                                                                    \
		{                                                                                                                \
			CMax.y = fabsf(Nodes[i].mAABB.mCenter.y);                                                                    \
		}                                                                                                                \
		if (fabsf(Nodes[i].mAABB.mCenter.z) > CMax.z)                                                                    \
		{                                                                                                                \
			CMax.z = fabsf(Nodes[i].mAABB.mCenter.z);                                                                    \
		}                                                                                                                \
		if (fabsf(Nodes[i].mAABB.mExtents.x) > EMax.x)                                                                   \
		{                                                                                                                \
			EMax.x = fabsf(Nodes[i].mAABB.mExtents.x);                                                                   \
		}                                                                                                                \
		if (fabsf(Nodes[i].mAABB.mExtents.y) > EMax.y)                                                                   \
		{                                                                                                                \
			EMax.y = fabsf(Nodes[i].mAABB.mExtents.y);                                                                   \
		}                                                                                                                \
		if (fabsf(Nodes[i].mAABB.mExtents.z) > EMax.z)                                                                   \
		{                                                                                                                \
			EMax.z = fabsf(Nodes[i].mAABB.mExtents.z);                                                                   \
		}                                                                                                                \
	}

#define INIT_QUANTIZATION                                                   \
	uint32_t nbc = 15; /* Keep one bit for sign */                          \
	uint32_t nbe = 15; /* Keep one bit for fix */                           \
	if (!gFixQuantized)                                                     \
	{                                                                       \
		nbe++;                                                              \
	}                                                                       \
                                                                            \
	/* Compute quantization coeffs */                                       \
	Point CQuantCoeff, EQuantCoeff;                                         \
	CQuantCoeff.x = CMax.x != 0.0f ? float((1 << nbc) - 1) / CMax.x : 0.0f; \
	CQuantCoeff.y = CMax.y != 0.0f ? float((1 << nbc) - 1) / CMax.y : 0.0f; \
	CQuantCoeff.z = CMax.z != 0.0f ? float((1 << nbc) - 1) / CMax.z : 0.0f; \
	EQuantCoeff.x = EMax.x != 0.0f ? float((1 << nbe) - 1) / EMax.x : 0.0f; \
	EQuantCoeff.y = EMax.y != 0.0f ? float((1 << nbe) - 1) / EMax.y : 0.0f; \
	EQuantCoeff.z = EMax.z != 0.0f ? float((1 << nbe) - 1) / EMax.z : 0.0f; \
	/* Compute and save dequantization coeffs */                            \
	mCenterCoeff.x = CQuantCoeff.x != 0.0f ? 1.0f / CQuantCoeff.x : 0.0f;   \
	mCenterCoeff.y = CQuantCoeff.y != 0.0f ? 1.0f / CQuantCoeff.y : 0.0f;   \
	mCenterCoeff.z = CQuantCoeff.z != 0.0f ? 1.0f / CQuantCoeff.z : 0.0f;   \
	mExtentsCoeff.x = EQuantCoeff.x != 0.0f ? 1.0f / EQuantCoeff.x : 0.0f;  \
	mExtentsCoeff.y = EQuantCoeff.y != 0.0f ? 1.0f / EQuantCoeff.y : 0.0f;  \
	mExtentsCoeff.z = EQuantCoeff.z != 0.0f ? 1.0f / EQuantCoeff.z : 0.0f;

#define PERFORM_QUANTIZATION                                                           \
	/* Quantize */                                                                     \
	mNodes[i].mAABB.mCenter[0] = int16_t(Nodes[i].mAABB.mCenter.x * CQuantCoeff.x);    \
	mNodes[i].mAABB.mCenter[1] = int16_t(Nodes[i].mAABB.mCenter.y * CQuantCoeff.y);    \
	mNodes[i].mAABB.mCenter[2] = int16_t(Nodes[i].mAABB.mCenter.z * CQuantCoeff.z);    \
	mNodes[i].mAABB.mExtents[0] = uint16_t(Nodes[i].mAABB.mExtents.x * EQuantCoeff.x); \
	mNodes[i].mAABB.mExtents[1] = uint16_t(Nodes[i].mAABB.mExtents.y * EQuantCoeff.y); \
	mNodes[i].mAABB.mExtents[2] = uint16_t(Nodes[i].mAABB.mExtents.z * EQuantCoeff.z); \
	/* Fix quantized boxes */                                                          \
	if (gFixQuantized)                                                                 \
	{                                                                                  \
		/* Make sure the quantized box is still valid */                               \
		Point Max = Nodes[i].mAABB.mCenter + Nodes[i].mAABB.mExtents;                  \
		Point Min = Nodes[i].mAABB.mCenter - Nodes[i].mAABB.mExtents;                  \
		/* For each axis */                                                            \
		for (uint32_t j = 0; j < 3; j++)                                               \
		{ /* Dequantize the box center */                                              \
			if (fabs(mExtentsCoeff[j]) < 0.00001)                                      \
			{                                                                          \
				mNodes[i].mAABB.mExtents[j] = 0xffff;                                  \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				float qc = float(mNodes[i].mAABB.mCenter[j]) * mCenterCoeff[j];        \
				bool FixMe = true;                                                     \
				do                                                                     \
				{ /* Dequantize the box extent */                                      \
					float qe = float(mNodes[i].mAABB.mExtents[j]) * mExtentsCoeff[j];  \
					/* Compare real & dequantized values */                            \
					if (qc + qe < Max[j] || qc - qe > Min[j])                          \
					{                                                                  \
						mNodes[i].mAABB.mExtents[j]++;                                 \
					}                                                                  \
					else                                                               \
					{                                                                  \
						FixMe = false;                                                 \
					}                                                                  \
					/* Prevent wrapping */                                             \
					if (!mNodes[i].mAABB.mExtents[j])                                  \
					{                                                                  \
						mNodes[i].mAABB.mExtents[j] = 0xffff;                          \
						FixMe = false;                                                 \
					}                                                                  \
				} while (FixMe);                                                       \
			}                                                                          \
		}                                                                              \
	}

#define REMAP_DATA(member)                                                \
	/* Fix data */                                                        \
	Data = Nodes[i].member;                                               \
	if (!(Data & 1))                                                      \
	{                                                                     \
		/* Compute box number */                                          \
		uint32_t Nb = (Data - uintptr_t(Nodes)) / Nodes[i].GetNodeSize(); \
		Data = uintptr_t(&mNodes[Nb]);                                    \
	}                                                                     \
	/* ...remapped */                                                     \
	mNodes[i].member = Data;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedNoLeafTree::AABBQuantizedNoLeafTree() : mNodes(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedNoLeafTree::~AABBQuantizedNoLeafTree()
{
	DELETEARRAY(mNodes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Build(AABBTree *tree)
{
	// Checkings
	if (!tree)
	{
		return false;
	}
	// Check the input tree is complete
	uint32_t NbTriangles = tree->GetNbPrimitives();
	uint32_t NbNodes = tree->GetNbNodes();
	if (NbNodes != NbTriangles * 2 - 1)
	{
		return false;
	}

	// Get nodes
	mNbNodes = NbTriangles - 1;
	DELETEARRAY(mNodes);
	AABBNoLeafNode *Nodes = new AABBNoLeafNode[mNbNodes];
	CHECKALLOC(Nodes);

	// Build the tree
	uint32_t CurID = 1;
	_BuildNoLeafTree(Nodes, 0, CurID, tree);

	// Quantize
	{
		mNodes = new AABBQuantizedNoLeafNode[mNbNodes];
		CHECKALLOC(mNodes);

		uint32_t i;
		// Get max values
		FIND_MAX_VALUES

		// Quantization
		INIT_QUANTIZATION

		// Quantize
		uintptr_t Data;
		for (i = 0; i < mNbNodes; i++)
		{
			PERFORM_QUANTIZATION
			REMAP_DATA(mPosData)
			REMAP_DATA(mNegData)
		}

		DELETEARRAY(Nodes);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the collision tree after vertices have been modified.
 *	\param		mesh_interface	[in] mesh interface for current model
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Refit(const MeshInterface * /*mesh_interface*/)
{

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree and call the user back for each node.
 *	\param		callback	[in] walking callback
 *	\param		user_data	[in] callback's user data
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Walk(GenericWalkingCallback callback, void *user_data) const
{
	if (!callback)
	{
		return false;
	}

	struct Local
	{
		static void _Walk(const AABBQuantizedNoLeafNode *current_node, GenericWalkingCallback callback, void *user_data)
		{
			if (!current_node || !(callback)(current_node, user_data))
			{
				return;
			}

			if (!current_node->HasPosLeaf())
			{
				_Walk(current_node->GetPos(), callback, user_data);
			}
			if (!current_node->HasNegLeaf())
			{
				_Walk(current_node->GetNeg(), callback, user_data);
			}
		}
	};
	Local::_Walk(mNodes, callback, user_data);
	return true;
}
