// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsamplervoice.h"

// audio
#include "xmsampler.h"
#include "samples.h"
#include "songio.h"
#include "constants.h"
// dsp
#include <noteperiods.h>
#include <linear.h>
// std
#include <assert.h>
#include <math.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"

static const int m_FineSineData[256] = {
	0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
	0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2
};

static const int m_FineRampDownData[256] = {
	64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
	56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
	48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
	40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
	32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
	24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
	16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
	8,  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,
	0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
	-8, -9, -9,-10,-10,-11,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,
	-16,-17,-17,-18,-18,-19,-19,-20,-20,-21,-21,-22,-22,-23,-23,-24,
	-24,-25,-25,-26,-26,-27,-27,-28,-28,-29,-29,-30,-30,-31,-31,-32,
	-32,-33,-33,-34,-34,-35,-35,-36,-36,-37,-37,-38,-38,-39,-39,-40,
	-40,-41,-41,-42,-42,-43,-43,-44,-44,-45,-45,-46,-46,-47,-47,-48,
	-48,-49,-49,-50,-50,-51,-51,-52,-52,-53,-53,-54,-54,-55,-55,-56,
	-56,-57,-57,-58,-58,-59,-59,-60,-60,-61,-61,-62,-62,-63,-63,-64
};

static const int m_FineSquareTable[256] = {
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,
	-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64 ,-64
};

// Random wave table (randomly choosen values. Not official)
static const int m_RandomTable[256] = {
	 48,-64,-21, 45, 51, 20,-32,-57, 62, 13,-35,-43,-33,-16, -8,-48,
	  8, 36, 52, -3, 58,-34,-31,-20,  5,-30, 32, 54, -9,-19, -6,-38,
	-11, 43, 10,-47,  2, 53, 11,-56,  3, 55,  9,-44,-15,  4,-63, 59,
	 21,-17, 44, -2,-25,-36, 12,-14, 56, 61, 42,-50,-46, 49,-27,-45,
	 30, 63,-28, 29, 33, 22,-41, 57, 47, 19,-51,-54,-42,-22, -7,-61,
	 14, 25, 34, -4, 40,-49,-40,-26,  7,-39, 24, 37, -10,-24, -5,-53,
	 -12, 27, 16,-59,  0, 35, 17, 50,  1, 38, 15,-55,-18,  6, 60, 41,
	 23,-23, 28, -1,-29,-52, 18,-13, 39, 46, 26,-62,-58, 31,-37,-59,
	 30, 63,-28, 29, 33, 22,-41, 57, 47, 19,-51,-54,-42,-22, -7,-61,
	 21,-17, 44, -2,-25,-36, 12,-14, 56, 61, 42,-50,-46, 49,-27,-45,
	 14, 25, 34, -4, 40,-49,-40,-26,  7,-39, 24, 37, -10,-24, -5,-53,
	 -11, 43, 10,-47,  2, 53, 11,-56,  3, 55,  9,-44,-15,  4,-63, 59,
	 -12, 27, 16,-59,  0, 35, 17, 50,  1, 38, 15,-55,-18,  6, 60, 41,
	 8, 36, 52, -3, 58,-34,-31,-20,  5,-30, 32, 54, -9,-19, -6,-38,
	 23,-23, 28, -1,-29,-52, 18,-13, 39, 46, 26,-62,-58, 31,-37,-59,
	 48,-64,-21, 45, 51, 20,-32,-57, 62, 13,-35,-43,-33,-16, -8,-48,
};

// XMSampler::WaveDataController Implementation
// psy_audio_SampleIterator

// EnvelopeController
void xmenvelopecontroller_init(XMEnvelopeController* self,
	psy_audio_XMSamplerVoice* invoice,
	XMEnvelopeValueType defValue)
{
	self->voice = invoice;
	self->defaultValue = defValue;
}

void xmenvelopecontroller_dispose(XMEnvelopeController* self)
{

}

void xmenvelopecontroller_initcontroller(XMEnvelopeController* self)
{
	self->m_Samples = 0;
	self->m_PositionIndex = 0;
	self->m_ModulationAmount = 0;
	self->m_Step = 0;
	self->m_NextEventSample = 0;
	self->m_Stage = XMENVELOPESTAGE_OFF;
	self->m_sRateDeviation = 0;
}

void xmenvelopecontroller_initcontroller_envelope(XMEnvelopeController* self,
		const psy_dsp_EnvelopeSettings* envelope) {
	self->m_pEnvelope = envelope;
	xmenvelopecontroller_initcontroller(self);
}

void xmenvelopecontroller_noteon(XMEnvelopeController* self)
{
	self->m_Samples = 0;
	self->m_PositionIndex = 0;
	self->m_NextEventSample = 0;
	self->m_Stage = XMENVELOPESTAGE_OFF;
	xmenvelopecontroller_recalcdeviation(self);
	self->m_ModulationAmount = self->defaultValue;
	// if there are no points, there is nothing to do.
	if (psy_dsp_envelopesettings_numofpoints(self->m_pEnvelope) > 0)
	{
		//if (m_pEnvelope->IsEnabled() && m_pEnvelope->GetTime(1) != XMInstrument::Envelope::INVALID)
		{
			xmenvelopecontroller_start(self);
		}
	}
}

/// NoteOff EnvelopeStage
		// Explanation:
		// First we check if the envelope is active. If it is, we release the envelope.
		// Then if the envelope state is paused, we check if it was because of a sustain loop and in that case
		// we reenable it. The next work call, it will recheck the status and decide what does it need to do.
void xmenvelopecontroller_noteoff(XMEnvelopeController* self)
{
	self->m_Stage = (XMEnvelopeStage)(self->m_Stage | XMENVELOPESTAGE_RELEASED);
	// If we are paused, check why
	if ((self->m_Stage & XMENVELOPESTAGE_PAUSED) &&
			psy_dsp_envelopesettings_sustainbegin(self->m_pEnvelope) == self->m_PositionIndex
		&& !(psy_dsp_envelopesettings_loopstart(self->m_pEnvelope) ==
			self->m_PositionIndex && psy_dsp_envelopesettings_loopend(self->m_pEnvelope) == self->m_PositionIndex))
	{
		xmenvelopecontroller_continue(self);		
	}
}

void xmenvelopecontroller_stop(XMEnvelopeController* self)
{
	self->m_Stage = (XMEnvelopeStage)(self->m_Stage & (~XMENVELOPESTAGE_DOSTEP));
}

void xmenvelopecontroller_start(XMEnvelopeController* self)
{
	if (self->m_PositionIndex < psy_dsp_envelopesettings_numofpoints(self->m_pEnvelope)) {
		self->m_Stage = (XMEnvelopeStage)(self->m_Stage | XMENVELOPESTAGE_DOSTEP);
		if (self->m_Samples == 0) {
			//envelope is stopped. Le'ts do the first calc.
			xmenvelopecontroller_newstep(self);
		}
	}
};

