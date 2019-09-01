//============================================================================
//
//				CNote.h
//
//============================================================================
#include "CNote.h"
//============================================================================
//				Static variables
//============================================================================
float *CNote::pnotetab = 0;
int CNote::numcents = 0;
int CNote::maxcent = 0;
//============================================================================
//				Init
//============================================================================
void CNote::Init(int ncents, float stf, float sps)
{
	if (!ncents || (stf == 0.0f) || (sps == 0.0f))
		return;
	Destroy();
	//
	numcents = ncents;
	maxcent = numcents * 120;
	pnotetab = new float[maxcent];
	//
	double cent;
	for (int i = 0; i < maxcent; i++) {
		cent = (double) i / (double) numcents - 69.0;
		pnotetab[i] = stf * 440.0f * (float) pow(2.0, cent / 12.0) / sps;
	}
}
//============================================================================
//				Destroy
//============================================================================
void CNote::Destroy()
{
	if (pnotetab) {
		delete[] pnotetab;
		pnotetab = 0;
	}
}
