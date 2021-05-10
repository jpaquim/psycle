#include "../detail/prefix.h"

#include "AllPass.hpp"
// #include <universalis/os/aligned_alloc.hpp>
#include <operations.h>
// #include <psycle/helpers/dsp.hpp>

// Allpass filter implementation
//
// Originally Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

allpass::allpass()
: buffer(0), feedback(0), bufsize(0), bufidx(0)
{}

allpass::~allpass()
{
	deletebuffer();
}

void allpass::setbuffer(int samples)
{
	if (bufsize) {
		deletebuffer();
	}
	bufsize = samples;
	buffer = (float*)dsp.memory_alloc(16, (bufsize + 3) & 0xFFFFFFFC);	
	if(bufidx>=bufsize) bufidx = 0;
}

void allpass::mute()
{
	dsp.clear(buffer, bufsize);
}

void allpass::setfeedback(float val) 
{
	feedback = val;
}

float allpass::getfeedback() 
{
	return feedback;
}

void allpass::deletebuffer() {
	if (bufsize) {
		dsp.memory_dealloc(buffer);		
	}
}
