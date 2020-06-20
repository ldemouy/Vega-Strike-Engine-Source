#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet_generic.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/unit_util.h"
#include "cmd/script/flightgroup.h"
#include "cmd/role_bitmask.h"
#include "cmd/ai/communication.h"
#include "universe_util.h"
#include <algorithm>
#include "cmd/unit_find.h"
#include "vs_random.h"
#include "lin_time.h" //DEBUG ONLY
#include "cmd/pilot.h"

extern int numprocessed;
extern double targetpick;

static bool NoDockWithClear()
{
    static bool nodockwithclear = XMLSupport::parse_bool(vs_config->getVariable("physics", "dock_with_clear_planets", "true"));
    return nodockwithclear;
}

VSRandom targrand(time(nullptr));

Unit *getAtmospheric(Unit *target)
{
    if (target)
    {
        Unit *unit;
        for (auto i = _Universe->activeStarSystem()->getUnitList().createIterator();
             (unit = *i) != nullptr;
             ++i)
        {
            if (unit->isUnit() == PLANETPTR)
            {
                if ((target->Position() - unit->Position()).Magnitude() < target->rSize() * .5)
                {
                    if (!(((Planet *)unit)->isAtmospheric()))
                    {
                        return unit;
                    }
                }
            }
        }
    }
    return nullptr;
}

bool RequestClearence(Unit *parent, Unit *targ, uint8_t sex)
{
    if (!targ->DockingPortLocations().size())
    {
        return false;
    }
    if (targ->isUnit() == PLANETPTR)
    {
        if (((Planet *)targ)->isAtmospheric() && NoDockWithClear())
        {
            targ = getAtmospheric(targ);
            if (!targ)
            {
                return false;
            }
            parent->Target(targ);
        }
    }
    CommunicationMessage message(parent, targ, nullptr, sex);
    message.SetCurrentState(message.fsm->GetRequestLandNode(), nullptr, sex);
    Order *order = targ->getAIState();
    if (order)
    {
        order->Communicate(message);
    }
    return true;
}

using Orders::FireAt;
bool FireAt::PursueTarget(Unit *unit, bool leader)
{
    if (leader)
    {
        return true;
    }
    if (unit == parent->Target())
    {
        return rand() < .9 * RAND_MAX;
    }
    if (parent->getRelation(unit) < 0)
    {
        return rand() < .2 * RAND_MAX;
    }
    return false;
}

bool CanFaceTarget(Unit *su, Unit *target, const Matrix &matrix)
{
    return true;

    float limitmin = su->Limits().limitmin;
    if (limitmin > -.99)
    {
        QVector position = (target->Position() - su->Position()).Normalize();
        QVector position_normal = position.Cast();
        Vector structurelimits = su->Limits().structurelimits;
        Vector worldlimit = TransformNormal(matrix, structurelimits);
        if (position_normal.Dot(worldlimit) < limitmin)
        {
            return false;
        }
    }
    Unit *ssu;
    for (auto i = su->getSubUnits(); (ssu = *i) != nullptr; ++i)
    {
        if (!CanFaceTarget(ssu, target, su->cumulative_transformation_matrix))
        {
            return false;
        }
    }
    return true;
}

void FireAt::ReInit(float aggressivitylevel)
{
    static float missileprob = XMLSupport::parse_float(vs_config->getVariable("AI", "Firing", "MissileProbability", ".01"));
    static float mintimetoswitch =
        XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MinTimeToSwitchTargets", "3"));
    lastmissiletime = UniverseUtil::GetGameTime() - 65536.;
    missileprobability = missileprob;
    delay = 0;
    agg = aggressivitylevel;
    distance = 1;
    //JS --- spreading target switch times
    lastchangedtarg = 0.0 - targrand.uniformInc(0, 1) * mintimetoswitch;
    had_target = false;
}

FireAt::FireAt(float aggressivitylevel) : CommunicatingAI(WEAPON, STARGET)
{
    ReInit(aggressivitylevel);
}

