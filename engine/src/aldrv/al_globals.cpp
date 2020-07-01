#include "al_globals.h"
#include "gfx/vec.h"
#include "hashtable.h"
#ifdef HAVE_AL
Hashtable<std::string, ALuint, 127> soundHash;
unsigned int maxallowedsingle = 10;
unsigned int maxallowedtotal = 40;
float scalepos;
bool usedoppler = false;
bool usepositional = true;
float scalevel;
std::vector<ALuint> unusedsrcs;
#endif
