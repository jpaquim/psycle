/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <math.h>

class OnePole
{
public:
	float				poleCoeff;
	float				sgain;
	//
	float				gain;
	float				output;
	float				last_output;
public:
	OnePole()
	{
		poleCoeff = 0.9f;
		gain = 1.0f;
		sgain = 0.1f;
		output = 0.0f;
		last_output = 0.0f;
	}

	~OnePole()
	{
		Destroy();
	}

	void Init(float thePole)
	{
		Destroy();
		poleCoeff = thePole;
		gain = 1.0f;
		sgain = 1.0f - (float) fabs(thePole);
		output = 0.0f;
		last_output = 0.0f;
	}

	void Destroy()
	{
	}

	void Clear()
	{
		output = 0.0f;
		last_output = 0.0f;
	}

	void SetPole(float aValue)
	{
		poleCoeff = aValue;
		if (poleCoeff > 0.0f)
			sgain = sgain * (1.0f - poleCoeff);
		else
			sgain = sgain * (1.0f + poleCoeff);
	}

	void SetGain(float aValue)
	{
		gain = aValue;
		if (poleCoeff > 0.0f)
			sgain = sgain * (1.0f - poleCoeff);
		else
			sgain = sgain * (1.0f + poleCoeff);
	}

	__forceinline float Tick(float sample)
	{
		output = (sgain * sample) + (poleCoeff * output);
		last_output = output;
		return last_output;
	}
};