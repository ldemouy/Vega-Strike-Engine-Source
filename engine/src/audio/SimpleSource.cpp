//
// C++ Implementation: Audio::SimpleSource
//

#include "SimpleSource.h"

namespace Audio
{

SimpleSource::~SimpleSource()
{
}

SimpleSource::SimpleSource(std::shared_ptr<Sound> sound, bool looping)
    : Source(sound, looping), playing(false), scene(0)
{
}

void SimpleSource::notifySceneAttached(SimpleScene *scn)
{
    scene = scn;
}

SimpleScene *SimpleSource::getScene() const
{
    return scene;
}

void SimpleSource::startPlayingImpl(Timestamp start)
{
    // If it's playing, must stop and restart - cannot simply play again.
    if (isPlaying())
    {
        stopPlaying();
    }

    setLastKnownPlayingTime(start);
    playing = true;

    if (getScene())
    {
        getScene()->notifySourcePlaying(std::shared_ptr<SimpleSource>(this), true);
    }
}

void SimpleSource::stopPlayingImpl()
{
    if (isPlaying())
    {
        playing = false;

        if (getScene())
        {
            getScene()->notifySourcePlaying(std::shared_ptr<SimpleSource>(this), false);
        }
    }
}

bool SimpleSource::isPlayingImpl() const
{
    return playing;
}

}; // namespace Audio
