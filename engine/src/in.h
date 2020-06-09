#ifndef IN_H
#define IN_H
#include <stdint.h>
enum KBSTATE
{
    UP,
    DOWN,
    PRESS,
    RELEASE,
    RESET
};

class KBData;
typedef void (*KBHandler)(const KBData &, KBSTATE);

typedef void (*MouseHandler)(KBSTATE, int32_t x, int32_t y, int32_t delx, int32_t dely, int32_t mod);

#endif
