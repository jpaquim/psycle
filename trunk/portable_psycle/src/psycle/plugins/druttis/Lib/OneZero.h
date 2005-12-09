//============================================================================
//
//	OneZero
//
//	druttis@darkface.pp.se
//
//============================================================================
#pragma once
//============================================================================
//	Class
//============================================================================
class OneZero
{
private:
	float	gain;
    float	inputs[1];
	float	lastOutput;
	//
	float	zeroCoeff;
	float	sgain;
public:
	OneZero();
	~OneZero();
	void Init();
	void Clear();
	void SetGain(float newGain);
	void SetCoeff(float newCoeff);
	//------------------------------------------------------------------------
	//	GetLastOutput
	//------------------------------------------------------------------------
	__forceinline float GetLastOutput()
	{
		return lastOutput;
	}
	//------------------------------------------------------------------------
	//	Tick
	//------------------------------------------------------------------------
	__forceinline float Tick(float sample)
	{
		float temp = sgain * sample;
		lastOutput = (inputs[0] * zeroCoeff) + temp;
		inputs[0] = temp;
		return lastOutput;
	}
};
