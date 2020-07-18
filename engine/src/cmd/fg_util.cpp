#include "fg_util.h"
#include "faction_generic.h"
#include "savegame.h"
#include <algorithm>

namespace fg_util
{
/// FIXME: Investigate Removing
void itoa(uint32_t dat, char *output)
{
    if (dat == 0)
    {
        *output++ = '0';
        *output = '\0';
    }
    else
    {
        char *s = output;
        while (dat)
        {
            *s++ = '0' + dat % 10;
            dat /= 10;
        }
        *s = '\0';
        std::reverse(output, s);
    }
}

bool IsFGKey(const std::string &fgcandidate)
{
    if (fgcandidate.length() > 3 && fgcandidate[0] == 'F' && fgcandidate[1] == 'G' && fgcandidate[2] == ':')
    {
        return true;
    }
    return false;
}

std::string MakeFGKey(const std::string &fgname, int32_t faction)
{
    /// FIXME: figure out what this means and give it a better name.
    const std::string gFG = "FG:";
    char tmp[16];
    tmp[0] = '|';
    itoa(faction, tmp + 1);
    return gFG + fgname + tmp;
}

uint32_t ShipListOffset()
{
    return 3;
}

uint32_t PerShipDataSize()
{
    return 3;
}

bool CheckFG(std::vector<std::string> &data)
{
    bool retval = false;
    uint32_t leg = data.size();
    uint32_t inc = PerShipDataSize();
    for (uint32_t i = ShipListOffset() + 1; i + 1 < leg; i += inc)
    {
        std::string *numlanded = &data[i + 1];
        std::string *numtotal = &data[i];
        if (*numlanded != *numtotal)
        {
            retval = true;
            *numlanded = *numtotal;
        }
    }
    return retval;
}

} // namespace fg_util
