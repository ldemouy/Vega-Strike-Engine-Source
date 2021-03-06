
#ifdef HAVE_PYTHON
#include <Python.h>
#include <boost/python/class.hpp>
#include <boost/version.hpp>
#endif

#include "cmd/ai/order.h"
#include "cmd/unit_generic.h"

#include "configxml.h"
#include "gfx/cockpit_generic.h"

#include "python/python_class.h"

#include "gnuhash.h"
#include "mission.h"
#include "pythonmission.h"
#include "savegame.h"

using std::cerr;
using std::cout;
using std::endl;
PYTHON_INIT_INHERIT_GLOBALS(Director, PythonMissionBaseClass);

float getSaveData(int whichcp, const string &key, unsigned int num)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<float> *ans = &(_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key));
    if (num >= ans->size())
    {
        return 0;
    }
    return (*ans)[num];
}

const vector<float> &getSaveData(int whichcp, const string &key)
{
    static vector<float> empty;
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return empty;
    }
    return _Universe->AccessCockpit(whichcp)->savegame->getMissionData(key);
}

string getSaveString(int whichcp, const string &key, unsigned int num)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return "";
    }
    vector<std::string> *ans = &(_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key));
    if (num >= ans->size())
    {
        return "";
    }
    return (*ans)[num];
}
unsigned int getSaveDataLength(int whichcp, const string &key)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    return _Universe->AccessCockpit(whichcp)->savegame->getMissionDataLength(key);
}
unsigned int getSaveStringLength(int whichcp, const string &key)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    return _Universe->AccessCockpit(whichcp)->savegame->getMissionStringDataLength(key);
}
unsigned int pushSaveData(int whichcp, const string &key, float val)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));

    ans->push_back(val);
    return ans->size() - 1;
}

unsigned int eraseSaveData(int whichcp, const string &key, unsigned int index)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));
    if (index < ans->size())
    {

        ans->erase(ans->begin() + index);
    }
    return ans->size();
}

unsigned int clearSaveData(int whichcp, const string &key)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));
    int ret = ans->size();
    if (!ret)
    {
        return 0;
    }

    ans->clear();
    return ret;
}

unsigned int pushSaveString(int whichcp, const string &key, const string &value)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));

    ans->push_back(std::string(value));
    return ans->size() - 1;
}

void putSaveString(int whichcp, const string &key, unsigned int num, const string &val)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));
    if (num < ans->size())
    {

        (*ans)[num] = val;
    }
}

void putSaveData(int whichcp, const string &key, unsigned int num, float val)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));
    if (num < ans->size())
    {

        (*ans)[num] = val;
    }
}

unsigned int eraseSaveString(int whichcp, const string &key, unsigned int index)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));
    if (index < ans->size())
    {

        ans->erase(ans->begin() + index);
    }
    return ans->size();
}

unsigned int clearSaveString(int whichcp, const string &key)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return 0;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));
    int ret = ans->size();
    if (!ret)
    {
        return 0;
    }

    ans->clear();
    return ret;
}

vector<string> loadStringList(int playernum, const string &mykey)
{
    if (playernum < 0 || (unsigned int)playernum >= _Universe->numPlayers())
    {
        return vector<string>();
    }

    SaveGame *savegame = _Universe->AccessCockpit(playernum)->savegame;
    vector<string> rez;

    const vector<string> &ans = savegame->readMissionStringData(mykey);
    if (ans.size() > 0)
    {
        /* This is the modern way to store string data: as strings */
        rez.reserve(ans.size());
        rez.insert(rez.end(), ans.begin(), ans.end());
    }
    else
    {
        /* This variant loads it from float data, converting to character data.
           It's a legacy variant, highly and unpleasantly convoluted and sub-performant,
           but we must support it to be compatible with old savegames */
        const vector<float> &ans = savegame->readMissionData(mykey);
        int lengt = ans.size();
        rez.reserve(ans.size());
        if (lengt >= 1)
        {
            string curstr;
            int length = (int)ans[0];
            for (int j = 0; j < length && j < lengt; j++)
            {
                char myint = (char)ans[j + 1];
                if (myint != '\0')
                {
                    curstr += myint;
                }
                else
                {
                    rez.push_back(curstr);
                    curstr = "";
                }
            }
        }
    }
    return rez;
}

const vector<string> &getStringList(int playernum, const string &mykey)
{
    if (playernum < 0 || (unsigned int)playernum >= _Universe->numPlayers())
    {
        static const vector<string> empty;
        return empty;
    }

    SaveGame *savegame = _Universe->AccessCockpit(playernum)->savegame;

    /* Should check old-style string lists, but it would defeat the purpose
       of this fast getter. Besides, any functionality that uses this one is
       using the new-style string lists, so we're cool (or ought to be) */
    return savegame->readMissionStringData(mykey);
}

