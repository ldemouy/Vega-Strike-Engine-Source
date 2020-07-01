#include <Python.h>
#include <string>

#include "cmd/planet.h"
#include "gfx/animation.h"
#include "gfx/aux_texture.h"
#include "gfx/cockpit.h"

bool PlanetHasLights(Unit *un)
{
    return ((GamePlanet *)un)->hasLights();
}