FireAt::FireAt() : CommunicatingAI(WEAPON, STARGET)
{
    static float aggr = XMLSupport::parse_float(vs_config->getVariable("AI", "Firing", "Aggressivity", "15"));
    ReInit(aggr);
}

void FireAt::SignalChosenTarget() {}
//temporary way of choosing
struct TargetAndRange
{
    Unit *target;
    float range;
    float relation;
    TargetAndRange(Unit *target, float range, float relation)
    {
        this->target = target;
        this->range = range;
        this->relation = relation;
    }
};

struct RangeSortedTurrets
{
    Unit *turret;
    float gunrange;
    RangeSortedTurrets(Unit *turret, float range)
    {
        this->turret = turret;
        gunrange = range;
    }
    bool operator<(const RangeSortedTurrets &o) const
    {
        return gunrange < o.gunrange;
    }
};

struct TurretBin
{
    float maxrange;
    vector<RangeSortedTurrets> turret;
    vector<TargetAndRange> listOfTargets[2]; //we have the over (and eq) 16 crowd and the under 16
    TurretBin()
    {
        maxrange = 0;
    }
    bool operator<(const TurretBin &o) const
    {
        return maxrange > o.maxrange;
    }
    void AssignTargets(const TargetAndRange &finalChoice, const Matrix &pos)
    {
        //go through modularly assigning as you go;
        int32_t count = 0;
        const size_t lotsize[2] = {listOfTargets[0].size(), listOfTargets[1].size()};
        for (auto turret_iter = turret.begin(); turret_iter != turret.end(); ++turret_iter)
        {
            bool foundfinal = false;
            turret_iter->turret->Target(nullptr);
            turret_iter->turret->TargetTurret(nullptr);
            if (finalChoice.target)
            {
                if (finalChoice.range < turret_iter->gunrange && ROLES::getPriority(turret_iter->turret->attackPreference())[finalChoice.target->unitRole()] < 31)
                {
                    if (CanFaceTarget(turret_iter->turret, finalChoice.target, pos))
                    {
                        turret_iter->turret->Target(finalChoice.target);
                        turret_iter->turret->TargetTurret(finalChoice.target);
                        foundfinal = true;
                    }
                }
            }
            if (!foundfinal)
            {
                for (uint32_t f = 0; f < 2 && !foundfinal; f++)
                {
                    for (uint32_t i = 0; i < lotsize[f]; i++)
                    {
                        const int32_t index = (count + i) % lotsize[f];
                        if (listOfTargets[f][index].range < turret_iter->gunrange)
                        {
                            if (CanFaceTarget(turret_iter->turret, finalChoice.target, pos))
                            {
                                turret_iter->turret->Target(listOfTargets[f][index].target);
                                turret_iter->turret->TargetTurret(listOfTargets[f][index].target);
                                count++;
                                foundfinal = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
};

void AssignTBin(Unit *su, vector<TurretBin> &tbin)
{
    uint32_t bnum = 0;
    for (; bnum < tbin.size(); bnum++)
    {
        if (su->attackPreference() == tbin[bnum].turret[0].turret->attackPreference())
        {
            break;
        }
    }
    if (bnum >= tbin.size())
    {
        tbin.push_back(TurretBin());
    }
    float gspeed, grange, mrange;
    grange = FLT_MAX;
    su->getAverageGunSpeed(gspeed, grange, mrange);
    {
        float ggspeed, ggrange, mmrange;
        Unit *ssu;
        for (auto i = su->getSubUnits(); (ssu = *i) != nullptr; ++i)
        {
            ssu->getAverageGunSpeed(ggspeed, ggrange, mmrange);
            if (ggspeed > gspeed)
            {
                gspeed = ggspeed;
            }
            if (ggrange > grange)
            {
                grange = ggrange;
            }
            if (mmrange > mrange)
            {
                mrange = mmrange;
            }
        }
    }
    if (tbin[bnum].maxrange < grange)
        tbin[bnum].maxrange = grange;
    tbin[bnum].turret.push_back(RangeSortedTurrets(su, grange));
}

float Priority(Unit *me, Unit *targ, float gunrange, float rangetotarget, float relationship, char *rolepriority)
{
    if (relationship >= 0)
    {
        return -1;
    }
    if (targ->GetHull() < 0)
    {
        return -1;
    }
    *rolepriority = ROLES::getPriority(me->attackPreference())[targ->unitRole()]; //number btw 0 and 31 higher better
    char invrolepriority = 31 - *rolepriority;
    if (invrolepriority <= 0)
    {
        return -1;
    }
    if (rangetotarget < 1 && rangetotarget > -1000)
    {
        rangetotarget = 1;
    }
    else
    {
        rangetotarget = fabs(rangetotarget);
    }
    if (rangetotarget < .5 * gunrange)
    {
        rangetotarget = .5 * gunrange;
    }
    if (gunrange <= 0)
    {
        static float mountless_gunrange =
            XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MountlessGunRange", "300000000"));
        gunrange = mountless_gunrange;
    } //probably a mountless capship. 50000 is chosen arbitrarily
    float inertial_priority = 0;
    {
        static float mass_inertial_priority_cutoff =
            XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MassInertialPriorityCutoff", "5000"));
        if (me->GetMass() > mass_inertial_priority_cutoff)
        {
            static float mass_inertial_priority_scale =
                XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MassInertialPriorityScale", ".0000001"));
            Vector normv(me->GetVelocity());
            float Speed = me->GetVelocity().Magnitude();
            normv *= Speed ? 1.0f / Speed : 1.0f;
            Vector ourToThem = targ->Position() - me->Position();
            ourToThem.Normalize();
            inertial_priority = mass_inertial_priority_scale * (.5 + .5 * (normv.Dot(ourToThem))) * me->GetMass() * Speed;
        }
    }
    static float threat_weight = XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "ThreatWeight", ".5"));
    float threat_priority = (me->Threat() == targ) ? threat_weight : 0;
    threat_priority += (targ->Target() == me) ? threat_weight : 0;
    float role_priority01 = ((float)*rolepriority) / 31.;
    float range_priority01 = .5 * gunrange / rangetotarget; //number between 0 and 1 for most ships 1 is best
    return range_priority01 * role_priority01 + inertial_priority + threat_priority;
}

float Priority(Unit *me, Unit *targ, float gunrange, float rangetotarget, float relationship)
{
    char rolepriority = 0;
    return Priority(me, targ, gunrange, rangetotarget, relationship, &rolepriority);
}

template <class T, size_t n>
class StaticTuple
{
public:
    T vec[n];
    size_t size()
    {
        return n;
    }
    T &operator[](size_t index)
    {
        return vec[index];
    }
    const T &operator[](size_t index) const
    {
        return vec[index];
    }
};

template <size_t numTuple>
class ChooseTargetClass
{
    Unit *parent;
    Unit *parentparent;
    vector<TurretBin> *tbin;
    StaticTuple<float, numTuple> maxinnerrangeless;
    StaticTuple<float, numTuple> maxinnerrangemore;
    float priority;
    char rolepriority;
    char maxrolepriority;
    bool reachedMore;
    bool reachedLess;
    FireAt *fireat;
    float gunrange;
    int32_t numtargets;
    int32_t maxtargets;

public:
    Unit *mytarg;
    ChooseTargetClass() {}
    void init(FireAt *fireat,
              Unit *un,
              float gunrange,
              vector<TurretBin> *tbin,
              const StaticTuple<float, numTuple> &innermaxrange,
              char maxrolepriority,
              int maxtargets)
    {
        this->fireat = fireat;
        this->tbin = tbin;
        this->parent = un;
        this->parentparent = un->owner ? UniverseUtil::getUnitByPtr(un->owner, un, false) : 0;
        mytarg = nullptr;
        double currad = 0;
        if (!is_null(un->location[Unit::UNIT_ONLY]))
        {
            currad = un->location[Unit::UNIT_ONLY]->getKey();
        }
        for (size_t i = 0; i < numTuple; ++i)
        {
            double tmpless = currad - innermaxrange[i];
            double tmpmore = currad + innermaxrange[i];
            this->maxinnerrangeless[i] = tmpless;
            this->maxinnerrangemore[i] = tmpmore;
        }
        this->maxrolepriority = maxrolepriority; //max priority that will allow gun range to be ok
        reachedMore = false;
        reachedLess = false;
        this->priority = -1;
        this->rolepriority = 31;
        this->gunrange = gunrange;
        this->numtargets = 0;
        this->maxtargets = maxtargets;
    }
    bool acquire(Unit *un, float distance)
    {
        double unkey = un->location[Unit::UNIT_ONLY]->getKey();
        bool lesscheck = unkey < maxinnerrangeless[0];
        bool morecheck = unkey > maxinnerrangemore[0];
        if (reachedMore == false || reachedLess == false)
        {
            if (lesscheck || morecheck)
            {
                if (lesscheck)
                {
                    reachedLess = true;
                }
                if (morecheck)
                {
                    reachedMore = true;
                }
                if (mytarg && rolepriority < maxrolepriority)
                {
                    return false;
                }
                else if (reachedLess == true && reachedMore == true)
                {
                    for (size_t i = 1; i < numTuple; ++i)
                    {
                        if (unkey > maxinnerrangeless[i] && unkey < maxinnerrangemore[i])
                        {
                            maxinnerrangeless[0] = maxinnerrangeless[i];
                            maxinnerrangemore[0] = maxinnerrangemore[i];
                            reachedLess = false;
                            reachedMore = false;
                        }
                    }
                }
            }
        }
        return ShouldTargetUnit(un, distance);
    }
    bool ShouldTargetUnit(Unit *unit, float distance)
    {
        if (unit->CloakVisible() > .8)
        {
            float rangetotarget = distance;
            float rel0 = parent->getRelation(unit);
            float rel[] = {
                rel0 //not initialized until after array
                ,
                (parentparent ? parentparent->getRelation(unit) : rel0) //not initialized till after array
            };
            float relationship = rel0;
            for (uint32_t i = 1; i < sizeof(rel) / sizeof(*rel); i++)
            {
                if (rel[i] < relationship)
                {
                    relationship = rel[i];
                }
            }
            char rp = 31;
            float tmp = Priority(parent, unit, gunrange, rangetotarget, relationship, &rp);
            if (tmp > priority)
            {
                mytarg = unit;
                priority = tmp;
                rolepriority = rp;
            }
            for (vector<TurretBin>::iterator k = tbin->begin(); k != tbin->end(); ++k)
            {
                if (rangetotarget > k->maxrange)
                {
                    break;
                }
                const char tprior = ROLES::getPriority(k->turret[0].turret->attackPreference())[unit->unitRole()];
                if (relationship < 0)
                {
                    if (tprior < 16)
                    {
                        k->listOfTargets[0].push_back(TargetAndRange(unit, rangetotarget, relationship));
                        numtargets++;
                    }
                    else if (tprior < 31)
                    {
                        k->listOfTargets[1].push_back(TargetAndRange(unit, rangetotarget, relationship));
                        numtargets++;
                    }
                }
            }
        }
        return (maxtargets == 0) || (numtargets < maxtargets);
    }
};

int32_t numpolled[2] = {0, 0}; //number of units that searched for a target

int32_t prevpollindex[2] = {10000, 10000}; //previous number of units touched (doesn't need to be precise)

int32_t pollindex[2] = {1, 1}; //current count of number of units touched (doesn't need to be precise)  -- used for "fairness" heuristic

void FireAt::ChooseTargets(int numtargs, bool force)
{
    float gunspeed, gunrange, missilerange;
    parent->getAverageGunSpeed(gunspeed, gunrange, missilerange);
    static float targettimer = UniverseUtil::GetGameTime(); //timer used to determine passage of physics frames
    static float mintimetoswitch =
        XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MinTimeToSwitchTargets", "3"));
    static float minnulltimetoswitch =
        XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MinNullTimeToSwitchTargets", "5"));
    static int32_t minnumpollers =
        float_to_int(XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MinNumberofpollersperframe", "5"))); //maximum number of vessels allowed to search for a target in a given physics frame
    static int32_t maxnumpollers =
        float_to_int(XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MaxNumberofpollersperframe", "49"))); //maximum number of vessels allowed to search for a target in a given physics frame
    static int32_t numpollers[2] = {maxnumpollers, maxnumpollers};

    static int nextframenumpollers[2] = {maxnumpollers, maxnumpollers};
    if (lastchangedtarg + mintimetoswitch > 0)
        return; //don't switch if switching too soon

    Unit *curtarg = parent->Target();
    int32_t hastarg = (curtarg == nullptr) ? 0 : 1;
    //Following code exists to limit the number of craft polling for a target in a given frame - this is an expensive operation, and needs to be spread out, or there will be pauses.
    static float simatom = XMLSupport::parse_float(vs_config->getVariable("general", "simulation_atom", "0.1"));
    if ((UniverseUtil::GetGameTime()) - targettimer >= simatom * .99)
    {
        //Check if one or more physics frames have passed
        numpolled[0] = numpolled[1] = 0; //reset counters
        prevpollindex[0] = pollindex[0];
        prevpollindex[1] = pollindex[1];
        pollindex[hastarg] = 0;
        targettimer = UniverseUtil::GetGameTime();
        numpollers[0] = float_to_int(nextframenumpollers[0]);
        numpollers[1] = float_to_int(nextframenumpollers[1]);
    }
    pollindex[hastarg]++;                         //count number of craft touched - will use in the next physics frame to spread out the vessels actually chosen to be processed among all of the vessels being touched
    if (numpolled[hastarg] > numpollers[hastarg]) //over quota, wait until next physics frame
    {
        return;
    }
    if (!(pollindex[hastarg] % ((prevpollindex[hastarg] / numpollers[hastarg]) + 1))) //spread out, in modulo fashion, the possibility of changing one's target. Use previous physics frame count of craft to estimate current number of craft
    {
        numpolled[hastarg]++; //if a more likely candidate, we're going to search for a target.
    }
    else
    {
        return; //skipped to achieve better fairness - see comment on modulo distribution above
    }
    if (curtarg)
    {
        if (isJumpablePlanet(curtarg))
        {
            return;
        }
    }
    bool wasnull = (curtarg == nullptr);
    Flightgroup *flight_group = parent->getFlightgroup();
    lastchangedtarg = 0 + targrand.uniformInc(0, 1) * mintimetoswitch; //spread out next valid time to switch targets - helps to ease per-frame loads.
    if (flight_group)
    {
        if (!flight_group->directive.empty())
        {
            if (curtarg != nullptr && (*flight_group->directive.begin()) == toupper(*flight_group->directive.begin()))
            {
                return;
            }
        }
    }
    //not   allowed to switch targets
    numprocessed++;
    vector<TurretBin> tbin;
    Unit *su = nullptr;
    auto subun = parent->getSubUnits();
    for (; (su = *subun) != nullptr; ++subun)
    {
        static uint32_t inert = ROLES::getRole("INERT");
        static uint32_t pointdef = ROLES::getRole("POINTDEF");
        static bool assignpointdef =
            XMLSupport::parse_bool(vs_config->getVariable("AI", "Targetting", "AssignPointDef", "true"));
        if ((su->attackPreference() != pointdef) || assignpointdef)
        {
            if (su->attackPreference() != inert)
            {
                AssignTBin(su, tbin);
            }
            else
            {
                Unit *ssu = nullptr;
                for (auto subturret = su->getSubUnits(); (ssu = (*subturret)); ++subturret)
                {
                    AssignTBin(ssu, tbin);
                }
            }
        }
    }
    std::sort(tbin.begin(), tbin.end());
    float efrel = 0;
    float mytargrange = FLT_MAX;
    static float unitRad =
        XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "search_extra_radius", "1000")); //Maximum target radius that is guaranteed to be detected
    static char maxrolepriority =
        XMLSupport::parse_int(vs_config->getVariable("AI", "Targetting", "search_max_role_priority", "16"));
    static int32_t maxtargets = XMLSupport::parse_int(vs_config->getVariable("AI", "Targetting", "search_max_candidates", "64")); //Cutoff candidate count (if that many hostiles found, stop search - performance/quality tradeoff, 0=no cutoff)
    UnitWithinRangeLocator<ChooseTargetClass<2>> unitLocator(parent->GetComputerData().radar.maxrange, unitRad);
    StaticTuple<float, 2> maxranges;

    maxranges[0] = gunrange;
    maxranges[1] = missilerange;
    if (tbin.size())
    {
        maxranges[0] = (tbin[0].maxrange > gunrange ? tbin[0].maxrange : gunrange);
    }
    double pretable = queryTime();
    unitLocator.action.init(this, parent, gunrange, &tbin, maxranges, maxrolepriority, maxtargets);
    static int32_t gcounter = 0;
    static int32_t min_rechoose_interval = XMLSupport::parse_int(vs_config->getVariable("AI", "min_rechoose_interval", "128"));
    if (curtarg)
    {
        if (gcounter++ < min_rechoose_interval || rand() / 8 < RAND_MAX / 9)
        {
            //in this case only look at potentially *interesting* units rather than huge swaths of nearby units...including target, threat, players, and leader's target
            unitLocator.action.ShouldTargetUnit(curtarg, UnitUtil::getDistance(parent, curtarg));
            uint32_t np = _Universe->numPlayers();
            for (uint32_t i = 0; i < np; ++i)
            {
                Unit *playa = _Universe->AccessCockpit(i)->GetParent();
                if (playa)
                    unitLocator.action.ShouldTargetUnit(playa, UnitUtil::getDistance(parent, playa));
            }
            Unit *lead = UnitUtil::getFlightgroupLeader(parent);
            if (lead != nullptr && lead != parent && (lead = lead->Target()) != nullptr)
            {
                unitLocator.action.ShouldTargetUnit(lead, UnitUtil::getDistance(parent, lead));
            }
            Unit *threat = parent->Threat();
            if (threat)
            {
                unitLocator.action.ShouldTargetUnit(threat, UnitUtil::getDistance(parent, threat));
            }
        }
        else
        {
            gcounter = 0;
        }
    }
    if (unitLocator.action.mytarg == nullptr) //decided to rechoose or did not have initial target
    {
        findObjects(
            _Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY], parent->location[Unit::UNIT_ONLY], &unitLocator);
    }
    Unit *mytarg = unitLocator.action.mytarg;
    targetpick += queryTime() - pretable;
    if (mytarg)
    {
        efrel = parent->getRelation(mytarg);
        mytargrange = UnitUtil::getDistance(parent, mytarg);
    }
    TargetAndRange my_target(mytarg, mytargrange, efrel);
    for (vector<TurretBin>::iterator k = tbin.begin(); k != tbin.end(); ++k)
    {
        k->AssignTargets(my_target, parent->cumulative_transformation_matrix);
    }
    parent->LockTarget(false);
    if (wasnull)
    {
        if (mytarg)
        {
            nextframenumpollers[hastarg] += 2;
            if (nextframenumpollers[hastarg] > maxnumpollers)
            {
                nextframenumpollers[hastarg] = maxnumpollers;
            }
        }
        else
        {
            lastchangedtarg += targrand.uniformInc(0, 1) * minnulltimetoswitch;
            nextframenumpollers[hastarg] -= .05;
            if (nextframenumpollers[hastarg] < minnumpollers)
            {
                nextframenumpollers[hastarg] = minnumpollers;
            }
        }
    }
    else
    {
        if (parent->Target() != mytarg)
        {
            nextframenumpollers[hastarg] += 2;
            if (nextframenumpollers[hastarg] > maxnumpollers)
            {
                nextframenumpollers[hastarg] = maxnumpollers;
            }
        }
        else
        {
            nextframenumpollers[hastarg] -= .01;
            if (nextframenumpollers[hastarg] < minnumpollers)
            {
                nextframenumpollers[hastarg] = minnumpollers;
            }
        }
    }
    parent->Target(mytarg);
    parent->LockTarget(true);
    SignalChosenTarget();
}

bool FireAt::ShouldFire(Unit *targ, bool &missilelock)
{
    float dist;
    if (!targ)
    {
        return false;

        static int test = 0;
        if (test++ % 1000 == 1)
        {
            VSFileSystem::vs_fprintf(stderr, "lost target");
        }
    }
    float gunspeed, gunrange, missilerange;
    parent->getAverageGunSpeed(gunspeed, gunrange, missilerange);
    float angle = parent->cosAngleTo(targ, dist, parent->GetComputerData().itts ? gunspeed : FLT_MAX, gunrange, false);
    missilelock = false;
    targ->Threaten(parent, angle / (dist < .8 ? .8 : dist));
    if (targ == parent->Target())
    {
        distance = dist;
    }
    static float firewhen = XMLSupport::parse_float(vs_config->getVariable("AI", "Firing", "InWeaponRange", "1.2"));
    static float fireangle_minagg =
        (float)cos(M_PI * XMLSupport::parse_float(vs_config->getVariable("AI", "Firing", "MaximumFiringAngle.minagg",
                                                                         "10")) /
                   180.); //Roughly 10 degrees
    static float fireangle_maxagg =
        (float)cos(M_PI * XMLSupport::parse_float(vs_config->getVariable("AI", "Firing", "MaximumFiringAngle.maxagg",
                                                                         "18")) /
                   180.); //Roughly 18 degrees
    float temp = parent->TrackingGuns(missilelock);
    bool isjumppoint = targ->isUnit() == PLANETPTR && ((Planet *)targ)->GetDestinations().empty() == false;
    float fangle = (fireangle_minagg + fireangle_maxagg * agg) / (1.0f + agg);
    bool retval =
        ((dist < firewhen) && ((angle > fangle) || (temp && (angle > temp)) || (missilelock && (angle > 0)))) && !isjumppoint;
    if (retval)
    {
        if (Cockpit::tooManyAttackers())
        {
            Cockpit *player = _Universe->isPlayerStarship(targ);
            if (player)
            {
                static int32_t max_attackers = XMLSupport::parse_int(vs_config->getVariable("AI", "max_player_attackers", "0"));
                int32_t attackers = player->number_of_attackers;
                if (attackers > max_attackers && max_attackers > 0)
                {
                    static float attacker_switch_time =
                        XMLSupport::parse_float(vs_config->getVariable("AI", "attacker_switch_time", "15"));
                    int32_t curtime = (int)fmod(floor(UniverseUtil::GetGameTime() / attacker_switch_time), (float)(1 << 24));
                    int32_t seed = ((((size_t)parent) & 0xffffffff) ^ curtime);
                    static VSRandom decide(seed);
                    decide.init_genrand(seed);
                    if (decide.genrand_int31() % attackers >= max_attackers)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return retval;
}

FireAt::~FireAt()
{
#ifdef ORDERDEBUG
    VSFileSystem::vs_fprintf(stderr, "fire%x\n", this);
    fflush(stderr);
#endif
}

uint32_t FireBitmask(Unit *parent, bool shouldfire, bool firemissile)
{
    uint32_t firebitm = ROLES::EVERYTHING_ELSE;
    Unit *un = parent->Target();
    if (un)
    {
        firebitm = (1 << un->unitRole());

        static bool AlwaysFireAutotrackers =
            XMLSupport::parse_bool(vs_config->getVariable("AI", "AlwaysFireAutotrackers", "true"));
        if (shouldfire)
        {
            firebitm |= ROLES::FIRE_GUNS;
        }
        if (AlwaysFireAutotrackers && !shouldfire)
        {
            firebitm |= ROLES::FIRE_GUNS;
            firebitm |= ROLES::FIRE_ONLY_AUTOTRACKERS;
        }
        if (firemissile)
        {
            firebitm = ROLES::FIRE_MISSILES; //stops guns
        }
    }
    return firebitm;
}

void FireAt::FireWeapons(bool shouldfire, bool lockmissile)
{
    static float missiledelay = XMLSupport::parse_float(vs_config->getVariable("AI", "MissileGunDelay", "4"));
    bool fire_missile = lockmissile && rand() < RAND_MAX * missileprobability * SIMULATION_ATOM;
    delay += SIMULATION_ATOM;
    if (shouldfire && delay < parent->pilot->getReactionTime())
    {
        return;
    }
    else if (!shouldfire)
    {
        delay = 0;
    }
    if (fire_missile)
    {
        lastmissiletime = UniverseUtil::GetGameTime();
    }
    else if (UniverseUtil::GetGameTime() - lastmissiletime < missiledelay && !fire_missile)
    {
        return;
    }
    parent->Fire(FireBitmask(parent, shouldfire, fire_missile), true);
}

bool FireAt::isJumpablePlanet(Unit *targ)
{
    bool istargetjumpableplanet = targ->isUnit() == PLANETPTR;
    if (istargetjumpableplanet)
    {
        istargetjumpableplanet = (!((Planet *)targ)->GetDestinations().empty()) && (parent->GetJumpStatus().drive >= 0);
    }
    return istargetjumpableplanet;
}

using std::string;
void FireAt::PossiblySwitchTarget(bool unused)
{
    static float targettime = XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "TimeUntilSwitch", "20"));
    if ((targettime <= 0) || (vsrandom.uniformInc(0, 1) < SIMULATION_ATOM / targettime))
    {
        bool choose_target = true;
        Flightgroup *flight_group;
        if ((flight_group = parent->getFlightgroup()))
        {
            if (flight_group->directive.find(".") != string::npos)
            {
                choose_target = (parent->Target() == nullptr);
            }
        }
        if (choose_target)
        {
            ChooseTarget();
        }
    }
}

void FireAt::Execute()
{
    lastchangedtarg -= SIMULATION_ATOM;
    bool missilelock = false;
    bool tmp = done;
    Order::Execute();
    done = tmp;
    Unit *targ;
    if (parent->isUnit() == UNITPTR)
    {
        static float cont_update_time = XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandUpdateTime", "1"));
        if (rand() < RAND_MAX * SIMULATION_ATOM / cont_update_time)
        {
            UpdateContrabandSearch();
        }
        static float cont_initiate_time = XMLSupport::parse_float(vs_config->getVariable("AI", "CommInitiateTime", "300"));
        if ((float)rand() < ((float)RAND_MAX * (SIMULATION_ATOM / cont_initiate_time)))
        {
            static float contraband_initiate_time =
                XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandInitiateTime", "3000"));
            static float comm_to_player = XMLSupport::parse_float(vs_config->getVariable("AI", "CommToPlayerPercent", ".05"));
            static float comm_to_target = XMLSupport::parse_float(vs_config->getVariable("AI", "CommToTargetPercent", ".25"));
            static float contraband_to_player =
                XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandToPlayerPercent", ".98"));
            static float contraband_to_target =
                XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandToTargetPercent", "0.001"));

            uint32_t modulo = ((uint32_t)(contraband_initiate_time / cont_initiate_time));
            if (modulo < 1)
            {
                modulo = 1;
            }
            if (rand() % modulo)
            {
                RandomInitiateCommunication(comm_to_player, comm_to_target);
            }
            else
            {
                InitiateContrabandSearch(contraband_to_player, contraband_to_target);
            }
        }
    }
    bool shouldfire = false;
    bool istargetjumpableplanet = false;
    if ((targ = parent->Target()))
    {
        istargetjumpableplanet = isJumpablePlanet(targ);
        if (targ->CloakVisible() > .8 && targ->GetHull() >= 0)
        {
            had_target = true;
            if (parent->GetNumMounts() > 0)
            {
                if (!istargetjumpableplanet)
                {
                    shouldfire |= ShouldFire(targ, missilelock);
                }
            }
        }
        else
        {
            if (had_target)
            {
                had_target = false;
                lastchangedtarg = -100000;
            }
            ChooseTarget();
        }
    }
    else if (had_target)
    {
        had_target = false;
        lastchangedtarg = -100000;
    }
    PossiblySwitchTarget(istargetjumpableplanet);
    if ((!istargetjumpableplanet) && parent->GetNumMounts() > 0)
    {
        FireWeapons(shouldfire, missilelock);
    }
}
