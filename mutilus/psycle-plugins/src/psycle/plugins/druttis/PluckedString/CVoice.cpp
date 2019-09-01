//============================================================================
//
//				CVoice.cpp
//				----------
//				druttis@darkface.pp.se
//
//============================================================================
#include <packageneric/pre-compiled.private.hpp>
#include "CVoice.h"
#include "../Lib/AMath.h"
//============================================================================
//				Constructor
//============================================================================
CVoice::CVoice()
{
}
//============================================================================
//				Destructor
//============================================================================
CVoice::~CVoice()
{
}
//============================================================================
//				Init
//============================================================================
void CVoice::Init()
{
	float lowestFreq = 20.0f;
	currentFreq = -1.0f;
	length = (int) (globals->srate / lowestFreq + 1.0f);
	lastLength = (float) length;
	lastFreq = lowestFreq;
	loopGain = 0.999f;
	delayLine.Init(length);
	combDelay.Init(length);
	pluckAmp = 0.3f;
	pluckPos = 0.25f;
	combDelay.SetDelay(pluckPos * lastLength + 0.5f);
	loopFilt.Init();
	loopFilt.SetB1(-1.03f);
	loopFilt.SetB2(0.2154f);
	loopFilt.SetA1(-1.33373f);
	loopFilt.SetA2(0.446191f);
	loopFilt.SetGain(0.604595f);
	Clear();
	vca.SetAttack(0.0f);
	vca.SetDecay(0.0f);
	vca.SetSustain(1.0f);
	vca.SetRelease(256.0f);

	globals->vib_speed = 192.0f / globals->srate;
	globals->vib_amount = 0.05f;
	globals->vib_delay = 256.0f;

}
//============================================================================
//				Clear
//============================================================================
void CVoice::Clear()
{
	delayLine.Clear();
	combDelay.Clear();
	loopFilt.Clear();
}
//============================================================================
//				SetFreq
//============================================================================
void CVoice::SetFreq(float frequency)
{
	float loopFilterDelay = 0.5f;
	lastFreq = frequency;
	lastLength = (globals->srate / lastFreq);
	delayLine.SetDelay(lastLength - loopFilterDelay);
	loopGain = globals->damping + (frequency * 0.00005f);
	if (loopGain > 1.0f)
		loopGain = 1.0f;
}
//============================================================================
//				Pluck
//============================================================================
void CVoice::Pluck(float amplitude)
{
	pluckAmp = amplitude;
	plucker = amplitude;
}
//============================================================================
//				Pluck
//============================================================================
void CVoice::Pluck(float amplitude, float position)
{
	pluckPos = ((position < 0.0f) ? 0.0f : ((position > 1.0f) ? 1.0f : position));
	combDelay.SetDelay(pluckPos * lastLength + 0.5f);
	Pluck(amplitude);
}
//============================================================================
//				Stop
//============================================================================
void CVoice::Stop()
{
	vca.Stop();
	currentFreq = -1.0f;
}
//============================================================================
//				NoteOff
//============================================================================
void CVoice::NoteOff()
{
	loopGain *= 0.995f;
	vca.NoteOff();
}
//============================================================================
//				NoteOn
//============================================================================
void CVoice::NoteOn(int note, int volume, int cmd, int val)
{
	float position = globals->pluckPos;
	//
	//
	if (cmd == 0x0e) {
//								position = (float) val / 255.0f;
		noteDelay = val;
	} else {
		noteDelay = 0;
	}
	//
	//
	float sn = globals->schoolness * (float) sin((float) note * PI2 / 9.12345f);
	SetFreq(GetFreq((float) note + sn));
	if ((cmd != 0x0d) || (currentFreq == -1)) {
		Clear();
		Pluck((float) volume / 255.0f, position);
		slideSpeed = 0.0f;
		currentFreq = lastFreq;
	} else {
		if (val == 0)
			val = 16;
		slideSpeed = (lastFreq - currentFreq) / (float) (val * 2);
	}
	lastCurrentFreq = currentFreq;
	//
	//
	vib_phase = 0.0f;
	vib_dtime = 0.0f;
	vca.NoteOn(0.0f);
}


