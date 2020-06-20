//
//C++ Implementation: vid_file
//

#include "vid_file.h"
#include "vsfilesystem.h"

#include <string.h>
#include <math.h>
#include <utility>

//define a 128k buffer for video streamers
#define BUFFER_SIZE (128 * (1 << 10))

#ifndef ENOENT
#define ENOENT (2)
#endif
#include <sys/types.h>

class VidFileImpl
{
private:
    VidFileImpl(size_t, bool) {}

public:
    //Avoid having to put ifdef's everywhere.
    float frameRate, duration;
    int width, height;
    void *frameBuffer;
    int frameBufferStride;
    bool seek(float time)
    {
        return false;
    }
};

VidFile::VidFile() : impl(NULL)
{
}

VidFile::~VidFile()
{
    if (impl)
        delete impl;
}

bool VidFile::isOpen() const
{
    return impl != NULL;
}

void VidFile::open(const std::string &path, size_t maxDimension, bool forcePOT)
{
}

void VidFile::close()
{
    if (impl)
    {
        delete impl;
        impl = 0;
    }
}

float VidFile::getFrameRate() const
{
    return impl ? impl->frameRate : 0;
}

float VidFile::getDuration() const
{
    return impl ? impl->duration : 0;
}

int VidFile::getWidth() const
{
    return impl ? impl->width : 0;
}

int VidFile::getHeight() const
{
    return impl ? impl->height : 0;
}

void *VidFile::getFrameBuffer() const
{
    return impl ? impl->frameBuffer : 0;
}

int VidFile::getFrameBufferStride() const
{
    return impl ? impl->frameBufferStride : 0;
}

bool VidFile::seek(float time)
{
    return (impl != 0) && impl->seek(time);
}
