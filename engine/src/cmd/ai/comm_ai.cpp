#include "comm_ai.h"
#include "faction_generic.h"
#include "communication.h"
#include "cmd/collection.h"
#include "gfx/cockpit_generic.h"
#include "cmd/images.h"
#include "configxml.h"
#include "vs_globals.h"
#include "cmd/script/flightgroup.h"
#include "cmd/unit_util.h"
#include "vs_random.h"
#include "cmd/unit_find.h"
#include "cmd/pilot.h"
#include "universe_util.h"

CommunicatingAI::CommunicatingAI(int32_t ttype,
                                 int32_t stype,
                                 float mood,
                                 float anger,
                                 float appeas,
                                 float moodswingyness,
                                 float randomresp) : Order(ttype, stype), anger(anger), appease(appeas), moodswingyness(moodswingyness), randomresponse(randomresp), mood(mood)
{
    if (appease > 665 && appease < 667)
    {
        static float appeas = XMLSupport::parse_float(vs_config->getVariable("AI", "EaseToAppease", ".5"));
        this->appease = appeas;
    }
    if ((anger > 665 && anger < 667) || (anger > -667 && anger < -665))
    {
        static float ang = XMLSupport::parse_float(vs_config->getVariable("AI", "EaseToAnger", "-.5"));
        this->anger = ang;
    }
    if (moodswingyness > 665 && moodswingyness < 667)
    {
        static float ang1 = XMLSupport::parse_float(vs_config->getVariable("AI", "MoodSwingLevel", ".2"));
        this->moodswingyness = ang1;
    }
    if (randomresp > 665 && moodswingyness < 667)
    {
        static float ang2 = XMLSupport::parse_float(vs_config->getVariable("AI", "RandomResponseRange", ".8"));
        this->randomresponse = ang2;
    }
}

bool MatchingMood(const CommunicationMessage &message, float mood, float randomresponse, float relationship)
{
    static float pos_limit = XMLSupport::parse_float(vs_config->getVariable("AI",
                                                                            "LowestPositiveCommChoice",
                                                                            "0"));
    static float neg_limit = XMLSupport::parse_float(vs_config->getVariable("AI",
                                                                            "LowestNegativeCommChoice",
                                                                            "-.00001"));
    const FSM::Node *node = (uint32_t)message.curstate < message.fsm->nodes.size() ? (&message.fsm->nodes[message.curstate]) : (&message.fsm->nodes[message.fsm->getDefaultState(relationship)]);

    for (auto i = node->edges.begin(); i != node->edges.end(); ++i)
    {
        if (message.fsm->nodes[*i].messagedelta >= pos_limit && relationship >= 0)
        {
            return true;
        }
        if (message.fsm->nodes[*i].messagedelta <= neg_limit && relationship < 0)
        {
            return true;
        }
    }
    return false;
}

int32_t CommunicatingAI::selectCommunicationMessageMood(CommunicationMessage &message, float mood)
{
    Unit *target = message.sender.GetUnit();
    float relationship = 0;
    if (target)
    {
        relationship = parent->pilot->GetEffectiveRelationship(parent, target);
        if (target == parent->Target() && relationship > -1.0)
        {
            relationship = -1.0;
        }
        mood += (1 - randomresponse) * relationship;
    }
    //breaks stuff
    if ((message.curstate >= message.fsm->GetUnDockNode()) || !MatchingMood(message, mood, randomresponse, relationship))
    {
        message.curstate = message.fsm->getDefaultState(relationship); //hijack the current state
    }
    return message.fsm->getCommMessageMood(message.curstate, mood, randomresponse, relationship);
}

using std::pair;

void GetMadAt(Unit *unit, Unit *parent, int32_t numhits = 0)
{
    if (numhits == 0)
    {
        static int32_t snumhits = XMLSupport::parse_int(vs_config->getVariable("AI", "ContrabandMadness", "5"));
        numhits = snumhits;
    }
    CommunicationMessage hit(unit, parent, nullptr, 0);
    hit.SetCurrentState(hit.fsm->GetHitNode(), nullptr, 0);
    for (int32_t i = 0; i < numhits; i++)
    {
        parent->getAIState()->Communicate(hit);
    }
}

