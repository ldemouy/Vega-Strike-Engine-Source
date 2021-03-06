///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple container class.
 *	\file		IceContainer.h
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICECONTAINER_H__
#define __ICECONTAINER_H__

#define CONTAINER_STATS
#include "IceMemoryMacros.h"
#include <stdint.h>

class Container
{
  public:
    // Constructor / Destructor
    Container();
    Container(const Container &object);
    ~Container();
    // Management
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	A O(1) method to add a value in the container. The container is automatically resized if needed.
     *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since
     *the resizing operation costs a lot more than the call overhead...
     *
     *	\param		entry		[in] a uint32_t to store in the container
     *	\see		Add(float entry)
     *	\see		Empty()
     *	\see		Contains(uint32_t entry)
     *	\return		Self-Reference
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline Container &Add(uint32_t entry)
    {
        // Resize if needed
        if (mCurNbEntries == mMaxNbEntries)
        {
            Resize();
        }

        // Add new entry
        mEntries[mCurNbEntries++] = entry;
        return *this;
    }

    inline Container &Add(const uint32_t *entries, uint32_t nb)
    {
        // Resize if needed
        if (mCurNbEntries + nb > mMaxNbEntries)
        {
            Resize(nb);
        }

        // Add new entry
        CopyMemory(&mEntries[mCurNbEntries], entries, nb * sizeof(uint32_t));
        mCurNbEntries += nb;
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Clears the container. All stored values are deleted, and it frees used ram.
     *	\see		Reset()
     *	\return		Self-Reference
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Container &Empty();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Resets the container. Stored values are discarded but the buffer is kept so that further calls don't need
     *resizing again. That's a kind of temporal coherence. \see		Empty()
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void Reset()
    {
        // Avoid the write if possible
        // ### CMOV
        if (mCurNbEntries)
            mCurNbEntries = 0;
    }

    // Data access.
    inline uint32_t GetNbEntries() const
    {
        return mCurNbEntries;
    } //!< Returns the current number of entries.
    inline uint32_t GetEntry(uint32_t i) const
    {
        return mEntries[i];
    } //!< Returns ith entry
    inline uint32_t *GetEntries() const
    {
        return mEntries;
    } //!< Returns the list of entries.

    // Stats
    uint32_t GetUsedRam() const;

#ifdef CONTAINER_STATS
  private:
    static uint32_t mNbContainers; //!< Number of containers around

    static uint32_t mUsedRam; //!< Amount of bytes used by containers in the system

#endif
  private:
    // Resizing
    bool Resize(uint32_t needed = 1);
    // Data
    uint32_t mMaxNbEntries; //!< Maximum possible number of entries
    uint32_t mCurNbEntries; //!< Current number of entries
    uint32_t *mEntries;     //!< List of entries
    float mGrowthFactor;    //!< Resize: new number of entries = old number * mGrowthFactor
};

#endif // __ICECONTAINER_H__
