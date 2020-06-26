///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a planes collider.
 *	\file		OPC_PlanesCollider.h
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_PLANESCOLLIDER_H__
#define __OPC_PLANESCOLLIDER_H__

struct PlanesCache : VolumeCache
{
	PlanesCache()
	{
	}
};

class PlanesCollider : public VolumeCollider
{
public:
	// Constructor / Destructor
	PlanesCollider();
	virtual ~PlanesCollider();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Generic collision query for generic OPCODE models. After the call, access the results:
		 *	- with GetContactStatus()
		 *	- with GetNbTouchedPrimitives()
		 *	- with GetTouchedPrimitives()
		 *
		 *	\param		cache			[in/out] a planes cache
		 *	\param		planes			[in] list of planes in world space
		 *	\param		nb_planes		[in] number of planes
		 *	\param		model			[in] Opcode model to collide with
		 *	\param		worldm			[in] model's world matrix, or null
		 *	\return		true if success
		 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Collide(PlanesCache &cache, const Plane *planes, uint32_t nb_planes, const Model &model, const Matrix4x4 *worldm = nullptr);

	// Mutant box-with-planes collision queries
	inline bool Collide(PlanesCache &cache, const OBB &box, const Model &model, const Matrix4x4 *worldb = nullptr, const Matrix4x4 *worldm = nullptr)
	{
		Plane PL[6];

		if (worldb)
		{
			// Create a new OBB in world space
			OBB WorldBox;
			box.Rotate(*worldb, WorldBox);
			// Compute planes from the sides of the box
			WorldBox.ComputePlanes(PL);
		}
		else
		{
			// Compute planes from the sides of the box
			box.ComputePlanes(PL);
		}

		// Collide with box planes
		return Collide(cache, PL, 6, model, worldm);
	}
	// Settings

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
		 *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
		 *	\return		null if everything is ok, else a string describing the problem
		 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const char *ValidateSettings() override;

protected:
	// Planes in model space
	uint32_t mNbPlanes;
	Plane *mPlanes;
	// Leaf description
	VertexPointers mVP;
	// Internal methods
	void _Collide(const AABBCollisionNode *node, uint32_t clip_mask);
	void _Collide(const AABBNoLeafNode *node, uint32_t clip_mask);
	void _Collide(const AABBQuantizedNode *node, uint32_t clip_mask);
	void _Collide(const AABBQuantizedNoLeafNode *node, uint32_t clip_mask);
	void _CollideNoPrimitiveTest(const AABBCollisionNode *node, uint32_t clip_mask);
	void _CollideNoPrimitiveTest(const AABBNoLeafNode *node, uint32_t clip_mask);
	void _CollideNoPrimitiveTest(const AABBQuantizedNode *node, uint32_t clip_mask);
	void _CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode *node, uint32_t clip_mask);
	// Overlap tests
	inline bool PlanesAABBOverlap(const Point &center, const Point &extents, uint32_t &out_clip_mask, uint32_t in_clip_mask);
	inline bool PlanesTriOverlap(uint32_t in_clip_mask);
	// Init methods
	bool InitQuery(PlanesCache &cache, const Plane *planes, uint32_t nb_planes, const Matrix4x4 *worldm = nullptr);
};

class HybridPlanesCollider : public PlanesCollider
{
public:
	// Constructor / Destructor
	HybridPlanesCollider();
	virtual ~HybridPlanesCollider();

	bool Collide(PlanesCache &cache, const Plane *planes, uint32_t nb_planes, const HybridModel &model, const Matrix4x4 *worldm = nullptr);

protected:
	Container mTouchedBoxes;
};

#endif // __OPC_PLANESCOLLIDER_H__