void AllUnitsCloseAndEngage(Unit *unit, int32_t faction)
{
    Unit *ally;
    static float contraband_assist_range =
        XMLSupport::parse_float(vs_config->getVariable("physics", "contraband_assist_range", "50000"));
    float relation = 0;
    static float minrel = XMLSupport::parse_float(vs_config->getVariable("AI", "max_faction_contraband_relation", "-.05"));
    static float adj = XMLSupport::parse_float(vs_config->getVariable("AI", "faction_contraband_relation_adjust", "-.025"));
    float delta;
    int32_t cp = _Universe->whichPlayerStarship(unit);
    if (cp != -1)
    {
        if ((relation = UnitUtil::getRelationFromFaction(unit, faction)) > minrel)
        {
            delta = minrel - relation;
        }
        else
        {
            delta = adj;
        }
        UniverseUtil::adjustRelationModifierInt(cp, faction, delta);
    }
    for (auto i = _Universe->activeStarSystem()->getUnitList().createIterator();
         (ally = *i) != nullptr;
         ++i)
    //Vector loc;
    {
        if (ally->faction == faction)
        {
            if ((ally->Position() - unit->Position()).Magnitude() < contraband_assist_range)
            {
                GetMadAt(unit, ally);
                Flightgroup *flight_group = ally->getFlightgroup();
                if (flight_group)
                {
                    if (flight_group->directive.empty() ? true : toupper(*flight_group->directive.begin()) != *flight_group->directive.begin())
                    {
                        ally->Target(unit);
                        ally->TargetTurret(unit);
                    }
                    else
                    {
                        ally->Target(unit);
                        ally->TargetTurret(unit);
                    }
                }
            }
        }
    }
    //}
}

void CommunicatingAI::TerminateContrabandSearch(bool contraband_detected)
{
    //reports success or failure
    Unit *un;
    uint8_t gender;
    std::vector<Animation *> *comm_face = parent->pilot->getCommFaces(gender);
    if ((un = contraband_searchee.GetUnit()))
    {
        CommunicationMessage c(parent, un, comm_face, gender);
        if (contraband_detected)
        {
            c.SetCurrentState(c.fsm->GetContrabandDetectedNode(), comm_face, gender);
            GetMadAt(un, 0);
            AllUnitsCloseAndEngage(un, parent->faction);
        }
        else
        {
            c.SetCurrentState(c.fsm->GetContrabandUnDetectedNode(), comm_face, gender);
        }
        Order *o = un->getAIState();
        if (o)
        {
            o->Communicate(c);
        }
    }
    contraband_searchee.SetUnit(nullptr);
}

void CommunicatingAI::GetMadAt(Unit *unit, int32_t numHitsPerContrabandFail)
{
    ::GetMadAt(unit, parent, numHitsPerContrabandFail);
}

static int32_t InList(std::string item, Unit *unit)
{
    float numcontr = 0;
    if (unit)
    {
        for (size_t i = 0; i < unit->numCargo(); i++)
        {
            if (unit->GetCargo(i).content == item)
            {
                if (unit->GetCargo(i).quantity > 0)
                {
                    numcontr++;
                }
            }
        }
    }
    return float_to_int(numcontr);
}

