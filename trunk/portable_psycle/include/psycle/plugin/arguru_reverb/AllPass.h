// AllPass.h: interface for the CAllPass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALLPASS_H__588D20C3_3565_11D4_8913_DD4E36C5D249__INCLUDED_)
#define AFX_ALLPASS_H__588D20C3_3565_11D4_8913_DD4E36C5D249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	int l_Counter;
	int r_Counter;

};

inline void CAllPass::Work(float l_input,float r_input,float g)
{
	
	left_output=(l_input*-g)+leftBuffer[l_delayedCounter];
	right_output=(r_input*-g)+rightBuffer[r_delayedCounter];

	leftBuffer[l_Counter]=l_input+left_output*g;
	rightBuffer[r_Counter]=r_input+right_output*g;

	if(++l_Counter>=MAX_ALLPASS_DELAY)l_Counter=0;
	if(++r_Counter>=MAX_ALLPASS_DELAY)r_Counter=0;
	if(++l_delayedCounter>=MAX_ALLPASS_DELAY)l_delayedCounter=0;
	if(++r_delayedCounter>=MAX_ALLPASS_DELAY)r_delayedCounter=0;
}

#endif // !defined(AFX_ALLPASS_H__588D20C3_3565_11D4_8913_DD4E36C5D249__INCLUDED_)
