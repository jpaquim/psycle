#include "LowPass.hpp"

CLowpass::CLowpass(): o1(0.0f), cutoff(1.0f)
{
}

CLowpass::~CLowpass() throw()
{

}

void CLowpass::setCutoff(float c) {
	cutoff = (c < 1.0f) ? c : 1.0f;
}