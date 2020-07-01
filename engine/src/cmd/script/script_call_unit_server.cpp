#include "cmd/planet_generic.h"
#include "gfx/cockpit_generic.h"
#include <string>

void AddAnimation(Cockpit *cp, std::string anim)
{
}

bool PlanetHasLights(Unit *un)
{
    return ((Planet *)un)->hasLights();
}