void xmenvelopecontroller_pause(XMEnvelopeController* self)
{
	self->m_Stage = (XMEnvelopeStage)(self->m_Stage | XMENVELOPESTAGE_PAUSED);
}

void xmenvelopecontroller_continue(XMEnvelopeController* self)
{
	self->m_Stage = (XMEnvelopeStage)(self->m_Stage & (~XMENVELOPESTAGE_PAUSED));
}

void xmenvelopecontroller_setposition(XMEnvelopeController* self, int posi)
{
	self->m_PositionIndex = posi;
	xmenvelopecontroller_start(self);
	xmenvelopecontroller_newstep(self);
}

void xmenvelopecontroller_recalcdeviation(XMEnvelopeController* self)
{
	if (psy_dsp_envelopesettings_mode(self->m_pEnvelope) == psy_dsp_ENVELOPETIME_TICK) {
		self->m_sRateDeviation =
			(psy_audio_xmsamplervoice_samplerate(self->voice) * 60) /
			(psy_audio_machine_bpm(psy_audio_xmsampler_base(self->voice->m_pSampler)) *
			psy_audio_machine_ticksperbeat(psy_audio_xmsampler_base(self->voice->m_pSampler)));			
	} else if (psy_dsp_envelopesettings_mode(self->m_pEnvelope) == psy_dsp_ENVELOPETIME_SECONDS) {
		self->m_sRateDeviation = psy_audio_xmsamplervoice_samplerate(self->voice) / 1000.f;		
	}
}

void xmenvelopecontroller_work(XMEnvelopeController* self)
{
	if (!(self->m_Stage & XMENVELOPESTAGE_PAUSED))
	{
		if (++self->m_Samples >= self->m_NextEventSample) // m_NextEventSample is updated inside CalcStep()
		{
			self->m_PositionIndex++;
			xmenvelopecontroller_newstep(self);
		} else
		{
			self->m_ModulationAmount += self->m_Step;
		}
	}
}

void xmenvelopecontroller_newstep(XMEnvelopeController* self)
{
	if (psy_dsp_envelopesettings_sustainbegin(self->m_pEnvelope) != psy_dsp_ENVELOPEPOINT_INVALID
		&& !(self->m_Stage & XMENVELOPESTAGE_RELEASED))
	{
		if (self->m_PositionIndex == psy_dsp_envelopesettings_sustainend(self->m_pEnvelope))
		{
			// if begin==end, pause the envelope.
			if (psy_dsp_envelopesettings_sustainbegin(self->m_pEnvelope) ==
				psy_dsp_envelopesettings_sustainend(self->m_pEnvelope))
			{
				xmenvelopecontroller_pause(self);
			} else { self->m_PositionIndex = (int)psy_dsp_envelopesettings_sustainbegin(self->m_pEnvelope); }
		}
	} else if (psy_dsp_envelopesettings_loopstart(self->m_pEnvelope) != psy_dsp_ENVELOPEPOINT_INVALID)
	{
		if (self->m_PositionIndex >= psy_dsp_envelopesettings_loopend(self->m_pEnvelope))
		{
			// if begin==end, pause the envelope.
			if (psy_dsp_envelopesettings_loopstart(self->m_pEnvelope) == psy_dsp_envelopesettings_loopend(self->m_pEnvelope))
			{
				xmenvelopecontroller_pause(self);
			} else { self->m_PositionIndex = psy_dsp_envelopesettings_loopstart(self->m_pEnvelope); }
		}
	}
	if (psy_dsp_envelopesettings_time(self->m_pEnvelope, self->m_PositionIndex + 1) == psy_dsp_ENVELOPEPOINT_INVALID)
	{
		if (self->m_Stage & XMENVELOPESTAGE_PAUSED) {
			xmenvelopecontroller_calcstep(self, self->m_PositionIndex, self->m_PositionIndex);
		} else {
			self->m_Stage = XMENVELOPESTAGE_OFF;
			self->m_PositionIndex = psy_dsp_envelopesettings_numofpoints(self->m_pEnvelope);
			self->m_ModulationAmount = psy_dsp_envelopesettings_value(self->m_pEnvelope, self->m_PositionIndex - 1);
		}
	} else xmenvelopecontroller_calcstep(self, self->m_PositionIndex, self->m_PositionIndex + 1);
}

void xmenvelopecontroller_calcstep(XMEnvelopeController* self, int start, int end)
{
	const XMEnvelopeValueType ystep = (psy_dsp_envelopesettings_value(self->m_pEnvelope, end) - psy_dsp_envelopesettings_value(self->m_pEnvelope, start));
	const XMEnvelopeValueType xstep = (psy_dsp_envelopesettings_time(self->m_pEnvelope, end) - psy_dsp_envelopesettings_time(self->m_pEnvelope, start));
	xmenvelopecontroller_recalcdeviation(self);
	self->m_Samples = psy_dsp_envelopesettings_time(self->m_pEnvelope, start) * xmenvelopecontroller_sratedeviation(self);
	self->m_NextEventSample = psy_dsp_envelopesettings_time(self->m_pEnvelope, end) * xmenvelopecontroller_sratedeviation(self);
	self->m_ModulationAmount = psy_dsp_envelopesettings_value(self->m_pEnvelope, start);
	if (xstep != 0) self->m_Step = ystep / (xstep * xmenvelopecontroller_sratedeviation(self));
	else self->m_Step = 0;
}

void xmenvelopecontroller_setpositioninsamples(XMEnvelopeController* self, int samplePos)
{
	int i = 0;
	while (psy_dsp_envelopesettings_time(self->m_pEnvelope, i) != psy_dsp_ENVELOPEPOINT_INVALID)
	{
		if ((int)(psy_dsp_envelopesettings_time(self->m_pEnvelope, i) *
				xmenvelopecontroller_sratedeviation(self)) > samplePos) {
			break;
		}
		i++;
	}
	if (i == 0) return; //Invalid Envelope. either GetTime(0) is INVALID, or samplePos is negative.
	else if (psy_dsp_envelopesettings_time(self->m_pEnvelope, i) == psy_dsp_ENVELOPEPOINT_INVALID) {
		//Destination point is invalid or is exactly the last one.
		i--;
		self->m_Stage = XMENVELOPESTAGE_OFF;
		self->m_PositionIndex = psy_dsp_envelopesettings_numofpoints(
			self->m_pEnvelope);
		self->m_ModulationAmount =
			psy_dsp_envelopesettings_value(self->m_pEnvelope, i);
	} else {
		i--;
		if (self->m_PositionIndex != i) {
			self->m_PositionIndex = i;
			xmenvelopecontroller_newstep(self);
		}
		self->m_Samples = samplePos;
		int samplesThisIndex = self->m_Samples - (int)
			(psy_dsp_envelopesettings_time(self->m_pEnvelope, i) *
				xmenvelopecontroller_sratedeviation(self));
		self->m_ModulationAmount += self->m_Step * samplesThisIndex;
	}
}

