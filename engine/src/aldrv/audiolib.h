#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_
#include "gfx/vec.h"
#include <string>
bool AUDInit();
void AUDDestroy();
/// Sets the size in which all sounds are going to be played
void AUDListenerSize(const float &size);
void AUDListener(const QVector &pos, const Vector &vel);
QVector AUDListenerLocation();
/// Checks if sounds are still playing
void AUDRefreshSounds();
/// Will the sound be played
void AUDListenerOrientation(const Vector &i, const Vector &j, const Vector &k);
void AUDListenerGain(const float &gain);
float AUDGetListenerGain();
/// creates a buffer if one doesn't already exists, and then creates a source
int32_t AUDCreateSoundWAV(const std::string &, const bool LOOP = false);
/// creates a buffer for an mp3 sound if one doesn't already exist, then creates a source
int32_t AUDCreateSoundMP3(const std::string &, const bool LOOP = false);
/// copies other sound loaded through AUDCreateSound
int32_t AUDCreateSound(int32_t sound, const bool LOOP = false);
/// guesses the type of sound by extension
int32_t AUDCreateSound(const std::string &, const bool LOOP = false);

void AUDStopAllSounds(int32_t except_this_one = -1);
int32_t AUDHighestSoundPlaying();
/// deletes a given sound
void AUDDeleteSound(int32_t sound, bool music = false);
/// Changes the velocity and/or position of a given sound
void AUDAdjustSound(const int32_t &sound, const QVector &pos, const Vector &vel);
/// Setup the sound as a streaming source (this means right now only that it doesn't do 3D positional stuff)
void AUDStreamingSound(const int32_t &sound);
/// Changes the gain of a loaded sound
void AUDSoundGain(int32_t sound, float gain, bool music = false);
/// Is a loaded sound still playing
bool AUDIsPlaying(const int32_t &sound);
/// Stops a loaded sound
void AUDStopPlaying(const int32_t &sound);
/// Plays a loaded sound
void AUDStartPlaying(const int32_t &sound);
/// Queries if the sound should be culled. If not, plays
void AUDPlay(const int32_t &sound, const QVector &pos, const Vector &vel, const float gain);
/// Changes the volume (generally 0 or between 1 and 1000)
void AUDChangeVolume(float volume);
float AUDGetVolume();
/// changes the scale used for doppler...generally between 0 for off or .01 and 10
void AUDChangeDoppler(float doppler);
/// Gets current doppler val
float AUDGetDoppler();
//#define PERFRAMESOUND
#endif
