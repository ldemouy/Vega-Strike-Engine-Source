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
 *  xml Mission written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#include <Python.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <assert.h>
#include "cmd/unit_generic.h"
#include "mission.h"
#include "flightgroup.h"
#include "cmd/briefing.h"
#include "python/python_class.h"
#include "savegame.h"
#include "cmd/unit_factory.h"
#include "cmd/asteroid_generic.h"
#include "cmd/nebula_generic.h"

/* *********************************************************** */
using std::cerr;
using std::cout;
using std::endl;
extern const vector<string> &ParseDestinations(const string &value);
extern BLENDFUNC parse_alpha(const char *);
Mission::~Mission()
{
    VSFileSystem::vs_fprintf(stderr, "Mission Cleanup Not Yet Implemented");
    //do not delete msgcenter...could be vital
}
double Mission::gametime = 0.0;
int32_t Mission::number_of_ships = 0;

vector<Flightgroup *> Mission::flightgroups;

Mission::Mission(const char *filename, const std::string &script, bool loadscripts)
{
    ConstructMission(filename, script, loadscripts);
}
Mission::Mission(const char *filename, bool loadscripts)
{
    ConstructMission(filename, string(""), loadscripts);
}
void Mission::ConstructMission(const char *configfile, const std::string &script, bool loadscripts)
{
    player_autopilot = global_autopilot = AUTO_NORMAL;
    player_num = 0;
    briefing = nullptr;
    runtime.pymissions = nullptr;
    nextpythonmission = nullptr;
    if (script.length() > 0)
    {
        nextpythonmission = new char[script.length() + 2];
        nextpythonmission[script.length() + 1] = 0;
        nextpythonmission[script.length()] = 0;
        strcpy(nextpythonmission, script.c_str());
    }
    easyDomFactory<missionNode> domf;
    top = domf.LoadXML(configfile);
    static bool dontpanic = false;
    if (top == nullptr && !dontpanic)
    {
        cout << "Panic exit - mission file " << configfile << " not found" << endl;
        exit(0);
    }
    else
    {
        dontpanic = true;
    }
    if (top == nullptr)
    {
        return;
    }

    variables = nullptr;
    origin_node = nullptr;

#ifndef VS_MIS_SEL
    if (loadscripts)
    {
        initTagMap();
        top->Tag(&tagmap);
    }
#endif
}
Unit *Mission::Objective::getOwner()
{
    Unit *Nawl = nullptr;
    if (Owner != Nawl)
    {
        Unit *ret = Owner.GetUnit();
        if (ret == nullptr)
        {
            objective = ""; //unit died
        }
        return ret;
    }
    return Owner.GetUnit();
}
MessageCenter *Mission::msgcenter = nullptr;
void Mission::initMission(bool loadscripts)
{

    if (!top)
    {
        return;
    }
    if (msgcenter == nullptr)
    {
        msgcenter = new MessageCenter();
    }
    checkMission(top, loadscripts);
    mission_name = getVariable("mission_name", "");
}

/* *********************************************************** */

bool Mission::checkMission(easyDomNode *node, bool loadscripts)
{

    if (node->Name() != "mission")
    {
        cout << "this is no Vegastrike mission file" << endl;
        return false;
    }
    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++)
    {
        if ((*siter)->Name() == "variables")
        {
            doVariables(*siter);
        }
        else if (((*siter)->Name() == "flightgroups"))
        {
            doFlightgroups(*siter);
        }
        else if (((*siter)->Name() == "settings"))
        {
            doSettings(*siter);
        }
        else if (((*siter)->Name() == "python") && (!this->nextpythonmission))
        {
            //I need to get rid of an extra whitespace at the end that expat may have added... Python is VERY strict about that... :(
            string locals = (*siter)->attr_value(textAttr);
            const char *constdumbstr = locals.c_str(); //get the text XML attribute
            size_t i = strlen(constdumbstr);           //constdumbstr is the string I wish to copy... i is its length.
            char *dumbstr = new char[i + 2];           //allocate 2 extra bytes for a double-null-terminated string.
            strncpy(dumbstr, constdumbstr, i);         //i copy constdumbstr to dumbstr.
            dumbstr[i] = '\0';                         //I make sure that it has 2 null bytes at the end.
            dumbstr[i + 1] = '\0';                     //I am allowed to use i+1 because I allocated 2 extra bytes
            for (i -= 1; i >= 0; i--)
            {
                //start from the end-1, or i-1 and set i to that value(i-=1)
                if (dumbstr[i] == '\t' || dumbstr[i] == ' ' || dumbstr[i] == '\r' || dumbstr[i] == '\n')
                {
                    //I check if dumbstr[i] contains whitespace
                    dumbstr[i] = '\0'; //if so, I truncate the string
                }
                else
                {
                    dumbstr[i + 1] = '\n'; //otherwise I add a new line (python sometimes gets mad...)
                    dumbstr[i + 2] = '\0'; //and add a null byte (If i is at the end of the allocated memory, I will use the extra byte
                    break;                 //get out of the loop so that it doesn't endlessly delete the newlines that I added.
                }
            }
            this->nextpythonmission = dumbstr;
        }
        else
        {
            cout << "warning: Unknown tag: " << (*siter)->Name() << endl;
        }
    }
    return true;
}