// implementation
void psy_audio_xmsamplervoice_init(psy_audio_XMSamplerVoice* self)
{
	// :m_AmplitudeEnvelope(*this, 1.0f), m_PanEnvelope(*this, 0.0f), m_PtchEnvelope(*this, 0.0f), m_FilterEnvelope(*this, 1.0f) {
	// Reset
	psy_dsp_slider_init(&self->rampl);
	psy_dsp_slider_init(&self->rampr);
	xmenvelopecontroller_init(&self->m_AmplitudeEnvelope, self, 1.0f);
	xmenvelopecontroller_init(&self->m_PanEnvelope, self, 0.0f);
	xmenvelopecontroller_init(&self->m_PitchEnvelope, self, 0.0f);
	xmenvelopecontroller_init(&self->m_FilterEnvelope, self, 1.0f);
	filter_init_samplerate(&self->m_Filter, 44100);
	psy_dsp_slider_init(&self->rampl);
	psy_dsp_slider_init(&self->rampr);
	psy_dsp_slider_resetto(&self->rampl, 0.f);
	psy_dsp_slider_resetto(&self->rampr, 0.f);
	psy_audio_sampleiterator_init(&self->m_WaveDataController,
		NULL, psy_dsp_RESAMPLERQUALITY_LINEAR);
}

void psy_audio_xmsamplervoice_dispose(psy_audio_XMSamplerVoice* self)
{
	xmenvelopecontroller_dispose(&self->m_AmplitudeEnvelope);
	xmenvelopecontroller_dispose(&self->m_PanEnvelope);
	xmenvelopecontroller_dispose(&self->m_PitchEnvelope);
	xmenvelopecontroller_dispose(&self->m_FilterEnvelope);
}

void psy_audio_xmsamplervoice_reset(psy_audio_XMSamplerVoice* self)
{
	self->m_ChannelNum = -1;
	self->m_pChannel = NULL;

	self->_instrument = -1;
	self->m_pInstrument = NULL;

	xmenvelopecontroller_initcontroller(&self->m_AmplitudeEnvelope);
	xmenvelopecontroller_initcontroller(&self->m_FilterEnvelope);
	xmenvelopecontroller_initcontroller(&self->m_PitchEnvelope);
	xmenvelopecontroller_initcontroller(&self->m_PanEnvelope);
	
	filter_reset(&self->m_Filter);
	// self->m_FilterIT.Reset();
	// self->m_FilterClassic.Reset();
	
	self->m_CutOff = 127;
	self->m_Ressonance = 0;
	self->_coModify = 0;


	self->m_bPlay = FALSE;
	self->m_Background = FALSE;
	self->m_Stopping = FALSE;
	self->m_Period = 0;
	self->m_Note = psy_audio_NOTECOMMANDS_EMPTY;
	self->m_Volume = 128;
	self->m_RealVolume = 1.0f;
	
	psy_dsp_slider_resetto(&self->rampl, 0.f);
	psy_dsp_slider_resetto(&self->rampr, 0.f);	

	self->m_PanFactor = 0.5f;
	self->m_PanRange = 0.5f;
	self->m_Surround = FALSE;
	psy_audio_xmsamplervoice_reseteffects(self);
}

void psy_audio_xmsamplervoice_reseteffects(psy_audio_XMSamplerVoice* self)
{
	self->m_Slide2NoteDestPeriod = 0;
	self->m_PitchSlideSpeed = 0;

	self->m_VolumeFadeSpeed = 0;
	self->m_VolumeFadeAmount = 1;

	self->m_VolumeSlideSpeed = 0;

	self->m_VibratoSpeed = 0;
	self->m_VibratoDepth = 0;
	self->m_VibratoPos = 0;
	self->m_VibratoAmount = 0.0;

	self->m_TremoloSpeed = 0;
	self->m_TremoloDepth = 0;
	self->m_TremoloAmount = 0.0f;
	self->m_TremoloPos = 0;

	// Panbrello
	self->m_PanbrelloSpeed = 0;
	self->m_PanbrelloDepth = 0;
	self->m_PanbrelloAmount = 0.0f;
	self->m_PanbrelloPos = 0;
	self->m_PanbrelloRandomCounter = 0;

	// Tremor
	self->m_TremorOnTicks = 0;
	self->m_TremorOffTicks = 0;
	self->m_TremorTickChange = 0;
	self->m_bTremorMute = FALSE;

	self->m_AutoVibratoAmount = 0.0;
	self->m_AutoVibratoPos = 0;
	self->m_AutoVibratoDepth = 0;

	self->m_RetrigTicks = 0;
}

void psy_audio_xmsamplervoice_voiceinit(psy_audio_XMSamplerVoice* self,
	struct psy_audio_Instrument* _inst,
	int channelNum,
	int instrumentNum)
{
	psy_audio_xmsamplervoice_setisbackground(self, FALSE);
	psy_audio_xmsamplervoice_setisstopping(self, FALSE);
	self->m_ChannelNum = channelNum;
	psy_audio_xmsamplervoice_setpchannel(self,
		psy_audio_xmsampler_rchannel(self->m_pSampler, channelNum));	
	psy_audio_xmsamplervoice_setinstrumentnum(self, instrumentNum);
	self->m_pInstrument = _inst;

	// Envelopes
	xmenvelopecontroller_initcontroller_envelope(&self->m_AmplitudeEnvelope,
		&_inst->volumeenvelope);
	xmenvelopecontroller_initcontroller_envelope(&self->m_PanEnvelope,
		&_inst->panenvelope);
	xmenvelopecontroller_initcontroller_envelope(&self->m_PitchEnvelope,
		&_inst->pitchenvelope);
	xmenvelopecontroller_initcontroller_envelope(&self->m_FilterEnvelope,
		&_inst->filterenvelope);
	
	filter_init_samplerate(&self->m_Filter, 44100);

	if (psy_audio_instrument_filtertype(_inst) != F_NONE) {
		psy_audio_xmsamplervoice_setfiltertype(self,
			psy_audio_instrument_filtertype(_inst));
	} else {
		psy_audio_xmsamplervoice_setfiltertype(self,
			psy_audio_xmsamplerchannel_defaultfiltertype(
				psy_audio_xmsamplervoice_rchannel(self)));
	}

	if (_inst->filtercutoff < 1.f || _inst->filterres > 0)
	{
		//\todo: add the missing  Random options
/*			if (psy_audio_instrument_randomcutoff(_inst) > 0.f) {
				psy_audio_xmsamplervoice_setcutoff(self,
					psy_audio_instrument_filtercutoff(_inst)*
					(float)rand() * psy_audio_instrument_randomcutoff(_inst) / 3276800.0f);
			} else */ {
			psy_audio_xmsamplervoice_setcutoff(self,
				psy_audio_instrument_filtercutoff(_inst));
		}
/*			if (_inst.RandomResonance() > 0.f) {
					Ressonance(_inst.FilterResonance() * (float)rand()* _inst.RandomResonance() / 3276800.f);
			} else */ {
			psy_audio_xmsamplervoice_setressonance(self,
				psy_audio_instrument_filterresonance(_inst));
		}
	}
	else {
		psy_audio_xmsamplervoice_setcutoff(self,
			psy_audio_xmsamplerchannel_cutoff(
				psy_audio_xmsamplervoice_rchannel(self)));
		psy_audio_xmsamplervoice_setressonance(self,
			psy_audio_xmsamplerchannel_ressonance(
				psy_audio_xmsamplervoice_rchannel(self)));			
	}
	psy_audio_xmsamplervoice_reseteffects(self);
}

