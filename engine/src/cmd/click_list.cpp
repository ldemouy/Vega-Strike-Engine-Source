#include "click_list.h"
#include "gfx/camera.h"
#include "gldrv/gfxlib.h"
#include "unit_generic.h"
#include "vegastrike.h"
#include "vs_globals.h"
extern Vector mouseline;
extern vector<Vector> perplines;

Vector MouseCoordinate(int32_t mouseX, int32_t mouseY)
{
    return GFXDeviceToEye(mouseX, mouseY);
}

bool ClickList::queryShip(int32_t mouseX, int32_t mouseY, Unit *ship)
{
    Vector mousePoint = MouseCoordinate(mouseX, mouseY);
    Vector CamP, CamQ, CamR;
    QVector CamPos;
    _Universe->AccessCamera()->GetPQR(CamP, CamQ, CamR);
    mousePoint = Transform(CamP, CamQ, CamR, mousePoint);
    _Universe->AccessCamera()->GetPosition(CamPos);

    mousePoint.Normalize();
    mouseline = mousePoint + CamPos.Cast();

    return false;
}

ClickList::ClickList(StarSystem *parSystem, UnitCollection *parIter)
{
    lastSelected = nullptr;
    lastCollection = nullptr;
    parentSystem = parSystem;
    parentIter = parIter;
}

UnitCollection *ClickList::requestIterator(int32_t minX, int32_t minY, int32_t maxX, int32_t maxY)
{
    UnitCollection *uc = new UnitCollection(); /// arrgh dumb last collection thing to cycel through ships
    if (minX == maxX || minY == maxY)
    {
        return uc; // nothing in it
    }

    Matrix view;
    float frustmat[16];
    float l, r, b, t, n, f;
    float drivel[16];
    GFXGetFrustumVars(true, &l, &r, &b, &t, &n, &f);
    GFXFrustum(frustmat, drivel, l * (-2. * minX / g_game.x_resolution + 1) /*  *g_game.MouseSensitivityX*/,
               r * (2. * maxX / g_game.x_resolution - 1) /*  *g_game.MouseSensitivityX*/,
               t * (-2. * minY / g_game.y_resolution + 1) /*  *g_game.MouseSensitivityY*/,
               b * (2. * maxY / g_game.y_resolution - 1) /*  *g_game.MouseSensitivityY*/, n, f);
    _Universe->AccessCamera()->GetView(view);
    double frustum[6][4];
    GFXCalculateFrustum(frustum, view, frustmat);
    Unit *un;
    for (auto myParent = parentIter->createIterator(); (un = *myParent) != nullptr; ++myParent)
    {
        if ((un)->queryFrustum(frustum))
        {
            uc->prepend(un);
        }
    }
    return uc;
}

UnitCollection *ClickList::requestIterator(int32_t mouseX, int32_t mouseY)
{
    perplines = vector<Vector>();
    UnitCollection *uc = new UnitCollection();
    Unit *un;
    for (auto myParent = parentIter->createIterator(), UAye = uc->createIterator(); (un = *myParent) != nullptr;
         ++myParent)
    {
        if (queryShip(mouseX, mouseY, un))
        {
            UAye.preinsert(un);
        }
    }
    return uc;
}

Unit *ClickList::requestShip(int32_t mouseX, int32_t mouseY)
{
    bool equalCheck = false;
    UnitCollection *uc = requestIterator(mouseX, mouseY);
    if (lastCollection != nullptr)
    {
        equalCheck = true;
        Unit *lastun;
        Unit *un;
        for (auto lastiter = lastCollection->createIterator(), UAye = uc->createIterator();
             (lastun = *lastiter) && (un = *UAye) && equalCheck; ++lastiter, ++UAye)
        {
            if (un != lastun)
            {
                equalCheck = false;
            }
        }
        delete lastCollection;
    }
    float minDistance = 1e+10;
    float tmpdis;
    Unit *targetUnit = nullptr;
    if (equalCheck && lastSelected)
    {
        // the person clicked the same place and wishes to cycle through units from front to back
        float morethan =
            lastSelected->getMinDis(_Universe->AccessCamera()->GetPosition()); // parent system for access cam
        Unit *un;
        for (auto UAye = uc->createIterator(); (un = *UAye) != nullptr; ++UAye)
        {
            tmpdis = un->getMinDis(_Universe->AccessCamera()->GetPosition()); // parent_system? FIXME (for access cam
            if (tmpdis > morethan && tmpdis < minDistance)
            {
                minDistance = tmpdis;
                targetUnit = un;
            }
        }
    }
    if (targetUnit == nullptr)
    {
        // ok the click location is either different, or
        // he clicked on the back of the list and wishes to start over
        Unit *un;
        for (auto UAye = uc->createIterator(); (un = *UAye) != nullptr; ++UAye)
        {
            tmpdis = un->getMinDis(_Universe->AccessCamera()->GetPosition()); // parent_system FIXME
            if (tmpdis < minDistance)
            {
                minDistance = tmpdis;
                targetUnit = un;
            }
        }
    }
    lastCollection = uc;
    lastSelected = targetUnit;
    return targetUnit;
}