void CommunicatingAI::UpdateContrabandSearch()
{
    static size_t contraband_search_batch_update =
        XMLSupport::parse_int(vs_config->getVariable("AI", "num_contraband_scans_per_search", "10"));
    for (size_t rep = 0; rep < contraband_search_batch_update; ++rep)
    {
        Unit *unit = contraband_searchee.GetUnit();
        if (unit && (unit->faction != parent->faction))
        {
            //don't scan your buddies
            if (which_cargo_item < (int32_t)unit->numCargo())
            {
                if (unit->GetCargo(which_cargo_item).quantity > 0)
                {
                    int32_t which_carg_item_bak = which_cargo_item;
                    std::string item = unit->GetManifest(which_cargo_item++, parent, SpeedAndCourse);
                    static bool use_hidden_cargo_space =
                        XMLSupport::parse_bool(vs_config->getVariable("physics", "use_hidden_cargo_space", "true"));
                    static float speed_course_change =
                        XMLSupport::parse_float(vs_config->getVariable("AI", "PercentageSpeedChangeToStopSearch", "1"));
                    if (unit->CourseDeviation(SpeedAndCourse, unit->GetVelocity()) > speed_course_change)
                    {
                        uint8_t gender;
                        std::vector<Animation *> *comm_face = parent->pilot->getCommFaces(gender);
                        CommunicationMessage c(parent, unit, comm_face, gender);
                        c.SetCurrentState(c.fsm->GetContrabandWobblyNode(), comm_face, gender);
                        Order *order;
                        if ((order = unit->getAIState()))
                        {
                            order->Communicate(c);
                        }
                        GetMadAt(unit, 1);
                        SpeedAndCourse = unit->GetVelocity();
                    }
                    float HiddenTotal = use_hidden_cargo_space ? (unit->getHiddenCargoVolume()) : (0);
                    Unit *contrabandlist = FactionUtil::GetContraband(parent->faction);
                    if (InList(item, contrabandlist) > 0)
                    {
                        //inlist now returns an integer so that we can do this at all...
                        if (HiddenTotal == 0 || unit->GetCargo(which_carg_item_bak).quantity > HiddenTotal)
                        {
                            TerminateContrabandSearch(true); //BUCO this is where we want to check against free hidden cargo space.
                        }
                        else
                        {
                            uint32_t max = unit->numCargo();
                            uint32_t quantity = 0;
                            for (size_t i = 0; i < max; ++i)
                                if (InList(unit->GetCargo(i).content, contrabandlist) > 0)
                                {
                                    quantity += unit->GetCargo(i).quantity;
                                    if (quantity > HiddenTotal)
                                    {
                                        TerminateContrabandSearch(true);
                                        break;
                                    }
                                }
                        }
                    }
                }
                else
                {
                    TerminateContrabandSearch(false);
                }
            }
        }
    }
}

static bool isDockedAtAll(Unit *unit)
{
    return (unit->docked & (Unit::DOCKED_INSIDE | Unit::DOCKED)) != 0;
}

void CommunicatingAI::Destroy()
{
    for (size_t i = 0; i < _Universe->numPlayers(); ++i)
    {
        Unit *target = _Universe->AccessCockpit(i)->GetParent();
        if (target)
        {
            FSM *fsm = FactionUtil::GetConversation(this->parent->faction, target->faction);
            if (fsm->StopAllSounds((uint8_t)(parent->pilot->getGender())))
            {
                _Universe->AccessCockpit(i)->SetStaticAnimation();
                _Universe->AccessCockpit(i)->SetStaticAnimation();
            }
        }
    }
    this->Order::Destroy();
}

void CommunicatingAI::InitiateContrabandSearch(float playerprob, float targprob)
{
    Unit *u = GetRandomUnit(playerprob, targprob);
    if (u)
    {
        Unit *un = FactionUtil::GetContraband(parent->faction);
        if (un)
        {
            if (un->numCargo() > 0 && UnitUtil::getUnitSystemFile(u) == UnitUtil::getUnitSystemFile(parent) && !UnitUtil::isDockableUnit(parent))
            {
                Unit *v;
                if ((v = contraband_searchee.GetUnit()))
                {
                    if (v == u)
                    {
                        return;
                    }
                    TerminateContrabandSearch(false);
                }
                contraband_searchee.SetUnit(u);
                SpeedAndCourse = u->GetVelocity();
                uint8_t gender;
                std::vector<Animation *> *comm_face = parent->pilot->getCommFaces(gender);
                CommunicationMessage c(parent, u, comm_face, gender);
                c.SetCurrentState(c.fsm->GetContrabandInitiateNode(), comm_face, gender);
                if (u->getAIState())
                {
                    u->getAIState()->Communicate(c);
                }
                which_cargo_item = 0;
            }
        }
    }
}

