//
// C++ Interface: Audio::OpenALRenderableSource
//
#ifndef __AUDIO_OPENALRENDERABLESOURCE_H__INCLUDED__
#define __AUDIO_OPENALRENDERABLESOURCE_H__INCLUDED__

#include "al.h"

#include "../../RenderableSource.h"

#include "../../Exceptions.h"
#include "../../Types.h"

namespace Audio
{

    /**
     * OpenAL Renderable Source class
     *
     * @remarks This class implements the RenderableSource interface for the
     *      OpenAL renderer.
     *
     */
    class OpenALRenderableSource : public RenderableSource
    {
        ALuint alSource;
        bool alBuffersAttached;

    public:
        OpenALRenderableSource(Source *source);

        virtual ~OpenALRenderableSource();

    protected:
        /** @see RenderableSource::startPlayingImpl. */
        virtual void startPlayingImpl(Timestamp start);

        /** @see RenderableSource::stopPlayingImpl. */
        virtual void stopPlayingImpl();

        /** @see RenderableSource::isPlayingImpl. */
        virtual bool isPlayingImpl() const;

        /** @see RenderableSource::getPlayingTimeImpl. */
        virtual Timestamp getPlayingTimeImpl() const;

        /** @see RenderableSource::updateImpl. */
        virtual void updateImpl(int flags, const Listener &sceneListener);

        /** @see RenderableSource::seekImpl. */
        virtual void seekImpl(Timestamp time);

        /** Derived classes may use the underlying AL source handle to set additional attributes */
        ALuint getALSource() const { return alSource; }

        /** Attach AL buffers from the source's AL sound, if not attached already.
         * @note It will fail with an assertion if the attached sound isn't an OpenAL sound.
         */
        void attachALBuffers();
    };

}; // namespace Audio

#endif //__AUDIO_OPENALRENDERABLESOURCE_H__INCLUDED__
