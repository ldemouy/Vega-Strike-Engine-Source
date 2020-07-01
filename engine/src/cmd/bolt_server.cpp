#include "bolt.h"
#include "gfx/mesh.h"
#include "gldrv/gfxlib.h"
#include "gldrv/gfxlib_struct.h"
#include <vector>

#include "configxml.h"
#include "unit_generic.h"
#include <algorithm>
#include <string>
GFXVertexList *bolt_draw::boltmesh = nullptr;

bolt_draw::~bolt_draw()
{

    for (uint32_t i = 0; i < balls.size(); i++)
    {
        for (int32_t j = balls[i].size() - 1; j >= 0; j--)
        {
            balls[i][j].Destroy(j);
        }
    }
    for (uint32_t i = 0; i < bolts.size(); i++)
    {
        for (int32_t j = balls[i].size() - 1; j >= 0; j--)
        {
            bolts[i][j].Destroy(j);
        }
    }
}

bolt_draw::bolt_draw()
{
    boltmesh = nullptr;
    boltdecals = nullptr;
}
int32_t Bolt::AddTexture(bolt_draw *q, std::string file)
{
    int32_t decal = 0;
    if (decal >= (int32_t)q->bolts.size())
    {
        q->bolts.push_back(vector<Bolt>());
    }
    return decal;
}
int32_t Bolt::AddAnimation(bolt_draw *q, std::string file, QVector cur_position)
{
    int32_t decal = 0;
    if (decal >= (int32_t)q->balls.size())
    {
        q->balls.push_back(vector<Bolt>());
    }
    return decal;
}

void Bolt::Draw()
{
}
extern void BoltDestroyGeneric(Bolt *whichbolt, uint32_t index, int32_t decal, bool isBall);
void Bolt::Destroy(uint32_t index)
{
    BoltDestroyGeneric(this, index, decal, type->type != weapon_info::BOLT);
}
