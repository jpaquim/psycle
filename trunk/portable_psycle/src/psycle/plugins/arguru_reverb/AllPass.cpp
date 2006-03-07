#include <packageneric/pre-compiled.private.hpp>
#include "AllPass.hpp"

CAllPass::CAllPass()
{
	for(int c=0;c<MAX_ALLPASS_DELAY;c++)
	{
		leftBuffer[c]=0;
		rightBuffer[c]=0;
	}

	left_output=0;
	right_output=0;
	smpleft = reinterpret_cast<unsigned int*>(&tmpleft);
	smpright = reinterpret_cast<unsigned int*>(&tmpright);

}

CAllPass::~CAllPass() throw()
{

}

void CAllPass::Initialize(int time, int stph)
{
	l_Counter=MAX_ALLPASS_DELAY-4;
	r_Counter=MAX_ALLPASS_DELAY-4;
	l_delayedCounter=l_Counter-time;
	r_delayedCounter=r_Counter-(time+stph);
	
	if(l_delayedCounter<0)l_delayedCounter=0;
	if(r_delayedCounter<0)r_delayedCounter=0;

}
