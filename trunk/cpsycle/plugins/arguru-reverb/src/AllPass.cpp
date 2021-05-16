#include "../detail/prefix.h"


#include "AllPass.hpp"
#include <algorithm>
#include <operations.h>
// #include <universalis/os/aligned_alloc.hpp>
// #include <psycle/helpers/dsp.hpp>

CAllPass::CAllPass(): l_delayedCounter(0), r_delayedCounter(0),Counter(0),
left_output_(0.0f), right_output_(0.0f), bufferSize(0) {
}

CAllPass::~CAllPass() throw() {
	DeleteBuffer();
}

void CAllPass::Clear() {
	dsp.clear(leftBuffer, bufferSize);
	dsp.clear(rightBuffer, bufferSize);
}

void CAllPass::Initialize(int max_size, int time, int stph) {
	DeleteBuffer();
	//properly aligned.
	bufferSize = (max_size+3)&0xFFFFFF00;
	leftBuffer = (float*)dsp.memory_alloc(16, bufferSize);
	rightBuffer = (float*)dsp.memory_alloc(16, bufferSize);
	Clear();
	if(Counter>=bufferSize) Counter = 0;
	SetDelay(time,stph);
}

void CAllPass::SetDelay(int time, int stph) {
	l_delayedCounter = Counter - std::min(time, bufferSize);
	r_delayedCounter = l_delayedCounter - std::min(stph, bufferSize);
	if(l_delayedCounter < 0) l_delayedCounter += bufferSize;
	if(r_delayedCounter < 0) r_delayedCounter += bufferSize;
}

void CAllPass::DeleteBuffer() {
	if(bufferSize) {
		dsp.memory_dealloc(leftBuffer);
		dsp.memory_dealloc(rightBuffer);
	}
}
