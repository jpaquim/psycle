///\file
///\brief interface file for psycle::host::Filter.
#pragma once
#include <cmath>
#include "Configuration.hpp"
namespace psycle
{
	namespace host
	{
	namespace dsp
	{
		#define TPI 6.28318530717958647692528676655901

		enum FilterType{
			F_LOWPASS12 = 0,
			F_HIGHPASS12 = 1,
			F_BANDPASS12 = 2,
			F_BANDREJECT12 = 3,
			F_NONE = 4
		};

		class FilterCoeff
		{
		public:
			float _coeffs[5][128][128][5];
			FilterCoeff() { _inited = false; };
			inline void Init(void)
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
								ComputeCoeffs(f, q, r);
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
			void ComputeCoeffs(int freq, int r, int t)
			{
				///\todo use unified function instead
				float omega = float(TPI*Cutoff(freq)/Global::pConfig->_pOutputDriver->_samplesPerSec);
				float sn = (float)sin(omega);
				float cs = (float)cos(omega);
				float alpha;
				if (t < 2) alpha = float(sn / Resonance( r *(freq+70)/(127.0f+70)));
				else alpha = float(sn * sinh( Bandwidth( r) * omega/sn));
				float a0, a1, a2, b0, b1, b2;
				switch (t)
				{
				case F_LOWPASS12:
					b0 =  (1 - cs)/2;
					b1 =   1 - cs;
					b2 =  (1 - cs)/2;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_HIGHPASS12:
					b0 =  (1 + cs)/2;
					b1 = -(1 + cs);
					b2 =  (1 + cs)/2;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_BANDPASS12:
					b0 =   alpha;
					b1 =   0;
					b2 =  -alpha;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_BANDREJECT12:
					b0 =   1;
					b1 =  -2*cs;
					b2 =   1;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				default:
					b0 =  0;
					b1 =  0;
					b2 =  0;
					a0 =  0;
					a1 =  0;
					a2 =  0;
				}
				_coeff[0] = b0/a0;
				_coeff[1] = b1/a0;
				_coeff[2] = b2/a0;
				_coeff[3] = -a1/a0;
				_coeff[4] = -a2/a0;
			};

			static inline float Cutoff(int v)
			{
				return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
			};
			
			static inline float Resonance(float v)
			{
				return float(pow( v/127.0, 4)*150+0.1);
			};
			
			static inline float Bandwidth(int v)
			{
				return float(pow( v/127.0, 4)*4+0.1);
			};
		};

		/// filter.
		class Filter
		{
		public:
			FilterType _type;
			int _cutoff;
			int _q;

			Filter();

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


		class ITFilter
		{
		#define LOG10 2.30258509299 // neperian log10
		public:
			ITFilter()
				:  iSampleRate(44100)
			{
				Reset();
			};
			virtual ~ITFilter(){};
			void Reset(void)
			{
				iCutoff=127;
				iRes=0;
				fLastSampleLeft[0]=0.0f;
				fLastSampleLeft[1]=0.0f;
				fLastSampleRight[0]=0.0f;
				fLastSampleRight[1]=0.0f;
				Update();
			};
			void Cutoff(int _iCutoff) { if ( _iCutoff != iCutoff) { iCutoff = _iCutoff; Update(); }};
			void Ressonance(int _iRes) { if ( _iRes != iRes ) { iRes = _iRes; Update(); }};
			void SampleRate(int _iSampleRate) { if ( _iSampleRate != iSampleRate) {iSampleRate = _iSampleRate; Update(); }};
			inline void Work(float& _fSample)
			{
				const float ftemp = fLastSampleLeft[1];
				fLastSampleLeft[1] = (_fSample* fCoeff[0]) + (fLastSampleLeft[1]* fCoeff[1]) + (fLastSampleLeft[0]* fCoeff[2]);
				fLastSampleLeft[0] = ftemp;
				_fSample=fLastSampleLeft[1];
			}
			inline void WorkStereo(float& _fLeft, float& _fRight)
			{
				const float ftempL = fLastSampleLeft[1];
				const float ftempR = fLastSampleRight[1];

				fLastSampleLeft[1] = (_fLeft* fCoeff[0]) + (fLastSampleLeft[1]* fCoeff[1]) + (fLastSampleLeft[0]* fCoeff[2]);
				fLastSampleLeft[0] = ftempL;
				fLastSampleRight[1] = (_fRight* fCoeff[0]) + (fLastSampleRight[1]* fCoeff[1]) + (fLastSampleRight[0]* fCoeff[2]);
				fLastSampleRight[0] = ftempR;
				_fLeft=fLastSampleLeft[1];
				_fRight=fLastSampleRight[1];
			}
		protected:
			void Update(void);

			int iSampleRate;
			int iCutoff;
			int iRes;
			float fCoeff[3];
			float fLastSampleLeft[2];
			float fLastSampleRight[2];
		};
	}
	}
}
/*
static void it_reset_filter_state(IT_FILTER_STATE *state)
{
	state->currsample = 0;
	state->prevsample = 0;
}



*/
/* IMPORTANT: This function expects one extra sample in 'src' so it can apply
* click removal. It reads size samples, starting from src[0], and writes its
* output starting at dst[pos]. The pos parameter is required for getting
* click removal right.
*/
/*
static void it_filter( IT_FILTER_STATE *state, sample_t *dst, long pos, sample_t *src, long size, int sampfreq, int cutoff, int resonance)
{
	float currsample = state->currsample;
	float prevsample = state->prevsample;

	float a, b, c;

	{
		float inv_angle = (float)(sampfreq * pow(0.5, 0.25 + cutoff*(1.0/(24<<IT_ENVELOPE_SHIFT))) * (1.0/(2*3.14159265358979323846*110.0)));
		float loss = (float)exp(resonance*(-LOG10*1.2/128.0));
		float d, e;

		d = (1.0f - loss) / inv_angle;
		if (d > 2.0f) d = 2.0f;
		d = (loss - d) * inv_angle;
		e = inv_angle * inv_angle;
		a = 1.0f / (1.0f + d + e);
		c = -e * a;
		b = 1.0f - a - c;
	}

	dst += pos;

#define MULSCA(a, b) ((int)((LONG_LONG)((a) << 4) * (b) >> 32))
#define SCALEB 12
	{
		int ai = (int)(a * (1 << (16+SCALEB)));
		int bi = (int)(b * (1 << (16+SCALEB)));
		int ci = (int)(c * (1 << (16+SCALEB)));
		sample_t csi = (sample_t)currsample;
		sample_t psi = (sample_t)prevsample;
		sample_t *dst_end = dst + size;
		while (dst < dst_end) {
			{
				sample_t nsi = MULSCA(*src++, ai) + MULSCA(csi, bi) + MULSCA(psi, ci);
				psi = csi;
				csi = nsi;
			}
			*dst++ += csi;
		}
		currsample = csi;
		prevsample = psi;
	}

	state->currsample = currsample;
	state->prevsample = prevsample;
}

#undef LOG10

*/