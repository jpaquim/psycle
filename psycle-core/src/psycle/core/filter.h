// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psy::core::Filter.

#ifndef PSYCLE__CORE__FILTER__INCLUDED
#define PSYCLE__CORE__FILTER__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

namespace psy { namespace core { namespace dsp {

#define TPI 6.28318530717958647692528676655901

enum FilterType {
	F_LOWPASS12 = 0,
	F_HIGHPASS12 = 1,
	F_BANDPASS12 = 2,
	F_BANDREJECT12 = 3,
	F_NONE = 4
};

class PSYCLE__CORE__DECL FilterCoeff {
	public:
		float _coeffs[5][128][128][5];
		FilterCoeff();
		void setSampleRate(float samplerate);
		static FilterCoeff singleton;
	private:
		float samplerate;
		double _coeff[5];
		void ComputeCoeffs(int freq, int r, int t);

		static float Cutoff(int v);
		static float Resonance(float v);
		static float Bandwidth(int v);
};

/// filter.
class PSYCLE__CORE__DECL Filter {
	public:
		FilterType _type;
		int _cutoff;
		int _q;

		Filter();
		void Init(int sampleRate);
		void Update(void);
		float Work(float x) {
			float y = _coeff0*x + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
			_y2 = _y1;
			_y1 = y;
			_x2 = _x1;
			_x1 = x;
			return y;
		}
		void WorkStereo(float& l, float& r) {
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
		}
	protected:
		float _coeff0;
		float _coeff1;
		float _coeff2;
		float _coeff3;
		float _coeff4;
		float _x1, _x2, _y1, _y2;
		float _a1, _a2, _b1, _b2;
};


class PSYCLE__CORE__DECL ITFilter {
	#define LN10 2.30258509299 // neperian-base log of 10

	public:
		ITFilter() {
			SampleRate(44100);
			Reset();
		}

		virtual inline ~ITFilter() throw() {}

		void Reset(void) {
			ftFilter= F_NONE;
			iCutoff=127;
			iRes=0;
			iSampleCurrentSpeed = iSampleRate;
			dBand[0] = 0;
			dBand[1] = 0;
			dLow[0] = 0;
			dLow[1] = 0;
			fLastSampleLeft[0]=0.0f;
			fLastSampleLeft[1]=0.0f;
			fLastSampleRight[0]=0.0f;
			fLastSampleRight[1]=0.0f;
			Update();
		}

		void Cutoff(int _iCutoff) { if ( _iCutoff != iCutoff) { iCutoff = _iCutoff; Update(); }}
		void Ressonance(int _iRes) { if ( _iRes != iRes ) { iRes = _iRes; Update(); }}
		void SampleRate(int _iSampleRate) { if ( _iSampleRate != iSampleRate) {iSampleRate = _iSampleRate; Update(); }}
		void SampleSpeed(int _iSampleSpeed) { if ( iSampleCurrentSpeed != _iSampleSpeed) {iSampleCurrentSpeed = _iSampleSpeed; Update(); }}
		void Type (FilterType newftype) { if ( newftype != ftFilter ) { ftFilter = newftype; Update(); }}
		FilterType Type (void) { return ftFilter; }
	
		void Work(float & sample) {
			// This filter code comes from the musicdsp.org archive,
			// State Variable Filter (Double Sampled, Stable)
			// Type : 2 Pole Low, High, Band, Notch and Peaking
			// References : Posted by Andrew Simper

			double notch, high, out;

			notch = sample - fCoeff[damp]*dBand[0];
			dLow[0] = dLow[0] + fCoeff[freq]*dBand[0];
			high = notch - dLow[0];
			dBand[0] = fCoeff[freq]*high + dBand[0];
			//out = 0.5*(notch or low or high or band or peak);
			out = 0.5*dLow[0];
			notch = sample - fCoeff[damp]*dBand[0];
			dLow[0] = dLow[0] + fCoeff[freq]*dBand[0];
			high = notch - dLow[0];
			dBand[0] = fCoeff[freq]*high + dBand[0];
			//out += 0.5*(same out as above);
			out += 0.5*dLow[0];
			sample = static_cast< float >(out);
		}

