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
/// Enumerates functions for python modules
///

#ifndef _MISSION_H_
#define _MISSION_H_
//#include <gnuhash.h>

#include <expat.h>
#include <fstream>
#include <string>

#include "easydom.h"

#ifndef VS_MIS_SEL
#include "cmd/container.h"
#include "msgcenter.h"
#include "vegastrike.h"
#include "vs_globals.h"
class Unit;
class Order;
class MessageCenter;
#endif

#include <assert.h>
using std::string;

class Flightgroup;
#ifndef VS_MIS_SEL

enum tag_type
{
    DTAG_UNKNOWN,
    DTAG_MISSION,
    DTAG_SETTINGS,
    DTAG_ORIGIN,
    DTAG_VARIABLES,
    DTAG_FLIGHTGROUPS,
    DTAG_ROT,
    DTAG_POS,
    DTAG_ORDER,
    DTAG_MODULE,
    DTAG_SCRIPT,
    DTAG_IF,
    DTAG_BLOCK,
    DTAG_SETVAR,
    DTAG_EXEC,
    DTAG_CALL,
    DTAG_WHILE,
    DTAG_AND_EXPR,
    DTAG_OR_EXPR,
    DTAG_NOT_EXPR,
    DTAG_TEST_EXPR,
    DTAG_FMATH,
    DTAG_VMATH,
    DTAG_VAR_EXPR,
    DTAG_DEFVAR,
    DTAG_CONST,
    DTAG_ARGUMENTS,
    DTAG_GLOBALS,
    DTAG_RETURN,
    DTAG_IMPORT
};

class missionNode : public tagDomNode
{
};

/* *********************************************************** */

class pythonMission;
class PythonMissionBaseClass;
#endif // VS_MIS_SEL

class Mission
{
  public:
    enum MISSION_AUTO
    {
        AUTO_OFF = -1,
        AUTO_NORMAL = 0,
        AUTO_ON = 1
    };
    uint player_num;
    MISSION_AUTO player_autopilot;
    MISSION_AUTO global_autopilot;
    struct Objective
    {
        float completeness;
        std::string objective;
        Unit *getOwner();
        void setOwner(Unit *u)
        {
            Owner.SetUnit(u);
        }
        UnitContainer Owner;
        Objective()
        {
            completeness = 0;
        }
        Objective(float complete, std::string obj)
        {
            completeness = complete;
            objective = obj;
        }
    };
    vector<Objective> objectives;
    void SetUnpickleData(std::string dat)
    {
        unpickleData = dat;
    }
    class Briefing *briefing;
    static double gametime;
    string mission_name;
    void terminateMission();
    Unit *call_unit_launch(class CreateFlightgroup *fg, int32_t type /*clsptr type*/, const std::string &destinations);

    Mission(const char *configfile, bool loadscripts = true);
    Mission(const char *filename, const std::string &pythonscript, bool loadscripts = true);
    std::string Pickle();               // returns filename\npickleddata
    void UnPickle(std::string pickled); // takes in pickeddata
    void AddFlightgroup(Flightgroup *fg);
    void initMission(bool loadscripts = true);

    int getPlayerMissionNumber(); //-1 if not found or invalid player_num.
    static Mission *getNthPlayerMission(uint32_t cp, int32_t num);

    /// alex Please help me make this function...this is called between mission loops
    ~Mission();

    static int number_of_ships;

    static vector<Flightgroup *> flightgroups;

    Flightgroup *findFlightgroup(const string &fg_name, const string &faction);

    string getVariable(string name, string defaultval);

#ifndef VS_MIS_SEL
    void GetOrigin(QVector &pos, string &planetname);

    void DirectorLoop();
    void DirectorInitgame();
    void DirectorShipDestroyed(Unit *unit);

    void BriefingLoop();
    void BriefingUpdate();
    void BriefingEnd();

    double getGametime();

    static MessageCenter *msgcenter;

#endif // VS_MIS_SEL

  private:
    void ConstructMission(const char *configfile, const std::string &pythonscript, bool loadscripts = true);
    missionNode *top;

    easyDomNode *variables;
    easyDomNode *origin_node;

#ifndef VS_MIS_SEL

    tagMap tagmap;
    char *nextpythonmission;
    std::string unpickleData;

  public:
    struct Runtime
    {
        PythonMissionBaseClass *pymissions;
    } runtime;

  private:
    friend void UnpickleMission(std::string pickled);

    void initTagMap();

#endif // VS_MIS_SEL

    bool checkMission(easyDomNode *node, bool loadscripts);
    void doVariables(easyDomNode *node);
    void checkVar(easyDomNode *node);
    void doFlightgroups(easyDomNode *node);
    void doOrder(easyDomNode *node, Flightgroup *fg);
    void checkFlightgroup(easyDomNode *node);
    bool doPosition(easyDomNode *node, double pos[3], class CreateFlightgroup *);

    void doOrigin(easyDomNode *node);
    void doSettings(easyDomNode *node);
};

#endif //_MISSION_H_
