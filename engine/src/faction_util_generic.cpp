#include <assert.h>
#include "faction_generic.h"
#include "vsfilesystem.h"
#include "universe_generic.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "gfx/cockpit_generic.h"
#include "cmd/unit_generic.h"

#include "options.h"

using namespace FactionUtil;
int32_t FactionUtil::upgradefac = 0;
int32_t FactionUtil::planetfac = 0;
int32_t FactionUtil::neutralfac = 0;
FSM *FactionUtil::GetConversation(const int32_t &Myfaction, const int32_t &TheirFaction)
{
    assert(factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
    return factions[Myfaction]->faction[TheirFaction].conversation.get();
}

const char *FactionUtil::GetFaction(int32_t i)
{
    if (i >= 0 && i < (int)factions.size())
        return factions[i]->factionname;
    return "";
}

static int32_t GetFactionLookup(const char *factionname)
{
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
    for (size_t i = 0; i < factions.size(); i++)
        if (strcasecmp(factionname, factions[i]->factionname) == 0)
            return i;
    return 0;
}

Unit *FactionUtil::GetContraband(int32_t faction)
{
    return factions[faction]->contraband.get();
}
/**
 * Returns the relationship between myfaction and theirfaction
 * 1 is happy. 0 is neutral (btw 1 and 0 will not attack)
 * -1 is mad. <0 will attack
 */
int32_t FactionUtil::GetFactionIndex(const string &name)
{
    static Hashtable<string, int32_t, 47> factioncache;
    int32_t *tmp = factioncache.Get(name);
    if (tmp)
        return *tmp;
    int32_t i = GetFactionLookup(name.c_str());
    tmp = new int32_t;
    *tmp = i;
    factioncache.Put(name, tmp);
    return i;
}
bool FactionUtil::isCitizenInt(int32_t faction)
{
    return factions[faction]->citizen;
}
bool FactionUtil::isCitizen(const std::string &name)
{
    return isCitizenInt(GetFactionIndex(name));
}
string FactionUtil::GetFactionName(int32_t index)
{
    const char *tmp = GetFaction(index);
    if (tmp)
        return tmp;
    static std::string nullstr;
    return nullstr;
}

static bool isPlayerFaction(const int32_t &MyFaction)
{
    uint32_t numplayers = _Universe->numPlayers();
    for (size_t i = 0; i < numplayers; ++i)
    {
        Unit *un = _Universe->AccessCockpit(i)->GetParent();
        if (un)
            if (un->faction == MyFaction)
                return true;
    }
    return false;
}
void FactionUtil::AdjustIntRelation(const int &Myfaction, const int &TheirFaction, float factor, float rank)
{
    assert(factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
    if (strcmp(factions[Myfaction]->factionname, "neutral") != 0)
    {
        if (strcmp(factions[Myfaction]->factionname, "upgrades") != 0)
        {
            if (strcmp(factions[TheirFaction]->factionname, "neutral") != 0)
            {
                if (strcmp(factions[TheirFaction]->factionname, "upgrades") != 0)
                {
                    if (isPlayerFaction(TheirFaction) || game_options.AllowNonplayerFactionChange)
                    {
                        if (game_options.AllowCivilWar || Myfaction != TheirFaction)
                        {
                            factions[Myfaction]->faction[TheirFaction].relationship += factor * rank;
                            if (factions[Myfaction]->faction[TheirFaction].relationship > 1 && game_options.CappedFactionRating)
                                factions[Myfaction]->faction[TheirFaction].relationship = 1;
                            if (factions[Myfaction]->faction[TheirFaction].relationship < game_options.min_relationship)
                                factions[Myfaction]->faction[TheirFaction].relationship = game_options.min_relationship;
                            if (!game_options.AllowNonplayerFactionChange)
                                factions[TheirFaction]->faction[Myfaction].relationship =
                                    factions[Myfaction]->faction[TheirFaction].relationship; //reflect if player
                        }
                    }
                }
            }
        }
    }
}
int32_t FactionUtil::GetPlaylist(const int &myfaction)
{
    return factions[myfaction]->playlist; //can be -1
}
const float *FactionUtil::GetSparkColor(const int &myfaction)
{
    return factions[myfaction]->sparkcolor; //can be -1
}
uint32_t FactionUtil::GetNumFactions()
{
    return factions.size();
}
void FactionUtil::SerializeFaction(FILE *fp)
{
    for (size_t i = 0; i < factions.size(); i++)
    {
        for (size_t j = 0; j < factions[i]->faction.size(); j++)
        {
            VSFileSystem::vs_fprintf(fp, "%g ", factions[i]->faction[j].relationship);
        }
        VSFileSystem::vs_fprintf(fp, "\n");
    }
}
string FactionUtil::SerializeFaction()
{
    char temp[8192];
    string ret("");
    for (unsigned int i = 0; i < factions.size(); i++)
    {
        for (unsigned int j = 0; j < factions[i]->faction.size(); j++)
        {
            sprintf(temp, "%g ", factions[i]->faction[j].relationship);
            ret += string(temp);
        }
        sprintf(temp, "\n");
        ret += string(temp);
    }
    return ret;
}
int FactionUtil::numnums(const char *str)
{
    int count = 0;
    for (int i = 0; str[i]; i++)
        count += (str[i] >= '0' && str[i] <= '9') ? 1 : 0;
    return count;
}
void FactionUtil::LoadSerializedFaction(FILE *fp)
{
    for (size_t i = 0; i < factions.size(); i++)
    {
        char *tmp = new char[24 * factions[i]->faction.size()];
        fgets(tmp, 24 * factions[i]->faction.size() - 1, fp);
        char *tmp2 = tmp;
        if (numnums(tmp) == 0)
        {
            i--;
            continue;
        }
        for (size_t j = 0; j < factions[i]->faction.size(); j++)
        {
            if (1 != sscanf(tmp2, "%f ", &factions[i]->faction[j].relationship))
                printf("err");
            int32_t k = 0;
            bool founddig = false;
            while (tmp2[k])
            {
                if (isdigit(tmp2[k]))
                {
                    founddig = true;
                }
                if (founddig && (!isdigit(tmp2[k]) && tmp2[k] != '.'))
                {
                    break;
                }
                k++;
            }
            tmp2 += k;
            if (*tmp2 == '\r' || *tmp2 == '\n')
                break;
        }
        delete[] tmp;
    }
}
bool whitespaceNewline(char *inp)
{
    for (; *inp; inp++)
    {
        if (inp[0] == '\n' || inp[0] == '\r')
        {
            return true;
        }
        if (inp[0] != ' ' && inp[0] != '\t')
        {
            break;
        }
    }
    return false;
}
string savedFactions;
void FactionUtil::LoadSerializedFaction(char *&buf)
{
    if (buf == nullptr)
    {
        char *bleh = strdup(savedFactions.c_str());
        char *blah = bleh;
        LoadSerializedFaction(blah);
        free(bleh);
        return;
    }
    if (factions.size() == 0)
    {
        savedFactions = buf;
        return;
    }
    for (size_t i = 0; i < factions.size(); i++)
    {
        if (numnums(buf) == 0)
        {
            return;
        }
        for (unsigned int j = 0; j < factions[i]->faction.size(); j++)
        {
            sscanf(buf, "%f ", &factions[i]->faction[j].relationship);
            int32_t k = 0;
            bool founddig = false;
            while (buf[k])
            {
                if (isdigit(buf[k]))
                {
                    founddig = true;
                }
                if (founddig && (!isdigit(buf[k]) && buf[k] != '.'))
                {
                    break;
                }
                k++;
            }
            buf += k;
            if (whitespaceNewline(buf))
                break;
        }
    }
}
