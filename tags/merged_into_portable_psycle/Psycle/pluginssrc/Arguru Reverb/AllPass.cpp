// AllPass.cpp: implementation of the CAllPass class.
//
//////////////////////////////////////////////////////////////////////

#include "AllPass.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAllPass::CAllPass()
{
	for(int c=0;c<MAX_ALLPASS_DELAY;c++)
	{
		leftBuffer[c]=0;
		rightBuffer[c]=0;
	}

	left_output=0;
	right_output=0;
	
}

CAllPass::~CAllPass()
{
	
}

CAllPass::Initialize(int time, int stph)
{
	l_Counter=MAX_ALLPASS_DELAY-4;
	r_Counter=MAX_ALLPASS_DELAY-4;
	l_delayedCounter=l_Counter-time;
	r_delayedCounter=r_Counter-(time+stph);
	
	if(l_delayedCounter<0)l_delayedCounter=0;
	if(r_delayedCounter<0)r_delayedCounter=0;
}


void CAllPass::Work(float& l_input,float& r_input,float g)
{
	float const l_out = (l_input*-g)+leftBuffer[l_delayedCounter];
	float const r_out = (r_input*-g)+rightBuffer[r_delayedCounter];
	//left_output=(l_input*-g)+leftBuffer[l_delayedCounter];
	//right_output=(r_input*-g)+rightBuffer[r_delayedCounter];
	
	//leftBuffer[l_Counter]=l_input+left_output*g;
	//rightBuffer[r_Counter]=r_input+right_output*g;
	leftBuffer[l_Counter]=l_input+l_out*g;
	rightBuffer[r_Counter]=r_input+r_out*g;
	
	if(++l_Counter>=MAX_ALLPASS_DELAY)l_Counter=0;
	if(++r_Counter>=MAX_ALLPASS_DELAY)r_Counter=0;
	if(++l_delayedCounter>=MAX_ALLPASS_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_ALLPASS_DELAY)r_delayedCounter=0;
}
