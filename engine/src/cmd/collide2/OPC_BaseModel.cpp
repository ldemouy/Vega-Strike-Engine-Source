///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains base model interface.
 *	\file		OPC_BaseModel.cpp
 *	\author		Pierre Terdiman
 *	\date		May, 18, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	The base class for collision models.
 *
 *	\class		BaseModel
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		May, 18, 2003
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "OPC_BaseModel.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OPCODECREATE::OPCODECREATE()
{
	mIMesh = nullptr;
	mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
	mSettings.mLimit = 1; // Mandatory for complete trees
	mNoLeaf = true;
	mQuantized = true;
	mKeepOriginal = false;
	mCanRemap = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BaseModel::BaseModel() : mIMesh(nullptr), mModelCode(0), mSource(nullptr), mTree(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BaseModel::~BaseModel()
{
	ReleaseBase();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases everything.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BaseModel::ReleaseBase()
{
	DELETESINGLE(mSource);
	DELETESINGLE(mTree);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Creates an optimized tree according to user-settings, and setups mModelCode.
 *	\param		no_leaf		[in] true for "no leaf" tree
 *	\param		quantized	[in] true for quantized tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BaseModel::CreateTree(bool no_leaf, bool quantized)
{
	DELETESINGLE(mTree);

	// Setup model code
	if (no_leaf)
		mModelCode |= OPC_NO_LEAF;
	else
		mModelCode &= ~OPC_NO_LEAF;

	if (quantized)
		mModelCode |= OPC_QUANTIZED;
	else
		mModelCode &= ~OPC_QUANTIZED;

	// Create the correct class
	if (mModelCode & OPC_NO_LEAF)
	{
		if (mModelCode & OPC_QUANTIZED)
			mTree = new AABBQuantizedNoLeafTree;
		else
			mTree = new AABBNoLeafTree;
	}
	else
	{
		if (mModelCode & OPC_QUANTIZED)
			mTree = new AABBQuantizedTree;
		else
			mTree = new AABBCollisionTree;
	}
	CHECKALLOC(mTree);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the collision model. This can be used to handle dynamic meshes. Usage is:
 *	1. modify your mesh vertices (keep the topology constant!)
 *	2. refit the tree (call this method)
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BaseModel::Refit()
{
	// Refit the optimized tree
	return mTree->Refit(mIMesh);

	// Old code kept for reference : refit the source tree then rebuild !
	//	if(!mSource)	return false;
	//	// Ouch...
	//	mSource->Refit(&mTB);
	//	// Ouch...
	//	return mTree->Build(mSource);
}
