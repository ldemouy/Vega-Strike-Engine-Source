#ifndef _GFX_CLICK_LIST_H_
#define _GFX_CLICK_LIST_H_

//#include "vegastrike.h"
#include "gfx/matrix.h"
#include "collection.h"
#include "star_system.h"
Vector MouseCoordinate(int32_t x, int32_t y); //FIXME

class ClickList
{
private:
    UnitCollection *parentIter;
    StarSystem *parentSystem;
    UnitCollection *lastCollection;
    Unit *lastSelected;

public:
    //gets passed in unnormalized mouse values btw 0 and g_game.x_resolution&& g_game.y_resolution
    bool queryShip(int32_t mouseX, int32_t mouseY, Unit *); //returns if the ship's in iterator utilizes
    ClickList(StarSystem *parSystem, UnitCollection *parentIter);
    ~ClickList() {}
    UnitCollection *requestIterator(int32_t mouseX, int32_t mouseY);
    UnitCollection *requestIterator(int32_t minX, int32_t minY, int32_t maxX, int32_t maxY);
    Unit *requestShip(int32_t mouseX, int32_t mouseY);
};
#endif
