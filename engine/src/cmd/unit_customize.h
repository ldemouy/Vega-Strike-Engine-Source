#ifndef __UNIT_CUSTOMIZE_CPP__
#define __UNIT_CUSTOMIZE_CPP__

#include "aldrv/audiolib.h"
#include "config_xml.h"
#include "gfx/cockpit.h"
#include "images.h"
#include "savegame.h"
#include "unit.h"
#include "unit_const_cache.h"
#include "unit_factory.h"
#include "universe.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "xml_serializer.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
extern int GetModeFromName(const char *);

extern Unit *CreateGameTurret(std::string tur, int faction);

template <class UnitType>
bool GameUnit<UnitType>::UpgradeSubUnits(const Unit *up, int subunitoffset, bool touchme, bool downgrade, int &numave,
                                         double &percentage)
{
    bool bl =
        this->UpgradeSubUnitsWithFactory(up, subunitoffset, touchme, downgrade, numave, percentage, &CreateGameTurret);
    if (!up->SubUnits.empty())
        this->DisableTurretAI();
    return bl;
}

extern char *GetUnitDir(const char *);

template <class UnitType>
double GameUnit<UnitType>::Upgrade(const std::string &file, int mountoffset, int subunitoffset, bool force,
                                   bool loop_through_mounts)
{
    return Unit::Upgrade(file, mountoffset, subunitoffset, force, loop_through_mounts);
}

#endif