static std::vector<Mission *> Mission_delqueue;

int32_t Mission::getPlayerMissionNumber()
{
    int32_t num = 0;

    vector<Mission *> *active_missions = ::active_missions.Get();

    if (active_missions->begin() == active_missions->end())
    {
        return -1;
    }

    for (auto pl = active_missions->begin(); pl != active_missions->end(); ++pl)
    {
        if ((*pl)->player_num == this->player_num)
        {
            if (*pl == this)
            {
                return num;
            }
            else
            {
                num++;
            }
        }
    }

    return -1;
}
Mission *Mission::getNthPlayerMission(uint32_t player, int32_t missionnum)
{

    vector<Mission *> *active_missions = ::active_missions.Get();
    Mission *activeMis = nullptr;
    if (missionnum >= 0)
    {
        int32_t num = -1;
        if (active_missions->begin() == active_missions->end())
        {
            return nullptr;
        }
        for (auto pl = active_missions->begin(); pl != active_missions->end(); ++pl)
        {
            if ((*pl)->player_num == player)
            {
                num++;
            }
            if (num == missionnum)
            {
                //Found it!
                activeMis = (*pl);
                break;
            }
        }
    }
    return activeMis;
}
void Mission::terminateMission()
{
    vector<Mission *> *active_missions = ::active_missions.Get();

    auto mission = std::find(Mission_delqueue.begin(), Mission_delqueue.end(), this);
    if (mission != Mission_delqueue.end())
    {
        BOOST_LOG_TRIVIAL(info) << boost::format("Not deleting mission twice: %1%") % this->mission_name;
    }

    mission = std::find(active_missions->begin(), active_missions->end(), this);

    // Debugging aid for persistent missions bug
    if (g_game.vsdebug >= 1)
    {
        int32_t misnum = -1;
        for (vector<Mission *>::iterator i = active_missions->begin(); i != active_missions->end(); ++i)
        {
            if ((*i)->player_num == player_num)
            {
                ++misnum;
                BOOST_LOG_TRIVIAL(info) << boost::format("   Mission #%1%: %2%") % misnum % (*i)->mission_name;
            }
        }
    }

    int32_t queuenum = -1;
    if (mission != active_missions->end())
    {
        queuenum = getPlayerMissionNumber(); //-1 used as error code, 0 is first player mission

        active_missions->erase(mission);
    }
    if (this != (*active_missions)[0]) //Shouldn't this always be true?
    {
        Mission_delqueue.push_back(this); //only delete if we arent' the base mission
    }
    //NETFIXME: This routine does not work properly yet.
    BOOST_LOG_TRIVIAL(info) << boost::format("Terminating mission %1% #%2%") % this->mission_name % queuenum;
    if (queuenum >= 0)
    {
        // queuenum - 1 since mission #0 is the base mission (main_menu) and is persisted
        // in savegame.cpp:LoadSavedMissions, and it has no correspondin active_scripts/active_missions entry,
        // meaning the actual active_scripts index is offset by 1.
        uint32_t num = queuenum - 1;

        vector<std::string> *scripts = &_Universe->AccessCockpit(player_num)->savegame->getMissionStringData("active_scripts");
        BOOST_LOG_TRIVIAL(info) << boost::format("Terminating mission #%1% - got %2% scripts") % queuenum % scripts->size();
        if (num < scripts->size())
        {
            scripts->erase(scripts->begin() + num);
        }
        vector<std::string> *missions = &_Universe->AccessCockpit(player_num)->savegame->getMissionStringData("active_missions");
        BOOST_LOG_TRIVIAL(info) << boost::format("Terminating mission #%1% - got %2% missions") % queuenum % missions->size();
        if (num < missions->size())
        {
            missions->erase(missions->begin() + num);
        }
        BOOST_LOG_TRIVIAL(info) << boost::format("Terminating mission #%1% - %2% scripts remain") % queuenum % scripts->size();
        BOOST_LOG_TRIVIAL(info) << boost::format("Terminating mission #%1% - %2% missions remain") % queuenum % missions->size();
    }
    if (runtime.pymissions)
    {
        runtime.pymissions->Destroy();
    }
    runtime.pymissions = nullptr;
}

