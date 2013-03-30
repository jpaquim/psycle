// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psycle::helpers::Filter.

#pragma once
#include <universalis.hpp>

namespace psycle { namespace helpers { namespace dsp {

const double TPI = 6.28318530717958647692528676655901;

enum FilterType {
	F_LOWPASS12 = 0,
	F_HIGHPASS12 = 1,
	F_BANDPASS12 = 2,
	F_BANDREJECT12 = 3,
	F_NONE = 4,//This one is kept here because it is used in load/save
	F_ITLOWPASS = 5,
	F_NUMFILTERS
};

class FilterCoeff {
	public:
		static FilterCoeff singleton;

		void setSampleRate(float samplerate);
		float getSampleRate() {return samplerate; };

		float _coeffs[F_NUMFILTERS-1][128][128][5];
	protected:
		FilterCoeff();
		void ComputeCoeffs(int freq, int r, int t);

		static float Cutoff(int v);
		static float Resonance(float v);
		static float Bandwidth(int v);

		float samplerate;
		double _coeff[5];
};

/// filter.
class Filter {
	public:
		Filter();
		virtual inline ~Filter(){};
		void Init(int sampleRate);
		void Reset(void);//Same as init, without samplerate

		void Cutoff(int iCutoff) { if ( _cutoff != iCutoff) { _cutoff = iCutoff; Update(); }}
		void Ressonance(int iRes) { if ( _q != iRes ) { _q = iRes; Update(); }}
		void SampleRate(int iSampleRate) {
			if ( FilterCoeff::singleton.getSampleRate() != iSampleRate) {
				FilterCoeff::singleton.setSampleRate(iSampleRate);
				Update();
			}
		}
		void Type (FilterType newftype) { if ( newftype != _type ) { _type = newftype; Update(); }}
		FilterType Type (void) { return _type; }


		virtual inline float Work(float x);
		virtual inline void WorkStereo(float& l, float& r);
	protected:
		void Update(void);

		FilterType _type;
		int _cutoff;
		int _q;

		float _coeff0;
		float _coeff1;
		float _coeff2;
		float _coeff3;
		float _coeff4;
		float _x1, _x2, _y1, _y2;
		float _a1, _a2, _b1, _b2;
};


class ITFilter : public Filter {

	public:
		ITFilter(){}
		virtual inline ~ITFilter() throw() {}
	
		/*override*/ inline float Work(float sample);
		/*override*/ inline void WorkStereo(float & left, float & right);
};


inline float Filter::Work(float x) {
	float y = x*_coeff0 + _x1*_coeff1 + _x2*_coeff2 + _y1*_coeff3 + _y2*_coeff4;
	_y2 = _y1;  _y1 = y;
	_x2 = _x1;  _x1 = x;
	return y;
}
inline void Filter::WorkStereo(float& l, float& r) {
	float y = l*_coeff0 + _x1*_coeff1 + _x2*_coeff2 + _y1*_coeff3 + _y2*_coeff4;
	_y2 = _y1;  _y1 = y;
	_x2 = _x1;  _x1 = l;
	l = y;
	float b = r*_coeff0 + _a1*_coeff1 + _a2*_coeff2 + _b1*_coeff3 + _b2*_coeff4;
	_b2 = _b1;  _b1 = b;
	_a2 = _a1;  _a1 = r;
	r = b;
}

/*Code from Schism Tracker, with small modifications. Note: full scale is 32768*/
#define CLAMP(N,L,H) (((N)>(H))?(H):(((N)<(L))?(L):(N)))
#define FILT_CLIP(i) CLAMP(i, -4096.f, 4096.f)
inline float ITFilter::Work(float sample) {
	float y = sample * _coeff0
		+ _y1 * (_coeff3 + _coeff4)  + FILT_CLIP((_y2-_y1) * _coeff4);
	_y2 = _y1;   _y1 = y;
	_x2 = _x1;   _x1 = sample;
	return y;
}
inline void ITFilter::WorkStereo(float & left, float & right) {
	float y = left * _coeff0 
		+ _y1 * (_coeff3 + _coeff4)  + FILT_CLIP((_y2-_y1) * _coeff4);
	_y2 = _y1;   _y1 = y;
	_x2 = _x1;   _x1 = left;
	left = y;

	y = right * _coeff0 
		+ _b1 * (_coeff3 + _coeff4) + FILT_CLIP((_b2-_b1) * _coeff4);
	_b2 = _b1;   _b1 = y;
	_a2 = _a1;   _a1 = right;
	right = y;
}
#undef CLAMP
#undef FILT_CLIP

}}}

