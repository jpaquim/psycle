#pragma once

#include "comb.hpp"
#include "AllPass.hpp"
#include "tuning.hpp"

// Reverb model declaration
//
// Originally Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

class revmodel
{
public:
				revmodel();
		void	mute();
		void	recalculatebuffers(int samplerate);
		void	setdefaultvalues(int samplerate);
		void	processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip);
		void	processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip);
		void	setroomsize(float value);
		float	getroomsize();
		void	setdamp(float value);
		float	getdamp();
		void	setwet(float value);
		float	getwet();
		void	setdry(float value);
		float	getdry();
		void	setwidth(float value);
		float	getwidth();
		void	setmode(float value);
		float	getmode();
private:
		void	update();
private:
	float		gain;
	float		roomsize,roomsize1;
	float		damp,damp1;
	float		wet,wet1,wet2;
	float		dry;
	float		width;
	float		mode;

	// Comb filters
	comb				combL[numcombs];
	comb				combR[numcombs];

	// Allpass filters
	allpass				allpassL[numallpasses];
	allpass				allpassR[numallpasses];
};