/* *********************************************************** */

void Mission::doOrigin(easyDomNode *node)
{
    origin_node = node;
}

/* *********************************************************** */

#ifndef VS_MIS_SEL
void Mission::GetOrigin(QVector &pos, string &planetname)
{
    if (origin_node == nullptr)
    {
        pos.i = pos.j = pos.k = 0.0;
        planetname = string();
        return;
    }
    bool ok = doPosition(origin_node, &pos.i, nullptr);
    if (!ok)
    {
        pos.i = pos.j = pos.k = 0.0;
    }
    planetname = origin_node->attr_value("planet");
}

#endif
/* *********************************************************** */

void Mission::doSettings(easyDomNode *node)
{
    for (auto siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++)
    {
        easyDomNode *mnode = *siter;
        if (mnode->Name() == "origin")
        {
            doOrigin(mnode);
        }
    }
}

/* *********************************************************** */

void Mission::doVariables(easyDomNode *node)
{
    if (variables != nullptr)
    {
        cout << "only one variable section allowed" << endl;
        return;
    }
    variables = node;

    for (auto siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++)
    {
        checkVar(*siter);
    }
}

/* *********************************************************** */

void Mission::checkVar(easyDomNode *node)
{

    if (node->Name() != "var")
    {
        cout << "not a variable" << endl;
        return;
    }
}

/* *********************************************************** */

void Mission::doFlightgroups(easyDomNode *node)
{
    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++)
    {
        checkFlightgroup(*siter);
    }
}
void Mission::AddFlightgroup(Flightgroup *fg)
{
    fg->flightgroup_nr = flightgroups.size();
    flightgroups.push_back(fg);
}
/* *********************************************************** */

void Mission::checkFlightgroup(easyDomNode *node)
{
    if (node->Name() != "flightgroup")
    {
        cout << "not a flightgroup" << endl;
        return;
    }
    //nothing yet
    string texture = node->attr_value("logo");
    string texture_alpha = node->attr_value("logo_alpha");
    string name = node->attr_value("name");
    string faction = node->attr_value("faction");
    string type = node->attr_value("type");
    string ainame = node->attr_value("ainame");
    string waves = node->attr_value("waves");
    string nr_ships = node->attr_value("nr_ships");
    string terrain_nr = node->attr_value("terrain_nr");
    string unittype = node->attr_value("unit_type");
    if (name.empty() || faction.empty() || type.empty() || ainame.empty() || waves.empty() || nr_ships.empty())
    {
        cout << "no valid flightgroup decsription" << endl;
        return;
    }
    if (unittype.empty())
    {
        unittype = string("unit");
    }
    int32_t waves_i = atoi(waves.c_str());
    int32_t nr_ships_i = atoi(nr_ships.c_str());

    bool have_pos = false;

    double pos[3];
    float rot[3];

    rot[0] = rot[1] = rot[2] = 0.0;
    CreateFlightgroup cf;
    cf.fg = Flightgroup::newFlightgroup(name, type, faction, ainame, nr_ships_i, waves_i, texture, texture_alpha, this);

    for (auto siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++)
    {
        if ((*siter)->Name() == "pos")
        {
            have_pos = doPosition(*siter, pos, &cf);
        }
        //        else if ( (*siter)->Name() == "rot" )
        //            doRotation( *siter, rot, &cf );  This function isn't implemented yet
        else if ((*siter)->Name() == "order")
        {
            doOrder(*siter, cf.fg);
        }
    }
    if (!have_pos)
    {
        cout << "don;t have a position in flightgroup " << name << endl;
    }
    if (terrain_nr.empty())
    {
        cf.terrain_nr = -1;
    }
    else
    {
        if (terrain_nr == "mission")
        {
            cf.terrain_nr = -2;
        }
        else
        {
            cf.terrain_nr = atoi(terrain_nr.c_str());
        }
    }
    cf.unittype = CreateFlightgroup::UNIT;
    if (unittype == "vehicle")
    {
        cf.unittype = CreateFlightgroup::VEHICLE;
    }
    if (unittype == "building")
    {
        cf.unittype = CreateFlightgroup::BUILDING;
    }
    cf.nr_ships = nr_ships_i;
    cf.domnode = (node); //don't hijack node

    cf.fg->pos.i = pos[0];
    cf.fg->pos.j = pos[1];
    cf.fg->pos.k = pos[2];
    for (uint8_t i = 0; i < 3; i++)
    {
        cf.rot[i] = rot[i];
    }
    cf.nr_ships = nr_ships_i;
    number_of_ships += nr_ships_i;
}

