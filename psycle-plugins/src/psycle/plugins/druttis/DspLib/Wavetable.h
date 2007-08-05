//////////////////////////////////////////////////////////////////////
//
//	Wavetable.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
//	Wavetable class
//////////////////////////////////////////////////////////////////////

class Wavetable  
{

	//////////////////////////////////////////////////////////////////
	//	Variables
	//////////////////////////////////////////////////////////////////

private:

	float	*_pSamples;
	int		_length;
	int		_mask;
	float	_dummy;

	//////////////////////////////////////////////////////////////////
	//	Methods
	//////////////////////////////////////////////////////////////////

public:

	//////////////////////////////////////////////////////////////////
	//	Constructor
	//////////////////////////////////////////////////////////////////

	Wavetable()
	{
		_dummy = 0.0f;
		Init();
	}

	//////////////////////////////////////////////////////////////////
	//	Destructor
	//////////////////////////////////////////////////////////////////

	virtual ~Wavetable()
	{
	}

	//////////////////////////////////////////////////////////////////
	//	Init
	//////////////////////////////////////////////////////////////////

	void Init()
	{
		Reset();
	}

	//////////////////////////////////////////////////////////////////
	//	Reset
	//////////////////////////////////////////////////////////////////

	void Reset()
	{
		SetSamples(&_dummy, 1);
	}

	//////////////////////////////////////////////////////////////////
	//	GetLength
	//////////////////////////////////////////////////////////////////

	inline int GetLength()
	{
		return _length;
	}

	//////////////////////////////////////////////////////////////////
	//	GetSamples
	//////////////////////////////////////////////////////////////////

	inline float *GetSamples()
	{
		return _pSamples;
	}

	//////////////////////////////////////////////////////////////////
	//	SetSamples	( Length must be 2^n. 1, 2, ... , 256, 512, etc. )
	//////////////////////////////////////////////////////////////////

	inline void SetSamples(float *pSamples, int length)
	{
		_pSamples = pSamples;
		_length = length;
		_mask = length - 1;
	}

	//////////////////////////////////////////////////////////////////
	//	ClipPhase
	//////////////////////////////////////////////////////////////////

	inline float ClipPhase(float phase)
	{
		return fand(phase, _mask);
	}

	//////////////////////////////////////////////////////////////////
	//	GetSample
	//////////////////////////////////////////////////////////////////

	inline float GetSampleN(float phase)
	{
		return get_sample_n(_pSamples, phase, _mask);
	}

	//////////////////////////////////////////////////////////////////
	//	GetLinear
	//////////////////////////////////////////////////////////////////

	inline float GetSampleL(float phase)
	{
		return get_sample_l(_pSamples, phase, _mask);
	}

};
