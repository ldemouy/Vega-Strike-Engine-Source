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
#include <gnuhash.h>

#include <expat.h>
#include <string>
#include <fstream>

//#include "xml_support.h"
#include "easydom.h"

#ifndef VS_MIS_SEL
#include "vegastrike.h"
#include "vs_globals.h"
#include "msgcenter.h"
#include "cmd/container.h"
class Unit;
class Order;
class MessageCenter;
#endif

#include <assert.h>
using std::string;

using XMLSupport::AttributeList;

#define qu(x) ("\"" + x + "\"")

/* *********************************************************** */

#ifdef VS_MIS_SEL
#define missionNode easyDomNode
#endif

class varInst;

//typedef vector<varInst *> olist_t;
//typedef vsUMap<string, varInst *> omap_t;

class Flightgroup;
#ifndef VS_MIS_SEL

/* *********************************************************** */
enum callback_module_type
{
    CMT_UNKNOWN = 0,
    CMT_IO,
    CMT_STD,
    CMT_STRING,
    CMT_OLIST,
    CMT_OMAP,
    CMT_ORDER,
    CMT_UNIT,
    CMT_BRIEFING
};

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
enum var_type
{
    VAR_FAILURE,
    VAR_BOOL,
    VAR_FLOAT,
    VAR_INT,
    VAR_OBJECT,
    VAR_VOID,
    VAR_ANY
};

/* *********************************************************** */

class missionNode;

enum scope_type
{
    VI_GLOBAL,
    VI_MODULE,
    VI_LOCAL,
    VI_TEMP,
    VI_IN_OBJECT,
    VI_ERROR,
    VI_CONST,
    VI_CLASSVAR
};

class varInst
{
public:
    varInst(scope_type sctype)
    {
        scopetype = sctype;
        objectname = string();
        object = nullptr;
    }
    varInst()
    {
        std::cout << "varInst() obsolete\n"
                  << std::endl;
        assert(0);
    }

    string name;
    var_type type;

    scope_type scopetype;
    double float_val;
    bool bool_val;
    int int_val;
    string string_val;

    string objectname;
    void *object;

    missionNode *defvar_node;
    missionNode *block_node;

    unsigned int varId;
};

/* *********************************************************** */

class varInstVec : public vector<varInst *>
{
public:
    unsigned int addVar(varInst *vi)
    {
        push_back(vi);
        int index = size() - 1;
        vi->varId = index;
        return index;
    }
};
class varInstMap : public vsUMap<string, varInst *>
{
public:
    varInstVec varVec;
};

/* *********************************************************** */

class scriptContext
{
public:
    varInstMap *varinsts;
    missionNode *block_node;

    scriptContext()
    {
        varinsts = nullptr;
        block_node = nullptr;
    }
};

/* *********************************************************** */

class contextStack
{
public:
    vector<scriptContext *> contexts;
    varInst *return_value;
};

/* *********************************************************** */

class missionNode;

class missionThread
{
protected:
public:
    virtual ~missionThread() {}
    vector<contextStack *> exec_stack;
    vector<missionNode *> module_stack;
    vector<unsigned int> classid_stack;
};

/* *********************************************************** */

class missionNode : public tagDomNode
{
public:
    struct script_t
    {
        string name;                    //script,defvar,module
        varInstMap variables;           //script,module
        vector<varInstMap *> classvars; //module
        varInst *varinst;               //defvar,const
        missionNode *if_block[3];       //if
        missionNode *while_arg[2];      //while
        int tester;                     //test
        missionNode *test_arg[2];       //test
        enum var_type vartype;          //defvar,script
        string initval;
        missionNode *context_block_node;       //defvar
        vsUMap<string, missionNode *> scripts; //module
        missionNode *exec_node;                //exec, return
        int nr_arguments;                      //script
        missionNode *argument_node;            //script
        missionNode *module_node;              //exec
        unsigned int classinst_counter;
        int context_id;
        int varId;
        callback_module_type callback_module_id;
        int method_id;
    } script;
};

/* *********************************************************** */

class pythonMission;
class PythonMissionBaseClass;
#endif //VS_MIS_SEL

class Mission
{
public:
    enum MISSION_AUTO
    {
        AUTO_OFF = -1,
        AUTO_NORMAL = 0,
        AUTO_ON = 1
    };
    unsigned int player_num;
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
    Unit *call_unit_launch(class CreateFlightgroup *fg, int type /*clsptr type*/, const std::string &destinations);

    Mission(const char *configfile, bool loadscripts = true);
    Mission(const char *filename, const std::string &pythonscript, bool loadscripts = true);
    std::string Pickle();               //returns filename\npickleddata
    void UnPickle(std::string pickled); //takes in pickeddata
    void AddFlightgroup(Flightgroup *fg);
    void initMission(bool loadscripts = true);

    int getPlayerMissionNumber(); //-1 if not found or invalid player_num.
    static Mission *getNthPlayerMission(int cp, int num);

    ///alex Please help me make this function...this is called between mission loops
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

#endif //VS_MIS_SEL

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
        vector<missionThread *> threads;
        PythonMissionBaseClass *pymissions;
        vsUMap<string, missionNode *> modules;
        int thread_nr;
        missionThread *cur_thread;
        vsUMap<string, missionNode *> global_variables;
        varInstVec global_varvec;
        //vector<const void *()> callbacks;
    } runtime;

private:
    friend void UnpickleMission(std::string pickled);

    void initTagMap();

#endif //VS_MIS_SEL

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
