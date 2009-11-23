//////////////////////////////////////////////////////////////////////
//
//	BiQuad.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
//	BiQuad class
//////////////////////////////////////////////////////////////////////

class BiQuad
{

	//////////////////////////////////////////////////////////////////
	//	Variables
	//////////////////////////////////////////////////////////////////

private:

	float	inputs[2];
	float	outputs[2];
	float	freq;
	float	res;

	//////////////////////////////////////////////////////////////////
	//	Methods
	//////////////////////////////////////////////////////////////////

public:

	//////////////////////////////////////////////////////////////////
	//	Constructor
	//////////////////////////////////////////////////////////////////

	AllPass()
	{
		Init();
	}

	//////////////////////////////////////////////////////////////////
	//	Destructor
	//////////////////////////////////////////////////////////////////

	virtual ~AllPass()
	{
	}

	//////////////////////////////////////////////////////////////////
	//	Init
	//////////////////////////////////////////////////////////////////

	__forceinline void Init()
	{
		LowPass
		SetDelay(0.0f);
		Reset();
	}

	//////////////////////////////////////////////////////////////////
	//	Reset
	//////////////////////////////////////////////////////////////////

	__forceinline void Reset()
	{
		_z = 0.0f;
	}

	//////////////////////////////////////////////////////////////////
	//	GetDelay
	//////////////////////////////////////////////////////////////////

	__forceinline float GetDelay()
	{
		return _delay;
	}

	//////////////////////////////////////////////////////////////////
	//	SetDelay
	//////////////////////////////////////////////////////////////////

	__forceinline void SetDelay(float freq, float res)
	{
		c = 1.0f / tan(
		_delay = delay;
		_a = (1.0f - delay) / (1.0f + delay);
	}

	//////////////////////////////////////////////////////////////////
	//	GetSample
	//////////////////////////////////////////////////////////////////

	__forceinline float GetSample(float in)
	{
		float out = a1 * in + a2 * inputs[0] + a3 * inputs[1] - b1 * outputs[0] - b2 * outputs[1];
		inputs[1] = inputs[0];
		inputs[0] = in;
		outputs[1] = outputs[0];
		outputs[0] = out;
		return out;
	}

};