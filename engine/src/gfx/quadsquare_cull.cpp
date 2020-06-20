#include "quadsquare.h"

void quadsquare::ResetTree()
{
    //Clear all enabled flags, and delete all non-static child nodes.
    int i;
    for (i = 0; i < 4; i++)
        if (Child[i])
        {
            Child[i]->ResetTree();
            if (Child[i]->Static == false)
            {
                delete Child[i];
                Child[i] = 0;
            }
        }
    EnabledFlags = 0;
    SubEnabledCount[0] = 0;
    SubEnabledCount[1] = 0;
    Dirty = true;
}

void quadsquare::StaticCullData(const quadcornerdata &cd, float ThresholdDetail)
{
    //Examine the tree and remove nodes which don't contain necessary
    //detail.  Necessary detail is defined as vertex data with a
    //edge-length to height ratio less than ThresholdDetail.
    //First, clean non-static nodes out of the tree.
    ResetTree();
    //Make sure error values are up-to-date.
    if (Dirty)
        RecomputeErrorAndLighting(cd);
    //Recursively check all the nodes and do necessary removal.
    //We must start at the bottom of the tree, and do one level of
    //the tree at a time, to ensure the dependencies are accounted
    //for properly.
    int level;
    for (level = 0; level < 15; level++)
        StaticCullAux(cd, ThresholdDetail, level);
}

void quadsquare::StaticCullAux(const quadcornerdata &cd, float ThresholdDetail, int TargetLevel)
{
    //Check this node and its descendents, and remove nodes which don't contain
    //necessary detail.
    int i, j;
    quadcornerdata q;
    if (cd.Level > TargetLevel)
    {
        //Just recurse to child nodes.
        for (j = 0; j < 4; j++)
        {
            if (j < 2)
                i = 1 - j;
            else
                i = j;
            if (Child[i])
            {
                SetupCornerData(&q, cd, i);
                Child[i]->StaticCullAux(q, ThresholdDetail, TargetLevel);
            }
        }
        return;
    }
    //We're at the target level.  Check this node to see if it's OK to delete it.

    //Check edge vertices to see if they're necessary.
    float size = 2 << cd.Level; //Edge length.
    if (Child[0] == nullptr && Child[3] == nullptr && Error[0] * ThresholdDetail < size)
    {
        quadsquare *s = GetFarNeighbor(0, cd);
        if (s == nullptr || (s->Child[1] == nullptr && s->Child[2] == nullptr))
        {
            //Force vertex height to the edge value.
            unsigned short y = (unsigned short)((cd.Verts[0].Y + cd.Verts[3].Y) * 0.5);
            Vertex[1].Y = y;
            Error[0] = 0;
            //Force alias vertex to match.
            if (s)
                s->Vertex[3].Y = y;
            Dirty = true;
        }
    }
    if (Child[2] == nullptr && Child[3] == nullptr && Error[1] * ThresholdDetail < size)
    {
        quadsquare *s = GetFarNeighbor(3, cd);
        if (s == nullptr || (s->Child[0] == nullptr && s->Child[1] == nullptr))
        {
            unsigned short y = (unsigned short)((cd.Verts[2].Y + cd.Verts[3].Y) * 0.5);
            Vertex[4].Y = y;
            Error[1] = 0;
            if (s)
                s->Vertex[2].Y = y;
            Dirty = true;
        }
    }
    //See if we have child nodes.
    bool StaticChildren = false;
    for (i = 0; i < 4; i++)
        if (Child[i])
        {
            StaticChildren = true;
            if (Child[i]->Dirty)
                Dirty = true;
        }
    //If we have no children and no necessary edges, then see if we can delete ourself.
    if (StaticChildren == false && cd.Parent != nullptr)
    {
        bool NecessaryEdges = false;
        for (i = 0; i < 4; i++)
        {
            //See if vertex deviates from edge between corners.
            float diff = fabs(Vertex[i + 1].Y - (cd.Verts[i].Y + cd.Verts[(i + 3) & 3].Y) * 0.5);
            if (diff > 0.00001)
                NecessaryEdges = true;
        }
        if (!NecessaryEdges)
        {
            size *= 1.414213562; //sqrt(2), because diagonal is longer than side.
            if (cd.Parent->Square->Error[2 + cd.ChildIndex] * ThresholdDetail < size)
            {
                delete cd.Parent->Square->Child[cd.ChildIndex]; //Delete this.
                cd.Parent->Square->Child[cd.ChildIndex] = 0;    //Clear the pointer.
            }
        }
    }
}
