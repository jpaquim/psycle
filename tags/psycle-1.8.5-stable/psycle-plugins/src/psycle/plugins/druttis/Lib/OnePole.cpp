//============================================================================
//
//				OnePole
//
//				druttis@darkface.pp.se
//
//============================================================================
#include <math.h>
#include "OnePole.h"
//============================================================================
//				Constructor
//============================================================================
OnePole::OnePole()
{
}
//============================================================================
//				Destructor
//============================================================================
OnePole::~OnePole()
{
}
//============================================================================
//				Init
//============================================================================
void OnePole::Init()
{
	Init(0.9f);
}
//============================================================================
//				Init
//============================================================================
void OnePole::Init(float thePole)
{
	poleCoeff = thePole;
	gain = 1.0f;
	sgain = 1.0f - (float) fabs(thePole);
	Clear();
}
//============================================================================
//				Clear
//============================================================================
void OnePole::Clear()
{
	outputs[0] = 0.0f;
	lastOutput = 0.0f;
}
//============================================================================
//				SetB0
//============================================================================
void OnePole::SetB0(float newB0)
{
	sgain = newB0;
}
//============================================================================
//				SetNum
//============================================================================
void OnePole::SetNum(float *values)
{
	sgain = values[0];
}
//============================================================================
//				SetA1
//============================================================================
void OnePole::SetA1(float newA1)
{
	poleCoeff = -newA1;
}
//============================================================================
//				SetDen
//============================================================================
void OnePole::SetDen(float *values)
{
	poleCoeff = -values[0];
}
//============================================================================
//				SetPole
//============================================================================
void OnePole::SetPole(float newPole)
{
	poleCoeff = newPole;
	if (poleCoeff > 0.0f)
		sgain = gain * (1.0f - poleCoeff);
	else
		sgain = gain * (1.0f + poleCoeff);
}
//============================================================================
//				SetGain
//============================================================================
void OnePole::SetGain(float newGain)
{
	gain = newGain;
	if (poleCoeff > 0.0f)
		sgain = gain * (1.0f - poleCoeff);
	else
		sgain = gain * (1.0f + poleCoeff);
}
