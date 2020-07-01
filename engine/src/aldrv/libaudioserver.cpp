#include "aldrv/audiolib.h"
#include "gfx/cockpit_generic.h"
#include <string>

void AUDAdjustSound(int32_t i, QVector const &qv, Vector const &vv)
{
}

bool AUDIsPlaying(int32_t snd)
{
    return false;
}
void AUDSoundGain(int32_t snd, float howmuch, bool)
{
}
void AUDRefreshSounds()
{
}
int32_t AUDCreateSoundWAV(const std::string &, const bool LOOP)
{
    return -1;
}
int32_t AUDCreateSoundMP3(const std::string &, const bool LOOP)
{
    return -1;
}
int32_t AUDCreateSound(int32_t sound, const bool LOOP)
{
    return -1;
}
int32_t AUDCreateSound(const std::string &, const bool LOOP)
{
    return -1;
}
void AUDStartPlaying(int32_t i)
{
}
void AUDStopPlaying(int32_t i)
{
}
void AUDDeleteSound(int32_t i)
{
}
void AUDDeleteSound(int32_t i, bool b)
{
}
void AUDPlay(const int32_t &sound, const QVector &pos, const Vector &vel, const float gain)
{
}
QVector AUDListenerLocation()
{
    return QVector(0, 0, 0);
}

// From communication_xml.cpp
int32_t createSound(std::string file, bool val)
{
    return -1;
}

// soundContainer::~soundContainer () {}
