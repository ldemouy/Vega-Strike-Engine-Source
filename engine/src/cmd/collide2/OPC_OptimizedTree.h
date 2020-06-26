///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for optimized trees.
 *	\file		OPC_OptimizedTree.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_OPTIMIZEDTREE_H__
#define __OPC_OPTIMIZEDTREE_H__
#include "Ice/IceMemoryMacros.h"
#include "Ice/IcePoint.h"

//! Common interface for a node of an implicit tree
#define IMPLEMENT_IMPLICIT_NODE(base_class, volume)                               \
public:                                                                           \
	/* Constructor / Destructor */                                                \
	inline base_class() : mData(0) {}                                             \
	inline ~base_class() {}                                                       \
	/* Leaf test */                                                               \
	inline bool IsLeaf() const { return mData & 1; }                              \
	/* Data access */                                                             \
	inline const base_class *GetPos() const { return (base_class *)mData; }       \
	inline const base_class *GetNeg() const { return ((base_class *)mData) + 1; } \
	inline uint32_t GetPrimitive() const { return (uint32_t)(mData >> 1); }       \
	/* Stats */                                                                   \
	inline uint32_t GetNodeSize() const { return SIZEOFOBJECT; }                  \
                                                                                  \
	volume mAABB;                                                                 \
	uintptr_t mData;

//! Common interface for a node of a no-leaf tree
#define IMPLEMENT_NOLEAF_NODE(base_class, volume)                                 \
public:                                                                           \
	/* Constructor / Destructor */                                                \
	inline base_class() : mPosData(0), mNegData(0) {}                             \
	inline ~base_class() {}                                                       \
	/* Leaf tests */                                                              \
	inline bool HasPosLeaf() const { return mPosData & 1; }                       \
	inline bool HasNegLeaf() const { return mNegData & 1; }                       \
	/* Data access */                                                             \
	inline const base_class *GetPos() const { return (base_class *)mPosData; }    \
	inline const base_class *GetNeg() const { return (base_class *)mNegData; }    \
	inline uint32_t GetPosPrimitive() const { return (uint32_t)(mPosData >> 1); } \
	inline uint32_t GetNegPrimitive() const { return (uint32_t)(mNegData >> 1); } \
	/* Stats */                                                                   \
	inline uint32_t GetNodeSize() const { return SIZEOFOBJECT; }                  \
                                                                                  \
	volume mAABB;                                                                 \
	uintptr_t mPosData;                                                           \
	uintptr_t mNegData;

class AABBCollisionNode
{
	IMPLEMENT_IMPLICIT_NODE(AABBCollisionNode, CollisionAABB)

	inline float GetVolume() const { return mAABB.mExtents.x * mAABB.mExtents.y * mAABB.mExtents.z; }
	inline float GetSize() const { return mAABB.mExtents.SquareMagnitude(); }
	inline uint32_t GetRadius() const
	{
		uint32_t *Bits = (uint32_t *)&mAABB.mExtents.x;
		uint32_t Max = Bits[0];
		if (Bits[1] > Max)
			Max = Bits[1];
		if (Bits[2] > Max)
			Max = Bits[2];
		return Max;
	}

	// NB: using the square-magnitude or the true volume of the box, seems to yield better results
	// (assuming UNC-like informed traversal methods). I borrowed this idea from PQP. The usual "size"
	// otherwise, is the largest box extent. In SOLID that extent is computed on-the-fly each time it's
	// needed (the best approach IMHO). In RAPID the rotation matrix is permuted so that Extent[0] is
	// always the greatest, which saves looking for it at runtime. On the other hand, it yields matrices
	// whose determinant is not 1, i.e. you can't encode them anymore as unit quaternions. Not a very
	// good strategy.
};

class AABBQuantizedNode
{
	IMPLEMENT_IMPLICIT_NODE(AABBQuantizedNode, QuantizedAABB)

	inline uint16_t GetSize() const
	{
		const uint16_t *Bits = mAABB.mExtents;
		uint16_t Max = Bits[0];
		if (Bits[1] > Max)
			Max = Bits[1];
		if (Bits[2] > Max)
			Max = Bits[2];
		return Max;
	}
	// NB: for quantized nodes I don't feel like computing a square-magnitude with integers all
	// over the place.......!
};

class AABBNoLeafNode
{
	IMPLEMENT_NOLEAF_NODE(AABBNoLeafNode, CollisionAABB)
};

class AABBQuantizedNoLeafNode
{
	IMPLEMENT_NOLEAF_NODE(AABBQuantizedNoLeafNode, QuantizedAABB)
};

//! Common interface for a collision tree
#define IMPLEMENT_COLLISION_TREE(base_class, node)                                      \
public:                                                                                 \
	/* Constructor / Destructor */                                                      \
	base_class();                                                                       \
	virtual ~base_class();                                                              \
	/* Builds from a standard tree */                                                   \
	virtual bool Build(AABBTree *tree) override;                                        \
	/* Refits the tree */                                                               \
	virtual bool Refit(const MeshInterface *mesh_interface) override;                   \
	/* Walks the tree */                                                                \
	virtual bool Walk(GenericWalkingCallback callback, void *user_data) const override; \
	/* Data access */                                                                   \
	inline const node *GetNodes() const { return mNodes; }                              \
	/* Stats */                                                                         \
	virtual uint32_t GetUsedBytes() const override { return mNbNodes * sizeof(node); }  \
                                                                                        \
private:                                                                                \
	node *mNodes;

typedef bool (*GenericWalkingCallback)(const void *current, void *user_data);

class AABBOptimizedTree
{
public:
	// Constructor / Destructor
	AABBOptimizedTree() : mNbNodes(0)
	{
	}
	virtual ~AABBOptimizedTree() {}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Builds the collision tree from a generic AABB tree.
		 *	\param		tree			[in] generic AABB tree
		 *	\return		true if success
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool Build(AABBTree *tree) = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Refits the collision tree after vertices have been modified.
		 *	\param		mesh_interface	[in] mesh interface for current model
		 *	\return		true if success
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool Refit(const MeshInterface *mesh_interface) = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Walks the tree and call the user back for each node.
		 *	\param		callback	[in] walking callback
		 *	\param		user_data	[in] callback's user data
		 *	\return		true if success
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool Walk(GenericWalkingCallback callback, void *user_data) const = 0;

	// Data access
	virtual uint32_t GetUsedBytes() const = 0;
	inline uint32_t GetNbNodes() const { return mNbNodes; }

protected:
	uint32_t mNbNodes;
};

class AABBCollisionTree : public AABBOptimizedTree
{
	IMPLEMENT_COLLISION_TREE(AABBCollisionTree, AABBCollisionNode)
};

class AABBNoLeafTree : public AABBOptimizedTree
{
	IMPLEMENT_COLLISION_TREE(AABBNoLeafTree, AABBNoLeafNode)
};

class AABBQuantizedTree : public AABBOptimizedTree
{
	IMPLEMENT_COLLISION_TREE(AABBQuantizedTree, AABBQuantizedNode)

public:
	Point mCenterCoeff;
	Point mExtentsCoeff;
};

class AABBQuantizedNoLeafTree : public AABBOptimizedTree
{
	IMPLEMENT_COLLISION_TREE(AABBQuantizedNoLeafTree, AABBQuantizedNoLeafNode)

public:
	Point mCenterCoeff;
	Point mExtentsCoeff;
};

#endif // __OPC_OPTIMIZEDTREE_H__
