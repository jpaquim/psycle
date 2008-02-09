/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <math.h>

class DLineL
{
public:
	int								inPoint;
	int								outPoint;
	int								length;
	float				currentDelay;
	float				alpha;
	float				omAlpha;
	float				last_output;
	float				*inputs;
public:
	DLineL() {
		inputs = 0;
	}

	~DLineL() {
		Destroy();
	}

	void Init(int max_length)
	{
		Destroy();
		length = max_length;
		inputs = new float[length];
		this->Clear();
		inPoint = 0;
		outPoint = length >> 1;
		currentDelay = (float) outPoint;
	}

	void Destroy() {
		if (inputs) {
			delete inputs;
			inputs = 0;
		}
	}

	void Clear()
	{
		for (int i = 0; i <length ; i++)
			inputs[i] = 0.0f;
		last_output = 0.0f;
	}

	void SetDelay(float lag)
	{
		float				outPointer;

		if (lag > length - 1) {
			outPointer = (float) inPoint + 1.0f;
			currentDelay = (float) length - 1.0f;
		} else {
			outPointer = inPoint - lag;
			currentDelay = lag;
		}

		while (outPointer < 0)
			outPointer += length;

		outPoint = (int) outPointer;

		alpha = outPointer - (float) outPoint;
		omAlpha = 1.0f - alpha;
	}

	__forceinline float Tick(float sample)
	{
		inputs[inPoint++] = sample;
		if (inPoint == length)
			inPoint -= length;
		last_output = inputs[outPoint++] * omAlpha;
		if (outPoint < length) {
			last_output += inputs[outPoint] * alpha;
		} else {
			last_output += inputs[0] * alpha;
			outPoint -= length;
		}
		return last_output;
	}
};
