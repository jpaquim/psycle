//////////////////////////////////////////////////////////////////////
//
//	Delay.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "dspalgs.h"

//////////////////////////////////////////////////////////////////////
//	Delay class
//////////////////////////////////////////////////////////////////////

class Delay
{
	//////////////////////////////////////////////////////////////////
	//	Variables
	//////////////////////////////////////////////////////////////////

private:

	float	*_pSamples;
	int		_mask;
	float	_delay;
	int		_ipos;
	int		_opos;
	float	_frac;

	//////////////////////////////////////////////////////////////////
	//	Methods
	//////////////////////////////////////////////////////////////////

public:

	//////////////////////////////////////////////////////////////////
	//	Constructor
	//////////////////////////////////////////////////////////////////

	Delay()
	{
		_pSamples = 0;
		Init(256);
	}

	//////////////////////////////////////////////////////////////////
	//	Destructor
	//////////////////////////////////////////////////////////////////

	virtual ~Delay()
	{
		delete _pSamples;
	}

	//////////////////////////////////////////////////////////////////
	//	Init
	//////////////////////////////////////////////////////////////////

	__forceinline void Init(int maxDelay)
	{
		_mask = v2m(maxDelay);
		delete _pSamples;
		_pSamples = new float[_mask + 1];
		Reset();
		_ipos = 0;
		SetDelay(0.0f);
	}

	//////////////////////////////////////////////////////////////////
	//	Reset
	//////////////////////////////////////////////////////////////////

	__forceinline void Reset()
	{
		for (int i = _mask; i >= 0; i--)
			_pSamples[i] = 0.0f;
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

	__forceinline void SetDelay(float delay)
	{
/*		if (delay < 0.0f)
			delay = 0.0f;
		if (delay > (float) _maxDelay)
			delay = (float) _maxDelay;
		_delay = delay; */
		float pos = (float) _ipos - delay;
		_opos = f2i(pos);
		_frac = pos - (float) _opos;
		_opos &= _mask;
	}

	//////////////////////////////////////////////////////////////////
	//	GetSampleN
	//////////////////////////////////////////////////////////////////

	__forceinline float GetSampleN(float in)
	{
		_pSamples[_ipos++] = in;
		_ipos &= _mask;
		float out = _pSamples[_opos++];
		_opos &= _mask;
		return out;
	}

	//////////////////////////////////////////////////////////////////
	//	GetSampleL
	//////////////////////////////////////////////////////////////////

	__forceinline float GetSampleL(float in)
	{
		_pSamples[_ipos++] = in;
		_ipos &= _mask;
		float out = _pSamples[_opos++];
		_opos &= _mask;
		return out + _frac * (_pSamples[_opos & _mask] - out);
	}

	//////////////////////////////////////////////////////////////////
	//	GetLastL
	//////////////////////////////////////////////////////////////////

	__forceinline float GetLastL()
	{
		float out = _pSamples[_opos];
		return out + _frac * (_pSamples[(_opos + 1) & _mask] - out);
	}

};