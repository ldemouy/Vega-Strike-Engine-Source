/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "gl_globals.h"

#include "gldrv/gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include <stdio.h>

GFXQuadList::GFXQuadList(GFXBOOL color) : numVertices(0), numQuads(0)
{
    data.vertices = nullptr;
    Dirty = GFXFALSE;
    isColor = color;
}

GFXQuadList::~GFXQuadList()
{
    if (isColor && data.colors)
        free(data.colors);
    else if (!isColor && data.vertices)
        free(data.vertices);
}

void GFXQuadList::Draw()
{
    if (!numQuads)
        return;
    if (isColor)
        glInterleavedArrays(GL_T2F_C4F_N3F_V3F, sizeof(GFXColorVertex), &data.colors[0]);
    else
        glInterleavedArrays(GL_T2F_N3F_V3F, sizeof(GFXVertex), &data.vertices[0]);
    glDrawArrays(GL_QUADS, 0, numQuads * 4);
    if (isColor)
        GFXColor(1, 1, 1, 1);
}
