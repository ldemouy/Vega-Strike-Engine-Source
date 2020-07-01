//
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_OPENAL_RENDERER_H__INCLUDED__
#define __AUDIO_OPENAL_RENDERER_H__INCLUDED__

#include "../../Exceptions.h"
#include "../../Format.h"
#include "../../Renderer.h"
#include "../../Types.h"

#include <memory>

namespace Audio
{

namespace __impl
{

namespace OpenAL
{
// Forward declaration of internal renderer data
struct RendererData;
}; // namespace OpenAL

}; // namespace __impl

/**
 * OpenAL Renderer implementation
 *
 * @remarks Audio renderer implementation based on OpenAL.
 *
 */
class OpenALRenderer : public Renderer
{
  protected:
    std::shared_ptr<__impl::OpenAL::RendererData> data;

  public:
    /** Initialize the renderer with default or config-driven settings. */
    OpenALRenderer();

    virtual ~OpenALRenderer();

    /** @copydoc Renderer::getSound */
    virtual std::shared_ptr<Sound> getSound(const std::string &name,
                                            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile,
                                            bool streaming = false);

    /** @copydoc Renderer::owns */
    virtual bool owns(std::shared_ptr<Sound> sound);

    /** @copydoc Renderer::attach(std::shared_ptr<Source>) */
    virtual void attach(std::shared_ptr<Source> source);

    /** @copydoc Renderer::attach(std::shared_ptr<Listener>) */
    virtual void attach(std::shared_ptr<Listener> listener);

    /** @copydoc Renderer::detach(std::shared_ptr<Source>) */
    virtual void detach(std::shared_ptr<Source> source);

    /** @copydoc Renderer::detach(std::shared_ptr<Listener>) */
    virtual void detach(std::shared_ptr<Listener> listener);

    /** @copydoc Renderer::setMeterDistance */
    virtual void setMeterDistance(Scalar distance);

    /** @copydoc Renderer::setDopplerFactor */
    virtual void setDopplerFactor(Scalar factor);

    /** @copydoc Renderer::setOutputFormat */
    virtual void setOutputFormat(const Format &format);

    /** @copydoc Renderer::beginTransaction */
    virtual void beginTransaction();

    /** @copydoc Renderer::commitTransaction */
    virtual void commitTransaction();

  protected:
    /** Makes sure the AL context is valid, creating one if necessary */
    virtual void checkContext();

    /** Sets expected defaults into the context */
    virtual void initContext();

    /** Sets doppler effect globals into the context */
    void setupDopplerEffect();
};

}; // namespace Audio

#endif //__AUDIO_OPENAL_RENDERER_H__INCLUDED__
