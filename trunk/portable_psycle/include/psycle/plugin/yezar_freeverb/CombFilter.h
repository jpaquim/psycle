#pragma once

// CombFilter.h: interface for the CCombFilter class.
//
//////////////////////////////////////////////////////////////////////

#define MAX_COMB_DELAY			88200

class CCombFilter  
{
public:
	Initialize(int time, int stph);
	CCombFilter();
	virtual ~CCombFilter();
	Work(float l_input,float r_input);
	float left_output;
	float right_output;

private:
	float leftBuffer[MAX_COMB_DELAY];
	float rightBuffer[MAX_COMB_DELAY];
	int l_delayedCounter;
	int r_delayedCounter;
	int l_Counter;
	int r_Counter;

};
