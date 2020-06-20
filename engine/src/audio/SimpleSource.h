//
// C++ Interface: Audio::SimpleSource
//
#ifndef __AUDIO_SIMPLESOURCE_H__INCLUDED__
#define __AUDIO_SIMPLESOURCE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"

#include "Source.h"
#include "SimpleScene.h"
#include <memory>

namespace Audio
{

    /**
     * SimpleSource implementation of the Source interface for the basic SceneManager
     *
     * @remarks This implementation merely tracks playing state with a boolean, and notifies
     *      the attached scene of any changes. It also keeps track of the scene to which
     *      it is attached through a raw pointer, to avoid circular references.
     *      @par The scene attached must be a SimpleScene derivative, since the scene itself is
     *      responsible for detaching itself when destroyed.
     *
     */
    class SimpleSource : public Source, public SharedFromThis<SimpleSource>
    {
    private:
        bool playing;
        SimpleScene *scene;

    public:
        virtual ~SimpleSource();

        /** Construct a simple source */
        SimpleSource(std::shared_ptr<Sound> sound, bool looping = false);

        /** Notify attachment to a scene */
        void notifySceneAttached(SimpleScene *scene);

        /** Get the scene to which it is attached */
        SimpleScene *getScene() const;

        // The following section contains all the virtual functions that need be implemented
        // by a concrete Sound class. All are protected, so the stream interface is independent
        // of implementations.
    protected:
        /** @copydoc Source::startPlayingImpl */
        virtual void startPlayingImpl(Timestamp start);

        /** @copydoc Source::stopPlayingImpl */
        virtual void stopPlayingImpl();

        /** @copydoc Source::isPlayingImpl*/
        virtual bool isPlayingImpl() const;
    };

}; // namespace Audio

#endif //__AUDIO_SIMPLESOURCE_H__INCLUDED__
