//
// C++ Interface: Audio::SourceTemplate
//
#ifndef __AUDIO_SOURCETEMPLATE_H__INCLUDED__
#define __AUDIO_SOURCETEMPLATE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include <string>

#include "vsfilesystem.h"

namespace Audio
{

/**
 * SourceTemplate class
 *
 * @remarks This class holds enough information to spawn new sources
 *      independent of the SceneManager or underlying implementation.
 *      @par The TemplateManager may be used to construct them out of external
 *      resources.
 *      @par They are better suited for aggregation into entities that might
 *      spawn active sources, templates are optimal inactive sources as they do not
 *      contain usually bulky state information required for active ones. SceneManagers
 *      know how to derive a Source out of a SourceTemplate, so they can be converted
 *      into those at a moment's notice.
 *      @par Furthermore, since they do not require any state information, source templates
 *      may be shared among multiple entities, further reducing overhead.
 * @see TemplateManager
 *
 */
class SourceTemplate
{
  private:
    std::string soundName;
    VSFileSystem::VSFileType soundType;

    Range<Scalar> cosAngleRange;

    PerFrequency<Scalar> pfRadiusRatios;
    PerFrequency<Scalar> referenceFreqs;

    Scalar gain;

    struct
    {
        int32_t looping : 1;
        int32_t streaming : 1;
        int32_t attenuated : 1;
        int32_t relative : 1;
    } flags;

  public:
    /** Construct a template
     * @see SceneManager::createSource, Renderer::getSound,
     */
    SourceTemplate(const std::string &soundName, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile,
                   bool looping = false);

    ~SourceTemplate();

    /** Get the sound resource name */
    std::string getSoundName() const
    {
        return soundName;
    }

    /** Set the sound resource name */
    void setSoundName(const std::string &newName)
    {
        soundName = newName;
    }

    /** Get the sound file type */
    VSFileSystem::VSFileType getSoundType() const
    {
        return soundType;
    }

    /** Set the sound file type */
    void setSoundType(VSFileSystem::VSFileType type)
    {
        soundType = type;
    }

    /** Return the source's minimum/maximum propagation angle
     * @remarks Sound will fully propagate in directions within minimum
     *      directional drift. Further drift will attenuate the sound
     *      until it (practically) disappears by the maximum propagation angle.
     */
    Range<Scalar> getAngleRange() const;

    /** @see getAngleRange */
    void setAngleRange(Range<Scalar> r);

    /** @see getAngleRange @remarks This version returns cosine-angles rather than radians, much quicker */
    Range<Scalar> getCosAngleRange() const
    {
        return cosAngleRange;
    }

    /** @see getAngleRange @remarks This version takes cosine-angles rather than radians, much quicker */
    void setCosAngleRange(Range<Scalar> r)
    {
        cosAngleRange = r;
    }

    /** Get the source's frequency-dependant radius ratios
     * @remarks Sound propagation goes different for low and high frequencies than
     *      for others. Sound creation does too - this specifies the ratio of a source's
     *      volume that is generating high/low frequency vibrations. This will affect
     *      propagation of those frequencies over distance.
     */
    PerFrequency<Scalar> getPerFrequencyRadiusRatios() const
    {
        return pfRadiusRatios;
    }

    /** Set the source's frequency-dependant radius ratios
     * @see getRadiusRatios
     */
    void setPerFrequencyRadiusRatios(PerFrequency<Scalar> val)
    {
        pfRadiusRatios = val;
    }

    /** Get the source's refernece frequencies */
    PerFrequency<Scalar> getReferenceFreqs() const
    {
        return referenceFreqs;
    }

    /** Set the source's reference frequencies */
    void setReferenceFreqs(PerFrequency<Scalar> val)
    {
        referenceFreqs = val;
    }

    /** Get the source's main gain */
    Scalar getGain() const
    {
        return gain;
    }

    /** Set the source's main gain */
    void setGain(Scalar g)
    {
        gain = g;
    }

    /** Is the source in looping mode? */
    bool isLooping() const
    {
        return flags.looping != 0;
    }

    /** Set the source's looping mode */
    void setLooping(bool loop)
    {
        flags.looping = loop ? 1 : 0;
    }

    /** Is the source's position always relative to the root listener?
     * @see Source::isRelative()
     */
    bool isRelative() const
    {
        return flags.relative != 0;
    }

    /** Set whether the source's position is always relative to the root listener */
    void setRelative(bool relative)
    {
        flags.relative = relative ? 1 : 0;
    }

    /** Is the source using distance attenuation? */
    bool isAttenuated() const
    {
        return flags.attenuated != 0;
    }

    /** Set whether the source will use distance attenuation */
    void setAttenuated(bool loop)
    {
        flags.attenuated = (loop ? 1 : 0);
    }

    /** Is the source in streaming mode? */
    bool isStreaming() const
    {
        return flags.streaming != 0;
    }

    /** Set the source's streaming mode */
    void setStreaming(bool stream)
    {
        flags.streaming = stream ? 1 : 0;
    }
};

}; // namespace Audio

#endif //__AUDIO_SOURCETEMPLATE_H__INCLUDED__
