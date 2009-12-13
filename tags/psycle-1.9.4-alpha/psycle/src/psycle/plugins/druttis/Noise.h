#pragma once
#include <stdlib.h>

class Noise
{
public:
	float	last_output;
	float	scale;
public:
	Noise()
	{
		last_output = 0.0f;
		scale = 2.0f / (float) RAND_MAX;
	}

	~Noise()
	{
	}

	__forceinline float Tick()
	{
		last_output = (float) rand() * scale - 1.0f;
		return last_output;
	}
};