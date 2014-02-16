// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\implementation psycle::helpers::Filter.

#include "filter.hpp"

#include <cmath>
#include <algorithm>

namespace psycle { namespace helpers { namespace dsp {

FilterCoeff FilterCoeff::singleton;

/*Table from Schism Tracker*/
static const float dmpfac[] = {
	/**/ 131072, 128272, 125533, 122852, 120229, 117661, 115148, 112689,
	/**/ 110283, 107928, 105623, 103367, 101160,  98999,  96885,  94816,
	/**/  92791,  90810,  88870,  86973,  85115,  83298,  81519,  79778,
	/**/  78074,  76407,  74775,  73178,  71615,  70086,  68589,  67125,
	/**/  65691,  64288,  62915,  61572,  60257,  58970,  57711,  56478,
	/**/  55272,  54092,  52937,  51806,  50700,  49617,  48557,  47520,
	/**/  46506,  45512,  44540,  43589,  42658,  41747,  40856,  39983,
	/**/  39130,  38294,  37476,  36676,  35893,  35126,  34376,  33642,
	/**/  32923,  32220,  31532,  30859,  30200,  29555,  28924,  28306,
	/**/  27701,  27110,  26531,  25964,  25410,  24867,  24336,  23816,
	/**/  23308,  22810,  22323,  21846,  21380,  20923,  20476,  20039,
	/**/ 19611,  19192,  18782,  18381,  17989,  17604,  17228,  16861,
	/**/ 16500,  16148,  15803,  15466,  15135,  14812,  14496,  14186,
	/**/ 13883,  13587,  13297,  13013,  12735,  12463,  12197,  11936,
	/**/ 11681,  11432,  11188,  10949,  10715,  10486,  10262,  10043,
	/**/  9829,   9619,   9413,   9212,   9015,   8823,   8634,   8450,
	/**/  8270,   8093,   7920,   7751,   7585,   7423,   7265,   7110,
	/**/  6958,   6809,   6664,   6522,   6382,   6246,   6113,   5982,
	/**/  5854,   5729,   5607,   5487,   5370,   5255,   5143,   5033,
	/**/  4926,   4820,   4718,   4617,   4518,   4422,   4327,   4235,
	/**/  4144,   4056,   3969,   3884,   3801,   3720,   3641,   3563,
	/**/  3487,   3412,   3340,   3268,   3198,   3130,   3063,   2998,
	/**/  2934,   2871,   2810,   2750,   2691,   2634,   2577,   2522,
	/**/  2468,   2416,   2364,   2314,   2264,   2216,   2169,   2122,
	/**/  2077,   2032,   1989,   1947,   1905,   1864,   1824,   1786,
	/**/  1747,   1710,   1674,   1638,   1603,   1569,   1535,   1502,
	/**/  1470,   1439,   1408,   1378,   1348,   1320,   1291,   1264,
	/**/  1237,   1210,   1185,   1159,   1135,   1110,   1087,   1063,
	/**/  1041,   1018,    997,    975,    955,    934,    914,    895,
	/**/   876,    857,    838,    821,    803,    786,    769,    753,
	/**/   737,    721,    705,    690,    676,    661,    647,    633,
	/**/   620,    606,    593,    581,    568,    556,    544,    533
};

FilterCoeff::FilterCoeff() {
	samplerate = -1;
	//table is initialized with Filter::Init()
}

void FilterCoeff::setSampleRate(float samplerate)
{
	if (samplerate != this->samplerate)
	{
		this->samplerate = samplerate;
		for (int t=0; t<F_NUMFILTERS; t++)
		{
			int tdest = t;
			if (t == F_NONE) continue;//Skip filter F_NONE.
			else if (t > F_NONE) tdest--;

			for (int f=0; f<128; f++)
			{
				for (int q=0; q<128; q++)
				{
					ComputeCoeffs(f, q, t);
					_coeffs[tdest][f][q][0] = (float)_coeff[0];
					_coeffs[tdest][f][q][1] = (float)_coeff[1];
					_coeffs[tdest][f][q][2] = (float)_coeff[2];
					_coeffs[tdest][f][q][3] = (float)_coeff[3];
					_coeffs[tdest][f][q][4] = (float)_coeff[4];
				}
			}
		}
	}
}

float FilterCoeff::Cutoff(int v)
{
	return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
}

float FilterCoeff::Resonance(float v)
{
	return float(pow( v/127.0, 4)*150+0.1);
}

float FilterCoeff::Bandwidth(int v)
{
	return float(pow( v/127.0, 4)*4+0.1);
}

void FilterCoeff::ComputeCoeffs(int freq, int r, int t)
{
	float omega = float(TPI*Cutoff(freq)/ samplerate);
	float sn = (float)sin(omega);
	float cs = (float)cos(omega);
	float alpha;
	if (t < F_BANDPASS12) alpha = float(sn / Resonance( r *(freq+70)/(127.0f+70.f)));
	else alpha = float(sn * sinh( Bandwidth( r) * omega/sn));
	float a0, a1, a2, b0, b1, b2;
	switch (t)
	{
	case F_LOWPASS12:
		b0 =  (1.f - cs)/2.f;
		b1 =   1.f - cs;
		b2 =  (1.f - cs)/2.f;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_HIGHPASS12:
		b0 =  (1.f + cs)/2.f;
		b1 = -(1.f + cs);
		b2 =  (1.f + cs)/2.f;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_BANDPASS12:
		b0 =   alpha;
		b1 =   0.f;
		b2 =  -alpha;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_BANDREJECT12:
		b0 =   1.f;
		b1 =  -2.f*cs;
		b2 =   1.f;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_ITLOWPASS:
		/*Coefficients from Schism Tracker, using formula from Modplug for cutoff*/
		double d,e;
		if ( r >0 || freq < 127 )
		{
			double fc = TPI* (110.0 * pow(2.0,(freq+6)/24.0)) / samplerate;  // ModPlug.
			double d2 = dmpfac[r] / (4.0*65536.0);
			d = (1.0 - d2) * fc;

			if (d > 2.0)
				d = 2.0;

			d = (d2 - d) / fc;
			e = 1.0 / (fc * fc);
		}
		else { e = 0; d = 0; }
		b0 = 1.0f;
		b1 = 0.0f;
		b2 = 0.0f;
		a0 = 1.0f + d + e;
		a1 = -(d + e + e);
		a2 = e;
		break;
	default:
		b0 =  1;
		b1 =  0;
		b2 =  0;
		a0 =  1;
		a1 =  0;
		a2 =  0;
	}
	_coeff[0] = b0/a0;
	_coeff[1] = b1/a0;
	_coeff[2] = b2/a0;
	_coeff[3] = -a1/a0;
	_coeff[4] = -a2/a0;
}

Filter::Filter()
{
	//Filtercoef is initialized when calling setSampleRate().
	Init(44100);
}

void Filter::Init(int sampleRate)
{
	FilterCoeff::singleton.setSampleRate(sampleRate);
	Reset();
}
void Filter::Reset(void) {
	_cutoff=127;
	_q=0;
	_type = F_NONE;
	_x1 = _x2 = _y1 = _y2 = 0;
	_a1 = _a2 = _b1 = _b2 = 0;
	Update();
}

void Filter::Update()
{
	if (_type == F_NONE) {
		_coeff0 =  1;
		_coeff1 =  0;
		_coeff2 =  0;
		_coeff3 =  0;
		_coeff4 =  0;
		return;
	}
	else {
		int type = _type;
		if (type > F_NONE) type--;
		_coeff0 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][0];
		_coeff1 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][1];
		_coeff2 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][2];
		_coeff3 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][3];
		_coeff4 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][4];
	}
}

}}}

