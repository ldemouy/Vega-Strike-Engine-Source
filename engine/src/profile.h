#ifdef WIN32
#include <time.h>
#else

#include <sys/time.h>
#include <unistd.h>

#define RESETTIME() startTime()
#define REPORTTIME(comment) endTime(comment, __FILE__, __LINE__)

static timeval start;
static inline void startTime()
{
    gettimeofday(&start, nullptr);
}

static inline void endTime(const char *comment, const char *file, int lineno)
{
    timeval end;
    gettimeofday(&end, nullptr);
    double time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
    std::clog << file << "(" << comment << "):" << lineno << ": " << time << std::endl;
}
#endif
