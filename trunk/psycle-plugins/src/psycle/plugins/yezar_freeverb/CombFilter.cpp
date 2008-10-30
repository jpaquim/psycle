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
	l_Counter=MAX_COMB_DELAY-4;
	r_Counter=MAX_COMB_DELAY-4;
	l_delayedCounter=l_Counter-time;
	r_delayedCounter=r_Counter-(time+stph);
	
	if(l_delayedCounter<0)l_delayedCounter=0;
	if(r_delayedCounter<0)r_delayedCounter=0;
}

void CCombFilter::Work(float l_input,float r_input)
{
	leftBuffer[l_Counter]=l_input;
	rightBuffer[r_Counter]=r_input;

	left_output=leftBuffer[l_delayedCounter];
	right_output=rightBuffer[r_delayedCounter];

	if(++l_Counter>=MAX_COMB_DELAY)l_Counter=0;
	if(++r_Counter>=MAX_COMB_DELAY)r_Counter=0;
	if(++l_delayedCounter>=MAX_COMB_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_COMB_DELAY)r_delayedCounter=0;
}
