#pragma once

#define MAX_ALLPASS_DELAY			8192

class CAllPass
{
public:
	CAllPass();
	virtual ~CAllPass() throw();
	void Initialize(int time, int stph);
	void Clear();
	inline void Work(float l_input,float r_input,float g);
	float left_output;
	float right_output;

private:
	float *leftBuffer;
	float *rightBuffer;
	int l_delayedCounter;
	int r_delayedCounter;
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
	leftBuffer[Counter] = tmpleft;
	rightBuffer[Counter] = tmpright;

	if(++Counter>=MAX_ALLPASS_DELAY)Counter=0;
	if(++l_delayedCounter>=MAX_ALLPASS_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_ALLPASS_DELAY)r_delayedCounter=0;
}
