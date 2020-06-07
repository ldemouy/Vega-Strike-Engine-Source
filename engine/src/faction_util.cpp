#include <assert.h>

#include "vs_globals.h"
#include "cmd/unit_generic.h"
#include "faction_generic.h"
#include "gfx/aux_texture.h"
#include "cmd/unit_util.h"
#include "cmd/unit_generic.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "cmd/music.h"

// DO NOT PUT INCLUDES AFTER using namespace

using namespace FactionUtil;

Faction::~Faction()
{
    delete[] factionname;
    if (contraband.get())
    {
        contraband->Kill();
    }
    delete logo;
}

Texture *FactionUtil::getForceLogo(int faction)
{
    std::shared_ptr<Faction> fac = factions[faction];
    if (fac->logo == 0)
    {
        if (!fac->logoName.empty())
        {
            if (!fac->logoAlphaName.empty())
                fac->logo = FactionUtil::createTexture(fac->logoName.c_str(), fac->logoAlphaName.c_str(), true);

            else
                fac->logo = FactionUtil::createTexture(fac->logoName.c_str(), true);
        }
        else
        {
            fac->logo = FactionUtil::createTexture("white.png", true);
        }
    }
    return factions[faction]->logo;
}
//fixme--add squads in here
Texture *FactionUtil::getSquadLogo(int faction)
{
    std::shared_ptr<Faction> fac = factions[faction];
    if (fac->secondaryLogo == 0)
    {
        if (!fac->secondaryLogoName.empty())
        {
            if (!fac->secondaryLogoAlphaName.empty())
                fac->secondaryLogo = FactionUtil::createTexture(
                    fac->secondaryLogoName.c_str(), fac->secondaryLogoAlphaName.c_str(), true);

            else
                fac->secondaryLogo = FactionUtil::createTexture(fac->secondaryLogoName.c_str(), true);
        }
        else
        {
            return getForceLogo(faction);
        }
    }
    return factions[faction]->secondaryLogo;
}

int32_t FactionUtil::GetNumAnimation(int faction)
{
    return factions[faction]->comm_faces.size();
}

//COMES FROM FACTION_XML.CPP

std::vector<Animation *> *FactionUtil::GetAnimation(int32_t faction, int32_t n, uint8_t &sex)
{
    sex = factions[faction]->comm_face_sex[n];
    return &factions[faction]->comm_faces[n].animations;
}

std::vector<Animation *> *FactionUtil::GetRandCommAnimation(int32_t faction, Unit *un, uint8_t &sex)
{
    bool dockable = UnitUtil::isDockableUnit(un);
    bool base = UnitUtil::getFlightgroupName(un) == "Base";
    int32_t size = factions[faction]->comm_faces.size();
    if (size > 0)
    {
        for (size_t i = 0; i < 8 + size; ++i)
        {
            int32_t ind = i < 8 ? rand() % size : i - 8;
            Faction::comm_face_t *tmp = &factions[faction]->comm_faces[ind];
            if (tmp->dockable == Faction::comm_face_t::CEITHER || (tmp->dockable == Faction::comm_face_t::CYES && dockable) || (tmp->dockable == Faction::comm_face_t::CNO && !dockable))
            {
                if (tmp->base == Faction::comm_face_t::CEITHER || (tmp->base == Faction::comm_face_t::CYES && base) || (tmp->base == Faction::comm_face_t::CNO && !base))
                {
                    return GetAnimation(faction, ind, sex);
                }
            }
            if (tmp->base == Faction::comm_face_t::CYES && base)
            {
                return GetAnimation(faction, ind, sex); //bases may be dockable but we have set dockable_only to no
            }
        }
        fprintf(stderr,
                "Error picking comm animation for %d faction with bas:%d dock:%d\n",
                faction,
                (int32_t)base,
                (int32_t)dockable);
        return GetAnimation(faction, rand() % size, sex);
    }
    else
    {
        sex = 0;
        return NULL;
    }
}

Animation *FactionUtil::GetRandExplosionAnimation(int32_t whichfaction, std::string &which)
{
    if (whichfaction < (int32_t)factions.size())
    {
        if (factions[whichfaction]->explosion_name.size())
        {
            int32_t whichexp = rand() % factions[whichfaction]->explosion_name.size();
            which = factions[whichfaction]->explosion_name[whichexp];
            return factions[whichfaction]->explosion[whichexp].get();
        }
    }
    return NULL;
}

void FactionUtil::LoadFactionPlaylists()
{
    for (size_t i = 0; i < factions.size(); i++)
    {
        string fac = FactionUtil::GetFaction(i);
        fac += ".m3u";
        factions[i]->playlist = muzak->Addlist(fac.c_str());
    }
}

Animation *FactionUtil::createAnimation(const char *anim)
{
    return new Animation(anim);
}

Texture *FactionUtil::createTexture(const char *tex, bool force)
{
    if (force)
    {
        return new Texture(tex, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE);
    }
    else
    {
        return new Texture(tex, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXFALSE);
    }
}

Texture *FactionUtil::createTexture(const char *tex, const char *tmp, bool force)
{
    if (force)
    {
        return new Texture(tex, tmp, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1, 0, GFXTRUE);
    }
    else
    {
        return new Texture(tex, tmp, 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1, 0, GFXFALSE);
    }
}
