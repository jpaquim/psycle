#include "CombFilter.hpp"
// #include <universalis/os/aligned_alloc.hpp>
#include <operations.h>
#include <algorithm>

CCombFilter::CCombFilter()
:
	leftBuffer(0), rightBuffer(0),
	l_delayedCounter(0), r_delayedCounter(0),
	Counter(0), bufferSize(0)
{}

CCombFilter::~CCombFilter() throw() {
	DeleteBuffer();
}

void CCombFilter::Clear() {
	dsp.clear(leftBuffer, bufferSize);	
	dsp.clear(rightBuffer, bufferSize);
}

void CCombFilter::SetDelay(int time, int stph) {
	l_delayedCounter = Counter - std::min(time, bufferSize);
	r_delayedCounter = l_delayedCounter - std::min(stph, bufferSize);
	if(l_delayedCounter < 0) l_delayedCounter += bufferSize;
	if(r_delayedCounter < 0) r_delayedCounter += bufferSize;
}

void CCombFilter::Initialize(int new_rate, int time, int stph){
	DeleteBuffer();
	//Allow up to 0.75 seconds of delay, properly aligned.
	bufferSize = (int(new_rate*0.75)+3)&0xFFFFFF00;
	leftBuffer = (float*)dsp.memory_alloc(16, bufferSize);
	rightBuffer = (float*)dsp.memory_alloc(16, bufferSize);
	Clear();
	if(Counter>=bufferSize)Counter=0;
	SetDelay(time, stph);
}

void CCombFilter::DeleteBuffer() {
	if (bufferSize) {
		dsp.memory_dealloc(leftBuffer);
		dsp.memory_dealloc(rightBuffer);		
	}
}
