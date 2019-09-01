//============================================================================
//
//				CNote.h
//
//============================================================================
#pragma once
#include <math.h>
//============================================================================
//
//				Class
//
//============================================================================
class CNote
{
private:
	static float				*pnotetab;
	static int								numcents;
	static int								maxcent;
public:
	static void								Init(int ncents, float stf, float sps);
	static void								Destroy();
	static inline int GetCent(int oct, int semi, int cent)
	{
		return (oct * 12 + semi) * numcents + cent;
	}
	static inline float GetCentFreq(int cent)
	{
		if (cent < 0)
			cent = 0;
		if (cent >= maxcent)
			cent = maxcent - 1;
		return pnotetab[cent];
	}
};