void saveStringList(int playernum, const string &mykey, const vector<string> &names)
{
    if (playernum < 0 || (unsigned int)playernum >= _Universe->numPlayers())
    {
        return;
    }

    SaveGame *savegame = _Universe->AccessCockpit(playernum)->savegame;

    // Erase old-style string lists
    if (savegame->getMissionDataLength(mykey) != 0)
    {
        clearSaveData(playernum, mykey);
    }

    vector<string> &ans = savegame->getMissionStringData(mykey);
    clearSaveString(playernum, mykey);
    for (vector<string>::const_iterator i = names.begin(); i != names.end(); ++i)
    {

        ans.push_back(*i);
    }
}

void saveDataList(int whichcp, const string &key, const vector<float> &values)
{
    if (whichcp < 0 || (unsigned int)whichcp >= _Universe->numPlayers())
    {
        return;
    }

    clearSaveData(whichcp, key);

    vector<float> &ans = _Universe->AccessCockpit(whichcp)->savegame->getMissionData(key);
    for (vector<float>::const_iterator i = values.begin(); i != values.end(); ++i)
    {

        ans.push_back(*i);
    }
}

static float getSaveDataPy(int whichcp, string key, unsigned int num)
{
    return getSaveData(whichcp, key, num);
}

static string getSaveStringPy(int whichcp, string key, unsigned int num)
{
    return getSaveString(whichcp, key, num);
}

static unsigned int getSaveDataLengthPy(int whichcp, string key)
{
    return getSaveDataLength(whichcp, key);
}

static unsigned int getSaveStringLengthPy(int whichcp, string key)
{
    return getSaveStringLength(whichcp, key);
}

static unsigned int pushSaveDataPy(int whichcp, string key, float val)
{
    return pushSaveData(whichcp, key, val);
}

static unsigned int eraseSaveDataPy(int whichcp, string key, unsigned int index)
{
    return eraseSaveData(whichcp, key, index);
}

static unsigned int clearSaveDataPy(int whichcp, string key)
{
    return clearSaveData(whichcp, key);
}

static unsigned int pushSaveStringPy(int whichcp, string key, string value)
{
    return pushSaveString(whichcp, key, value);
}

static void putSaveStringPy(int whichcp, string key, unsigned int num, string val)
{
    putSaveString(whichcp, key, num, val);
}

static void putSaveDataPy(int whichcp, string key, unsigned int num, float val)
{
    putSaveData(whichcp, key, num, val);
}

static unsigned int eraseSaveStringPy(int whichcp, string key, unsigned int index)
{
    return eraseSaveString(whichcp, key, index);
}

static unsigned int clearSaveStringPy(int whichcp, string key)
{
    return clearSaveString(whichcp, key);
}

static vector<string> loadStringListPy(int playernum, string mykey)
{
    return loadStringList(playernum, mykey);
}

static void saveStringListPy(int playernum, string mykey, vector<string> names)
{
    saveStringList(playernum, mykey, names);
}

PYTHON_BEGIN_MODULE(Director)
PYTHON_BEGIN_INHERIT_CLASS(Director, pythonMission, PythonMissionBaseClass, "Mission")
PYTHON_DEFINE_METHOD_DEFAULT(Class, &PythonMissionBaseClass::Pickle, "Pickle", pythonMission::default_Pickle);
PYTHON_DEFINE_METHOD_DEFAULT(Class, &PythonMissionBaseClass::UnPickle, "UnPickle", pythonMission::default_UnPickle);
PYTHON_DEFINE_METHOD_DEFAULT(Class, &PythonMissionBaseClass::Execute, "Execute", pythonMission::default_Execute);
PYTHON_END_CLASS(Director, pythonMission)
PYTHON_DEFINE_GLOBAL(Director, &putSaveDataPy, "putSaveData");
PYTHON_DEFINE_GLOBAL(Director, &pushSaveDataPy, "pushSaveData");
PYTHON_DEFINE_GLOBAL(Director, &eraseSaveDataPy, "eraseSaveData");
PYTHON_DEFINE_GLOBAL(Director, &clearSaveDataPy, "clearSaveData");
PYTHON_DEFINE_GLOBAL(Director, &getSaveDataPy, "getSaveData");
PYTHON_DEFINE_GLOBAL(Director, &getSaveDataLengthPy, "getSaveDataLength");
PYTHON_DEFINE_GLOBAL(Director, &putSaveStringPy, "putSaveString");
PYTHON_DEFINE_GLOBAL(Director, &pushSaveStringPy, "pushSaveString");
PYTHON_DEFINE_GLOBAL(Director, &getSaveStringPy, "getSaveString");
PYTHON_DEFINE_GLOBAL(Director, &getSaveStringLengthPy, "getSaveStringLength");
PYTHON_DEFINE_GLOBAL(Director, &eraseSaveStringPy, "eraseSaveString");
PYTHON_DEFINE_GLOBAL(Director, &clearSaveStringPy, "clearSaveString");
PYTHON_DEFINE_GLOBAL(Director, &loadStringListPy, "loadStringList");
PYTHON_DEFINE_GLOBAL(Director, &saveStringListPy, "saveStringList");
PYTHON_END_MODULE(Director)

void InitDirector()
{
    PyImport_AppendInittab("Director", PYTHON_MODULE_INIT_FUNCTION(Director));
}

void InitDirector2()
{
    Python::reseterrors();
    PYTHON_INIT_MODULE(Director);
    Python::reseterrors();
}
