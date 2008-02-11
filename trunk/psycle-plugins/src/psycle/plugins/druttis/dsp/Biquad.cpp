// -*- mode:c++; indent-tabs-mode:t -*-
//////////////////////////////////////////////////////////////////////
//
//				Biquad.cpp
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include "Biquad.h"

//////////////////////////////////////////////////////////////////////
//
//				Biquad constructor
//
//////////////////////////////////////////////////////////////////////
Biquad::Biquad()
{
	a0 = a1 = a2 = 0.0;
	b0 = b1 = b2 = 0.0;
	c0 = c1 = c2 = c3 = c4 = 0.0;
	Reset();
}

//////////////////////////////////////////////////////////////////////
//
//				Biquad destructor
//
//////////////////////////////////////////////////////////////////////
Biquad::~Biquad()
{
}

//////////////////////////////////////////////////////////////////////
//
//				Reset
//
//////////////////////////////////////////////////////////////////////
void Biquad::Reset()
{
	x1 = x2 = 0.0;
	y1 = y2 = 0.0;
}

//////////////////////////////////////////////////////////////////////
//
//				Init
//
//////////////////////////////////////////////////////////////////////
void Biquad::Init(int type, float dbGain, float freq, int sr, float bandwidth)
{
	double A = pow(10.0, (double) dbGain / 40.0);
	double omega = 2.0 * PI * (double) freq / (double) sr;
	double sn = sin(omega);
	double cs = cos(omega);
	double alpha = sn * sinh(LN / 2.0 * (double) bandwidth * omega / sn);
	double beta;
	if (bandwidth != 0.0f)
	{
		beta = sqrt(A * A + 1.0) / (double) bandwidth - ((A - 1.0) * (A - 1.0));
	}
	else
	{
		beta = sqrt(A + A);
	}

	switch (type)
	{
	case PEQ :
		b0 = 1.0 + (alpha * A);
		b1 = -2.0 * cs;
		b2 = 1.0 - (alpha * A);
		a0 = 1.0 + (alpha / A);
		a1 = -2.0 * cs;
		a2 = 1.0 - (alpha / A);
		break;
	case LSH :
		b0 = A * ((A + 1.0) - (A - 1.0) * cs + beta * sn);
		b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cs);
		b2 = A * ((A + 1.0) - (A - 1.0) * cs - beta * sn);
		a0 = (A + 1.0) + (A - 1.0) * cs + beta * sn;
		a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cs);
		a2 = (A + 1.0) + (A - 1.0) * cs - beta * sn;
		break;
	case HSH :
		b0 = A * ((A + 1.0) + (A - 1.0) * cs + beta * sn);
		b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
		b2 = A * ((A + 1.0) + (A - 1.0) * cs - beta * sn);
		a0 = (A + 1.0) - (A - 1.0) * cs + beta * sn;
		a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
		a2 = (A + 1.0) - (A - 1.0) * cs - beta * sn;
		break;
	}
	c0 = b0 / a0;
	c1 = b1 / a0;
	c2 = b2 / a0;
	c3 = a1 / a0;
	c4 = a2 / a0;
}
