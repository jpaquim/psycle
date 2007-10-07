//============================================================================
//
//	FIR (Finite Impulse Response)
//
//	druttis@darkface.pp.se
//	
//============================================================================
#pragma once
//============================================================================
//	Class
//============================================================================
class FIR
{
private:
	int		length;
	float	*inputs;
	float	*coeffs;
	int		inPoint;
	float	delay;
	float	lastOutput;
public:
	FIR();
	~FIR();
	void Init(int maxLength);
	void Clear();
	void SetCoeffs(float *newCoeffs);
	//------------------------------------------------------------------------
	//	GetLastOutput
	//------------------------------------------------------------------------
	__forceinline float GetLastOutput()
	{
		return lastOutput;
	}
	//------------------------------------------------------------------------
	//	GetDelay
	//------------------------------------------------------------------------
	__forceinline float GetDelay(float freq)
	{
		return delay;
	}
	//------------------------------------------------------------------------
	//	GetLength
	//------------------------------------------------------------------------
	__forceinline int GetLength()
	{
		return length;
	}
	//------------------------------------------------------------------------
	//	Tick
	//------------------------------------------------------------------------
	__forceinline float Tick(float sample)
	{
		int i;
		lastOutput = sample * coeffs[0];
		for (i = 1; i < length; i++)
			lastOutput += coeffs[i] * inputs[inPoint - 1];
		inputs[inPoint++] = sample;
		if (inPoint >= 2 * length) {
			inPoint = length;
			for (i = 0; i < length; i++)
				inputs[i] = inputs[length + i];
		}
		return lastOutput;
	}
};
