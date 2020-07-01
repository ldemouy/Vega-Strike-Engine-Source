#include "ikarus.h"
#include "aggressive.h"
#include "cmd/script/flightgroup.h"
#include "cmd/script/mission.h"
#include "cmd/unit_generic.h"
#include "communication.h"
#include "config_xml.h"
#include "event_xml.h"
#include "flybywire.h"
#include "gfx/cockpit_generic.h"
#include "hard_coded_scripts.h"
#include "script.h"
#include "universe_util.h"
#include "vs_globals.h"
#include "xml_support.h"
#include <list>
#include <vector>
using namespace Orders;
Ikarus::Ikarus() : AggressiveAI("default.agg.xml")
{
    last_time = cur_time = 0;
}
void Ikarus::ExecuteStrategy(Unit *target)
{
    WillFire(target);
    if (queryType(Order::FACING) == nullptr)
    {
        // we have nothing to do now
        FaceTarget(false);
        AddReplaceLastOrder(true); // true stands for override any that are there
    }
    if (queryType(Order::MOVEMENT) == nullptr)
    {
        MatchLinearVelocity(true, Vector(0, 0, 10000), false,
                            true); // all ahead full! (not with afterburners but in local coords)
        AddReplaceLastOrder(true);
    }

    cur_time += SIMULATION_ATOM;
    if (cur_time - last_time > 5)
    {
        // dosomething

        last_time = cur_time;
    }
}

void Ikarus::WillFire(Unit *target)
{
    bool missilelockp = false;
    if (ShouldFire(target, missilelockp)) // this is a function from fire.cpp  you probably want to write a better one
    {
        parent->Fire(false);
    }
    if (missilelockp)
    {
        parent->Fire(true); // if missiles locked fire
        parent->ToggleWeapon(true);
    }
}
/// you should certainly edit this!!
void Ikarus::DecideTarget()
{
    Unit *targ = parent->Target();
    if (!targ)
    {
        Unit *un = nullptr;
        for (UniverseUtil::PythonUnitIter i = UniverseUtil::getUnitList(); (un = *i); ++i)
        {
            if (parent->getRelation(un) < 0)
            {
                parent->Target(un);
                break;
            }
        }
    }
}

/// you can ignore the function below unless it causes problems...this merely makes it so that the AI responds to your
/// commands
void Ikarus::Execute()
{
    Flightgroup *fg = parent->getFlightgroup();
    ReCommandWing(fg);
    CommunicatingAI::Execute();
    DecideTarget();
    if (!ProcessCurrentFgDirective(fg))
    {
        Unit *target = parent->Target();
        bool isjumpable = target ? ((!target->GetDestinations().empty()) && parent->GetJumpStatus().drive >= 0) : false;
        if (isjumpable)
        {
            AfterburnTurnTowards(this, parent);
        }
        else
        {
            ExecuteStrategy(target);
        }
    }
}