void psy_audio_xmsamplervoice_work(psy_audio_XMSamplerVoice * self,
	int numSamples, float* pSamplesL, float* pSamplesR)
{
	float left_output = 0.0f;
	float right_output = 0.0f;

	if (!psy_audio_xmsamplervoice_rinstrument(self) ||
		!psy_audio_xmsamplervoice_rinstrument(self)->enabled)
	{
		psy_audio_xmsamplervoice_setisplaying(self, FALSE);
		return;
	}

	while (numSamples) {
		int nextsamples = psy_min(
			psy_audio_sampleiterator_prework(&self->m_WaveDataController, (int)numSamples,
				psy_audio_xmsamplervoice_isstopping(self)),
			(int)numSamples);

		numSamples -= nextsamples;
#ifndef NDEBUG
		if (numSamples > 256 || numSamples < 0) {
			TRACE("738: numSamples invalid bug triggered!\n");
		}
#endif
		while (nextsamples)
		{
			float volume;
			float lVolDest;
			float rVolDest;			

			//////////////////////////////////////////////////////////////////////////
			//  Step 1 : Get the unprocessed wave data.

			left_output = psy_audio_sampleiterator_work(&self->m_WaveDataController, 0);
			right_output = psy_audio_sampleiterator_work(&self->m_WaveDataController, 1);

			//////////////////////////////////////////////////////////////////////////
			//  Step 2 : Process the Envelopes.

				// Amplitude Envelope 
				// Voice::RealVolume() returns the calculated volume out of "WaveData.WaveGlobVol() * Instrument.Volume() * Voice.NoteVolume()"
			volume = psy_audio_xmsamplervoice_realvolume(self) *
				psy_audio_xmsamplerchannel_volume(psy_audio_xmsamplervoice_rchannel(self));				
			if (xmenvelopecontroller_stage(&self->m_AmplitudeEnvelope) & XMENVELOPESTAGE_DOSTEP)
			{
				xmenvelopecontroller_work(&self->m_AmplitudeEnvelope);
				if (xmenvelopecontroller_stage(&self->m_AmplitudeEnvelope) == XMENVELOPESTAGE_OFF)
				{
					if (xmenvelopecontroller_modulationamount(&self->m_AmplitudeEnvelope) <= 0.0f)
					{ 
						psy_audio_xmsamplervoice_setisplaying(self, FALSE);
						return;
					} else if (self->m_VolumeFadeSpeed <= 0.0f && psy_audio_instrument_volumefadespeed(self->m_pInstrument) > 0.f) {
						psy_audio_xmsamplervoice_notefadeout(self);
					}
					//else the voice continues playing until notecut (ECX in psycle, since there isn't an explicit notecut)
					//or the sample reaches the end.
				}
			}
			//Volume is outside of the if, because commands EE7/EE8 can start/stop the envelope, but volume is still active.
			volume *= xmenvelopecontroller_modulationamount(&self->m_AmplitudeEnvelope);
			// Volume Fade Out
			if (self->m_VolumeFadeSpeed > 0.0f)
			{
				psy_audio_xmsamplervoice_updatefadeout(self);
				if (self->m_VolumeFadeAmount <= 0) {
					psy_audio_xmsamplervoice_setisplaying(self, FALSE);
					return;
				}
				volume *= self->m_VolumeFadeAmount;
			}

			lVolDest = 0.f;
			rVolDest = 0.f;
			if (psy_audio_xmsamplervoice_issurround(self)) {
				if (psy_audio_xmsampler_panningmode(self->m_pSampler) == psy_audio_PANNING_LINEAR) {
					lVolDest = 0.5f * volume;
					rVolDest = -0.5f * volume;
				} else if (psy_audio_xmsampler_panningmode(self->m_pSampler) == psy_audio_PANNING_TWOWAY) {
					lVolDest = volume;
					rVolDest = -1.f * volume;
				} else if (psy_audio_xmsampler_panningmode(self->m_pSampler) == psy_audio_PANNING_EQUALPOWER) {
					lVolDest = 0.705f * volume;
					rVolDest = -0.705f * volume;
				}
			} else {
				// Panning Envelope 
				// (actually, the correct word for panning is panoramization. "panning" comes from the diminutive "pan")
				// PanFactor() contains the pan calculated at note start ( pan of note, wave pan, instrument pan, NoteModPan sep, and channel pan)
				float lvol = 0;
				float rvol = psy_audio_xmsamplervoice_panfactor(self) + self->m_PanbrelloAmount;

				if (xmenvelopecontroller_stage(&self->m_PanEnvelope) & XMENVELOPESTAGE_DOSTEP) {
					xmenvelopecontroller_work(&self->m_PanEnvelope);
				}
				// PanRange() is a Range delimiter for the envelope, which is set whenever the pan is changed.
				rvol += (xmenvelopecontroller_modulationamount(&self->m_PanEnvelope) * psy_audio_xmsamplervoice_panrange(self));

				if (psy_audio_xmsampler_panningmode(self->m_pSampler) == psy_audio_PANNING_LINEAR) {
					lvol = (1.0f - rvol);
					// PanningMode::Linear is already on rvol, so we omit the case.
				} else if (psy_audio_xmsampler_panningmode(self->m_pSampler) == psy_audio_PANNING_TWOWAY) {
					lvol = psy_min(1.0f, (1.0f - rvol) * 2);
					rvol = psy_min(1.0f, rvol * 2.0f);
				} else if (psy_audio_xmsampler_panningmode(self->m_pSampler) == psy_audio_PANNING_EQUALPOWER) {
					//lvol = powf((1.0f-rvol),0.5f); // This is the commonly used one
					lvol = log10f(((1.0f - rvol) * 9.0f) + 1.0f); // This is a faster approximation
					//rvol = powf(rvol, 0.5f);// This is the commonly used one
					rvol = log10f((rvol * 9.0f) + 1.0f); // This is a faster approximation.
				}
				lVolDest = lvol * volume;
				rVolDest = rvol * volume;
			}
			//Volume Ramping.
			psy_dsp_slider_settarget(&self->rampl, lVolDest);
			psy_dsp_slider_settarget(&self->rampr, rVolDest);
			// if (!psy_audio_sampleiterator_stereo(&self->m_WaveDataController)) {
				// Monoaural output‚ copy left to right output.
				right_output = left_output;
			//}

			// Filter section
			if (filter_type(&self->m_Filter) != F_NONE)
			{
				if (xmenvelopecontroller_stage(&self->m_FilterEnvelope) & XMENVELOPESTAGE_DOSTEP) {
					xmenvelopecontroller_work(&self->m_FilterEnvelope);
				}
				if (psy_audio_xmsampler_usefilters(self->m_pSampler))
				{
					int tmpCO = (int)(self->m_CutOff * 
						xmenvelopecontroller_modulationamount(&self->m_FilterEnvelope));
					if (tmpCO < 0) { tmpCO = 0; } else if (tmpCO > 127) { tmpCO = 127; }
					filter_setcutoff(&self->m_Filter, tmpCO);					
					filter_workstereo_virtual(&self->m_Filter, &left_output, &right_output);
				}
			}
			//Volume after the filter, like schism/IT.
			//If placed before the filter, 303.IT sounds bad (uncontrolled ressonance, replicable in schism if removing the volume changes).
			// left_output *= m_rampL.GetNext();
			// right_output *= m_rampR.GetNext();
			
			left_output *= psy_dsp_slider_getnext(&self->rampl);
			right_output *= psy_dsp_slider_getnext(&self->rampr);
			// Pitch Envelope. Currently, the pitch envelope Amount is only updated on NewLine().
			if (xmenvelopecontroller_stage(&self->m_PitchEnvelope) & XMENVELOPESTAGE_DOSTEP) {
				xmenvelopecontroller_work(&self->m_PitchEnvelope);				
			}


			//////////////////////////////////////////////////////////////////////////
			//  Step 3: Add the processed data to the sampler's buffer.

			if (psy_audio_xmsamplerchannel_ismute(self->m_pChannel)) {
				pSamplesL++;
				pSamplesR++;
			} else {
				*pSamplesL++ += left_output;
				*pSamplesR++ += right_output;
			}

			nextsamples--;
		}
		psy_audio_sampleiterator_postwork(&self->m_WaveDataController);
		if (!psy_audio_sampleiterator_playing(&self->m_WaveDataController)) {
			psy_audio_xmsamplervoice_setisplaying(self, FALSE);
			return;
		}
	}
}
// This one is Tracker-Tick (Mod-Tick).
void psy_audio_xmsamplervoice_tick(psy_audio_XMSamplerVoice* self)
{
	psy_audio_xmsamplervoice_updatespeed(self);
}

