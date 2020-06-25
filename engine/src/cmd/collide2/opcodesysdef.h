#ifndef CS_COMPAT_H_
#define CS_COMPAT_H_

//#define OPC_USE_CALLBACKS 1

#define ICE_NO_DLL
#define CS_PROCESSOR_X86
#define CS_NO_QSQRT

#include "gfx/quaternion.h"
#define SMALL_EPSILON .000001
#define EPSILON .00001
#define __CS_CSSYSDEFS_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

class csObject
{
};
struct iBase
{
};

#define CS_ASSERT assert

#include "opcodetypes.h"
#endif
