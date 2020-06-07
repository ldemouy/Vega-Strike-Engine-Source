#include "faction_generic.h"
#include "cmd/unit_generic.h"

using namespace FactionUtil;

vector<std::shared_ptr<Faction>> factions; //the factions

// TODO: look at switching over to Range Based for loops

void Faction::ParseAllAllies()
{
    for (size_t i = 0; i < factions.size(); i++)
    {
        factions[i]->ParseAllies(i);
    }
    for (size_t i = 0; i < factions.size(); i++)
    {
        factions[i]->faction[i].relationship = 1;
    }
}
void Faction::ParseAllies(uint32_t thisfaction)
{
    vector<faction_stuff> tempvec;
    for (size_t i = 0; i < faction.size(); i++)
        for (size_t j = 0; j < factions.size(); j++)
            if (strcmp(faction[i].stats.name, factions[j]->factionname) == 0)
            {
                delete[] faction[i].stats.name;
                faction[i].stats.index = j;
                break;
            }
    for (size_t i = 0; i < factions.size(); i++)
    {
        tempvec.push_back(faction_stuff());
        tempvec[i].stats.index = i;
        tempvec[i].relationship = ((i == thisfaction) ? 1 : 0);
    }
    for (size_t i = 0; i < faction.size(); i++)
    {
        faction_stuff::faction_name tmp = tempvec[faction[i].stats.index].stats;
        tempvec[faction[i].stats.index] = faction[i];
        tempvec[faction[i].stats.index].stats = tmp;
    }
    faction.swap(tempvec);
}
