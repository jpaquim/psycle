#include "LowPass.hpp"

CLowpass::CLowpass(): cutoff(1.0f), o1(0.0f) {}

void CLowpass::setCutoff(float c) {
	cutoff = (c < 1.0f) ? c : 1.0f;
}
