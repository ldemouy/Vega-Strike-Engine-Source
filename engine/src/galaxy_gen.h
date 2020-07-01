#ifndef _STARSYSGEN_H_
#define _STARSYSGEN_H_
#include <string>
#include <vector>
using std::string;
using std::vector;

/// All the properties from the galaxy in a system.
struct SystemInfo
{
    string sector;
    string name;
    string filename;
    float sunradius;
    float compactness;
    int32_t numstars;
    bool nebulae;
    bool asteroids;
    int32_t numun1;
    int32_t numun2;
    string faction;
    string names;
    string stars;
    string planetlist;
    string smallun;
    string nebulaelist;
    string asteroidslist;
    string ringlist;
    string backgrounds;
    vector<string> jumps;
    int32_t seed;
    bool force;
};

/// appends .system
std::string getStarSystemFileName(const std::string &input);
/// finds the name after all / characters and capitalizes the first letter
std::string getStarSystemName(const std::string &in);
/// finds the name before the first /  this is the sector name
std::string getStarSystemSector(const std::string &in);
string getUniversePath();
void readnames(vector<string> &entity, const char *filename);
void generateStarSystem(SystemInfo &si);
#endif
