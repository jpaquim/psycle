// -*- mode:c++; indent-tabs-mode:t -*-
#include "LowPass.hpp"

CLowpass::CLowpass()
{
	o1=0.0f;
}

CLowpass::~CLowpass()
{

}

float CLowpass::Process(float i,float c)
{
	float output= o1 + c * (i-o1);
	o1=output;

	return output;
}
