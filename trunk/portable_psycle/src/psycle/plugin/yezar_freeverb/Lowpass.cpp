#include <project.h>
#include <psycle/plugin/yezar_freverb/LowPass.h>

// Lowpass.cpp: implementation of the CLowpass class.
//
//////////////////////////////////////////////////////////////////////

CLowpass::CLowpass()
{
	o1=0.0f;
}

CLowpass::~CLowpass()
{

}

float CLowpass::Process(float i,float c)
{
	float output= o1 + c * (i-o1);
	o1=output;

	return output;
}