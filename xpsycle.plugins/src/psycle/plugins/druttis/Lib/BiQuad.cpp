//============================================================================
//
//	BiQuad (2 pole, 2 zero) filter
//
//	druttis@darkface.pp.se
//
//============================================================================
#include <packageneric/pre-compiled.private.hpp>
#include "BiQuad.h"
#include <cmath>
//============================================================================
//	Constructor
//============================================================================
BiQuad::BiQuad()
{
	inputs = 0;
}
//============================================================================
//	Destructor
//============================================================================
BiQuad::~BiQuad()
{
	delete inputs;
}
//============================================================================
//	Init
//============================================================================
void BiQuad::Init()
{
	inputs = new float[2 * sizeof(float)];
	zeroCoeffs[0] = 0.0f;
	zeroCoeffs[1] = 0.0f;
	poleCoeffs[0] = 0.0f;
	poleCoeffs[1] = 0.0f;
	gain = 1.0f;
	Clear();
}
//============================================================================
//	Clear
//============================================================================
void BiQuad::Clear()
{
	inputs[0] = 0.0f;
	inputs[1] = 0.0f;
	lastOutput = 0.0f;
}
//============================================================================
//	SetA1
//============================================================================
void BiQuad::SetA1(float a1)
{
	poleCoeffs[0] = -a1;
}
//============================================================================
//	SetA2
//============================================================================
void BiQuad::SetA2(float a2)
{
	poleCoeffs[1] = -a2;
}
//============================================================================
//	SetB1
//============================================================================
void BiQuad::SetB1(float b1)
{
	zeroCoeffs[0] = b1;
}
//============================================================================
//	SetB2
//============================================================================
void BiQuad::SetB2(float b2)
{
	zeroCoeffs[1] = b2;
}
//============================================================================
//	SetPoleCoeffs
//============================================================================
void BiQuad::SetPoleCoeffs(float *coeffs)
{
	poleCoeffs[0] = coeffs[0];
	poleCoeffs[1] = coeffs[1];
}
//============================================================================
//	SetZeroCoeffs
//============================================================================
void BiQuad::SetZeroCoeffs(float * coeffs)
{
	zeroCoeffs[0] = coeffs[0];
	zeroCoeffs[1] = coeffs[1];
}
//============================================================================
//	SetFreqAndRes
//============================================================================
void BiQuad::SetFreqAndRes(float freq, float res, float srate)
{
	poleCoeffs[0] = 2.0f * res * (float) cos(freq * 6.2831853072f / srate);
	poleCoeffs[1] = -(res * res);
}
//============================================================================
//	SetEqualGainZeroes
//============================================================================
void BiQuad::SetEqualGainZeroes()
{
	zeroCoeffs[0] = 0.0f;
	zeroCoeffs[1] = -1.0f;
}
//============================================================================
//	SetGain
//============================================================================
void BiQuad::SetGain(float newGain)
{
	gain = newGain;
}
