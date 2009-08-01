#include "AllPass.hpp"
#include <universalis/os/aligned_memory_alloc.hpp>

CAllPass::CAllPass() {
	universalis::os::aligned_memory_alloc(16, leftBuffer, MAX_ALLPASS_DELAY);
	universalis::os::aligned_memory_alloc(16, rightBuffer, MAX_ALLPASS_DELAY);
	smpleft = reinterpret_cast<unsigned int*>(&tmpleft);
	smpright = reinterpret_cast<unsigned int*>(&tmpright);
	Counter = MAX_ALLPASS_DELAY - 4;
	l_delayedCounter = 0;
	r_delayedCounter = 0;
	Clear();
}

CAllPass::~CAllPass() throw() {
	universalis::os::aligned_memory_dealloc(leftBuffer);
	universalis::os::aligned_memory_dealloc(rightBuffer);
}

void CAllPass::Clear() {
	for(int c = 0; c < MAX_ALLPASS_DELAY; ++c) {
		leftBuffer[c] = 0;
		rightBuffer[c] = 0;
	}
	left_output = 0;
	right_output = 0;
}

void CAllPass::Initialize(int time, int stph) {
	l_delayedCounter = Counter - time;
	r_delayedCounter = l_delayedCounter - stph;
	if(l_delayedCounter < 0) l_delayedCounter += MAX_ALLPASS_DELAY;
	if(r_delayedCounter < 0) r_delayedCounter += MAX_ALLPASS_DELAY;
}
