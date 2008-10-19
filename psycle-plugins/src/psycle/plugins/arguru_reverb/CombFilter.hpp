// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once

#define MAX_COMB_DELAY 88200

class CCombFilter  
{
public:
	CCombFilter();
	virtual ~CCombFilter() throw();
	void Clear();
	void Initialize(int time, int stph);
	inline void Work(float l_input,float r_input);
	float left_output;
	float right_output;

private:
	float *leftBuffer;
	float *rightBuffer;
	int l_delayedCounter;
	int r_delayedCounter;
	int Counter;

};

inline void CCombFilter::Work(float l_input,float r_input)
{
	leftBuffer[Counter]=l_input;
	rightBuffer[Counter]=r_input;

	left_output=leftBuffer[l_delayedCounter];
	right_output=rightBuffer[r_delayedCounter];

	if(++Counter>=MAX_COMB_DELAY)Counter=0;
	if(++l_delayedCounter>=MAX_COMB_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_COMB_DELAY)r_delayedCounter=0;
}
