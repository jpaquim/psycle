#include <packageneric/pre-compiled.private.hpp>
#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
	#include <xmmintrin.h>
#endif
#include "AllPass.hpp"

CAllPass::CAllPass()
{
#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
	leftBuffer = static_cast<float*>(_aligned_malloc(MAX_ALLPASS_DELAY*sizeof(float),16));
	rightBuffer = static_cast<float*>(_aligned_malloc(MAX_ALLPASS_DELAY*sizeof(float),16));
#elif defined DIVERSALIS__PROCESSOR__X86 &&  defined DIVERSALIS__COMPILER__GNU
	posix_memalign(reinterpret_cast<void**>(&leftBuffer),16,MAX_ALLPASS_DELAY*sizeof(float));
	posix_memalign(reinterpret_cast<void**>(&rightBuffer),16,MAX_ALLPASS_DELAY*sizeof(float));
#else
	leftBuffer= new float[MAX_ALLPASS_DELAY];
	rightBuffer = new float[MAX_ALLPASS_DELAY];
#endif

	smpleft = reinterpret_cast<unsigned int*>(&tmpleft);
	smpright = reinterpret_cast<unsigned int*>(&tmpright);
	Counter=MAX_ALLPASS_DELAY-4;
	l_delayedCounter=0;
	r_delayedCounter=0;
	Clear();
}

CAllPass::~CAllPass() throw()
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

void CAllPass::Clear()
{
	for(int c=0;c<MAX_ALLPASS_DELAY;c++)
	{
		leftBuffer[c]=0;
		rightBuffer[c]=0;
	}
	left_output=0;
	right_output=0;
}
void CAllPass::Initialize(int time, int stph)
{
	l_delayedCounter=Counter-time;
	r_delayedCounter=l_delayedCounter-stph;
	
	if(l_delayedCounter<0)l_delayedCounter+=MAX_ALLPASS_DELAY;
	if(r_delayedCounter<0)r_delayedCounter+=MAX_ALLPASS_DELAY;

}
