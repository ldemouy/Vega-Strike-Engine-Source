/*
    Copyright (C) 2000 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
-------------------------------------------------------------------------
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*                  ported to VS from CS by Ed Sweetman
-------------------------------------------------------------------------
*/
#include "CSopcodecollider.h"

static std::vector<csCollisionPair> pairs;

csOPCODECollider::csOPCODECollider(const std::vector<mesh_polygon> &polygons)
{
    m_pCollisionModel = nullptr;
    vertholder = nullptr;
    TreeCollider.SetFirstContact(true);
    TreeCollider.SetFullBoxBoxTest(false);
    TreeCollider.SetTemporalCoherence(false);
    opcMeshInt.SetCallback(&MeshCallback, this);
    GeometryInitialize(polygons);
    CollisionFace collFace;
    rCollider.SetFirstContact(true);
}

void csOPCODECollider::GeometryInitialize(const std::vector<mesh_polygon> &polygons)
{
    OPCODECREATE OPCC;
    unsigned int tri_count = 0;
    std::vector<Vector>::size_type vert_count = 0;
    for (std::vector<mesh_polygon>::size_type i = 0; i < polygons.size(); ++i)
    {
        vert_count += polygons[i].v.size();
    }
    tri_count = vert_count / 3;
    if (tri_count)
    {
        m_pCollisionModel = new Model;
        if (!m_pCollisionModel)
        {
            return;
        }

        vertholder = new Point[vert_count];

        csBox3 tmp;
        tmp.StartBoundingBox();
        uint32_t last = 0;

        /* Copies the Vector's in mesh_polygon to Point's in vertholder.
         * This sucks but i dont see anyway around it */
        for (std::vector<mesh_polygon>::size_type i = 0; i < polygons.size(); ++i)
        {
            const mesh_polygon *p = (&polygons[i]);
            for (std::vector<Vector>::size_type j = 0; j < p->v.size(); ++j)
            {
                vertholder[last++].Set(p->v[j].i, p->v[j].j, p->v[j].k);
                tmp.AddBoundingVertex(p->v[j]);
            }
        }
        radius = std::max({tmp.MaxX() - tmp.MinX(), tmp.MaxY() - tmp.MinY(), tmp.MaxZ() - tmp.MinZ()});
        opcMeshInt.SetNbTriangles(tri_count);
        opcMeshInt.SetNbVertices(last);

        // Mesh data
        OPCC.mIMesh = &opcMeshInt;
        OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
        /* NoLeaf and quantized creates an optimized, both in organization and
         * memory overhead, tree.*/
        OPCC.mNoLeaf = true;
        OPCC.mQuantized = true;
    }
    else
    {
        return;
    }

    // bool status = m_pCollisionModel->Build (OPCC);
    m_pCollisionModel->Build(OPCC);
}

csOPCODECollider::~csOPCODECollider()
{

    if (m_pCollisionModel)
    {
        delete m_pCollisionModel;
        m_pCollisionModel = nullptr;
    }
    delete[] vertholder;
}

void csOPCODECollider::MeshCallback(uint32_t triangle_index, VertexPointers &triangle, void *user_data)
{
    csOPCODECollider *collider = (csOPCODECollider *)user_data;
    Point *vertholder = collider->vertholder;
    int32_t index = 3 * triangle_index;
    triangle.Vertex[0] = &vertholder[index];
    triangle.Vertex[1] = &vertholder[index + 1];
    triangle.Vertex[2] = &vertholder[index + 2];
}

bool csOPCODECollider::rayCollide(const Ray &boltbeam, Vector &norm, float &distance)
{
    rCollider.SetHitCallback(&csOPCODECollider::RayCallback);
    rCollider.SetUserData(this);
    rCollider.SetFirstContact(false);
    // rCollider.SetClosestHit(true);
    collFace.mDistance = FLT_MAX;
    bool retval = rCollider.Collide(boltbeam, *m_pCollisionModel);
    rCollider.SetUserData(nullptr);
    if (retval)
    {
        retval = collFace.mDistance != FLT_MAX;
        if (retval)
        {
            distance = collFace.mDistance;
#ifdef VS_DEBUG
            printf("Opcode actually reported a hit at %f meters!\n", distance);
#endif
        }
        else
        {
            return true; // FIXME: buggy! this should return false but the math is obviously broken with opcode, so
                         // opcode is rarely telling us about intersections
        }
        // FIXME set normal
    }
    return retval;
}

void csOPCODECollider::RayCallback(const CollisionFace &faceHit, void *user_data)
{
    csOPCODECollider *collider = (csOPCODECollider *)user_data;
    if (collider)
    {
        if (collider->collFace.mDistance > faceHit.mDistance)
        {
            collider->collFace = faceHit;
        }
    }
}