/* *********************************************************** */

bool Mission::doPosition(easyDomNode *node, double pos[3], CreateFlightgroup *cf)
{
    string x = node->attr_value("x");
    string y = node->attr_value("y");
    string z = node->attr_value("z");
    if (x.empty() || y.empty() || z.empty())
    {
        cout << "no valid position" << endl;
        return false;
    }
    pos[0] = strtod(x.c_str(), nullptr);
    pos[1] = strtod(y.c_str(), nullptr);
    pos[2] = strtod(z.c_str(), nullptr);
    if (cf != nullptr)
    {
        pos[0] += cf->fg->pos.i;
        pos[1] += cf->fg->pos.j;
        pos[2] += cf->fg->pos.k;
    }
    return true;
}

/* *********************************************************** */

Flightgroup *Mission::findFlightgroup(const string &offset_name, const string &fac)
{
    vector<Flightgroup *>::const_iterator siter;
    for (siter = flightgroups.begin(); siter != flightgroups.end(); siter++)
    {
        if ((*siter)->name == offset_name && (fac.empty() || (*siter)->faction == fac))
        {
            return *siter;
        }
    }
    return nullptr;
}

/* *********************************************************** */

void Mission::doOrder(easyDomNode *node, Flightgroup *fg)
{
    //nothing yet
    string order = node->attr_value("order");
    string target = node->attr_value("target");
    if (order.empty() || target.empty())
    {
        cout << "you have to give an order and a target" << endl;
        return;
    }
    //the tmptarget is evaluated later
    //because the target may be a flightgroup that's not yet defined
    fg->ordermap[order] = target;
}

/* *********************************************************** */

string Mission::getVariable(string name, string defaultval)
{
    vector<easyDomNode *>::const_iterator siter;
    for (siter = variables->subnodes.begin(); siter != variables->subnodes.end(); siter++)
    {
        string scan_name = (*siter)->attr_value("name");
        if (scan_name == name)
        {
            return (*siter)->attr_value("value");
        }
    }
    return defaultval;
}

double Mission::getGametime()
{
    return gametime;
}

std::string Mission::Pickle()
{
    if (!runtime.pymissions)
    {
        return "";
    }
    else
    {
        return runtime.pymissions->Pickle();
    }
}
void Mission::UnPickle(string pickled)
{
    if (runtime.pymissions)
    {
        runtime.pymissions->UnPickle(pickled);
    }
}

void Mission::DirectorInitgame()
{
    this->player_num = _Universe->CurrentCockpit();
    if (nextpythonmission)
    {
        //CAUSES AN UNRESOLVED EXTERNAL SYMBOL FOR PythonClass::last_instance ?!?!
#ifndef _WIN32
        char *tmp = nextpythonmission;
        while (*tmp)
        {
            if (tmp[0] == '\r')
            {
                tmp[0] = '\n';
            }
            tmp++;
        }
#endif
        runtime.pymissions = pythonMission::FactoryString(nextpythonmission);
        delete[] nextpythonmission; //delete the allocated memory
        nextpythonmission = nullptr;
        if (!this->unpickleData.empty())
        {
            if (runtime.pymissions)
            {
                runtime.pymissions->UnPickle(unpickleData);
                unpickleData = "";
            }
        }
    }
}

