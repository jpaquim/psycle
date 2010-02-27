// Waveform.h
// druttis@darkface.pp.se

#pragma once
#include "DspMath.h"

// Constants
#define WAVESIBI 8
#define WAVESIZE 256
#define WAVEMASK 255
#define WAVEFSIZE 256.0f

// Waveform constants
#define WF_BLANK 0
#define WF_SINE 1
#define WF_BLTRIANGLE 2
#define WF_BLSQUARE 3
#define WF_BLSAWTOOTH 4
#define WF_BLPARABOLA 5
#define WF_TRIANGLE 6
#define WF_SQUARE 7
#define WF_SAWTOOTH 8
#define WF_REVSAWTOOTH 9

/// WAVEFORM type
struct WAVEFORM {
	int index; // Index of waveform (wavenumber)
	//int count; // How many shares this wave now.
	char const *pname; // Name
	float *pdata; // Data, partial or non partial
	int *preverse; // Lookup table to find pdata offset
};

extern float incr2freq;

/// Waveform
class Waveform {
	private:
		WAVEFORM m_wave;
	public:
		Waveform();
		~Waveform();
		
		inline WAVEFORM *Get() { return &m_wave; }
		bool Get(int index);

		/// Returns a linear interpolated sample by phase (NO BANDLIMIT)
		inline float GetSample(float phase) {
			register int offset = lrint<int>(phase);
			const float frac = phase - (float) offset;
			const float out = m_wave.pdata[offset & WAVEMASK];
			return out + (m_wave.pdata[++offset & WAVEMASK] - out) * frac;
		}

		/// Returns a linear interpolated sample by phase and index
		inline float GetSample(float phase, int index) {
			const float *pdata = &m_wave.pdata[m_wave.preverse[index] << WAVESIBI];
			register int offset = lrint<int>(phase);
			const float frac = phase - (float) offset;
			const float out = pdata[offset & WAVEMASK];
			return out + (pdata[++offset & WAVEMASK] - out) * frac;
		}

		/// Returns a linear interpolated sample by phase and incr
		inline float GetSample(float phase, float incr) {
			return GetSample(phase, lrint<int>(incr * incr2freq) & 0xffff);
		}
};
