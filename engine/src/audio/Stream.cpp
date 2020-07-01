//
// C++ Implementation: Audio::Stream
//

#include "Stream.h"

#include <cstring>
#include <utility>

// using namespace std;

namespace Audio
{

using std::min;

Stream::Stream(const std::string &path)
{
}

Stream::~Stream()
{
}

double Stream::getLength()
{
    return getLengthImpl();
}

double Stream::getPosition() const
{
    return getPositionImpl();
}

void Stream::seek(double position)
{
    seekImpl(position);
}

uint32_t Stream::read(void *buffer, uint32_t bufferSize)
{
    void *rbuffer;
    void *rbufferEnd;
    uint32_t rbufferSize;
    uint32_t rode = 0;

    try
    {
        getBufferImpl(rbuffer, rbufferSize);
    }
    catch (const NoBufferException &)
    {
        nextBufferImpl();
        getBufferImpl(rbuffer, rbufferSize);
        curBufferPos = rbuffer;
    }
    rbufferEnd = ((char *)rbuffer) + rbufferSize;

    while (bufferSize > 0)
    {
        if (!((curBufferPos >= rbuffer) && (curBufferPos < rbufferEnd)))
        {
            nextBufferImpl();
            getBufferImpl(rbuffer, rbufferSize);
            curBufferPos = rbuffer;
            rbufferEnd = ((char *)rbuffer) + rbufferSize;
        }

        size_t remaining =
            min(bufferSize, (uint32_t)((char *)rbufferEnd - (char *)curBufferPos)); // is there no std::ptrdiff?
        memcpy(buffer, curBufferPos, remaining);
        buffer = (void *)((char *)buffer + remaining);
        curBufferPos = (void *)((char *)curBufferPos + remaining);
        bufferSize -= remaining;
        rode += remaining;
    }

    return rode;
}

}; // namespace Audio
