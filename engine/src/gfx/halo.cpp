#include "cmd/unit_generic.h"
#include "halo.h"
#include "gldrv/gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "aux_texture.h"
#include "decalqueue.h"
#include "config_xml.h"
#include "xml_support.h"
#include "point_to_cam.h"

static DecalQueue halodecal;
static vector<GFXQuadList *> halodrawqueue;

void Halo::ProcessDrawQueue()
{
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHWRITE);
    GFXPushBlendMode();
    GFXBlendMode(ONE, ONE);
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXLoadIdentity(MODEL);
    for (uint32_t decal = 0; decal < halodrawqueue.size(); decal++)
    {
        if (halodecal.GetTexture(decal))
        {
            halodecal.GetTexture(decal)->MakeActive();
            halodrawqueue[decal]->Draw();
        }
    }
    GFXEnable(DEPTHWRITE);
    GFXEnable(CULLFACE);
    GFXDisable(LIGHTING);
    GFXPopBlendMode();
}
