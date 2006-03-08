//============================================================================
//
//	DLineL (Linear Interpolating Delay Line)
//
//	druttis@darkface.pp.se
//
//============================================================================
#pragma once
//============================================================================
//	Class
//============================================================================
class DLineL
{
public:
    float	*inputs;
	int		inPoint;
	int		outPoint;
	int		length;
	float	delay;
	float	lastOutput;
	float	alpha;
	float	omAlpha;
public:
	DLineL();
	~DLineL();
	void Init();
	void Init(int maxLength);
	void Clear();
	void SetDelay(float lag);
	float GetDelay();
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
		inputs[inPoint++] = sample;
		while (inPoint >= length)
			inPoint -= length;
		lastOutput = inputs[outPoint++] * alpha;
		while (outPoint >= length)
			outPoint -= length;
		lastOutput += inputs[outPoint] * omAlpha;
		return lastOutput;
	}
};
