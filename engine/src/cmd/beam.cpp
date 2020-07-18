#include "beam.h"
#include "gfx/aux_texture.h"
#include "gfx/decalqueue.h"
#include "unit_generic.h"
#include "vegastrike.h"
#include <vector>
using std::vector;
#include "aldrv/audiolib.h"
#include "configxml.h"
#include "images.h"

struct BeamDrawContext
{
    Matrix m;
    class GFXVertexList *vlist;
    Beam *beam;
    BeamDrawContext()
    {
    }
    BeamDrawContext(const Matrix &a, GFXVertexList *vl, Beam *b) : m(a), vlist(vl), beam(b)
    {
    }
};

static DecalQueue beamdecals;
static vector<vector<BeamDrawContext>> beamdrawqueue;

Beam::Beam(const Transformation &trans, const weapon_info &clne, void *own, Unit *firer, int sound)
    : vlist(nullptr), Col(clne.r, clne.g, clne.b, clne.a)
{
    VSCONSTRUCT2('B')
    listen_to_owner = false; // warning this line of code is also present in beam_server.cpp change one, change ALL
#ifdef PERBOLTSOUND
    sound = AUDCreateSound(clne.sound, true);
#else
    this->sound = sound;
#endif
    decal = beamdecals.AddTexture(clne.file.c_str(), TRILINEAR);
    if (decal >= beamdrawqueue.size())
        beamdrawqueue.push_back(vector<BeamDrawContext>());
    Init(trans, clne, own, firer);
    impact = UNSTABLE;
}

Beam::~Beam()
{
    VSDESTRUCT2
#ifdef PERBOLTSOUND
    AUDDeleteSound(sound);
#endif
#ifdef BEAMCOLQ
    RemoveFromSystem(true);
#endif
    // DO NOT DELETE - shared vlist
    // delete vlist;
    beamdecals.DelTexture(decal);
}

extern void AdjustMatrixToTrackTarget(Matrix &mat, const Vector &vel, Unit *target, float speed, bool lead, float cone);

void Beam::Draw(const Transformation &trans, const Matrix &m, Unit *targ, float tracking_cone)
{
    // hope that the correct transformation is on teh stack
    if (curthick == 0)
    {
        return;
    }
    Matrix cumulative_transformation_matrix;
    local_transformation.to_matrix(cumulative_transformation_matrix);
    Transformation cumulative_transformation = local_transformation;
    cumulative_transformation.Compose(trans, m);
    cumulative_transformation.to_matrix(cumulative_transformation_matrix);
    AdjustMatrixToTrackTarget(cumulative_transformation_matrix, Vector(0, 0, 0), targ, speed, false, tracking_cone);
#ifdef PERFRAMESOUND
    AUDAdjustSound(sound, cumulative_transformation.position,
                   speed * Vector(cumulative_transformation_matrix[8], cumulative_transformation_matrix[9],
                                  cumulative_transformation_matrix[10]));
#endif
    AUDSoundGain(sound, curthick * curthick / (thickness * thickness));

    beamdrawqueue[decal].push_back(BeamDrawContext(cumulative_transformation_matrix, vlist, this));
}

void Beam::ProcessDrawQueue()
{
    GFXDisable(LIGHTING);
    GFXDisable(CULLFACE); // don't want lighting on this baby
    GFXDisable(DEPTHWRITE);
    GFXPushBlendMode();
    static bool blendbeams = XMLSupport::parse_bool(vs_config->getVariable("graphics", "BlendGuns", "true"));
    GFXBlendMode(ONE, blendbeams ? ONE : ZERO);

    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    BeamDrawContext c;
    for (uint32_t decal = 0; decal < beamdrawqueue.size(); decal++)
    {
        Texture *tex = beamdecals.GetTexture(decal);
        if (tex)
        {
            tex->MakeActive(0);
            GFXTextureEnv(0, GFXMODULATETEXTURE);
            GFXToggleTexture(true, 0);
            if (beamdrawqueue[decal].size())
            {
                while (beamdrawqueue[decal].size())
                {
                    c = beamdrawqueue[decal].back();
                    beamdrawqueue[decal].pop_back();

                    c.beam->RecalculateVertices(c.m);
                    GFXLoadMatrixModel(c.m);
                    c.vlist->DrawOnce();
                }
            }
        }
    }
    GFXEnable(DEPTHWRITE);
    GFXEnable(CULLFACE);
    GFXDisable(LIGHTING);
    GFXPopBlendMode();
}

