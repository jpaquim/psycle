// CombFilter.h: interface for the CCombFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMBFILTER_H__588D20C2_3565_11D4_8913_DD4E36C5D249__INCLUDED_)
#define AFX_COMBFILTER_H__588D20C2_3565_11D4_8913_DD4E36C5D249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_COMB_DELAY			88200

class CCombFilter  
{
public:
	CCombFilter();
	virtual ~CCombFilter() throw();
	void Initialize(int time, int stph);
	void Work(float l_input,float r_input);
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

#endif // !defined(AFX_COMBFILTER_H__588D20C2_3565_11D4_8913_DD4E36C5D249__INCLUDED_)
