#include <project.h>
#include <assert.h>
#include <psycle/plugin/jm_drums/oscillator.h>

// oscillator.cpp: implementation of the Coscillator class.
//
//////////////////////////////////////////////////////////////////////

Coscillator::Coscillator()
{
	hz=44100;
	hztosamples=MAX_ENVPOS/44100.0;
	incremspeed=0.015;
	envpos=0;
}

Coscillator::~Coscillator()
{

}

void Coscillator::setEnvPos(float epos)
{
	assert(epos<=MAX_ENVPOS);
	envpos=epos;
	
}
void Coscillator::setSampleRate(int nsamplerate)
{
	hz=nsamplerate;
	hztosamples=(float)MAX_ENVPOS/nsamplerate;
}
