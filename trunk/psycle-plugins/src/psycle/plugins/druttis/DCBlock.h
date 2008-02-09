/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <math.h>

class DCBlock
{
public:
	float				input;
	float				output;
	float				last_output;
public:
	DCBlock()
	{
		Clear();
	}

	~DCBlock()
	{
	}

	void Clear()
	{
		input = 0.0f;
		output = 0.0f;
		last_output = 0.0f;
	}

	inline float Tick(float sample)
	{
		output = sample - input + 0.99f * output;
		input = sample;
		last_output = output;
		return last_output;
	}
};