void psy_audio_xmsamplervoice_newline(psy_audio_XMSamplerVoice* self)
{
	self->m_bTremorMute = FALSE;
	self->m_VibratoAmount = 0;
	if (psy_audio_xmsamplervoice_isautovibrato(self)) {
		psy_audio_xmsamplervoice_autovibrato(self);
	}
	psy_audio_xmsamplervoice_updatespeed(self);
	xmenvelopecontroller_recalcdeviation(&self->m_AmplitudeEnvelope);
	xmenvelopecontroller_recalcdeviation(&self->m_PanEnvelope);
	xmenvelopecontroller_recalcdeviation(&self->m_PitchEnvelope);
	xmenvelopecontroller_recalcdeviation(&self->m_FilterEnvelope);	
}

void psy_audio_xmsamplervoice_noteon(psy_audio_XMSamplerVoice* self,
	uint8_t note, int16_t playvol, bool reset)
{	
	/*XMInstrument::NotePair pair = rInstrument().NoteToSample(note);
	int wavelayer = pair.second;
	if (Global::song().samples.IsEnabled(wavelayer) == false) return;	
	const XMInstrument::WaveData<>& wave = Global::song().samples[wavelayer];
	*/
	psy_List* entries;
	psy_audio_Sample* sample;

	entries = psy_audio_instrument_entriesintersect(
		psy_audio_xmsamplervoice_rinstrument(self), note, 127, 0);
	if (entries) {
		psy_audio_InstrumentEntry* entry;
		psy_audio_Samples* samples;

		samples = psy_audio_machine_samples(psy_audio_xmsampler_base(self->m_pSampler));
		entry = (psy_audio_InstrumentEntry*)entries->entry;
		sample = psy_audio_samples_at(samples, entry->sampleindex);
		psy_audio_sampleiterator_dispose(&self->m_WaveDataController);
		psy_audio_sampleiterator_init(&self->m_WaveDataController,
			sample, psy_dsp_RESAMPLERQUALITY_LINEAR);
		self->m_Note = note;
		//\todo : add pInstrument().LinesMode
		self->m_Period = psy_audio_xmsamplervoice_notetoperiod(self,
			note, FALSE);
		self->m_NNA = psy_audio_instrument_nna(
			psy_audio_xmsamplervoice_rinstrument(self));
		psy_audio_xmsamplervoice_resetvolandpan(self, playvol, reset);
		//A new note does not necessarily mean an envelope reset
		if (xmenvelopecontroller_stage(&self->m_AmplitudeEnvelope) == XMENVELOPESTAGE_OFF) {
			xmenvelopecontroller_noteon(&self->m_AmplitudeEnvelope);
			if (psy_dsp_envelopesettings_iscarry(self->m_AmplitudeEnvelope.m_pEnvelope)) {
				xmenvelopecontroller_setpositioninsamples(
					&self->m_AmplitudeEnvelope,
					psy_audio_xmsamplerchannel_lastampenvelopeposinsamples(
						psy_audio_xmsamplervoice_rchannel(self)));
			}
		}
		if (xmenvelopecontroller_stage(&self->m_PanEnvelope) == XMENVELOPESTAGE_OFF) {
			xmenvelopecontroller_noteon(&self->m_PanEnvelope);
			if (psy_dsp_envelopesettings_iscarry(self->m_PanEnvelope.m_pEnvelope)) {
				xmenvelopecontroller_setpositioninsamples(
					&self->m_PanEnvelope,
					psy_audio_xmsamplerchannel_lastpanenvelopeposinsamples(
						psy_audio_xmsamplervoice_rchannel(self)));
			}
		}
		if (xmenvelopecontroller_stage(&self->m_FilterEnvelope) == XMENVELOPESTAGE_OFF) {
			xmenvelopecontroller_noteon(&self->m_FilterEnvelope);
			if (psy_dsp_envelopesettings_iscarry(self->m_FilterEnvelope.m_pEnvelope)) {
				xmenvelopecontroller_setpositioninsamples(
					&self->m_FilterEnvelope,
					psy_audio_xmsamplerchannel_lastfilterenvelopeposinsamples(
						psy_audio_xmsamplervoice_rchannel(self)));
			}
		}
		if (xmenvelopecontroller_stage(&self->m_PitchEnvelope) == XMENVELOPESTAGE_OFF) {
			xmenvelopecontroller_noteon(&self->m_PitchEnvelope);
			if (psy_dsp_envelopesettings_iscarry(self->m_PitchEnvelope.m_pEnvelope)) {
				xmenvelopecontroller_setpositioninsamples(
					&self->m_PitchEnvelope,
					psy_audio_xmsamplerchannel_lastpitchenvelopeposinsamples(
						psy_audio_xmsamplervoice_rchannel(self)));
			}
		}
		// if (rWave().Wave().IsAutoVibrato())
		// {
		//	m_AutoVibratoPos = 0;
		//	m_AutoVibratoDepth = 0;
		//	AutoVibrato();
		//}
		//Important, put it after m_PitchEnvelope.NoteOn();	
		psy_audio_xmsamplervoice_updatespeed(self);

		psy_audio_sampleiterator_setplaying(&self->m_WaveDataController, TRUE);
		psy_audio_xmsamplervoice_setisplaying(self, TRUE);
	}
}

