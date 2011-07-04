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
	float Process(float i,float c);

private:
	float o1;
};

#endif // !defined(AFX_LOWPASS_H__255C5520_359E_11D4_8913_8C356380CA49__INCLUDED_)
