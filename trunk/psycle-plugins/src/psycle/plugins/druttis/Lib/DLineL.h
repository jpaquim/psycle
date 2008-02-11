// -*- mode:c++; indent-tabs-mode:t -*-
//============================================================================
//
//				DLineL (Linear Interpolating Delay Line)
//
//				druttis@darkface.pp.se
//
//============================================================================
#pragma once
//============================================================================
//				Class
//============================================================================
#include <psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp>
using namespace psycle::helpers::math;

class DLineL
{
public:
	float				*inputs;
	int								inPoint;
	int								outPoint;
	int								length;
	float				delay;
	float				lastOutput;
	float				alpha;
	float				omAlpha;
public:
	DLineL();
	~DLineL();
	void Init();
	void Init(int maxLength);
	void Clear();
	void SetDelay(float lag);
	float GetDelay();
	//------------------------------------------------------------------------
	//				GetLastOutput
	//------------------------------------------------------------------------
	inline float GetLastOutput()
	{
		return lastOutput;
	}
	//------------------------------------------------------------------------
	//				Tick
	//------------------------------------------------------------------------
	inline float Tick(float sample)
	{
		erase_all_nans_infinities_and_denormals(sample);
		inputs[inPoint++] = sample;
		while (inPoint >= length)
			inPoint -= length;
		lastOutput = inputs[outPoint++] * alpha;
		while (outPoint >= length)
			outPoint -= length;
		lastOutput += inputs[outPoint] * omAlpha;
		erase_all_nans_infinities_and_denormals(lastOutput);
		return lastOutput;
	}
};
