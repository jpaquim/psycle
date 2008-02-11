// -*- mode:c++; indent-tabs-mode:t -*-
#include <packageneric/pre-compiled.private.hpp>
#include "AllPass.hpp"

// Allpass filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

allpass::allpass()
{
	bufidx = 0;
}

void allpass::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

void allpass::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void allpass::setfeedback(float val) 
{
	feedback = val;
}

float allpass::getfeedback() 
{
	return feedback;
}
