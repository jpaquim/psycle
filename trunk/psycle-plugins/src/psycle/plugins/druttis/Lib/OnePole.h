/* -*- mode:c++, indent-tabs-mode:t -*- */
//============================================================================
//
//				OnePole
//
//				druttis@darkface.pp.se
//
//============================================================================
#pragma once
//============================================================================
//				Class
//============================================================================
class OnePole
{
private:
	float				gain;
	float				outputs[1];
	float				lastOutput;
	//
	float				poleCoeff;
	float				sgain;
public:
	OnePole();
	~OnePole();
	void Init();
	void Init(float thePole);
	void Clear();
	void SetB0(float newB0);
	void SetNum(float *values);
	void SetA1(float newA1);
	void SetDen(float *values);
	void SetPole(float newPole);
	void SetGain(float newGain);
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
		outputs[0] = (sgain * sample) + (poleCoeff * outputs[0]);
		lastOutput = outputs[0];
		return lastOutput;
	}
};
