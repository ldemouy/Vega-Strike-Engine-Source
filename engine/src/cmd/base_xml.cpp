#include <boost/python/object.hpp>
#include <boost/version.hpp>

#include "base.h"
#include "base_util.h"
#include "python/python_class.h"
#include "vsfilesystem.h"
#include <Python.h>
#include <math.h>
#include <string>
#include <vector>

static FILE *withAndWithout(std::string filename, std::string time_of_day_hint)
{
    string with(filename + "_" + time_of_day_hint + BASE_EXTENSION);
    FILE *fp = VSFileSystem::vs_open(with.c_str(), "r");
    if (!fp)
    {
        string without(filename + BASE_EXTENSION);
        fp = VSFileSystem::vs_open(without.c_str(), "r");
    }
    return fp;
}
static FILE *getFullFile(std::string filename, std::string time_of_day_hint, std::string faction)
{
    FILE *fp = withAndWithout(filename + "_" + faction, time_of_day_hint);
    if (!fp)
    {
        fp = withAndWithout(filename, time_of_day_hint);
    }
    return fp;
}
void BaseInterface::Load(const char *filename, const char *time_of_day_hint, const char *faction)
{

    FILE *inFile = getFullFile(string("bases/") + filename, time_of_day_hint, faction);
    if (!inFile)
    {
        bool planet = false;
        Unit *baseun = this->baseun.GetUnit();
        if (baseun)
        {
            planet = (baseun->isUnit() == PLANETPTR);
        }
        string basestring("bases/unit");
        if (planet)
        {
            basestring = "bases/planet";
        }
        inFile = getFullFile(basestring, time_of_day_hint, faction);
        if (!inFile)
        {
            return;
        }
    }

    // now that we have a FILE * named inFile and a std::string named newfile we can finally begin the python
    string compilefile = string(filename) + time_of_day_hint + string(faction) + BASE_EXTENSION;
    Python::reseterrors();
    PyRun_SimpleFile(inFile, compilefile.c_str());
    Python::reseterrors();
    VSFileSystem::vs_close(inFile);
}
