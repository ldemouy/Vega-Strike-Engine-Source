#include "cmd/unit_generic.h"
#include "faction_generic.h"
using namespace FactionUtil;
Faction::~Faction()
{
    delete[] factionname;
    if (contraband.get())
    {
        contraband->Kill();
    }
}

std::vector<Animation *> *FactionUtil::GetRandCommAnimation(int32_t faction, Unit *, uint8_t &sex)
{
    return NULL;
}

std::vector<Animation *> *FactionUtil::GetAnimation(int32_t faction, int32_t n, uint8_t &sex)
{
    return NULL;
}

Animation *FactionUtil::createAnimation(const char *anim)
{
    return NULL;
}
Texture *FactionUtil::createTexture(const char *tex, const char *tmp, bool force)
{
    return NULL;
}
Texture *FactionUtil::createTexture(const char *tex, bool force)
{
    return NULL;
}
void FactionUtil::LoadFactionPlaylists() {}

Texture *FactionUtil::getForceLogo(int32_t faction)
{
    return NULL;
}
Texture *FactionUtil::getSquadLogo(int32_t faction)
{
    return NULL;
}
int32_t FactionUtil::GetNumAnimation(int32_t faction)
{
    return 0;
}
