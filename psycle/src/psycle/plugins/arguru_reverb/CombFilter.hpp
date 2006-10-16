#pragma once

#define MAX_COMB_DELAY			88200

class CCombFilter  
{
public:
	CCombFilter();
	virtual ~CCombFilter() throw();
	void Initialize(int time, int stph);
	inline void Work(float l_input,float r_input);
	float left_output;
	float right_output;

private:
	float leftBuffer[MAX_COMB_DELAY];
	float rightBuffer[MAX_COMB_DELAY];
	int l_delayedCounter;
	int r_delayedCounter;
	//int l_Counter;
	//int r_Counter;
	int Counter;

};

inline void CCombFilter::Work(float l_input,float r_input)
{
	//leftBuffer[l_Counter]=l_input;
	//rightBuffer[r_Counter]=r_input;
	leftBuffer[Counter]=l_input;
	rightBuffer[Counter]=r_input;


	left_output=leftBuffer[l_delayedCounter];
	right_output=rightBuffer[r_delayedCounter];

	if(++Counter>=MAX_COMB_DELAY)Counter=0;
	//if(++l_Counter>=MAX_COMB_DELAY)l_Counter=0;
	//if(++r_Counter>=MAX_COMB_DELAY)r_Counter=0;
	if(++l_delayedCounter>=MAX_COMB_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_COMB_DELAY)r_delayedCounter=0;
}
