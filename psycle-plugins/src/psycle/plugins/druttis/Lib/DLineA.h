/* -*- mode:c++, indent-tabs-mode:t -*- */
//============================================================================
//
//				DLineA (AllPass Interpolating Delay Line)
//
//				druttis@darkface.pp.se
//
//============================================================================
#pragma once
//============================================================================
//				Class
//============================================================================
class DLineA
{
private:
	float				*inputs;
	int								inPoint;
	int								outPoint;
	int								length;
	float				delay;
	float				lastOutput;
	float				alpha;
	float				coeff;
	float				lastIn;
public:
	DLineA();
	~DLineA();
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
		float temp;
		inputs[inPoint++] = sample;
		while (inPoint >= length)
			inPoint -= length;
		temp = inputs[outPoint++];
		while (outPoint >= length)
			outPoint -= length;
		lastOutput = -coeff * lastOutput;
		lastOutput += lastIn + (coeff * temp);
		lastIn = temp;
		return lastOutput;
	}
};
