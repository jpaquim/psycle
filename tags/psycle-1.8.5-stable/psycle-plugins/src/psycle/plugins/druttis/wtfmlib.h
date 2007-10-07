///\file
///\brief wave tables
#pragma once
#include <algorithm>

/// wave size
unsigned int const WAVESIZE = 4096;
/// wave size - 1
unsigned int const WAVEMASK = 4095;

/// afloat structure (source, target, current) values
struct afloat {
	float source;
	float target;
	float current;
};

/// SetAFloat
inline void SetAFloat(afloat *afloat, float value) {
	afloat->source = afloat->current;
	afloat->target = value;
}

/// AnimateAFloat
inline void AnimateAFloat(afloat* p, float fac) {
	if (fac == 0.0f) p->current = p->target;
	float const dist = p->target - p->source;
	float const maxdist = dist * fac;
	if (dist > 0.0f) {
		p->current += std::min(maxdist, dist);
		if (p->current > p->target) p->current = p->target;
	} else if (p->current > p->target) {
		p->current += std::max(maxdist, dist);
		if (p->current < p->target) p->current = p->target;
	}
	if (p->current == p->target) p->source = p->current;
}

/// Function to replace ordinary float to int operation
///\todo use psycle::helpers::math
inline int f2i(double d) {
	const double magic = 6755399441055744.0;
	union result_union {
		double d;
		int i;
	} result;
	result.d = (d - 0.5) + magic;
	return result.i;
}

/// Clips phase for wavetable
inline float ClipWTPhase(float phase) {
	while(phase < 0.0f) phase += (float) WAVESIZE;
	while(phase >= (float) WAVESIZE) phase -= (float) WAVESIZE;
	return phase;
	// return (float) (f2i(phase) & WAVEMASK) + (phase - (float) f2i(phase));
}

/// Get wavetable sample
inline float GetWTSample(float *wavetable, float phase) {
	return wavetable[f2i(phase) & WAVEMASK];
}

/// Get wavetable sample with linear interpolation
inline float GetWTSampleLinear(float *wavetable, float phase) {
	int const pos = f2i(phase);
	float const fraction = phase - (float) pos;
	float const out = wavetable[pos & WAVEMASK];
	return out + fraction * (wavetable[(pos + 1) & WAVEMASK] - out);
}

