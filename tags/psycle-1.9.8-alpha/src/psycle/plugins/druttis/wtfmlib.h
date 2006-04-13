//============================================================================
//
//	wtfmlib.h
//
//============================================================================
#pragma once
//============================================================================
//	Defines
//============================================================================
#ifndef WAVESIZE
#define WAVESIZE 4096
#define WAVEMASK 4095
#endif
//============================================================================
//	Always good to have defines
//============================================================================
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

//============================================================================
//	afloat structure (source, target, current) values
//============================================================================
typedef struct
{
	float	source;
	float	target;
	float	current;
}
afloat;
//============================================================================
//	SetAFloat
//============================================================================
__forceinline void SetAFloat(afloat *afloat, float value)
{
	afloat->source = afloat->current;
	afloat->target = value;
}
//============================================================================
//	AnimateAFloat
//============================================================================
__forceinline void AnimateAFloat(afloat* p, float fac)
{
	if (fac == 0.0f)
		p->current = p->target;
	float dist = p->target - p->source;
	float maxdist = dist * fac;
	if (dist > 0.0f) {
		p->current += min(maxdist, dist);
		if (p->current > p->target)
			p->current = p->target;
	} else if (p->current > p->target) {
		p->current += max(maxdist, dist);
		if (p->current < p->target)
			p->current = p->target;
	}
	if (p->current == p->target)
		p->source = p->current;
}
//============================================================================
//	Function to replace ordinary float to int operation
//============================================================================
__forceinline int f2i(double d)
{
	const double magic = 6755399441055744.0;
	register double tmp = (d - 0.5) + magic;
	return *(int*) &tmp;
};
//============================================================================
//	Clips phase for wavetable
//============================================================================
__forceinline float ClipWTPhase(float phase)
{
	while (phase < 0.0f)
		phase += (float) WAVESIZE;
	while (phase >= (float) WAVESIZE)
		phase -= (float) WAVESIZE;
	return phase;
//	return (float) (f2i(phase) & WAVEMASK) + (phase - (float) f2i(phase));
}
//============================================================================
//	Get wavetable sample
//============================================================================
__forceinline float GetWTSample(float *wavetable, float phase)
{
	return wavetable[f2i(phase) & WAVEMASK];
}
//============================================================================
//	Get wavetable sample with linear interpolation
//============================================================================
__forceinline float GetWTSampleLinear(float *wavetable, float phase)
{
	register int pos = f2i(phase);
	static float fraction = phase - (float) pos;
	register float out = wavetable[pos & WAVEMASK];
	++pos;
	return out + fraction * (wavetable[pos & WAVEMASK] - out);
}
