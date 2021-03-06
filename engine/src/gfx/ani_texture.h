#ifndef __ANI_TEXTURE_H__
#define __ANI_TEXTURE_H__

#include "aux_texture.h"
#include "vid_file.h"
#include "vsfilesystem.h"

#include "../SharedPool.h"
#include <memory>
#include <stdio.h>

#include "audio/Source.h"
#include "audio/Types.h"

class AnimatedTexture : public Texture
{
    Texture **Decal;
    uint32_t activebound; // For video mode
    double physicsactive;
    bool loadSuccess;
    void AniInit();

    // For video mode
    bool vidMode;
    bool detailTex;
    enum FILTER ismipmapped;
    int32_t texstage;
    std::shared_ptr<Audio::Source> timeSource;

    vector<StringPool::Reference> frames; // Filenames for each frame
    vector<Vector> frames_maxtc;          // Maximum tcoords for each frame
    vector<Vector> frames_mintc;          // Minimum tcoords for each frame

    VidFile *vidSource;

    StringPool::Reference wrapper_file_path;
    VSFileSystem::VSFileType wrapper_file_type;

    // Options
    enum optionenum
    {
        optInterpolateFrames = 0x01,
        optInterpolateTCoord = 0x02,
        optLoopInterp = 0x04,
        optLoop = 0x08,
        optSoundTiming = 0x10
    };
    uint8_t options;

    // Implementation
    GFXColor multipass_interp_basecolor;
    enum ADDRESSMODE defaultAddressMode; // default texture address mode, read from .ani

  protected:
    uint32_t numframes;
    float timeperframe;
    uint32_t active;
    uint32_t nextactive;   // It is computable, but it's much more convenient this way
    float active_fraction; // For interpolated animations
    double curtime;

    // for video de-jittering
    double lastcurtime;
    double lastrealtime;

    bool constframerate;
    bool done;

  public:
    virtual void setTime(double tim);

    virtual double curTime() const
    {
        return curtime;
    }

    virtual uint32_t numFrames() const
    {
        return numframes;
    }

    virtual float framesPerSecond() const
    {
        return 1 / timeperframe;
    }

    virtual uint32_t numLayers() const;

    virtual uint32_t numPasses() const;

    virtual bool canMultiPass() const
    {
        return true;
    }

    virtual bool constFrameRate() const
    {
        return constframerate;
    }

    AnimatedTexture();
    AnimatedTexture(int32_t stage, enum FILTER imm, bool detailtexture = false);
    AnimatedTexture(const char *file, int32_t stage, enum FILTER imm, bool detailtexture = false);
    AnimatedTexture(VSFileSystem::VSFile &openedfile, int32_t stage, enum FILTER imm, bool detailtexture = false);

    void Load(VSFileSystem::VSFile &f, int32_t stage, enum FILTER ismipmapped, bool detailtex = false);
    void LoadAni(VSFileSystem::VSFile &f, int32_t stage, enum FILTER ismipmapped, bool detailtex = false);
    void LoadVideoSource(VSFileSystem::VSFile &f);

    virtual void LoadFrame(int32_t num); // For video mode

    void Destroy();

    virtual const Texture *Original() const;
    virtual Texture *Original();
    ~AnimatedTexture();
    virtual Texture *Clone();

    virtual void MakeActive()
    {
        MakeActive(texstage, 0);
    } // MSVC bug seems to hide MakeActive() if we define MakeActive(int,int) - the suckers!

    virtual void MakeActive(int32_t stage)
    {
        MakeActive(stage, 0);
    } // MSVC bug seems to hide MakeActive(int) if we define MakeActive(int,int) - the suckers!

    virtual void MakeActive(int32_t stage, int32_t pass);

    bool SetupPass(int32_t pass, int32_t stage, const enum BLENDFUNC src, const enum BLENDFUNC dst);

    bool SetupPass(int32_t pass, const enum BLENDFUNC src, const enum BLENDFUNC dst)
    {
        return SetupPass(pass, texstage, src, dst);
    }

    void SetInterpolateFrames(bool set)
    {
        options = (options & ~optInterpolateFrames) | (set ? optInterpolateFrames : 0);
    }

    void SetInterpolateTCoord(bool set)
    {
        options = (options & ~optInterpolateTCoord) | (set ? optInterpolateTCoord : 0);
    }

    void SetLoopInterp(bool set)
    {
        options = (options & ~optLoopInterp) | (set ? optLoopInterp : 0);
    }

    void SetLoop(bool set)
    {
        options = (options & ~optLoop) | (set ? optLoop : 0);
    }

    bool GetInterpolateFrames() const
    {
        return (options & optInterpolateFrames) != 0;
    }

    bool GetInterpolateTCoord() const
    {
        return (options & optInterpolateTCoord) != 0;
    }

    bool GetLoopInterp() const
    {
        return (options & optLoopInterp) != 0;
    }

    bool GetLoop() const
    {
        return (options & optLoop) != 0;
    }

    std::shared_ptr<Audio::Source> GetTimeSource() const
    {
        return (options & optSoundTiming) ? timeSource : std::shared_ptr<Audio::Source>();
    }

    void SetTimeSource(std::shared_ptr<Audio::Source> source);

    void ClearTimeSource();

    static void UpdateAllPhysics();
    static void UpdateAllFrame();

    // resets the animation to beginning
    void Reset();
    bool Done() const;

    virtual bool LoadSuccess();

    // Some useful factory methods -- also defined in ani_texture.cpp
    static AnimatedTexture *CreateVideoTexture(const std::string &fname, int32_t stage = 0,
                                               enum FILTER ismipmapped = BILINEAR, bool detailtex = false);
};

#endif