void Mission::DirectorLoop()
{
    double oldgametime = gametime;
    gametime += SIMULATION_ATOM; //elapsed;
    if (getTimeCompression() >= .1)
    {
        if (gametime <= oldgametime)
        {
            gametime = SIMULATION_ATOM;
        }
    }
    try
    {
        BriefingLoop();
        if (runtime.pymissions)
        {
            runtime.pymissions->Execute();
        }
    }
    catch (...)
    {
        if (PyErr_Occurred())
        {
            PyErr_Print();
            PyErr_Clear();
            fflush(stderr);
            fflush(stdout);
        }
        throw;
    }
}

void Mission::DirectorShipDestroyed(Unit *unit)
{
    Flightgroup *fg = unit->getFlightgroup();
    if (fg == nullptr)
    {
        printf("ship destroyed-no flightgroup\n");
        return;
    }
    if (fg->nr_ships_left <= 0 && fg->nr_waves_left > 0)
    {
        printf("WARNING: nr_ships_left<=0\n");
        return;
    }
    fg->nr_ships_left -= 1;

    char buf[512];

    if ((fg->faction.length() + fg->type.length() + fg->name.length() + 12 + 30) < sizeof(buf))
    {
        sprintf(buf, "Ship destroyed: %s:%s:%s-%d", fg->faction.c_str(), fg->type.c_str(),
                fg->name.c_str(), unit->getFgSubnumber());
    }
    else
    {
        sprintf(buf, "Ship destroyed: (ERROR)-%d", unit->getFgSubnumber());
    }

    msgcenter->add("game", "all", buf);

    if (fg->nr_ships_left == 0)
    {
        BOOST_LOG_TRIVIAL(debug) << boost::format("no ships left in fg %1%") % fg->name;
        if (fg->nr_waves_left > 0)
        {
            BOOST_LOG_TRIVIAL(info) << boost::format("Relaunching %1% wave") % fg->name;

            //launch new wave
            fg->nr_waves_left -= 1;
            fg->nr_ships_left = fg->nr_ships;

            Order *order = nullptr;
            order = unit->getAIState() ? unit->getAIState()->findOrderList() : nullptr;

            CreateFlightgroup cf;
            cf.fg = fg;
            cf.unittype = CreateFlightgroup::UNIT;
            cf.fg->pos = unit->Position();
            cf.waves = fg->nr_waves_left;
            cf.nr_ships = fg->nr_ships;

            call_unit_launch(&cf, UNITPTR, string(""));
        }
        else
        {
            mission->msgcenter->add("game", "all", "Flightgroup " + fg->name + " destroyed");
        }
    }
}

void Mission::BriefingUpdate()
{
    if (briefing)
    {
        briefing->Update();
    }
}

void Mission::BriefingLoop()
{
    if (briefing)
    {
        if (runtime.pymissions)
        {
            runtime.pymissions->callFunction("loopbriefing");
        }
    }
}

void Mission::BriefingEnd()
{
    if (briefing)
    {
        if (runtime.pymissions)
        {
            runtime.pymissions->callFunction("endbriefing");
        }
        delete briefing;
        briefing = nullptr;
    }
}

