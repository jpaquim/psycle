// Lowpass.h: interface for the CLowpass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOWPASS_H__255C5520_359E_11D4_8913_8C356380CA49__INCLUDED_)
#define AFX_LOWPASS_H__255C5520_359E_11D4_8913_8C356380CA49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLowpass  
{
public:
	CLowpass();
	virtual ~CLowpass();
	inline float Process(float i,float c);

private:
	float o1;
};

inline float CLowpass::Process(float i,float c)
{
	float output= o1 + c * (i-o1);
	o1=output;

	return output;
}
#endif // !defined(AFX_LOWPASS_H__255C5520_359E_11D4_8913_8C356380CA49__INCLUDED_)
