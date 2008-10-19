// -*- mode:c++; indent-tabs-mode:t -*-
#include "CombFilter.hpp"
#include <diversalis/diversalis.hpp>
#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
	#include <xmmintrin.h>
#endif
#include <cstdlib>
#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	extern "C" int posix_memalign(void **, std::size_t, std::size_t) throw();
#endif

CCombFilter::CCombFilter()
{
#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
	leftBuffer = static_cast<float*>(_aligned_malloc(MAX_COMB_DELAY*sizeof(float),16));
	rightBuffer = static_cast<float*>(_aligned_malloc(MAX_COMB_DELAY*sizeof(float),16));
#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__GNU
	///\todo mingw
	leftBuffer= new float[MAX_COMB_DELAY];
	rightBuffer = new float[MAX_COMB_DELAY];
#elif defined DIVERSALIS__PROCESSOR__X86 &&  defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	posix_memalign(reinterpret_cast<void**>(&leftBuffer),16,MAX_COMB_DELAY*sizeof(float));
	posix_memalign(reinterpret_cast<void**>(&rightBuffer),16,MAX_COMB_DELAY*sizeof(float));
#else
	leftBuffer= new float[MAX_COMB_DELAY];
	rightBuffer = new float[MAX_COMB_DELAY];
#endif

	Counter=MAX_COMB_DELAY-4;
}

CCombFilter::~CCombFilter() throw()
{
#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
	_aligned_free(static_cast<void*>(leftBuffer));
	_aligned_free(static_cast<void*>(rightBuffer));
#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__GNU
	free(leftBuffer);
	free(rightBuffer);
#else
	delete [] leftBuffer;
	delete [] rightBuffer;
#endif

}

void CCombFilter::Clear()
{
	for(int c=0;c<MAX_COMB_DELAY;c++)
	{
		leftBuffer[c]=0;
		rightBuffer[c]=0;
	}

	left_output=0;
	right_output=0;
}

void CCombFilter::Initialize(int time, int stph)
{
	l_delayedCounter=Counter-time;
	r_delayedCounter=l_delayedCounter-stph;
	
	if(l_delayedCounter<0)l_delayedCounter=0;
	if(r_delayedCounter<0)r_delayedCounter=0;
}


