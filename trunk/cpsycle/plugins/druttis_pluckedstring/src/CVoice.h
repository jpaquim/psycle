//============================================================================
//
//				CVoice.h
//				--------
//				druttis@darkface.pp.se
//
//============================================================================
#pragma once
#include "CDsp.h"
#include "CEnvelope.h"
#include "DLineN.h"
#include "BiQuad.h"
// #include <psycle/helpers/math.hpp>

#pragma warning(disable: 4244)

namespace math {
	/******************************************************************************************/
	/// C1999 *rint* - converts a floating point number to an integer by rounding in an unspecified way.
	/// This function has the same semantic as C1999's *rint* series of functions,
	/// but with C++ overload support, we don't need different names for each type.
	/// On C1999, the rounding mode may be set with fesetround, but msvc does not support it, so the mode is unspecified.
	/// Because the rounding mode is left as is, this is faster than using static_cast (trunc), floor, ceil, or round, which need to change it temporarily.
	template<typename IntegralResult, typename Real> // UNIVERSALIS__COMPILER__CONST_FUNCTION
	IntegralResult inline rint(Real x) noexcept {
		return x;
	}
};

using namespace math;


#define EXCITATION_FILTER_LENGTH 3

//============================================================================
//				Voice globals
//============================================================================
struct GLOBALS
{
	//
	//				This should always be here
	int					srate;
	//
	//position of the pluck [0..1] (affects waveform)
	float				pluckPos;
	//factor of amplitude reduction per sample (affects the note fade)
	float				damping;
	//volume of the out signal (also max value of signal)
	float				volume;
	//amount of vibrato, in octaves.
	float				vib_amount;
	//speed change for each num_ticks 
	float				vib_speed;
	//amount of samples to wait to start vibrating (divided by num_ticks)
	float				vib_delay;
	//Max seminote deviation [0..0.5]
	float				schoolness;
	//Mode to generate the waveform 0 = old mode, 1 = new mode.
	float				mode;
};
//============================================================================
//				CVoice class
//============================================================================
class CVoice
{
	//------------------------------------------------------------------------
	//				Data
	//------------------------------------------------------------------------
public:
	//
	//				These two should always be here
	GLOBALS				*globals;
	int					ticks_remaining;
	//				
	//				Declare your runtime variables here
	//	Delay used for the feedback to the filter
	DLineN				delayLine;
	//  Delay to define the waveform (see pluckAmp and plucked)
	DLineN				combDelay;
	//	Filter for the signal.
	BiQuad				loopFilt;
	//  Feedback amount to the filter. Used in conjunction with damping to determine
	//  the fadeout of the signal
	float				loopGain;
	//  frequency (Hz) of the last used note.
	float				lastFreq;
	//  lenth in samples of the period of the last used note.
	float				lastLength;
	//amplitude of the plucker
	//it is used to define the waveform.
	float				pluckAmp;
	//on/off of the pluck signal.
	float				plucker;
	// amplitude envelope. It is used only for the noteoff.
	CEnvelope			vca;
	//
	//				Runtime variables
	// Phase position for vibrato [0..1]
	float				vib_phase;
	float				vib_dtime;
	float				vib_out;
	//
	//
	float				currentFreq;
	//	
	float				slideSpeed;
	//		delay in samples/NUM_TICKS
	int					noteDelay;
	float				lastCurrentFreq;
	//------------------------------------------------------------------------
	//				Methods
	//------------------------------------------------------------------------
public:
	void Init();
	void Clear();
	void SetFreq(float frequency);
	void Pluck(float amplitude);
	void UpdateSampleRate();
	//
	void Stop();
	void NoteOff();
	void NoteOn(int note, int volume, int cmd, int val);
	inline bool IsActive();
	inline static void GlobalTick();
	inline void VoiceTick();
	inline void Work(float *left, float *right, int samps);
};
//------------------------------------------------------------------------
//				IsFinished
//				returns true if voice is done playing
//------------------------------------------------------------------------
inline bool CVoice::IsActive()
{
	return !vca.IsFinished();
}
//------------------------------------------------------------------------
//				GlobalTick
//				Method to handle parameter inertia and suchs things
//------------------------------------------------------------------------
inline void CVoice::GlobalTick()
{
}
//------------------------------------------------------------------------
//				VoiceTick
//				Method to handle voice specific things as LFO and envelopes
//				* tips, dont handle amplitude envelopes or lfo's here
//------------------------------------------------------------------------
inline void CVoice::VoiceTick()
{
	//
	//				Slide control
	if (noteDelay == 0) {
		if (slideSpeed != 0.0f) {
			currentFreq += slideSpeed;
			if (((slideSpeed < 0.0f) && (currentFreq < lastFreq)) || ((slideSpeed > 0.0f) && (currentFreq > lastFreq))) {
				currentFreq = lastFreq;
				slideSpeed = 0.0f;
			}
		}
		//
		//				Vibrato
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
		vib_phase -= (float) rint<int>(vib_phase);								// Mod! :)
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
//				Work
//				all sound generation is done here
//------------------------------------------------------------------------
inline void CVoice::Work(float *left, float *right, int samps)
{	
	float out = 0.0f;
	float stringInput = plucker;
	if (noteDelay == 0) {
		do {
			stringInput -= combDelay.Tick(plucker * pluckAmp);
			plucker=0.0f;
			out = delayLine.Tick(loopFilt.Tick(stringInput + delayLine.GetLastOutput())
				* loopGain)	* vca.Next() * globals->volume;
			*++left += out;
			*++right += out;
		} while (--samps);
	}
}