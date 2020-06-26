///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article "Radix Sort Revisited".
 *	\file		IceRevisitedRadix.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERADIXSORT_H__
#define __ICERADIXSORT_H__
#include <stdint.h>

//! Allocate histograms & offsets locally
#define RADIX_LOCAL_RAM

enum RadixHint
{
	RADIX_SIGNED,	//!< Input values are signed
	RADIX_UNSIGNED, //!< Input values are unsigned

	RADIX_FORCE_DWORD = 0x7fffffff
};

class RadixSort
{
public:
	// Constructor/Destructor
	RadixSort();
	~RadixSort();
	// Sorting methods
	RadixSort &Sort(const uint32_t *input, uint32_t nb, RadixHint hint = RADIX_SIGNED);
	RadixSort &Sort(const float *input, uint32_t nb);

	//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
	inline const uint32_t *GetRanks() const { return mRanks; }

	//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
	inline uint32_t *GetRecyclable() const { return mRanks2; }

	// Stats
	uint32_t GetUsedRam() const;
	//! Returns the total number of calls to the radix sorter.
	inline uint32_t GetNbTotalCalls() const { return mTotalCalls; }
	//! Returns the number of eraly exits due to temporal coherence.
	inline uint32_t GetNbHits() const { return mNbHits; }

private:
#ifndef RADIX_LOCAL_RAM
	uint32_t *mHistogram; //!< Counters for each byte
	uint32_t *mOffset;	  //!< Offsets (nearly a cumulative distribution function)
#endif
	uint32_t mCurrentSize; //!< Current size of the indices list
	uint32_t *mRanks;	   //!< Two lists, swapped each pass
	uint32_t *mRanks2;
	// Stats
	uint32_t mTotalCalls; //!< Total number of calls to the sort routine
	uint32_t mNbHits;	  //!< Number of early exits due to coherence
						  // Internal methods
	void CheckResize(uint32_t nb);
	bool Resize(uint32_t nb);
};

#endif // __ICERADIXSORT_H__
