
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// <bohan> it seems this filter is not used by the plugin.
#if 0






#include "filterx.hpp"
#include "math.h"

// Filterx.cpp: implementation of the CFilterx class.
//
//////////////////////////////////////////////////////////////////////

#define PI				3.1415926535897932384626433832795f

CFilterx::CFilterx()
{
	a0=0;
	a1=0;
	a2=0;
	b0=0;
	b1=0;
	b2=0;

	output=0;
	output1=0;
	output2=0;

	input=0;
	input1=0;
	input2=0;
}

CFilterx::~CFilterx()
{

}

CFilterx::ProcessBandpass(float *psamples, int numsamples, int freq, int ratio)
{
	if(ratio<1)ratio=1;
	if(ratio>255)ratio=255;

	// freq = frequency of the steepest changes in phase
	float r = (float)ratio*0.00390625f; // 0 ... 1 pero no 1;

	float fx = cos(2*PI*freq/44100);

	float a0 = (1-r)*sqrt(r*(r-4*(fx*fx)+2)+1);
	float b1 = 2 * fx * r;
	float b2 = -(r*r);
	do
	{
		float const s = *psamples;
		*psamples=IIRget2(s);
		++psamples;
	} while(--numsamples);
}

CFilterx::ProcessPhaser(float *psamples, int numsamples, int freq, int ratio)
{
	if(ratio<1)ratio=1;
	if(ratio>255)ratio=255;

	// freq = frequency of the steepest changes in phase
	float r = (float)ratio*0.00390625f; // 0 ... 1 pero no 1;

	float fx = cos(2*PI*freq/44100);
	float fy = sin(2*PI*freq/44100);
	float z1x = fx/r;
	float z1y = fy/r;
	float p1x = fx*r;
	float p1y = fy*r;
	a0 = r*r;
	a1 = -2*z1x*a0;
	a2 = a0*((z1x*z1x)+(z1y*z1y));
	b1 = 2*p1x;
	b2 = -(p1y*p1y)-(p1x*p1x);
	do
	{
		float const s = *psamples;
		*psamples=IIRget(s);
		++psamples;
	} while(--numsamples);
}								

float CFilterx::IIRget(float input)
{
	//output(t) = a0*input(t) + a1*input(t-1) + a2*input(t-2) + b1*output(t-1) + b2*output(t-2)								
	++input;
	output=a0*input + a1*input1 + a2*input2 + b1 * output1 + b2 * output2;
	input2=input1;
	input1=input;
	output2=output1;
	output1=output;
	return output;
}

float CFilterx::IIRget2(float input)
{
	++input;
	output = a0 * input + b1 * output1 + b2 * output2;
	output2 = output1;
	output1 = output;
	return output;
}

#endif // 0