static bool beamCheckCollision(QVector pos, float len, const Collidable &un)
{
    return (un.GetPosition() - pos).MagnitudeSquared() <= len * len + 2 * len * un.radius + un.radius * un.radius;
}

void Beam::CollideHuge(const LineCollide &lc, Unit *targetToCollideWith, Unit *firer, Unit *superunit)
{
    QVector x0 = center;
    QVector v = direction * curlength;
    if (is_null(superunit->location[Unit::UNIT_ONLY]) && curlength)
    {
        if (targetToCollideWith)
        {
            this->Collide(targetToCollideWith, firer, superunit);
        }
    }
    else if (curlength)
    {
        CollideMap *cm = _Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY];

        CollideMap::iterator superloc = superunit->location[Unit::UNIT_ONLY];
        CollideMap::iterator tmore = superloc;
        if (!cm->Iterable(superloc))
        {
            CollideArray::CollidableBackref *br = static_cast<CollideArray::CollidableBackref *>(superloc);
            CollideMap::iterator tmploc = cm->begin() + br->toflattenhints_offset;
            if (tmploc == cm->end())
            {
                tmploc--;
            }
            tmore = superloc = tmploc; // don't decrease tless
        }
        else
        {
            ++tmore;
        }
        double r0 = x0.i;
        double r1 = x0.i + v.i;
        double minlook = r0 < r1 ? r0 : r1;
        double maxlook = r0 < r1 ? r1 : r0;
        bool targcheck = false;
        maxlook += (maxlook - (*superunit->location[Unit::UNIT_ONLY])->getKey()) + 2 * curlength; // double damage, yo
        minlook += (minlook - (*superunit->location[Unit::UNIT_ONLY])->getKey()) - 2 * curlength * curlength;
        //(a+2*b)^2-(a+b)^2 = 3b^2+2ab = 2b^2+(a+b)^2-a^2
        if (superloc != cm->begin() && minlook < (*superunit->location[Unit::UNIT_ONLY])->getKey())
        {
            // less traversal
            CollideMap::iterator tless = superloc;
            --tless;
            while ((*tless)->getKey() >= minlook)
            {
                CollideMap::iterator curcheck = tless;
                bool breakit = false;
                if (tless != cm->begin())
                {
                    --tless;
                }
                else
                {
                    breakit = true;
                }
                if ((*curcheck)->radius > 0)
                {
                    if (beamCheckCollision(center, curlength, (**curcheck)))
                    {
                        Unit *tmp = (**curcheck).ref.unit;
                        this->Collide(tmp, firer, superunit);
                        targcheck = (targcheck || tmp == targetToCollideWith);
                    }
                }
                if (breakit)
                {
                    break;
                }
            }
        }
        if (maxlook > (*superunit->location[Unit::UNIT_ONLY])->getKey())
        {
            // greater traversal
            while (tmore != cm->end() && (*tmore)->getKey() <= maxlook)
            {
                if ((*tmore)->radius > 0)
                {
                    Unit *un = (*tmore)->ref.unit;
                    if (beamCheckCollision(center, curlength, **tmore++))
                    {
                        this->Collide(un, firer, superunit);
                        targcheck = (targcheck || un == targetToCollideWith);
                    }
                }
                else
                {
                    ++tmore;
                }
            }
        }
        if (targetToCollideWith && !targcheck)
        {
            this->Collide(targetToCollideWith, firer, superunit);
        }
    }
}
