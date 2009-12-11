#pragma once
#include <psycle/helpers/math/erase_denormals.hpp>

// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

class allpass
{
public:
					allpass();
			void				setbuffer(float *buf, int size);
	inline  float				process(float inp);
			void				mute();
			void				setfeedback(float val);
			float				getfeedback();
// private:
	float				feedback;
	float				*buffer;
	int								bufsize;
	int								bufidx;
};

// Big to inline - but crucial for speed

inline float allpass::process(float input)
{
	float output;
	float bufout;
	
	bufout = buffer[bufidx];
	psycle::helpers::math::fast_erase_denormals_inplace(bufout);
	
	output = -input + bufout;
	buffer[bufidx] = input + (bufout*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}
