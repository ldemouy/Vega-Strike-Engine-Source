///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple pair class.
 *	\file		IcePairs.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPAIRS_H__
#define __ICEPAIRS_H__

//! A generic couple structure
struct Pair
{
	inline Pair() {}
	inline Pair(uint32_t i0, uint32_t i1) : id0(i0), id1(i1) {}

	uint32_t id0; //!< First index of the pair
	uint32_t id1; //!< Second index of the pair
};

class Pairs : private Container
{
public:
	// Constructor / Destructor
	Pairs() {}
	~Pairs() {}

	inline uint32_t GetNbPairs() const { return GetNbEntries() >> 1; }
	inline const Pair *GetPairs() const { return (const Pair *)GetEntries(); }
	inline const Pair *GetPair(uint32_t i) const { return (const Pair *)&GetEntries()[i + i]; }

	inline bool HasPairs() const { return IsNotEmpty(); }

	inline void ResetPairs() { Reset(); }
	inline void DeleteLastPair()
	{
		DeleteLastEntry();
		DeleteLastEntry();
	}

	inline void AddPair(const Pair &p) { Add(p.id0).Add(p.id1); }
	inline void AddPair(uint32_t id0, uint32_t id1) { Add(id0).Add(id1); }
};

#endif // __ICEPAIRS_H__
