/*
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
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

#include "guidefs.h"
#include "config_xml.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "xml_support.h"

GFXColor SaturatedColor(float r, float g, float b, float a)
{
    static float Saturation = XMLSupport::parse_float(vs_config->getVariable("graphics", "base_saturation", "1.0"));

    return GFXColor((r * Saturation * 3 + (r + b + g) * (1 - Saturation)) / 3,
                    (g * Saturation * 3 + (r + b + g) * (1 - Saturation)) / 3,
                    (b * Saturation * 3 + (r + b + g) * (1 - Saturation)) / 3, a);
}
GFXColor GUI_OPAQUE_BLACK()
{
    static GFXColor gui_black = vs_config->getColor("base_black", GFXColor(0, 0, 0, 1));
    return gui_black;
}
GFXColor GUI_OPAQUE_WHITE()
{
    static GFXColor gui_white = vs_config->getColor("base_white", GFXColor(1, 1, 1, 1));
    return gui_white;
}

GFXColor GUI_OPAQUE_LIGHT_GRAY()
{
    static GFXColor gui_light_gray = vs_config->getColor("base_light_gray", GFXColor(.25, .25, .25, 1));
    return gui_light_gray;
}
GFXColor GUI_OPAQUE_MEDIUM_GRAY()
{
    static GFXColor gui_gray = vs_config->getColor("base_gray", GFXColor(.5, .5, .5, 1));
    return gui_gray;
}
GFXColor GUI_OPAQUE_DARK_GRAY()
{
    static GFXColor gui_dark_gray = vs_config->getColor("base_dark_gray", GFXColor(.75, .75, .75, 1));
    return gui_dark_gray;
}

// Draw a rectangle using the specified color.
void drawRect(const Rect &rect, const GFXColor &color)
{
    GFXDisable(TEXTURE0);

    GFXColorf(color);

    const float verts[4 * 3] = {
        rect.left(),  rect.top(),    0, rect.right(), rect.top(),    0,
        rect.right(), rect.bottom(), 0, rect.left(),  rect.bottom(), 0,
    };
    GFXDraw(GFXQUAD, verts, 4);

    GFXEnable(TEXTURE0);
}

// Draw the outline of a rectangle using the specified color.
void drawRectOutline(const Rect &rect, const GFXColor &color, float lineWidth)
{
    GFXDisable(TEXTURE0);
    GFXLineWidth(lineWidth);
    GFXColorf(color);

    const float verts[5 * 3] = {
        rect.left(), rect.top(),    0, rect.right(), rect.top(), 0, rect.right(), rect.bottom(), 0,
        rect.left(), rect.bottom(), 0, rect.left(),  rect.top(), 0,
    };
    GFXDraw(GFXLINESTRIP, verts, 5);

    GFXEnable(TEXTURE0);
}

// Draw upper-left part of rectangle's "shadow".
void drawUpLeftShadow(const Rect &rect, const GFXColor &color, float lineWidth)
{
    GFXDisable(TEXTURE0);
    GFXLineWidth(lineWidth);
    GFXColorf(color);

    const float verts[3 * 3] = {
        rect.origin.x,
        rect.origin.y,
        0,
        rect.origin.x,
        rect.origin.y + rect.size.height,
        0,
        rect.origin.x + rect.size.width,
        rect.origin.y + rect.size.height,
        0,
    };
    GFXDraw(GFXLINESTRIP, verts, 3);

    GFXEnable(TEXTURE0);
}

// Draw lower-right part of rectangle's "shadow".
void drawLowRightShadow(const Rect &rect, const GFXColor &color, float lineWidth)
{
    GFXDisable(TEXTURE0);
    GFXLineWidth(lineWidth);
    GFXColorf(color);

    const float verts[3 * 3] = {
        rect.origin.x,
        rect.origin.y,
        0,
        rect.origin.x + rect.size.width,
        rect.origin.y,
        0,
        rect.origin.x + rect.size.width,
        rect.origin.y + rect.size.height,
        0,
    };
    GFXDraw(GFXLINESTRIP, verts, 3);

    GFXEnable(TEXTURE0);
}

// Fill a closed polygon.
void drawFilledPolygon(const std::vector<Point2> &coords, const GFXColor &color)
{
    GFXDisable(TEXTURE0);
    GFXColorf(color);

    std::vector<float> verts(coords.size() * 2);
    float *v = &verts[0];
    for (std::vector<Point2>::const_iterator i = coords.begin(); i != coords.end(); i++)
    {
        *v++ = i->x;
        *v++ = i->y;
    }
    GFXDraw(GFXPOLY, &verts[0], coords.size(), 2);

    GFXDisable(TEXTURE0);
}
