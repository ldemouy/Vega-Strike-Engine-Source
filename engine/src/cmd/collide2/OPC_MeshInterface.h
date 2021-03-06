///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a mesh interface.
 *	\file		OPC_MeshInterface.h
 *	\author		Pierre Terdiman
 *	\date		November, 27, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_MESHINTERFACE_H__
#define __OPC_MESHINTERFACE_H__

#include "Ice/IcePoint.h"

struct VertexPointers
{
    const Point *Vertex[3];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	User-callback, called by OPCODE to request vertices from the app.
 *	\param		triangle_index	[in] face index for which the system is requesting the vertices
 *	\param		triangle		[out] triangle's vertices (must be provided by the user)
 *	\param		user_data		[in] user-defined data from SetCallback()
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*RequestCallback)(uint32_t triangle_index, VertexPointers &triangle, void *user_data);

class MeshInterface
{
  public:
    // Constructor / Destructor
    MeshInterface();
    ~MeshInterface();
    // Common settings
    inline uint32_t GetNbTriangles() const
    {
        return mNbTris;
    }
    inline uint32_t GetNbVertices() const
    {
        return mNbVerts;
    }
    inline void SetNbTriangles(uint32_t nb)
    {
        mNbTris = nb;
    }
    inline void SetNbVertices(uint32_t nb)
    {
        mNbVerts = nb;
    }

    // Callback settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Callback control: setups object callback. Must provide triangle-vertices for a given triangle index.
     *	\param		callback	[in] user-defined callback
     *	\param		user_data	[in] user-defined data
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool SetCallback(RequestCallback callback, void *user_data);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Fetches a triangle given a triangle index.
     *	\param		vp		[out] required triangle's vertex pointers
     *	\param		index	[in] triangle index
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void GetTriangle(VertexPointers &vp, uint32_t index) const
    {
        (mObjCallback)(index, vp, mUserData);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Checks the mesh interface is valid, i.e. things have been setup correctly.
     *	\return		true if valid
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsValid() const;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Checks the mesh itself is valid.
     *	Currently we only look for degenerate faces.
     *	\return		number of degenerate faces
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t CheckTopology() const;

  private:
    // User callback
    void *mUserData;              //!< User-defined data sent to callback
    RequestCallback mObjCallback; //!< Object callback

    uint32_t mNbTris;  //!< Number of triangles in the input model
    uint32_t mNbVerts; //!< Number of vertices in the input model
};

#endif //__OPC_MESHINTERFACE_H__
