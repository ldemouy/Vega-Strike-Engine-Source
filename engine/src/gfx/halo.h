#ifndef __HALO_H
#define __HALO_H

#include "gldrv/gfxlib.h"
#include "vec.h"
#include "quaternion.h"

#define TranslucentWhite (GFXColor(1, 1, 1, .5))
#define ZeroQvector (QVector(0, 0, 0))

class Halo
{
    QVector position;
    float sizex;
    float sizey;
    int decal;
    int quadnum;

public:
    static void ProcessDrawQueue();
};

#endif