void psy_audio_xmsamplervoice_resetvolandpan(psy_audio_XMSamplerVoice* self,
	int16_t playvol, bool reset)
{
	float fpan = 0.5f;
	if (reset)
	{
		int therand = rand();
		self->m_CurrRandVol = 1.0f + (1.f - 2.f * ((float)therand / (float)RAND_MAX)) *
			psy_audio_instrument_randomvolume(psy_audio_xmsamplervoice_rinstrument(self));
		if (playvol != -1)
		{
			psy_audio_xmsamplervoice_setvolume(self, playvol);
		} else {
			psy_audio_xmsamplervoice_setvolume(self,
				psy_audio_sample_volume(self->m_WaveDataController.sample));
		}
		// Impulse Tracker panning had the following pan controls. All these are bypassed if
		// a panning command is explicitely put in a channel.
		// Note : m_pChannel->PanFactor() returns the panFactor of the last panning command (if any) or
		// in its absence, the pan position of the channel.
		if (psy_audio_sample_surround(self->m_WaveDataController.sample) || 
				psy_audio_xmsamplerchannel_issurround(psy_audio_xmsamplervoice_rchannel(self))) {
			psy_audio_xmsamplervoice_setissurround(self, TRUE);
		} else {
			psy_audio_xmsamplervoice_setissurround(self, FALSE);
		}
		//\todo :
//				In a related note, check range of Panbrello.
		// if (rWave().Wave().PanEnabled()) fpan = rWave().Wave().PanFactor();
		// else if (rInstrument().PanEnabled()) fpan = rInstrument().Pan();
		// else fpan = m_pChannel->PanFactor();
		//NoteModPansep is in the range -32..32, being 8=one step (0..64) each seminote.
		// fpan += (m_Note - rInstrument().NoteModPanCenter()) * rInstrument().NoteModPanSep() / 512.0f;
		// fpan += (float)(rand() - 16384.0f) * rInstrument().RandomPanning() / 16384.0f;

		if (fpan > 1.0f) fpan = 1.0f;
		else if (fpan < 0.0f) fpan = 0.0f;
	} else
	{
		self->m_CurrRandVol =
			psy_audio_xmsamplerchannel_lastvoicerandvol(
				psy_audio_xmsamplervoice_rchannel(self));
		psy_audio_xmsamplervoice_setvolume(self,
			psy_audio_xmsamplerchannel_lastvoicevolume(
				psy_audio_xmsamplervoice_rchannel(self)));
		fpan = psy_audio_xmsamplerchannel_lastvoicepanfactor(
			psy_audio_xmsamplervoice_rchannel(self));
		psy_audio_xmsamplervoice_setissurround(self,
			psy_audio_xmsamplerchannel_issurround(
			psy_audio_xmsamplervoice_rchannel(self)));
	}			
	psy_audio_xmsamplervoice_setpanfactor(self, fpan);

	//If a new note, let's start from volume zero. Else it's a glide.
	if (!psy_audio_xmsamplervoice_isplaying(self)) {
		psy_dsp_slider_resetto(&self->rampl, 0.f);
		psy_dsp_slider_resetto(&self->rampr, 0.f);		
	}
}

void psy_audio_xmsamplervoice_noteoff(psy_audio_XMSamplerVoice* self)
{
	if (!psy_audio_xmsamplervoice_isplaying(self)) {
		return;
	}
	psy_audio_xmsamplervoice_setisstopping(self, TRUE);
	if (xmenvelopecontroller_stage(&self->m_AmplitudeEnvelope) != XMENVELOPESTAGE_OFF)
	{
		xmenvelopecontroller_noteoff(&self->m_AmplitudeEnvelope);		
		// IT Type envelopes only do a fadeout() when it reaches the end of the envelope, except if it is looped.
		if (psy_dsp_envelopesettings_loopstart(self->m_AmplitudeEnvelope.m_pEnvelope)
			!= psy_dsp_ENVELOPEPOINT_INVALID)
		{
			psy_audio_xmsamplervoice_notefadeout(self);
		}
	}
	else if (psy_audio_instrument_volumefadespeed(
		psy_audio_xmsamplervoice_rinstrument(self)) > 0.0f)
	{
		psy_audio_xmsamplervoice_notefadeout(self);
	} else {
		psy_audio_xmsamplervoice_noteofffast(self);		
		return;
	}
	xmenvelopecontroller_noteoff(&self->m_PanEnvelope);
	xmenvelopecontroller_noteoff(&self->m_FilterEnvelope);
	xmenvelopecontroller_noteoff(&self->m_PitchEnvelope);
	// m_WaveDataController.NoteOff();
}

void psy_audio_xmsamplervoice_noteofffast(psy_audio_XMSamplerVoice* self)
{
	if (!psy_audio_xmsamplervoice_isplaying(self)) {
		return;
	}
	if (psy_audio_xmsamplervoice_realvolume(self) *
		psy_audio_xmsamplerchannel_volume(
			psy_audio_xmsamplervoice_rchannel(self)) == 0.0f) {
		psy_audio_xmsamplervoice_setisplaying(self, FALSE);
	}
	
	psy_audio_xmsamplervoice_setisstopping(self, TRUE);
	if (psy_dsp_envelopesettings_isenabled(self->m_AmplitudeEnvelope.m_pEnvelope)) {
		xmenvelopecontroller_noteoff(&self->m_AmplitudeEnvelope);		
	}
	// Fade Out Volume
	self->m_VolumeFadeSpeed = 1000.0f / (3.f * 
		psy_audio_xmsamplervoice_samplerate(self)); // 3 milliseconds of samples. (same as volume ramping)
	self->m_VolumeFadeAmount = 1.0f;

	xmenvelopecontroller_noteoff(&self->m_PanEnvelope);
	xmenvelopecontroller_noteoff(&self->m_FilterEnvelope);
	xmenvelopecontroller_noteoff(&self->m_PitchEnvelope);
	// m_WaveDataController.NoteOff();
}

