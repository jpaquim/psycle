#ifndef _FILTER_H
#define _FILTER_H

/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */
#include <math.h>
#include "configuration.h"

#define TPI 6.28318530717958647692528676655901

typedef enum
{
	FILTER_LP = 0,
	FILTER_HP = 1,
	FILTER_BP = 2,
	FILTER_BR = 3
}
FilterType;

class FilterCoeff
{
public:
	float _coeffs[5][128][128][5];

	FilterCoeff() { _inited = false; };

	void Init(const int samplesPerSec)
	{
		if (!_inited)
		{
			_inited = true;
			for (int r=0; r<5; r++)
			{
				for (int f=0; f<128; f++)
				{
					for (int q=0; q<128; q++)
					{
						ComputeCoeffs(f, q, r,samplesPerSec);
						_coeffs[r][f][q][0] = (float)_coeff[0];
						_coeffs[r][f][q][1] = (float)_coeff[1];
						_coeffs[r][f][q][2] = (float)_coeff[2];
						_coeffs[r][f][q][3] = (float)_coeff[3];
						_coeffs[r][f][q][4] = (float)_coeff[4];
					}
				}
			}
		}
	};
	
private:
	bool _inited;
	double _coeff[5];
	
	void ComputeCoeffs(const int freq, const int r, const int t,const int samplesPerSec)
	{
		float omega = 
			float(TPI * Cutoff(freq) / samplesPerSec);
		float sn = (float)sin(omega);
		float cs = (float)cos(omega);
		float alpha;
		
		if (t < 2)
		{
			alpha = float(sn / Resonance( r *(freq+70)/(127.0f+70)));
		}
		else
		{
			alpha = float(sn * sinh( Bandwidth( r) * omega/sn));
		}
		
		float a0(0.0f), a1(0.0f), a2(0.0f), b0(0.0f), b1(0.0f), b2(0.0f);

		switch (t)
		{
			
		case FILTER_LP:
			b0 =  (1.0f - cs) / 2.0f;
			b1 =   1.0f - cs;
			b2 =  (1.0f - cs) / 2.0f;
			a0 =   1.0f + alpha;
			a1 =  -2.0f * cs;
			a2 =   1.0f - alpha;
			break;
		case FILTER_HP:
			b0 =  (1.0f + cs) / 2.0f;
			b1 = -(1.0f + cs);
			b2 =  (1.0f + cs) / 2.0f;
			a0 =   1.0f + alpha;
			a1 =  -2.0f * cs;
			a2 =   1.0f - alpha;
			break;
		case FILTER_BP:
			b0 =   alpha;
			b1 =   0.0f;
			b2 =  -alpha;
			a0 =   1.0f + alpha;
			a1 =  -2.0f * cs;
			a2 =   1.0f - alpha;
			break;
		case FILTER_BR:
			b0 =   1.0f;
			b1 =  -2.0f * cs;
			b2 =   1.0f;
			a0 =   1.0f + alpha;
			a1 =  -2.0f * cs;
			a2 =   1.0f - alpha;
			break;
		}
		
		_coeff[0] = b0 / a0;
		_coeff[1] = b1 / a0;
		_coeff[2] = b2 / a0;
		_coeff[3] = -a1 / a0;
		_coeff[4] = -a2 / a0;
	};

	static inline float Cutoff(int v)
	{
		return float(pow( (v + 5.0)/(127.0 + 5.0), 1.7)*13000+30);
	};
	
	static inline float Resonance(float v)
	{
		return float(pow(v / 127.0, 4.0) * 150.0 + 0.1);
	};
	
	static inline float Bandwidth(int v)
	{
		return float(pow( v / 127.0, 4.0) * 4.0 + 0.1);
	};
};

class Filter
{
public:
	FilterType _type;
	int _cutoff;
	int _q;

	Filter(const int samplesPerSec);

	void Init(void);
	void Update(void);
	inline float Work(float x)
	{
		float y = _coeff0*x + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
		_y2 = _y1;
		_y1 = y;
		_x2 = _x1;
		_x1 = x;
		return y;
	};
	inline void WorkStereo(float& l, float& r)
	{
		float y = _coeff0*l + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
		_y2 = _y1;
		_y1 = y;
		_x2 = _x1;
		_x1 = l;
		l = y;
		float b = _coeff0*r + _coeff1*_a1 + _coeff2*_a2 + _coeff3*_b1 + _coeff4*_b2;
		_b2 = _b1;
		_b1 = b;
		_a2 = _a1;
		_a1 = r;
		r = b;
	};

protected:
	static FilterCoeff _coeffs;

	float _coeff0;
	float _coeff1;
	float _coeff2;
	float _coeff3;
	float _coeff4;

	float _x1, _x2, _y1, _y2;
	float _a1, _a2, _b1, _b2;
};

#endif
