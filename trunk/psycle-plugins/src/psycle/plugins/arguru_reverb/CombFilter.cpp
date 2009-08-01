#include "CombFilter.hpp"
#include <universalis/os/aligned_memory_alloc.hpp>

CCombFilter::CCombFilter() {
	universalis::os::aligned_memory_alloc(16, leftBuffer, MAX_COMB_DELAY);
	universalis::os::aligned_memory_alloc(16, rightBuffer, MAX_COMB_DELAY);
	Counter = MAX_COMB_DELAY - 4;
}

CCombFilter::~CCombFilter() throw() {
	universalis::os::aligned_memory_dealloc(leftBuffer);
	universalis::os::aligned_memory_dealloc(rightBuffer);
}

void CCombFilter::Clear() {
	for(int c = 0; c < MAX_COMB_DELAY; ++c) {
		leftBuffer[c] = 0;
		rightBuffer[c] = 0;
	}
	left_output = 0;
	right_output = 0;
}

void CCombFilter::Initialize(int time, int stph) {
	l_delayedCounter = Counter - time;
	r_delayedCounter = l_delayedCounter - stph;
	if(l_delayedCounter < 0) l_delayedCounter = 0;
	if(r_delayedCounter < 0) r_delayedCounter = 0;
}
