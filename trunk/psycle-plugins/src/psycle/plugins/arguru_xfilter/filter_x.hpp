// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once

// Filterx.h: interface for the CFilterx class.
//
//////////////////////////////////////////////////////////////////////

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
