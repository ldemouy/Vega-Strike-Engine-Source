///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for tree builders.
 *	\file		OPC_TreeBuilders.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_TREEBUILDERS_H__
#define __OPC_TREEBUILDERS_H__

#include "Ice/IcePoint.h"
#include "Ice/IceAABB.h"

//! Tree splitting rules
enum SplittingRules
{
	// Primitive split
	SPLIT_LARGEST_AXIS = (1 << 0),	  //!< Split along the largest axis
	SPLIT_SPLATTER_POINTS = (1 << 1), //!< Splatter primitive centers (QuickCD-style)
	SPLIT_BEST_AXIS = (1 << 2),		  //!< Try largest axis, then second, then last
	SPLIT_BALANCED = (1 << 3),		  //!< Try to keep a well-balanced tree
	SPLIT_FIFTY = (1 << 4),			  //!< Arbitrary 50-50 split
	// Node split
	SPLIT_GEOM_CENTER = (1 << 5), //!< Split at geometric center (else split in the middle)
	//
	SPLIT_FORCE_DWORD = 0x7fffffff
};

//! Simple wrapper around build-related settings [Opcode 1.3]
struct BuildSettings
{
	inline BuildSettings() : mLimit(1), mRules(SPLIT_FORCE_DWORD) {}

	uint32_t mLimit; //!< Limit number of primitives / node. If limit is 1, build a complete tree (2*N-1 nodes)
	uint32_t mRules; //!< Building/Splitting rules (a combination of SplittingRules flags)
};

class AABBTreeBuilder
{
public:
	//! Constructor
	AABBTreeBuilder() : mNbPrimitives(0),
						mNodeBase(nullptr),
						mCount(0),
						mNbInvalidSplits(0) {}
	//! Destructor
	virtual ~AABBTreeBuilder() {}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Computes the AABB of a set of primitives.
		 *	\param		primitives		[in] list of indices of primitives
		 *	\param		nb_prims		[in] number of indices
		 *	\param		global_box		[out] global AABB enclosing the set of input primitives
		 *	\return		true if success
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool ComputeGlobalBox(const uint32_t *primitives, uint32_t nb_prims, AABB &global_box) const = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Computes the splitting value along a given axis for a given primitive.
		 *	\param		index			[in] index of the primitive to split
		 *	\param		axis			[in] axis index (0,1,2)
		 *	\return		splitting value
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual float GetSplittingValue(uint32_t index, uint32_t axis) const = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Computes the splitting value along a given axis for a given node.
		 *	\param		primitives		[in] list of indices of primitives
		 *	\param		nb_prims		[in] number of indices
		 *	\param		global_box		[in] global AABB enclosing the set of input primitives
		 *	\param		axis			[in] axis index (0,1,2)
		 *	\return		splitting value
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual float GetSplittingValue(const uint32_t * /*primitives*/, uint32_t /*nb_prims*/, const AABB &global_box, uint32_t axis) const
	{
		// Default split value = middle of the axis (using only the box)
		return global_box.GetCenter(axis);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Validates node subdivision. This is called each time a node is considered for subdivision, during tree building.
		 *	\param		primitives		[in] list of indices of primitives
		 *	\param		nb_prims		[in] number of indices
		 *	\param		global_box		[in] global AABB enclosing the set of input primitives
		 *	\return		true if the node should be subdivised
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool ValidateSubdivision(const uint32_t * /*primitives*/, uint32_t nb_prims, const AABB & /*global_box*/)
	{
		// Check the user-defined limit
		if (nb_prims <= mSettings.mLimit)
			return false;

		return true;
	}

	BuildSettings mSettings; //!< Splitting rules & split limit [Opcode 1.3]
	uint32_t mNbPrimitives;	 //!< Total number of primitives.
	void *mNodeBase;		 //!< Address of node pool [Opcode 1.3]
	// Stats
	inline void SetCount(uint32_t nb) { mCount = nb; }
	inline void IncreaseCount(uint32_t nb) { mCount += nb; }
	inline uint32_t GetCount() const { return mCount; }
	inline void SetNbInvalidSplits(uint32_t nb) { mNbInvalidSplits = nb; }
	inline void IncreaseNbInvalidSplits() { mNbInvalidSplits++; }
	inline uint32_t GetNbInvalidSplits() const { return mNbInvalidSplits; }

private:
	uint32_t mCount;		   //!< Stats: number of nodes created
	uint32_t mNbInvalidSplits; //!< Stats: number of invalid splits
};

class AABBTreeOfVerticesBuilder : public AABBTreeBuilder
{
public:
	//! Constructor
	AABBTreeOfVerticesBuilder() : mVertexArray(nullptr) {}
	//! Destructor
	virtual ~AABBTreeOfVerticesBuilder() {}

	virtual bool ComputeGlobalBox(const uint32_t *primitives, uint32_t nb_prims, AABB &global_box) const override;
	virtual float GetSplittingValue(uint32_t index, uint32_t axis) const override;
	virtual float GetSplittingValue(const uint32_t *primitives, uint32_t nb_prims, const AABB &global_box, uint32_t axis) const override;

	const Point *mVertexArray; //!< Shortcut to an app-controlled array of vertices.
};

class AABBTreeOfAABBsBuilder : public AABBTreeBuilder
{
public:
	//! Constructor
	AABBTreeOfAABBsBuilder() : mAABBArray(nullptr) {}
	//! Destructor
	virtual ~AABBTreeOfAABBsBuilder() {}

	virtual bool ComputeGlobalBox(const uint32_t *primitives, uint32_t nb_prims, AABB &global_box) const override;
	virtual float GetSplittingValue(uint32_t index, uint32_t axis) const override;

	const AABB *mAABBArray; //!< Shortcut to an app-controlled array of AABBs.
};

class AABBTreeOfTrianglesBuilder : public AABBTreeBuilder
{
public:
	//! Constructor
	AABBTreeOfTrianglesBuilder() : mIMesh(nullptr) {}
	//! Destructor
	virtual ~AABBTreeOfTrianglesBuilder() {}

	virtual bool ComputeGlobalBox(const uint32_t *primitives, uint32_t nb_prims, AABB &global_box) const override;
	virtual float GetSplittingValue(uint32_t index, uint32_t axis) const override;
	virtual float GetSplittingValue(const uint32_t *primitives, uint32_t nb_prims, const AABB &global_box, uint32_t axis) const override;

	const MeshInterface *mIMesh; //!< Shortcut to an app-controlled mesh interface
};

#endif // __OPC_TREEBUILDERS_H__