void CommunicatingAI::AdjustRelationTo(Unit *unit, float factor)
{
    Order::AdjustRelationTo(unit, factor);
    float newrel = parent->pilot->adjustSpecificRelationship(parent, unit, factor, unit->faction);

    {
        int32_t whichCp = _Universe->whichPlayerStarship(parent);
        Flightgroup *to_flight_group;
        int32_t toFaction;
        float oRlyFactor = factor;
        if (whichCp != -1)
        {
            to_flight_group = unit->getFlightgroup();
            toFaction = unit->faction;
        }
        else
        {
            /* Instead use the Aggressor's cockpit? */
            whichCp = _Universe->whichPlayerStarship(unit);
            to_flight_group = parent->getFlightgroup();
            toFaction = parent->faction;
        }
        if (whichCp != -1)
        {
            if (to_flight_group && unit->faction != parent->faction)
            {
                oRlyFactor = factor * 0.5;
            }
            if (to_flight_group)
            {
                UniverseUtil::adjustFGRelationModifier(whichCp, to_flight_group->name, oRlyFactor * parent->pilot->getRank());
            }
            if (unit->faction != parent->faction)
            {
                UniverseUtil::adjustRelationModifierInt(whichCp, toFaction, oRlyFactor * parent->pilot->getRank());
            }
        }
    }
    if (newrel < anger || (parent->Target() == nullptr && newrel + UnitUtil::getFactionRelation(parent, unit) < 0))
    {
        if (parent->Target() == nullptr || (parent->getFlightgroup() == nullptr || parent->getFlightgroup()->directive.find(".") == string::npos))
        {
            parent->Target(unit);       //he'll target you--even if he's friendly
            parent->TargetTurret(unit); //he'll target you--even if he's friendly
        }
        else if (newrel > appease)
        {
            if (parent->Target() == unit)
            {
                if (parent->getFlightgroup() == nullptr || parent->getFlightgroup()->directive.find(".") == string::npos)
                {
                    parent->Target(nullptr);
                    parent->TargetTurret(nullptr); //he'll target you--even if he's friendly
                }
            }
        }
    }
    mood += factor * moodswingyness;
}

//modified not to check player when hostiles are around--unless player IS the hostile
Unit *CommunicatingAI::GetRandomUnit(float playerprob, float targprob)
{
    if (vsrandom.uniformInc(0, 1) < playerprob)
    {
        Unit *player = _Universe->AccessCockpit(rand() % _Universe->numPlayers())->GetParent();
        if (player)
        {
            if ((player->Position() - parent->Position()).Magnitude() - parent->rSize() - player->rSize())
            {
                return player;
            }
        }
    }
    if (vsrandom.uniformInc(0, 1) < targprob && parent->Target())
    {
        return parent->Target();
    }
    //FIXME FOR TESTING ONLY
    //return parent->Target();
    QVector where = parent->Position() + parent->GetComputerData().radar.maxrange * QVector(vsrandom.uniformInc(-1, 1),
                                                                                            vsrandom.uniformInc(-1, 1),
                                                                                            vsrandom.uniformInc(-1, 1));
    Collidable wherewrapper(0, 0, where);

    NearestUnitLocator unitLocator;
#ifdef VS_ENABLE_COLLIDE_KEY
    CollideMap *cm = _Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY];
    static float unitRad =
        XMLSupport::parse_float(vs_config->getVariable("graphics", "hud", "radar_search_extra_radius", "1000"));
    CollideMap::iterator iter = cm->lower_bound(wherewrapper);
    if (iter != cm->end() && (*iter)->radius > 0)
    {
        if ((*iter)->ref.unit != parent)
        {
            return (*iter)->ref.unit;
        }
    }
    findObjects(_Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY], iter, &unitLocator);

    Unit *target = unitLocator.retval.unit;
    if (target == nullptr || target == parent)
    {
        target = parent->Target();
    }
