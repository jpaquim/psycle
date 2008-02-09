/* -*- mode:c++, indent-tabs-mode:t -*- */
//============================================================================
//
//				BiQuad (2 pole, 2 zero) filter
//
//				druttis@darkface.pp.se
//
//============================================================================
#pragma once

#include <psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp>
using namespace psycle::helpers::math;

class BiQuad
{
protected:
	float				*inputs;
	float				gain;
	float				lastOutput;
	float				poleCoeffs[2];
	float				zeroCoeffs[2];
public:
	BiQuad();
	~BiQuad();
	void Init();
	void Clear();
	void SetA1(float a1);
	void SetA2(float a2);
	void SetB1(float b1);
	void SetB2(float b2);
	void SetPoleCoeffs(float *coeffs);
	void SetZeroCoeffs(float *coeffs);
	void SetFreqAndRes(float freq, float res, float srate);
	void SetEqualGainZeroes();
	void SetGain(float newGain);
	//------------------------------------------------------------------------
	//				tick
	//------------------------------------------------------------------------
	inline float Tick(float sample)
	{
		erase_all_nans_infinities_and_denormals(sample);
		static float temp;
		//
		//
		temp = sample * gain;
		temp += inputs[0] * poleCoeffs[0];
		temp += inputs[1] * poleCoeffs[1];
		//
		//
		erase_all_nans_infinities_and_denormals(temp);

		lastOutput = temp;
		lastOutput += (inputs[0] * zeroCoeffs[0]);
		lastOutput += (inputs[1] * zeroCoeffs[1]);
		inputs[1] = inputs[0];
		inputs[0] = temp;
		erase_all_nans_infinities_and_denormals(lastOutput);
		//
		//
		return lastOutput;
		//return sample; //<Sartorius> ???
	}
};
