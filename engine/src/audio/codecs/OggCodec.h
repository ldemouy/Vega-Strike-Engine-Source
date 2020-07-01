//
// C++ Interface: Audio::OggCodec
//
#ifndef __AUDIO_OGGCODEC_H__INCLUDED__
#define __AUDIO_OGGCODEC_H__INCLUDED__

#include "Codec.h"

namespace Audio
{

/**
 * OggCodec factory class, for Ogg audio streams.
 * @see CodecRegistry to create OggCodec instances.
 */
class OggCodec : public Codec
{
  public:
    OggCodec();

    virtual ~OggCodec();

    /** @see Codec::getExtensions */
    virtual const Extensions *getExtensions() const;

    /** @see Codec::canHandle */
    virtual bool canHandle(const std::string &path, bool canOpen,
                           VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);

    /** @see Codec::open */
    virtual Stream *open(const std::string &path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);
};

}; // namespace Audio

#endif //__AUDIO_OGGCODEC_H__INCLUDED__