void psy_audio_xmsamplervoice_notefadeout(psy_audio_XMSamplerVoice* self)
{
	psy_audio_xmsamplervoice_setisstopping(self, TRUE);
	self->m_VolumeFadeSpeed = psy_audio_instrument_volumefadespeed(self->m_pInstrument) /
		psy_audio_machine_samplespertick(psy_audio_xmsampler_base(self->m_pSampler));
	self->m_VolumeFadeAmount = 1.0f;
	if (psy_audio_xmsamplervoice_realvolume(self) *
		psy_audio_xmsamplerchannel_volume(
			psy_audio_xmsamplervoice_rchannel(self)) == 0.0f) {
		psy_audio_xmsamplervoice_setisplaying(self, FALSE);
	}
	//The following is incorrect, at least with looped envelopes that also have sustain loops.
	//else if ( m_AmplitudeEnvelope.Envelope().IsEnabled() && m_AmplitudeEnvelope.ModulationAmount() == 0.0f) IsPlaying(false);

	//This might not be correct, but since we are saying "IsStopping(true"), then the controller needs to recalculate the buffers.
	// m_WaveDataController.NoteOff();*/
}

void psy_audio_xmsamplervoice_updatefadeout(psy_audio_XMSamplerVoice* self)
{
	if (psy_audio_xmsamplervoice_realvolume(self) == 0.0f) {
		psy_audio_xmsamplervoice_setisplaying(self, FALSE);
	}
	self->m_VolumeFadeAmount -= self->m_VolumeFadeSpeed;
	if (self->m_VolumeFadeAmount <= 0) {
		psy_audio_xmsamplervoice_setisplaying(self, FALSE);
	}
}

void psy_audio_xmsamplervoice_slide2note(psy_audio_XMSamplerVoice* self)
{
	if (self->m_Slide2NoteDestPeriod > self->m_Period)
	{
		self->m_Period += self->m_PitchSlideSpeed;
		if (self->m_Period > self->m_Slide2NoteDestPeriod) {
			self->m_Period = self->m_Slide2NoteDestPeriod;
		}
		psy_audio_xmsamplervoice_updatespeed(self);
	} else if (self->m_Slide2NoteDestPeriod < self->m_Period)
	{	// m_PitchSlide is signed. (+/-)
		self->m_Period += self->m_PitchSlideSpeed;
		if (self->m_Period < self->m_Slide2NoteDestPeriod) {
			self->m_Period = self->m_Slide2NoteDestPeriod;
		}
		psy_audio_xmsamplervoice_updatespeed(self);
	}
}

void psy_audio_xmsamplervoice_volumeslide(psy_audio_XMSamplerVoice* self)
{
	if (self->m_VolumeSlideSpeed > 0) {
		psy_audio_xmsamplervoice_volumeup(self, self->m_VolumeSlideSpeed);
	} else {
		psy_audio_xmsamplervoice_volumedown(self, self->m_VolumeSlideSpeed);
	}
}
void psy_audio_xmsamplervoice_volumedown(psy_audio_XMSamplerVoice* self,
	int value) {
	int vol = psy_audio_xmsamplervoice_volume(self) + value;
	if (vol < 0) {
		vol = 0;
	}
	psy_audio_xmsamplervoice_setvolume(self, vol);
}
void psy_audio_xmsamplervoice_volumeup(psy_audio_XMSamplerVoice* self,
	int value) {
	int vol = psy_audio_xmsamplervoice_volume(self) + value;
	if (vol > 0x80) {
		vol = 0x80;
	}
	psy_audio_xmsamplervoice_setvolume(self, vol);
}

void psy_audio_xmsamplervoice_autovibrato(psy_audio_XMSamplerVoice* self)
{
/*	int targetDepth = rWave().Wave().VibratoDepth() << 8;
	if (rWave().Wave().VibratoAttack() && m_AutoVibratoDepth < targetDepth)
	{
		m_AutoVibratoDepth += rWave().Wave().VibratoAttack();
		if (m_AutoVibratoDepth > targetDepth)
		{
			m_AutoVibratoDepth = targetDepth;
		}
	} else {
		m_AutoVibratoDepth = targetDepth;
	}

	int vdelta = GetDelta(rWave().Wave().VibratoType(), m_AutoVibratoPos);
	vdelta = vdelta * (m_AutoVibratoDepth >> 8);
	m_AutoVibratoAmount = (double)vdelta / 64.0;
	m_AutoVibratoPos = (m_AutoVibratoPos - (rWave().Wave().VibratoSpeed())) & 0xFF;
	UpdateSpeed();*/

}

void psy_audio_samplervoice_vibrato(psy_audio_XMSamplerVoice* self)
{	
	/*int vdelta = GetDelta(rChannel().VibratoType(), m_VibratoPos);

	vdelta = vdelta * m_VibratoDepth;
	m_VibratoAmount = (double)vdelta / 32.0;
	m_VibratoPos = (m_VibratoPos - m_VibratoSpeed) & 0xFF;
	UpdateSpeed();*/
}// Vibrato() -------------------------------------

void psy_audio_samplervoice_tremolo(psy_audio_XMSamplerVoice* self)
{
	//\todo: verify that final volume doesn't go out of range (Redo RealVolume() ?)
	/*int vdelta = GetDelta(rChannel().TremoloType(), m_TremoloPos);

	vdelta = (vdelta * m_TremoloDepth);
	m_TremoloAmount = (double)vdelta / 2048.0;
	m_TremoloPos = (m_TremoloPos + m_TremoloSpeed) & 0xFF;*/


}// Tremolo() -------------------------------------------

void psy_audio_samplervoice_panbrello(psy_audio_XMSamplerVoice* self)
{
	//Yxy   Panbrello with speed x, depth y.
	//The random pan position can be achieved by setting the
	//waveform to 3 (ie. a S53 command). In this case *ONLY*, the
	//speed actually is interpreted as a delay in frames before
	//another random value is found. so Y14 will be a very QUICK
	//panbrello, and Y44 will be a slower panbrello.

		//\todo: verify that final pan doesn't go out of range (make a RealPan() similar to RealVolume() ?)
	int vdelta = 0; // psy_audio_samplervoice_getdelta(self,
		//psy_audio_xmsamplerchannel_panbrellotype(
			//psy_audio_samplervoice_rchannel(self)),
			//self->m_PanbrelloPos);

	vdelta = vdelta * self->m_PanbrelloDepth;
	self->m_PanbrelloAmount = vdelta / 2048.0f; // 64*16*2
	// if (rChannel().PanbrelloType() != XMInstrument::WaveData<>::WaveForms::RANDOM)
	// {
	// 	m_PanbrelloPos = (m_PanbrelloPos + m_PanbrelloSpeed) & 0xFF;
	// } else if (++m_PanbrelloRandomCounter >= m_PanbrelloSpeed)
	// {
	// 	m_PanbrelloPos++;
	// 	m_PanbrelloRandomCounter = 0;
	// }

}// Panbrello() -------------------------------------------

