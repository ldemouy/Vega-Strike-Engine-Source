///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for random generators.
 *	\file		IceRandom.h
 *	\author		Pierre Terdiman
 *	\date		August, 9, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERANDOM_H__
#define __ICERANDOM_H__

void SRand(udword seed);
udword Rand();

//! Returns a unit random floating-point value
inline float UnitRandomFloat() { return float(Rand()) * ONE_OVER_RAND_MAX; }

//! Returns a random index so that 0<= index < max_index
udword GetRandomIndex(udword max_index);

class BasicRandom
{
public:
	//! Constructor
	inline BasicRandom(udword seed = 0) : mRnd(seed) {}
	//! Destructor
	inline ~BasicRandom() {}

	inline void SetSeed(udword seed) { mRnd = seed; }
	inline udword GetCurrentValue() const { return mRnd; }
	inline udword Randomize()
	{
		mRnd = mRnd * 2147001325 + 715136305;
		return mRnd;
	}

private:
	udword mRnd;
};

#endif // __ICERANDOM_H__