#else
    Unit *target = parent->Target();
#endif
    return target;
}

void CommunicatingAI::RandomInitiateCommunication(float playerprob, float targprob)
{
    Unit *target = GetRandomUnit(playerprob, targprob);
    if (target != nullptr)
    {
        if (UnitUtil::getUnitSystemFile(target) == UnitUtil::getUnitSystemFile(parent) && UnitUtil::getFlightgroupName(parent) != "Base" && !isDockedAtAll(target) && UnitUtil::getDistance(parent, target) <= target->GetComputerData().radar.maxrange)
        {
            //warning--odd hack they can talk to you if you can sense them--it's like SETI@home
            for (std::list<CommunicationMessage *>::iterator i = messagequeue.begin(); i != messagequeue.end(); i++)
            {
                Unit *unit = (*i)->sender.GetUnit();
                if (unit == target)
                {
                    return;
                }
            }
            //ok we're good to put a default msg in the queue as a fake message;
            FSM *fsm = FactionUtil::GetConversation(target->faction, this->parent->faction);
            int32_t state = fsm->getDefaultState(parent->getRelation(target));
            uint8_t gender;
            std::vector<Animation *> *comm_face = parent->pilot->getCommFaces(gender);
            messagequeue.push_back(new CommunicationMessage(target, this->parent, state, state, comm_face, gender));
        }
    }
}

int32_t CommunicatingAI::selectCommunicationMessage(CommunicationMessage &message, Unit *unit)
{
    // TODO: Investigate because currently this does nothing
    if (0 && mood == 0)
    {
        FSM::Node *node = message.getCurrentState();
        if (node)
        {
            return rand() % node->edges.size();
        }
        else
        {
            return 0;
        }
    }
    else
    {
        static float moodmul = XMLSupport::parse_float(vs_config->getVariable("AI", "MoodAffectsRespose", "0"));
        static float angermul = XMLSupport::parse_float(vs_config->getVariable("AI", "AngerAffectsRespose", "1"));
        static float staticrelmul =
            XMLSupport::parse_float(vs_config->getVariable("AI", "StaticRelationshipAffectsResponse", "1"));
        return selectCommunicationMessageMood(message, moodmul * mood + angermul * parent->pilot->getAnger(parent, unit) + staticrelmul * UnitUtil::getFactionRelation(parent, unit));
    }
}

void CommunicatingAI::ProcessCommMessage(CommunicationMessage &message)
{
    if (messagequeue.back()->curstate < messagequeue.back()->fsm->GetUnDockNode())
    {
        Order::ProcessCommMessage(message);
        FSM *tmpfsm = message.fsm;
        Unit *targ = message.sender.GetUnit();
        if (targ && UnitUtil::getUnitSystemFile(targ) == UnitUtil::getUnitSystemFile(parent) && !isDockedAtAll(targ))
        {
            message.fsm = FactionUtil::GetConversation(parent->faction, targ->faction);
            FSM::Node *node = message.getCurrentState();
            if (node)
            {
                if (node->edges.size())
                {
                    Unit *unit = message.sender.GetUnit();
                    if (unit)
                    {
                        int32_t b = selectCommunicationMessage(message, unit);
                        Order *o = unit->getAIState();
                        uint8_t gender;
                        std::vector<Animation *> *comm_face = parent->pilot->getCommFaces(gender);
                        if (o)
                        {
                            o->Communicate(CommunicationMessage(parent, unit, message, b, comm_face, gender));
                        }
                    }
                }
            }
            message.fsm = tmpfsm;
        }
    }
}

CommunicatingAI::~CommunicatingAI() {}