Unit *Mission::call_unit_launch(CreateFlightgroup *fg, int32_t type, const string &destinations)
{
    int32_t faction_nr = FactionUtil::GetFactionIndex(fg->fg->faction);
    Unit **units = new Unit *[fg->nr_ships];

    Unit *par = _Universe->AccessCockpit()->GetParent();
    CollideMap::iterator metahint[2] = {
        _Universe->scriptStarSystem()->collidemap[Unit::UNIT_ONLY]->begin(),
        _Universe->scriptStarSystem()->collidemap[Unit::UNIT_BOLT]->begin()};
    CollideMap::iterator *hint = metahint;
    if (par && !is_null(par->location[Unit::UNIT_ONLY]) && !is_null(par->location[Unit::UNIT_BOLT]) && par->activeStarSystem == _Universe->scriptStarSystem())
    {
        hint = par->location;
    }
    for (int32_t u = 0; u < fg->nr_ships; u++)
    {
        Unit *my_unit;
        if (type == PLANETPTR)
        {
            float radius = 1;
            char *tex = strdup(fg->fg->type.c_str());
            char *nam = strdup(fg->fg->type.c_str());
            char *bsrc = strdup(fg->fg->type.c_str());
            char *bdst = strdup(fg->fg->type.c_str());
            char *citylights = strdup(fg->fg->type.c_str());
            tex[0] = '\0';
            bsrc[0] = '\0'; //have at least 1 char
            bdst[0] = '\0';
            citylights[0] = '\0';
            GFXMaterial mat;
            GFXGetMaterial(0, mat);
            BLENDFUNC s = ONE;
            BLENDFUNC d = ZERO;
            if (bdst[0] != '\0')
            {
                d = parse_alpha(bdst);
            }
            if (bsrc[0] != '\0')
            {
                s = parse_alpha(bsrc);
            }
            my_unit = UnitFactory::createPlanet(QVector(0, 0, 0), QVector(0, 0, 0), 0, Vector(0, 0, 0),
                                                0, 0, radius, tex, "", "", s,
                                                d, ParseDestinations(destinations),
                                                QVector(0, 0, 0), nullptr, mat,
                                                vector<GFXLightLocal>(), faction_nr, nam);
            free(bsrc);
            free(bdst);
            free(tex);
            free(nam);
            free(citylights);
        }
        else if (type == NEBULAPTR)
        {
            my_unit = UnitFactory::createNebula(
                fg->fg->type.c_str(), false, faction_nr, fg->fg, u + fg->fg->nr_ships - fg->nr_ships);
        }
        else if (type == ASTEROIDPTR)
        {
            my_unit = UnitFactory::createAsteroid(
                fg->fg->type.c_str(), faction_nr, fg->fg, u + fg->fg->nr_ships - fg->nr_ships, .01);
        }
        else
        {
            my_unit = UnitFactory::createUnit(fg->fg->type.c_str(), false, faction_nr, string(""), fg->fg, u + fg->fg->nr_ships - fg->nr_ships, nullptr);
        }
        units[u] = my_unit;
    }
    float fg_radius = units[0]->rSize();
    Unit *my_unit;
    for (int32_t u = 0; u < fg->nr_ships; u++)
    {
        my_unit = units[u];
        QVector pox;
        pox.i = fg->fg->pos.i + u * fg_radius * 3;
        pox.j = fg->fg->pos.j + u * fg_radius * 3;
        pox.k = fg->fg->pos.k + u * fg_radius * 3;
        my_unit->SetPosAndCumPos(pox);
        if (type == ASTEROIDPTR || type == NEBULAPTR)
        {
            my_unit->PrimeOrders();
        }
        else
        {
            my_unit->LoadAIScript(fg->fg->ainame);
            my_unit->SetTurretAI();
        }
        _Universe->scriptStarSystem()->AddUnit(my_unit);
        my_unit->UpdateCollideQueue(_Universe->scriptStarSystem(), hint);
        if (!is_null(my_unit->location[Unit::UNIT_ONLY]) && !is_null(my_unit->location[Unit::UNIT_BOLT]))
        {
            hint = my_unit->location;
        }
        my_unit->Target(nullptr);
    }
    my_unit = units[0];
    if (!_Universe->isPlayerStarship(fg->fg->leader.GetUnit()))
    {
        fg->fg->leader.SetUnit(my_unit);
    }
    delete[] units;
    return my_unit;
}

void Mission::initTagMap()
{
    tagmap["module"] = DTAG_MODULE;
    tagmap["script"] = DTAG_SCRIPT;
    tagmap["if"] = DTAG_IF;
    tagmap["block"] = DTAG_BLOCK;
    tagmap["setvar"] = DTAG_SETVAR;
    tagmap["exec"] = DTAG_EXEC;
    tagmap["call"] = DTAG_CALL;
    tagmap["while"] = DTAG_WHILE;
    tagmap["and"] = DTAG_AND_EXPR;
    tagmap["or"] = DTAG_OR_EXPR;
    tagmap["not"] = DTAG_NOT_EXPR;
    tagmap["test"] = DTAG_TEST_EXPR;
    tagmap["fmath"] = DTAG_FMATH;
    tagmap["vmath"] = DTAG_VMATH;
    tagmap["var"] = DTAG_VAR_EXPR;
    tagmap["defvar"] = DTAG_DEFVAR;
    tagmap["const"] = DTAG_CONST;
    tagmap["arguments"] = DTAG_ARGUMENTS;
    tagmap["globals"] = DTAG_GLOBALS;
    tagmap["return"] = DTAG_RETURN;
    tagmap["import"] = DTAG_IMPORT;
}