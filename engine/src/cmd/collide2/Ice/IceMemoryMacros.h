///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains all memory macros.
 *	\file		IceMemoryMacros.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEMEMORYMACROS_H__
#define __ICEMEMORYMACROS_H__

#include <cstring>
#include <stdint.h>
#undef ZeroMemory
#undef CopyMemory

//!	Clears a buffer.
//!	\param		addr	[in] buffer address
//!	\param		size	[in] buffer length
//!	\see		FillMemory
//!	\see		StoreDwords
//!	\see		CopyMemory
//!	\see		MoveMemory
inline void ZeroMemory(void *addr, uint32_t size)
{
    memset(addr, 0, size);
}

//!	Copies a buffer.
//!	\param		addr	[in] destination buffer address
//!	\param		addr	[in] source buffer address
//!	\param		size	[in] buffer length
//!	\see		ZeroMemory
//!	\see		FillMemory
//!	\see		StoreDwords
//!	\see		MoveMemory
inline void CopyMemory(void *dest, const void *src, uint32_t size)
{
    memcpy(dest, src, size);
}

#define SIZEOFOBJECT sizeof(*this) //!< Gives the size of current object. Avoid some mistakes (e.g. "sizeof(this)").

#define DELETESINGLE(x)                                                                                                \
    {                                                                                                                  \
        delete x;                                                                                                      \
        x = nullptr;                                                                                                   \
    } //!< Deletes an instance of a class.
#define DELETEARRAY(x)                                                                                                 \
    {                                                                                                                  \
        delete[] x;                                                                                                    \
        x = nullptr;                                                                                                   \
    } //!< Deletes an array.

#ifdef __ICEERROR_H__
#define CHECKALLOC(x)                                                                                                  \
    if (!x)                                                                                                            \
        return SetIceError("Out of memory.", EC_OUT_OF_MEMORY); //!< Standard alloc checking. HANDLE WITH CARE.
#else
#define CHECKALLOC(x)                                                                                                  \
    if (!x)                                                                                                            \
    {                                                                                                                  \
        return false;                                                                                                  \
    }
#endif

#endif // __ICEMEMORYMACROS_H__
