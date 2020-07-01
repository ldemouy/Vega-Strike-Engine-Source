/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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

/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can
// compile without it
#include <unistd.h>
#endif
#include "cmd/unit_factory.h"
#include "cmd/unit_generic.h"
#include "xml_support.h"
#include <expat.h>

#include "cmd/ai/aggressive.h"
#include "cmd/ai/order.h"
#include "cmd/asteroid_generic.h"
#include "cmd/collection.h"
#include "cmd/images.h"
#include "cmd/nebula_generic.h"
#include "cmd/pilot.h"
#include "cmd/planet_generic.h"
#include "cmd/unit_factory.h"
#include "cmd/unit_util.h"
#include "configxml.h"
#include "easydom.h"
#include "flightgroup.h"
#include "gfx/cockpit_generic.h"
#include "gldrv/gfxlib.h"
#include "hashtable.h"
#include "mission.h"
#include "msgcenter.h"
#include "savegame.h"
#include "vegastrike.h"
#include "vs_globals.h"

extern Unit &GetUnitMasterPartList();
extern bool PlanetHasLights(Unit *un);

#if 0
NEVER NEVER NEVER use Unit*to save a unit across frames
extern Unit *player_unit;
BAD BAD BAD

Better:
extern UnitContainer player_unit;

Best:
_Universe->AccessCockpit()->GetParent();
#endif

static Unit *getIthUnit(UnitCollection::UnitIterator uiter, int i);

extern BLENDFUNC parse_alpha(const char *);

static Unit *getIthUnit(UnitCollection::UnitIterator uiter, int unit_nr)
{
    Unit *unit = nullptr;
    for (int i = 0; (unit = *uiter); ++uiter, ++i)
    {
        if (i == unit_nr)
        {
            return unit;
        }
    }
    return nullptr;
}