		void WorkStereo(float & left, float & right) {
			double notch, high, out;

			notch = left - fCoeff[damp]*dBand[0];
			dLow[0] = dLow[0] + fCoeff[freq]*dBand[0];
			high  = notch - dLow[0];
			dBand[0] = fCoeff[freq]*high + dBand[0];
			//out   = 0.5*(notch or low or high or band or peak);
			out = 0.5*dLow[0];
			notch = left - fCoeff[damp]*dBand[0];
			dLow[0] = dLow[0] + fCoeff[freq]*dBand[0];
			high  = notch - dLow[0];
			dBand[0] = fCoeff[freq]*high + dBand[0];
			//out  += 0.5*(same out as above);
			out += 0.5*dLow[0];
			left = static_cast< float >(out);

			notch = right - fCoeff[damp]*dBand[1];
			dLow[1] = dLow[1] + fCoeff[freq]*dBand[1];
			high  = notch - dLow[1];
			dBand[1] = fCoeff[freq]*high + dBand[1];
			//out   = 0.5*(notch or low or high or band or peak);
			out = 0.5*dLow[1];
			notch = right - fCoeff[damp]*dBand[1];
			dLow[1] = dLow[1] + fCoeff[freq]*dBand[1];
			high  = notch - dLow[1];
			dBand[1] = fCoeff[freq]*high + dBand[1];
			//out  += 0.5*(same out as above);
			out += 0.5*dLow[1];
			right = static_cast< float >(out);
		}

		void WorkOld(float & sample) {
			try {
				const float fy = (sample * fCoeff[0]) + (fLastSampleLeft[1] * fCoeff[1]) + (fLastSampleLeft[0] * fCoeff[2]);
				fLastSampleLeft[0] = fLastSampleLeft[1];
				fLastSampleLeft[1] = fy - (sample * fCoeff[3]);
				sample = fy;
			}
			catch(...) //universalis::operating_system::exception const & e)
			{
				#if 0 ///\todo
					switch(e.code()) {
						case STATUS_FLOAT_DENORMAL_OPERAND:
						case STATUS_FLOAT_INVALID_OPERATION:
							fLastSampleLeft[1] = fLastSampleLeft[0] = 0;
							break;
						default: throw;
					}
				#endif
			}
		}

		void WorkStereoOld(float& left, float& right) {
			try {
				const float fyL = (left * fCoeff[0]) + (fLastSampleLeft[1] * fCoeff[1]) + (fLastSampleLeft[0] * fCoeff[2]);
				fLastSampleLeft[0] = fLastSampleLeft[1];
				fLastSampleLeft[1] = fyL - (left * fCoeff[3]);
				left = fyL;

				const float fyR = (right * fCoeff[0]) + (fLastSampleRight[1] * fCoeff[1]) + (fLastSampleRight[0] * fCoeff[2]);
				fLastSampleRight[0] = fLastSampleRight[1];
				fLastSampleRight[1] = fyR - (right * fCoeff[3]);
				right = fyR;
			}
			catch(...)//universalis::operating_system::exception const & e)
			{
				#if 0 ///\todo
					switch(e.code())
					{ 
									case STATUS_FLOAT_DENORMAL_OPERAND:
													fLastSampleLeft[0] = fLastSampleLeft[1] = fLastSampleRight[0] = fLastSampleRight[1] = 0;
													left *= fCoeff[0];
													right *= fCoeff[0];
													break;
									default: throw;
					}
				#endif
			}
		}

	protected:
		enum coeffNames {
			damp=0,
			band,
			freq
		};

		void Update(void);
		void UpdateOld(void);

		int iSampleRate;
		int iCutoff;
		int iRes;
		int iSampleCurrentSpeed;
		FilterType ftFilter;
		float fCoeff[4];
		double dBand[2];
		double dLow[2];
		float fLastSampleLeft[2];
		float fLastSampleRight[2];
};

}}}
#endif
