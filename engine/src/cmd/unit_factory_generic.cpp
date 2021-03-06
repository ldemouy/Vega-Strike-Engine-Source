#include "asteroid_generic.h"
#include "missile_generic.h"
#include "nebula_generic.h"
#include "planet_generic.h"
#include "unit_factory.h"
#include "unit_generic.h"
#include "universe_util.h"

Unit *UnitFactory::_masterPartList = nullptr;
Unit *UnitFactory::getMasterPartList()
{
    if (_masterPartList == nullptr)
    {
        static bool making = true;
        if (making)
        {
            making = false;
            _masterPartList = Unit::makeMasterPartList();
            making = true;
        }
    }
    return _masterPartList;
}
