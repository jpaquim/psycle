//============================================================================
//
//	CVoice.h
//	--------
//	druttis@darkface.pp.se
//
//============================================================================
#pragma once
#include "..\lib\AMath.h"
#include "..\lib\BiQuad.h"
#include "..\lib\DLineN.h"
#include "..\lib\CEnvelope.h"

#define EXCITATION_FILTER_LENGTH 3

//============================================================================
//	Voice globals
//============================================================================
typedef struct globals_t
{
	//
	//	This should always be here
	int		srate;
	//
	//	Declare your global synth variables here
	float	pluckPos;
	float	damping;
	float	volume;
	float	vib_amount;
	float	vib_speed;
	float	vib_delay;
	float	schoolness;
} GLOBALS;
//============================================================================
//	CVoice class
//============================================================================
class CVoice
{
	//------------------------------------------------------------------------
	//	Data
	//------------------------------------------------------------------------
public:
	//
	//	These two should always be here
	GLOBALS		*globals;
	int			ticks_remaining;
	//	
	//	Declare your runtime variables here
	DLineN		delayLine;
	DLineN		combDelay;
	BiQuad		loopFilt;
	long		length;
	float		loopGain;
	float		lastFreq;
	float		lastLength;
	float		pluckAmp;
	float		pluckPos;
	float		plucker;
	float		lastOutput;
	//
	CEnvelope	vca;
	//
	//	Runtime variables
	float		vib_phase;
	float		vib_dtime;
	float		vib_out;
	//
	//
	float		currentFreq;
	float		slideSpeed;
	int			noteDelay;
	float		lastCurrentFreq;
	//------------------------------------------------------------------------
	//	Methods
	//------------------------------------------------------------------------
public:
	CVoice();
	~CVoice();
	void Init();
	void Clear();
	void SetFreq(float frequency);
	void Pluck(float amplitude);
	void Pluck(float amplitude, float position);
	//
	void Stop();
	void NoteOff();
	void NoteOn(int note, int volume, int cmd, int val);
	//------------------------------------------------------------------------
	//	IsFinished
	//	returns true if voice is done playing
	//------------------------------------------------------------------------
	__forceinline bool IsActive()
	{
		return vca.IsActive();
	}
	//------------------------------------------------------------------------
	//	GlobalTick
	//	Method to handle parameter inertia and suchs things
	//------------------------------------------------------------------------
	__forceinline static void GlobalTick()
	{
	}
	//------------------------------------------------------------------------
	//	VoiceTick
	//	Method to handle voice specific things as LFO and envelopes
	//	* tips, dont handle amplitude envelopes or lfo's here
	//------------------------------------------------------------------------
	__forceinline void VoiceTick()
	{
		//
		//	Slide control
		if (noteDelay == 0) {
			if (slideSpeed != 0.0f) {
				currentFreq += slideSpeed;
				if (((slideSpeed < 0.0f) && (currentFreq < lastFreq)) || ((slideSpeed > 0.0f) && (currentFreq > lastFreq))) {
					currentFreq = lastFreq;
					slideSpeed = 0.0f;
				}
			}
			//
			//	Vibrato
			if (globals->vib_delay == 0.0f) {
				vib_out = 1.0f;
			} else {
				vib_out = vib_dtime;
				if (vib_dtime < 1.0f) {
					vib_dtime += 1.0f / globals->vib_delay;
					if (vib_dtime > 1.0f)
						vib_dtime = 1.0f;
				}
			}
			vib_out *= globals->vib_amount * (float) sin(vib_phase * PI2);
			vib_phase += globals->vib_speed;
			vib_phase -= (float) f2i(vib_phase);		// Mod! :)
			float loopFilterDelay = 0.5f;
			float freq = currentFreq + vib_out * currentFreq;
			float length = (globals->srate / freq);
			delayLine.SetDelay(length - loopFilterDelay);
			loopGain = globals->vib_amount * globals->vib_speed * 30.0f + globals->damping + (currentFreq * 0.000005f) + (float) fabs(currentFreq - lastCurrentFreq) * 0.00005f;
			if (loopGain > 1.1111f)
				loopGain = 1.1111f;
		} else {
			noteDelay--;
		}
	}
	//------------------------------------------------------------------------
	//	Work
	//	all sound generation is done here
	//------------------------------------------------------------------------
	__forceinline void Work(float *left, float *right, int samps)
	{
		float out = 0.0f;
		float stringInput = plucker;
		do {
			if (noteDelay == 0) {
				stringInput -= combDelay.Tick(plucker * pluckAmp);
				plucker = 0.0f;
				lastOutput = delayLine.Tick(loopFilt.Tick(stringInput + delayLine.GetLastOutput() * loopGain));
			} else {
				lastOutput = 0.0f;
			}
			out = lastOutput * vca.Tick() * globals->volume;
			*++left += out;
			*++right += out;
		} while (--samps);
	}
};
