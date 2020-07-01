//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_FORMAT_H__INCLUDED__
#define __AUDIO_FORMAT_H__INCLUDED__
#include <stdint.h>
namespace Audio
{

/**
 * Audio format information class
 *
 */
struct Format
{
    uint32_t sampleFrequency;
    uint8_t bitsPerSample;
    uint8_t channels;
    int32_t signedSamples : 1;
    int32_t nativeOrder : 1;

    Format()
    {
    }

    Format(uint32_t freq, uint8_t bps, uint8_t nch)
        : sampleFrequency(freq), bitsPerSample(bps), channels(nch), signedSamples((bps >= 16) ? 1 : 0), nativeOrder(1)
    {
    }

    uint32_t frameSize() const
    {
        return (bitsPerSample * channels + 7) / 8;
    }

    uint32_t bytesPerSecond() const
    {
        return frameSize() * sampleFrequency;
    }

    bool operator==(const Format &o) const
    {
        return (sampleFrequency == o.sampleFrequency) && (bitsPerSample == o.bitsPerSample) &&
               (channels == o.channels) && (signedSamples == o.signedSamples) && (nativeOrder == o.nativeOrder);
    }

    bool operator!=(const Format &o) const
    {
        return !(*this == o);
    }
};

}; // namespace Audio

#endif //__AUDIO_FORMAT_H__INCLUDED__