void psy_audio_samplervoice_tremor(psy_audio_XMSamplerVoice* self)
{
	//\todo: according to Impulse Tracker, this command uses its own counter, so with
	// speed 3, we can specify the command I41 ( x/y > speed), which, with the current implementation, doesn't work,
	if ((psy_audio_xmsampler_currenttick(self->m_pSampler) >= self->m_TremorTickChange))
	{
		if (self->m_bTremorMute)
		{
			self->m_TremorTickChange = psy_audio_xmsampler_currenttick(self->m_pSampler) + self->m_TremorOnTicks;
			self->m_bTremorMute = FALSE;
		} else
		{
			self->m_TremorTickChange = psy_audio_xmsampler_currenttick(self->m_pSampler) + self->m_TremorOffTicks;
			self->m_bTremorMute = FALSE;
		}
	}
}

void psy_audio_samplervoice_retrig(psy_audio_XMSamplerVoice* self)
{
	if ((psy_audio_xmsampler_currenttick(self->m_pSampler) % self->m_RetrigTicks) == 0)
	{
		psy_audio_xmsamplervoice_noteon(self, self->m_Note, -1, FALSE);
	}
}

int psy_audio_xmsamplervoice_samplerate(const psy_audio_XMSamplerVoice* self)
{
	return (int)psy_audio_machine_samplerate(psy_audio_xmsampler_base(self->m_pSampler));
}

int psy_audio_samplervoice_getdelta(const psy_audio_XMSamplerVoice* self,
	psy_audio_WaveForms wavetype, int wavepos)	
{
	switch (wavetype)
	{
	case psy_audio_WAVEFORMS_SAWDOWN:
		return m_FineRampDownData[wavepos];
	case psy_audio_WAVEFORMS_SAWUP:
		return m_FineRampDownData[0xFF - wavepos];
	case psy_audio_WAVEFORMS_SQUARE:
		return m_FineSquareTable[wavepos];
	case psy_audio_WAVEFORMS_RANDOM:
		return m_RandomTable[wavepos];
	case psy_audio_WAVEFORMS_SINUS:
	default:
		return m_FineSineData[wavepos];
	}	
}

void psy_audio_xmsamplervoice_updatespeed(psy_audio_XMSamplerVoice* self)
{
	double _period = self->m_Period;

	if (psy_audio_xmsamplerchannel_isarpeggio(psy_audio_xmsamplervoice_rchannel(self)))
	{	
		_period = psy_audio_xmsamplerchannel_arpeggioperiod(
			psy_audio_xmsamplervoice_rchannel(self));			
	}
	
	if (psy_audio_xmsamplerchannel_isgrissando(psy_audio_xmsamplervoice_rchannel(self)))
	{
		int _note = psy_audio_xmsamplervoice_periodtonote(self, _period);
		_period = psy_audio_xmsamplervoice_notetoperiod(self, _note, FALSE);
	}
	_period = _period + psy_audio_xmsamplervoice_vibratoamount(self);
	if (_period > 65535.0) {
		psy_audio_xmsamplervoice_noteofffast(self);
	} else if (_period < 1.0) {
		//This behaviour exists in ST3 and IT, and is in fact documented for the slide up command in the latter.
		psy_audio_xmsamplervoice_noteofffast(self);
	} else {
		double speed = psy_audio_xmsamplervoice_periodtospeed(self, (int)_period);
		//\todo: Attention, AutoVibrato always use linear slides with IT, but in FT2 it depends on amigaslides switch.
		speed *= pow(2.0, ((-psy_audio_xmsamplervoice_autovibratoamount(self)) / 768.0));
		psy_audio_sampleiterator_setspeed(&self->m_WaveDataController, speed);
		//// rWave().Speed(m_pSampler->Resampler(), speed);
	}	
}

double psy_audio_xmsamplervoice_periodtospeed(const psy_audio_XMSamplerVoice* self, int period)
{
	if (psy_audio_xmsampler_isamigaslides(self->m_pSampler))
	{
		// Amiga period mode. Original conversion:
		//	PAL:   7093789.2 / (428*2) = 8287.14 Hz   ( Amiga clock frequency, middle C period and final samplerate)
		//	NSTC:  7159090.5 / (428*2) = 8363.42 Hz   ( *2 is used to convert clock frequency to ticks).
		// in PC, the middle C period is 1712. It was increased by 4 to add extra fine pitch slides.
		// so 1712 *8363 = 14317456, which is used in IT and FT2 (and in ST3, if the value that exists in their doc is a typo).
		// One could also use 7159090.5 /2 *4 = 14318181
		return (14317456.0 / period) * pow(2.0, 
			(xmenvelopecontroller_modulationamount(&self->m_PitchEnvelope) * 16.0) / 12.0) /
				(double)psy_audio_xmsamplervoice_samplerate(self);		
	} else {
		// Linear Frequency
		// base_samplerate * 2^((7*12*64 - Period) / (12*64))
		// 12*64 = 12 notes * 64 finetune steps.
		// 7 = 12 - middle_C ( if C-4 is middle_C, then 8*12*64, if C-3, then 9*12*64, etc..)
		return	pow(2.0,
			((5376 - period + xmenvelopecontroller_modulationamount(&self->m_PitchEnvelope) * 1024.0)
				/ 768.0)
		)
		* self->m_WaveDataController.sample->samplerate / 
			(double)psy_audio_xmsamplervoice_samplerate(self);
	}
	return 1.f;
}

double psy_audio_xmsamplervoice_notetoperiod(const psy_audio_XMSamplerVoice* self,
	int noteIn, bool correctNote)
{
	psy_audio_Sample* wave;

	wave = self->m_WaveDataController.sample;	
	int note; // (correctNote) ? rInstrument().NoteToSample(noteIn).first : noteIn;

	note = noteIn;
	if (psy_audio_xmsampler_isamigaslides(self->m_pSampler))
	{				
		return psy_dsp_notetoamigaperiod(note,
			(double)wave->samplerate,
			psy_audio_sample_tune(wave),
			psy_audio_sample_finetune(wave));
	} else {
		return psy_dsp_notetoperiod(note, psy_audio_sample_tune(wave),
			psy_audio_sample_finetune(wave));		
	}
	return 0;
}

int psy_audio_xmsamplervoice_periodtonote(psy_audio_XMSamplerVoice* self, double period)
{
	psy_audio_Sample* wave;

	wave = self->m_WaveDataController.sample;
	if (psy_audio_xmsampler_isamigaslides(self->m_pSampler)) {		
		return psy_dsp_amigaperiodtonote(period, (double)wave->samplerate, wave->tune,
			wave->finetune);
	} else {		
		return psy_dsp_periodtonote(period, wave->tune, wave->finetune);		
	}
	return 0;
}
