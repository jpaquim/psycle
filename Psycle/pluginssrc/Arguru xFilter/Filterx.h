// Filterx.h: interface for the CFilterx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILTERX_H__3BBDB2A2_49D0_11D4_8913_DF30864C7E40__INCLUDED_)
#define AFX_FILTERX_H__3BBDB2A2_49D0_11D4_8913_DF30864C7E40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFilterx  
{
public:
	float IIRget(float input);
	float IIRget2(float input);

	ProcessPhaser(float *psamples, int numsamples, int freq, int ratio);
	ProcessBandpass(float *psamples, int numsamples, int freq, int ratio);

	CFilterx();
	virtual ~CFilterx();

private:

	// Filter past inputs/outputs
	float output,output1,output2;
	float input,input1,input2;

	// IIR Filter coefficients
	float a0,a1,a2,b0,b1,b2;
};

#endif // !defined(AFX_FILTERX_H__3BBDB2A2_49D0_11D4_8913_DF30864C7E40__INCLUDED_)
