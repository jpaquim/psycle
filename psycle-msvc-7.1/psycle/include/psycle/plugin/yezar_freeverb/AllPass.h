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
	Initialize(int time, int stph);
	CAllPass();
	virtual ~CAllPass();
	Work(float l_input,float r_input,float g);
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

#endif // !defined(AFX_ALLPASS_H__588D20C3_3565_11D4_8913_DD4E36C5D249__INCLUDED_)