bool csOPCODECollider::Collide(csOPCODECollider &otherCollider, const csReversibleTransform *trans1,
                               const csReversibleTransform *trans2)
{
    csOPCODECollider *col2 = (csOPCODECollider *)&otherCollider;
    ColCache.Model0 = this->m_pCollisionModel;
    ColCache.Model1 = col2->m_pCollisionModel;
    csMatrix3 m1;
    if (trans1)
    {
        m1 = trans1->GetT2O();
    }
    csMatrix3 m2;
    if (trans2)
    {
        m2 = trans2->GetT2O();
    }
    csVector3 u;
    Matrix4x4 transform1;
    transform1.m[0][3] = 0;
    transform1.m[1][3] = 0;
    transform1.m[2][3] = 0;
    transform1.m[3][3] = 1;
    Matrix4x4 transform2;
    transform2.m[0][3] = 0;
    transform2.m[1][3] = 0;
    transform2.m[2][3] = 0;
    transform2.m[3][3] = 1;
    u = m1.Row1();
    transform1.m[0][0] = u.x;
    transform1.m[1][0] = u.y;
    transform1.m[2][0] = u.z;
    u = m2.Row1();
    transform2.m[0][0] = u.x;
    transform2.m[1][0] = u.y;
    transform2.m[2][0] = u.z;
    u = m1.Row2();
    transform1.m[0][1] = u.x;
    transform1.m[1][1] = u.y;
    transform1.m[2][1] = u.z;
    u = m2.Row2();
    transform2.m[0][1] = u.x;
    transform2.m[1][1] = u.y;
    transform2.m[2][1] = u.z;
    u = m1.Row3();
    transform1.m[0][2] = u.x;
    transform1.m[1][2] = u.y;
    transform1.m[2][2] = u.z;
    u = m2.Row3();
    transform2.m[0][2] = u.x;
    transform2.m[1][2] = u.y;
    transform2.m[2][2] = u.z;
    if (trans1)
    {
        u = trans1->GetO2TTranslation();
    }
    else
    {
        u.Set(0, 0, 0);
    }
    transform1.m[3][0] = u.x;
    transform1.m[3][1] = u.y;
    transform1.m[3][2] = u.z;

    if (trans2)
    {
        u = trans2->GetO2TTranslation();
    }
    else
    {
        u.Set(0, 0, 0);
    }
    transform2.m[3][0] = u.x;
    transform2.m[3][1] = u.y;
    transform2.m[3][2] = u.z;
    if (TreeCollider.Collide(ColCache, &transform1, &transform2))
    {
        bool status = (TreeCollider.GetContactStatus() != false);
        if (status)
        {
            CopyCollisionPairs(this, col2);
        }
        return (status);
    }
    else
    {
        return (false);
    }
}

void csOPCODECollider::ResetCollisionPairs()
{
    pairs.clear();
}

csCollisionPair *csOPCODECollider::GetCollisions()
{
    return (pairs.data());
}

size_t csOPCODECollider::GetCollisionPairCount()
{
    return (pairs.size());
}

Vector csOPCODECollider::getVertex(unsigned int which) const
{
    // This function is used to position the damage particles
    if (!vertholder)
    {
        return (Vector(0, 0, 0));
    }
    return (Vector(vertholder[which].x, vertholder[which].y, vertholder[which].z));
}

void csOPCODECollider::CopyCollisionPairs(csOPCODECollider *col1, csOPCODECollider *col2)
{
    if (!col1 || !col2)
    {
        return;
    }

    uint32_t N_pairs = TreeCollider.GetNbPairs();
    if (N_pairs == 0)
    {
        return;
    }

    const Pair *colPairs = TreeCollider.GetPairs();
    Point *vertholder0 = col1->vertholder;
    Point *vertholder1 = col2->vertholder;
    int32_t j;
    size_t oldlen = pairs.size();
    if (oldlen == 0)
    {
        return;
    }
    for (uint32_t i = 0; i < N_pairs; ++i)
    {
        j = 3 * colPairs[i].id0;
        pairs[oldlen].a1 = vertholder0[j];
        pairs[oldlen].b1 = vertholder0[j + 1];
        pairs[oldlen].c1 = vertholder0[j + 2];
        j = 3 * colPairs[i].id1;
        pairs[oldlen].a2 = vertholder1[j];
        pairs[oldlen].b2 = vertholder1[j + 1];
        pairs[oldlen].c2 = vertholder1[j + 2];
        ++oldlen;
    }
}
