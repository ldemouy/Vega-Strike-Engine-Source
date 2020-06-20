#include "python/python_compile.h"
#include "docking.h"
#include "xml_support.h"
#include "config_xml.h"
#include "cmd/unit_generic.h"
#include "warpto.h"
#include "universe_util.h"
#include <string>
static void DockedScript(Unit *docker, Unit *base)
{
    static string script = vs_config->getVariable("AI", "DockedToScript", "");
    if (script.length() > 0)
    {
        Unit *targ = docker->Target();
        docker->GetComputerData().target.SetUnit(base);
        UniverseUtil::setScratchUnit(docker);
        CompileRunPython(script);
        UniverseUtil::setScratchUnit(nullptr);
        docker->GetComputerData().target.SetUnit(targ); //should be nullptr;
    }
}
namespace Orders
{
    DockingOps::DockingOps(Unit *unitToDockWith, Order *ai, bool physically_dock, bool keeptrying) : MoveTo(QVector(0, 0, 1),
                                                                                                            false,
                                                                                                            10, false),
                                                                                                     docking(unitToDockWith), state(GETCLEARENCE), oldstate(ai)
    {
        formerOwnerDoNotDereference = nullptr;
        this->keeptrying = keeptrying;
        facedtarget = false;
        physicallyDock = true;
        port = -1;
        static float temptimer = XMLSupport::parse_float(vs_config->getVariable("physics", "docking_time", "10"));
        timer = temptimer;
    }
    void DockingOps::SetParent(Unit *parent)
    {
        MoveTo::SetParent(parent);
        if (parent)
        {
            formerOwnerDoNotDereference = parent->owner;
            parent->SetOwner(docking.GetUnit());
        }
    }
    void DockingOps::Execute()
    {
        Unit *unit_to_dock_with = docking.GetUnit();
        if (parent == unit_to_dock_with || unit_to_dock_with == nullptr)
        {
            RestoreOldAI();
            Destroy();
            return;
        }
        switch (state)
        {
        case GETCLEARENCE:
            if (!RequestClearence(unit_to_dock_with))
            {
                if (!keeptrying)
                {
                    RestoreOldAI();
                    Destroy();
                    return;
                }
            }
            else
            {
                state = DOCKING;
                //no break
            }
        case DOCKING:
            if (DockToTarget(unit_to_dock_with))
            {
                state = DOCKED;
            }
            break;
        case DOCKED:
            if (PerformDockingOperations(unit_to_dock_with))
            {
                state = UNDOCKING;
            }
            break;
        case UNDOCKING:
            if (Undock(unit_to_dock_with))
            {
                RestoreOldAI();
                Destroy();
                return;
            }
            break;
        }
        parent->SetAngularVelocity(Vector(0, 0, 0)); //FIXME if you want it to turn to dock point
        done = false;
    }
    void DockingOps::Destroy()
    {
        if (parent)
        {
            if (oldstate)
            {
                oldstate->Destroy();
            }
            oldstate = nullptr;
            if (formerOwnerDoNotDereference)
            {
                parent->SetOwner((Unit *)formerOwnerDoNotDereference); //set owner will not deref
                formerOwnerDoNotDereference = nullptr;
            }
        }
        docking.SetUnit(nullptr);
    }
    void DockingOps::RestoreOldAI()
    {
        if (parent)
        {
            parent->aistate = oldstate; //that's me!
            if (formerOwnerDoNotDereference)
            {
                parent->SetOwner((Unit *)formerOwnerDoNotDereference);
                formerOwnerDoNotDereference = nullptr;
            }
            oldstate = nullptr;
        }
    }
    int SelectDockPort(Unit *unit_to_dock_with, Unit *parent)
    {
        const vector<DockingPorts> &dp = unit_to_dock_with->DockingPortLocations();
        float dist = FLT_MAX;
        int32_t num = -1;
        for (size_t i = 0; i < dp.size(); ++i)
            if (!dp[i].IsOccupied())
            {
                Vector rez = Transform(unit_to_dock_with->GetTransformation(), dp[i].GetPosition());
                float wdist = (rez - parent->Position()).MagnitudeSquared();
                if (wdist < dist)
                {
                    num = i;
                    dist = wdist;
                }
            }
        return num;
    }
    bool DockingOps::RequestClearence(Unit *unit_to_dock_with)
    {
        if (physicallyDock && !unit_to_dock_with->RequestClearance(parent))
        {
            return false;
        }
        port = SelectDockPort(unit_to_dock_with, parent);
        if (port == -1)
        {
            return false;
        }
        return true;
    }
    QVector DockingOps::Movement(Unit *unit_to_dock_with)
    {
        const QVector loc(Transform(unit_to_dock_with->GetTransformation(), unit_to_dock_with->DockingPortLocations()[port].GetPosition().Cast()));
        SetDest(loc);

        SetAfterburn(DistanceWarrantsTravelTo(parent, (loc - parent->Position()).Magnitude(), true));
        if (!facedtarget)
        {
            facedtarget = true;
            EnqueueOrder(new ChangeHeading(loc, 4, 1, true));
        }
        MoveTo::Execute();
        if (rand() % 256 == 0)
        {
            WarpToP(parent, unit_to_dock_with, true);
        }
        return loc;
    }
    bool DockingOps::DockToTarget(Unit *unit_to_dock_with)
    {
        if (unit_to_dock_with->DockingPortLocations()[port].IsOccupied())
        {
            if (keeptrying)
            {
                state = GETCLEARENCE;
                return false;
            }
            else
            {
                docking.SetUnit(nullptr);
                state = GETCLEARENCE;
                return false;
            }
        }
        QVector loc = Movement(unit_to_dock_with);
        float rad = unit_to_dock_with->DockingPortLocations()[port].GetRadius() + parent->rSize();
        float diss = (parent->Position() - loc).MagnitudeSquared() - .1;
        bool isplanet = unit_to_dock_with->isUnit() == PLANETPTR;
        static float MinimumCapacityToRefuelOnLand =
            XMLSupport::parse_float(vs_config->getVariable("physics", "MinimumWarpCapToRefuelDockeesAutomatically", "0"));
        if (diss <= (isplanet ? rad * rad : parent->rSize() * parent->rSize()))
        {
            DockedScript(parent, unit_to_dock_with);
            if (physicallyDock)
            {
                return parent->Dock(unit_to_dock_with);
            }
            else
            {
                float maxWillingToRefill = unit_to_dock_with->WarpCapData();
                if (maxWillingToRefill >= MinimumCapacityToRefuelOnLand)
                {
                    parent->RefillWarpEnergy(); //BUCO! This needs its own units.csv column to see how much we refill!
                }
                return true;
            }
        }
        else if (diss <= 1.2 * rad * rad)
        {
            timer += SIMULATION_ATOM;
            static float tmp = XMLSupport::parse_float(vs_config->getVariable("physics", "docking_time", "10"));
            if (timer >= 1.5 * tmp)
            {
                if (physicallyDock)
                {
                    return parent->Dock(unit_to_dock_with);
                }
                else
                {
                    float maxWillingToRefill = unit_to_dock_with->WarpCapData();
                    if (maxWillingToRefill >= MinimumCapacityToRefuelOnLand)
                    {
                        parent->RefillWarpEnergy(); //BUCO! This needs its own units.csv column to see how much we refill!
                    }
                    return true;
                }
            }
        }
        return false;
    }
    bool DockingOps::PerformDockingOperations(Unit *unit_to_dock_with)
    {
        timer -= SIMULATION_ATOM;
        bool isplanet = unit_to_dock_with->isUnit() == PLANETPTR;
        if (timer < 0)
        {
            static float tmp = XMLSupport::parse_float(vs_config->getVariable("physics", "un_docking_time", "180"));
            timer = tmp;
            EnqueueOrder(new ChangeHeading(parent->Position() * 2 - unit_to_dock_with->Position(), 4, 1, true));
            if (physicallyDock)
            {
                return parent->UnDock(unit_to_dock_with);
            }
            else
            {
                return true;
            }
        }
        else if (!physicallyDock)
        {
            if (isplanet)
            {
                //orbit;
                QVector cur = unit_to_dock_with->Position() - parent->Position();
                QVector up = QVector(0, 1, 0);
                if (up.i == cur.i && up.j == cur.j && up.k == cur.k)
                {
                    up = QVector(0, 0, 1);
                }
                SetDest(cur.Cross(up) * 10000);
                MoveTo::Execute();
            }
            else
            {
                Movement(unit_to_dock_with);
            }
        }
        return false;
    }
    bool DockingOps::Undock(Unit *unit_to_dock_with)
    {
        //this is a good heuristic... find the location where you are.compare with center...then fly the fuck away
        QVector awaydir = parent->Position() - unit_to_dock_with->Position();
        float len = ((unit_to_dock_with->rSize() + parent->rSize() * 2) / awaydir.Magnitude());
        awaydir *= len;
        SetDest(awaydir + unit_to_dock_with->Position());
        MoveTo::Execute();
        timer -= SIMULATION_ATOM;
        return (len < 1) || done || timer < 0;
    }
    DockingOps *DONOTUSEAI = nullptr;
} // namespace Orders
