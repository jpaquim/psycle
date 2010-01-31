/*

		Copyright (C) 1999 Juhana Sadeharju
						kouhia at nic.funet.fi

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef PSYCLE__PLUGINS__GVERB__GVERB__INCLUDED
#define PSYCLE__PLUGINS__GVERB__GVERB__INCLUDED

#include "gverbdsp.h"
#include <cstdlib>
#include <cmath>
#include <cstring>

#define FDNORDER 4

class gverb {
	public:
		gverb(int, float, float, float, float, float, float, float, float);
		~gverb();
		void flush();
		inline void process(float, float *, float *);
		inline void set_roomsize(float);
		inline void set_revtime(float);
		inline void set_damping(float);
		inline void set_inputbandwidth(float);
		void set_earlylevel(float a) { earlylevel_ = a; }
		void set_taillevel(float a) { taillevel_ = a; }
	private:
		inline void fdnmatrix(float *a, float *b);
	private:
		int rate_;
		float inputbandwidth_;
		float taillevel_;
		float earlylevel_;
		damper inputdamper_;
		float maxroomsize_;
		float roomsize_;
		float revtime_;
		float maxdelay_;
		float largestdelay_;
		fixeddelay ** fdndels_;
		float * fdngains_;
		int * fdnlens_;
		damper ** fdndamps_;
		float fdndamping_;
		diffuser ** ldifs_;
		diffuser ** rdifs_;
		fixeddelay tapdelay_;
		int * taps_;
		float * tapgains_;
		float * d_;
		float * u_;
		float * f_;
		double alpha_;
};

/*
	* This FDN reverb can be made smoother by setting matrix elements at the
	* diagonal and near of it to zero or nearly zero. By setting diagonals to zero
	* means we remove the effect of the parallel comb structure from the
	* reverberation.  A comb generates uniform impulse stream to the reverberation
	* impulse response, and thus it is not good. By setting near diagonal elements
	* to zero means we remove delay sequences having consequtive delays of the
	* similar lenths, when the delays are in sorted in length with respect to
	* matrix element index. The matrix described here could be generated by
	* differencing Rocchesso's circulant matrix at max diffuse value and at low
	* diffuse value (approaching parallel combs).
	*
	* Example 1:
	* Set a(k,k), for all k, equal to 0.
	*
	* Example 2:
	* Set a(k,k), a(k,k-1) and a(k,k+1) equal to 0.
	*
	* Example 3: The transition to zero gains could be smooth as well.
	* a(k,k-1) and a(k,k+1) could be 0.3, and a(k,k-2) and a(k,k+2) could
	* be 0.5, say.
	*/

inline void gverb::fdnmatrix(float *a, float *b) {
	const float dl0 = a[0], dl1 = a[1], dl2 = a[2], dl3 = a[3];

	b[0] = 0.5f * (+dl0 + dl1 - dl2 - dl3);
	b[1] = 0.5f * (+dl0 - dl1 - dl2 + dl3);
	b[2] = 0.5f * (-dl0 + dl1 - dl2 + dl3);
	b[3] = 0.5f * (+dl0 + dl1 + dl2 + dl3);
}

inline void gverb::process(float x, float *yl, float *yr) {
	if (/*isnan(x) ||*/ std::abs(x) > 100000.0f) {
		x = 0.0f;
	}

	float z = inputdamper_.process(x);

	z = ldifs_[0]->process(z);

	for(unsigned int i = 0; i < FDNORDER; ++i) {
		u_[i] = tapgains_[i] * tapdelay_.read(taps_[i]);
	}
	tapdelay_.write(z);

	for(unsigned int i = 0; i < FDNORDER; ++i) {
		d_[i] = fdndamps_[i]->process(fdngains_[i] * fdndels_[i]->read(fdnlens_[i]));
	}

	float sum = 0.0f;
	float sign = 1.0f;
	for(unsigned int i = 0; i < FDNORDER; ++i) {
		sum += sign * (taillevel_ * d_[i] + earlylevel_ * u_[i]);
		sign = -sign;
	}
	sum += x * earlylevel_;
	float lsum = sum;
	float rsum = sum;

	fdnmatrix(d_, f_);

	for(unsigned int i = 0; i < FDNORDER; ++i) {
		fdndels_[i]->write(u_[i] + f_[i]);
	}

	lsum = ldifs_[1]->process(lsum);
	lsum = ldifs_[2]->process(lsum);
	lsum = ldifs_[3]->process(lsum);
	rsum = rdifs_[1]->process(rsum);
	rsum = rdifs_[2]->process(rsum);
	rsum = rdifs_[3]->process(rsum);

	*yl = lsum;
	*yr = rsum;
}

inline void gverb::set_roomsize(const float a) {
	if (a <= 1.0 /*|| isnan(a)*/) {
		roomsize_ = 1.0;
	} else {
		roomsize_ = a;
	}
	largestdelay_ = rate_ * roomsize_ * 0.00294f;

	fdnlens_[0] = std::floor(1.000000f * largestdelay_);
	fdnlens_[1] = std::floor(0.816490f * largestdelay_);
	fdnlens_[2] = std::floor(0.707100f * largestdelay_);
	fdnlens_[3] = std::floor(0.632450f * largestdelay_);

	for(unsigned int i = 0; i < FDNORDER; ++i) {
		fdngains_[i] = -std::pow(alpha_, fdnlens_[i]);
	}

	taps_[0] = 5 + std::floor(0.410f * largestdelay_);
	taps_[1] = 5 + std::floor(0.300f * largestdelay_);
	taps_[2] = 5 + std::floor(0.155f * largestdelay_);
	taps_[3] = 5 + std::floor(0.000f * largestdelay_);

	for(unsigned int i = 0; i < FDNORDER; ++i) {
		tapgains_[i] = std::pow(alpha_, taps_[i]);
	}
}

inline void gverb::set_revtime(float a) {
	revtime_ = a;

	float ga = 60.0f;
	float gt = revtime_;

	ga = std::pow(10.0f, -ga / 20.0f);
	double n = rate_ * gt;
	alpha_ = std::pow((double)ga, 1.0 / n);

	for(unsigned int i = 0; i < FDNORDER; ++i) {
		fdngains_[i] = -std::pow(alpha_, fdnlens_[i]);
	}
}

inline void gverb::set_damping(float a) {
	fdndamping_ = a;
	for(unsigned int i = 0; i < FDNORDER; ++i) {
		fdndamps_[i]->set(fdndamping_);
	}
}

inline void gverb::set_inputbandwidth(float a) {
	inputbandwidth_ = a;
	inputdamper_.set(1 - a);
}

#endif