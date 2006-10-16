#include <packageneric/pre-compiled.private.hpp>
#include "CombFilter.hpp"

CCombFilter::CCombFilter()
{
	for(int c=0;c<MAX_COMB_DELAY;c++)
	{
		leftBuffer[c]=0;
		rightBuffer[c]=0;
	}

	left_output=0;
	right_output=0;

}

CCombFilter::~CCombFilter() throw()
{

}

void CCombFilter::Initialize(int time, int stph)
{
	//l_Counter=MAX_COMB_DELAY-4;
	//r_Counter=MAX_COMB_DELAY-4;
	Counter=MAX_COMB_DELAY-4;
	//l_delayedCounter=l_Counter-time;
	//r_delayedCounter=r_Counter-(time+stph);
	l_delayedCounter=Counter-time;
	r_delayedCounter=l_delayedCounter-stph;
	
	if(l_delayedCounter<0)l_delayedCounter=0;
	if(r_delayedCounter<0)r_delayedCounter=0;
}


