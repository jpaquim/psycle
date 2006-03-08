#include "stdafx.h"
#include "Filter.h"


FilterCoeff Filter::_coeffs;

Filter::Filter()
{
	_coeffs.Init();
	Init();
	_x1 = _x2 = _y1 = _y2 = 0;
	_a1 = _a2 = _b1 = _b2 = 0;
}

void Filter::Init(void)
{
}

void Filter::Update(void)
{
	_coeff0 = _coeffs._coeffs[_type][_cutoff][_q][0];
	_coeff1 = _coeffs._coeffs[_type][_cutoff][_q][1];
	_coeff2 = _coeffs._coeffs[_type][_cutoff][_q][2];
	_coeff3 = _coeffs._coeffs[_type][_cutoff][_q][3];
	_coeff4 = _coeffs._coeffs[_type][_cutoff][_q][4];
}
