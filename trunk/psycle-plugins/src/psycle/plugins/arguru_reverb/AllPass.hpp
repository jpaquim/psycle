#pragma once

#define MAX_ALLPASS_DELAY			8192

class CAllPass
{
public:
	CAllPass();
	virtual ~CAllPass() throw();
	void Initialize(int time, int stph);
	inline void Work(float l_input,float r_input,float g);
	float left_output;
	float right_output;

private:
	float leftBuffer[MAX_ALLPASS_DELAY];
	float rightBuffer[MAX_ALLPASS_DELAY];
	int l_delayedCounter;
	int r_delayedCounter;
	//int l_Counter;
	//int r_Counter;
	int Counter;
	float tmpleft;
	float tmpright;
	unsigned int *smpleft;
	unsigned int *smpright;

};

inline void CAllPass::Work(float l_input,float r_input,float g)
{
	left_output=(l_input*-g)+leftBuffer[l_delayedCounter];
	right_output=(r_input*-g)+rightBuffer[r_delayedCounter];

	tmpleft = l_input+left_output*g;
	tmpright = r_input+right_output*g;

	//  Added following code to correct denormals.
	*smpleft *= ((*smpleft < 0x7F800000) & ((*smpleft & 0x7F800000) > 0));
	*smpright *= ((*smpright < 0x7F800000) & ((*smpright & 0x7F800000) > 0));

	//leftBuffer[l_Counter] = tmpleft;
	//rightBuffer[l_Counter] = tmpright; // <-- ???
	leftBuffer[Counter] = tmpleft;
	rightBuffer[Counter] = tmpright;

	if(++Counter>=MAX_ALLPASS_DELAY)Counter=0;
	//if(++l_Counter>=MAX_ALLPASS_DELAY)l_Counter=0;
	//if(++r_Counter>=MAX_ALLPASS_DELAY)r_Counter=0;
	if(++l_delayedCounter>=MAX_ALLPASS_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_ALLPASS_DELAY)r_delayedCounter=0;
}
