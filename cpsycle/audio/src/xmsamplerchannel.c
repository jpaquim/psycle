// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsamplerchannel.h"

// local
#include "constants.h"
#include "machineparam.h"
#include "plugin_interface.h"
#include "samplerdefs.h"
#include "songio.h"
#include "xmsamplervoice.h"
#include "xmsampler.h"
// std
#include <assert.h>
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static const char E8VolMap[16] = {
	0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64
};

// prototypes
static void psy_audio_xmsamplerchannel_disposeparamview(psy_audio_XMSamplerChannel*);
static void psy_audio_xmsamplerchannel_filtertype_tweak(psy_audio_XMSamplerChannel*,
	psy_audio_ChoiceMachineParam* sender, float value);
static void psy_audio_xmsamplerchannel_filtertype_normvalue(psy_audio_XMSamplerChannel*,
	psy_audio_ChoiceMachineParam* sender, float* rv);
static void psy_audio_xmsamplerchannel_defaultpan_tweak(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float value);
static void psy_audio_xmsamplerchannel_defaultpan_normvalue(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float* rv);
static void psy_audio_xmsamplerchannel_slider_tweak(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float value);
static void psy_audio_xmsamplerchannel_slider_normvalue(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float* rv);
static void psy_audio_xmsamplerchannel_defaultismute_tweak(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float value);
static void psy_audio_xmsamplerchannel_defaultismute_normvalue(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float* rv);
static void psy_audio_xmsamplerchannel_defaultissurround_tweak(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float value);
static void psy_audio_xmsamplerchannel_defaultissurround_normvalue(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float* rv);

// implementation
void psy_audio_xmsamplerchannel_init(psy_audio_XMSamplerChannel* self)
{
	self->m_Index = 0;
	self->m_DelayedNote = NULL;
	psy_audio_xmsamplerchannel_initchannel(self);
}

void psy_audio_xmsamplerchannel_initchannel(psy_audio_XMSamplerChannel* self)
{
	self->m_InstrumentNo = 255; // psy_audio_NOTECOMMANDS_INST_EMPTY;
	self->m_pForegroundVoice = NULL;

	self->m_Note = psy_audio_NOTECOMMANDS_EMPTY;
	self->m_Period = 0;

	self->m_ChannelDefVolume = 200;// 0..200 , &0x100 = Mute.
	self->m_bMute = FALSE;

	self->m_DefaultPanFactor = 100;

	self->m_DefaultCutoff = 127;
	self->m_DefaultRessonance = 0;
	self->m_DefaultFilterType = F_NONE;

	self->m_DelayedNote = NULL;
	psy_audio_xmsamplerchannel_restore(self);
}

void psy_audio_xmsamplerchannel_dispose(psy_audio_XMSamplerChannel* self)
{
	psy_audio_xmsamplerchannel_disposeparamview(self);
	psy_list_deallocate(&self->m_DelayedNote, NULL);
}

void psy_audio_xmsamplerchannel_initparamview(psy_audio_XMSamplerChannel* self)
{
	char text[127];
	int index;
	uintptr_t ft;
	
	index = self->m_Index;
	psy_snprintf(text, 127, "Channel %d", (int)index);
	//}
	psy_audio_infomachineparam_init(&self->param_channel, text, "", MPF_SMALL);
	psy_audio_choicemachineparam_init(&self->param_filtertype,
		"Filter", "", MPF_STATE | MPF_SMALL,
		NULL, 0, filter_numfilters() - 1);
	for (ft = 0; ft < filter_numfilters(); ++ft) {
		psy_audio_choicemachineparam_setdescription(&self->param_filtertype,
			ft, filter_name(ft));
	}
	psy_signal_connect(&self->param_filtertype.machineparam.signal_tweak, self,
		psy_audio_xmsamplerchannel_filtertype_tweak);
	psy_signal_connect(&self->param_filtertype.machineparam.signal_normvalue, self,
		psy_audio_xmsamplerchannel_filtertype_normvalue);
	psy_audio_intmachineparam_init(&self->filter_cutoff,
		"", "", MPF_STATE | MPF_SMALL, &self->m_DefaultCutoff, 0, 200);
	psy_audio_intmachineparam_init(&self->filter_res,
		"", "", MPF_STATE | MPF_SMALL, &self->m_Ressonance, 0, 200);
	psy_audio_intmachineparam_init(&self->pan,
		"", "", MPF_STATE | MPF_SMALL, NULL, 0, 200);
	psy_signal_connect(&self->pan.machineparam.signal_tweak, self,
		psy_audio_xmsamplerchannel_defaultpan_tweak);
	psy_signal_connect(&self->pan.machineparam.signal_normvalue, self,
		psy_audio_xmsamplerchannel_defaultpan_normvalue);
	psy_audio_intmachineparam_init(&self->surround,
		"Surround", "Surround", MPF_SLIDERCHECK | MPF_SMALL, NULL, 0, 1);
	self->surround.machineparam.isslidergroup = TRUE;
	psy_signal_connect(&self->surround.machineparam.signal_tweak, self,
		psy_audio_xmsamplerchannel_defaultissurround_tweak);
	psy_signal_connect(&self->surround.machineparam.signal_normvalue, self,
		psy_audio_xmsamplerchannel_defaultissurround_normvalue);
	psy_audio_intmachineparam_init(&self->mute,
		"Mute", "Mute", MPF_SLIDERCHECK | MPF_SMALL, NULL, 0, 1);
	self->mute.machineparam.isslidergroup = TRUE;
	psy_signal_connect(&self->mute.machineparam.signal_tweak, self,
		psy_audio_xmsamplerchannel_defaultismute_tweak);
	psy_signal_connect(&self->mute.machineparam.signal_normvalue, self,
		psy_audio_xmsamplerchannel_defaultismute_normvalue);
	psy_audio_intmachineparam_init(&self->slider_param,
		"Volume", "", MPF_SLIDER | MPF_SMALL, NULL, 0, 200);
	psy_signal_connect(&self->slider_param.machineparam.signal_tweak, self,
		psy_audio_xmsamplerchannel_slider_tweak);
	psy_signal_connect(&self->slider_param.machineparam.signal_normvalue, self,
		psy_audio_xmsamplerchannel_slider_normvalue);	
}

void psy_audio_xmsamplerchannel_disposeparamview(psy_audio_XMSamplerChannel* self)
{
	psy_audio_infomachineparam_dispose(&self->param_channel);
	psy_audio_choicemachineparam_dispose(&self->param_filtertype);
	psy_audio_intmachineparam_dispose(&self->filter_cutoff);
	psy_audio_intmachineparam_dispose(&self->filter_res);
	psy_audio_intmachineparam_dispose(&self->pan);
	psy_audio_intmachineparam_dispose(&self->surround);
	psy_audio_intmachineparam_dispose(&self->mute);
	psy_audio_intmachineparam_dispose(&self->slider_param);	
}

void psy_audio_xmsamplerchannel_effectinit(psy_audio_XMSamplerChannel* self)
{
	//Effects
	self->m_EffectFlags = 0;

	self->m_PitchSlideSpeed = 0;

	self->m_GlobalVolSlideSpeed = 0.0f;
	self->m_ChanVolSlideSpeed = 0.0f;
	self->m_PanSlideSpeed = 0.0f;

	self->m_TremoloSpeed = 0;
	self->m_TremoloDepth = 0;
	self->m_TremoloDelta = 0;
	self->m_TremoloPos = 0;

	self->m_PanbrelloSpeed = 0;
	self->m_PanbrelloDepth = 0;
	self->m_PanbrelloDelta = 0;
	self->m_PanbrelloPos = 0;

	self->m_bGrissando = FALSE;
	self->m_VibratoType = psy_audio_WAVEFORMS_SINUS;
	self->m_TremoloType = psy_audio_WAVEFORMS_SINUS;
	self->m_PanbrelloType = psy_audio_WAVEFORMS_SINUS;

	self->m_TremorOnTime = 0;
	self->m_TremorOffTime = 0;

	self->m_RetrigOperation = 0;
	self->m_RetrigVol = 0;

	self->m_ArpeggioPeriod[0] = 0.0;
	self->m_ArpeggioPeriod[1] = 0.0;

	self->m_NoteCutTick = 0;

	self->m_MIDI_Set = 0;

	//Memory
	self->m_PanSlideMem = 0;
	self->m_ChanVolSlideMem = 0;
	self->m_PitchSlideMem = 0;
	self->m_TremorMem = 0;
	self->m_VibratoDepthMem = 0;
	self->m_VibratoSpeedMem = 0;
	self->m_TremoloDepthMem = 0;
	self->m_TremoloSpeedMem = 0;
	self->m_PanbrelloDepthMem = 0;
	self->m_PanbrelloSpeedMem = 0;
	self->m_VolumeSlideMem = 0;
	self->m_ArpeggioMem = 0;
	self->m_RetrigMem = 0;
	self->m_OffsetMem = 0;
	self->m_GlobalVolSlideMem = 0;
}

void psy_audio_xmsamplerchannel_restore(psy_audio_XMSamplerChannel* self)
{
	self->m_InstrumentNo = 255; // psy_audio_NOTECOMMANDS_INST_EMPTY;
	self->m_pForegroundVoice = NULL;

	self->m_LastVoiceVolume = 0;
	self->m_LastVoiceRandVol = 1.0f;
	self->m_LastVoicePanFactor = 0.0f;
	self->m_LastAmpEnvelopePosInSamples = 0;
	self->m_LastPanEnvelopePosInSamples = 0;
	self->m_LastFilterEnvelopePosInSamples = 0;
	self->m_LastPitchEnvelopePosInSamples = 0;

	self->m_Volume = psy_audio_xmsamplerchannel_defaultvolumefloat(self);
	self->m_PanFactor = psy_audio_xmsamplerchannel_defaultpanfactorfloat(self);
	self->m_bSurround = psy_audio_xmsamplerchannel_defaultissurround(self);

	psy_audio_xmsamplerchannel_effectinit(self);
	self->m_Cutoff = self->m_DefaultCutoff;
	self->m_Ressonance = self->m_DefaultRessonance;
	psy_list_deallocate(&self->m_DelayedNote, NULL);	
}

void psy_audio_xmsamplerchannel_setforegroundvoice(psy_audio_XMSamplerChannel* self,
	struct psy_audio_XMSamplerVoice* pVoice)
{
	if (self->m_pForegroundVoice)
	{
		const psy_dsp_Envelope* env;

		psy_audio_xmsamplerchannel_setlastvoicepanfactor(
			self, psy_audio_xmsamplervoice_panfactor(self->m_pForegroundVoice));
		psy_audio_xmsamplerchannel_setlastvoicevolume(
			self, psy_audio_xmsamplervoice_volume(self->m_pForegroundVoice));
		psy_audio_xmsamplerchannel_setlastvoicerandvol(
			self, psy_audio_xmsamplervoice_currrandvol(self->m_pForegroundVoice));

		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_amplitudeenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelope_isenabled(env) && psy_dsp_envelope_iscarry(env))
			psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_amplitudeenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(self, 0);
		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_panenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelope_isenabled(env) && psy_dsp_envelope_iscarry(env))
			psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_panenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(self, 0);
		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_filterenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelope_isenabled(env) && psy_dsp_envelope_iscarry(env))
			psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_filterenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(self, 0);
		env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_pitchenvelope(self->m_pForegroundVoice));
		if (psy_dsp_envelope_isenabled(env) && psy_dsp_envelope_iscarry(env))
			psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(self,
				xmenvelopecontroller_getpositioninsamples(
					psy_audio_xmsamplervoice_pitchenvelope(self->m_pForegroundVoice)));
		else psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(self, 0);
				
		if (self->m_pForegroundVoice != pVoice) {
			psy_audio_xmsamplervoice_setisbackground(
			self->m_pForegroundVoice, TRUE);
		}
	} else {
		psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(self, 0);
		psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(self, 0);
		psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(self, 0);
		psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(self, 0);
	}
	self->m_pForegroundVoice = pVoice;
}

void psy_audio_xmsamplerchannel_setnote(psy_audio_XMSamplerChannel* self, int note)
{
	self->m_Note = note;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		self->m_Period = psy_audio_xmsamplervoice_notetoperiod(
			psy_audio_xmsamplerchannel_foregroundvoice(self), note, TRUE);
	}
}

void psy_audio_xmsamplerchannel_setpanfactor(psy_audio_XMSamplerChannel* self,
	float value)
{
	self->m_PanFactor = value;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplervoice_setpanfactor(
			psy_audio_xmsamplerchannel_foregroundvoice(self),
			value);
	}
}

void psy_audio_xmsamplerchannel_seteffect(psy_audio_XMSamplerChannel* self,
	psy_audio_XMSamplerVoice* voice, int volcmd, int cmd, int parameter)
{
	int realSet;
	int	realValue;
	int slidval;

	// 1st check: Channel ( They can appear without an existing playing note
	// and are persistent when a new one comes)
	realSet = 0;
	realValue = 0;
	switch (volcmd & 0xF0)
	{
	case XM_SAMPLER_CMD_VOL_PANNING:
		psy_audio_xmsamplerchannel_setpanfactor(self,
			(volcmd & 0x0F) / 15.0f);
		break;
	case XM_SAMPLER_CMD_VOL_PANSLIDELEFT:
		// this command is actually fine pan slide
		psy_audio_xmsamplerchannel_panningslidespeed(self,
			(volcmd & 0x0F) << 4);
		break;
	case XM_SAMPLER_CMD_VOL_PANSLIDERIGHT:
		// this command is actually fine pan slide
		psy_audio_xmsamplerchannel_panningslidespeed(self,
			volcmd & 0x0F);
		break;
	default:
		break;
	}

	switch (cmd)
	{
	case XM_SAMPLER_CMD_PANNING:
		psy_audio_xmsamplerchannel_setissurround(self,
			FALSE);
		psy_audio_xmsamplerchannel_setpanfactor(self,
			parameter / 255.0f);
		break;
	case XM_SAMPLER_CMD_SET_CHANNEL_VOLUME:
		psy_audio_xmsamplerchannel_setvolume(self,
			(parameter < 64) ? (parameter / 64.0f) : 1.0f);
		break;
	case XM_SAMPLER_CMD_PANNINGSLIDE:
		psy_audio_xmsamplerchannel_panningslidespeed(self, parameter);
		break;
	case XM_SAMPLER_CMD_CHANNEL_VOLUMESLIDE:
		psy_audio_xmsamplerchannel_channelvolumeslidespeed(self, parameter);		
		break;
	case XM_SAMPLER_CMD_SET_GLOBAL_VOLUME:
		psy_audio_xmsampler_setglobalvolume(self->m_pSampler,
			parameter < 0x80 ? parameter : 0x80);
		break;
	case XM_SAMPLER_CMD_GLOBAL_VOLUME_SLIDE:
		psy_audio_xmsamplerchannel_globalvolslide(self,
			parameter);
		break;
	case XM_SAMPLER_CMD_EXTENDED:
		switch (parameter & 0xF0)
		{
		case XM_SAMPLER_CMD_E9:
			switch (parameter & 0x0F)
			{
			case XM_SAMPLER_CMD_E9_SURROUND_OFF:
				psy_audio_xmsamplerchannel_setissurround(self, FALSE);
				break;
			case XM_SAMPLER_CMD_E9_SURROUND_ON:
				psy_audio_xmsamplerchannel_setissurround(self, TRUE);
				break;
			case XM_SAMPLER_CMD_E9_REVERB_OFF:
				break;
			case XM_SAMPLER_CMD_E9_REVERB_FORCE:
				break;
			case XM_SAMPLER_CMD_E9_STANDARD_SURROUND:
				break;
			case XM_SAMPLER_CMD_E9_QUAD_SURROUND:
				break;
			case XM_SAMPLER_CMD_E9_GLOBAL_FILTER:
				break;
			case XM_SAMPLER_CMD_E9_LOCAL_FILTER:
				break;
			default:
				break;
			}
			break;
		case XM_SAMPLER_CMD_E_SET_PAN:
			psy_audio_xmsamplerchannel_setpanfactor(self, E8VolMap[(parameter & 0xf)] / 64.0f);
			break;
		case XM_SAMPLER_CMD_E_SET_MIDI_MACRO:
			//\todo : implement. For now, it maps directly to internal Filter commands
			self->m_MIDI_Set = parameter & 0x0F;
			break;
		case XM_SAMPLER_CMD_E_GLISSANDO_TYPE:
			psy_audio_xmsamplerchannel_setisgrissando(self,
				parameter != 0);
			break;
		case XM_SAMPLER_CMD_E_VIBRATO_WAVE:
			if (parameter <= psy_audio_WAVEFORMS_RANDOM) {
				psy_audio_xmsamplerchannel_setvibratotype(self,
					(psy_audio_WaveForms)parameter);			
			} else {
				psy_audio_xmsamplerchannel_setvibratotype(self,
					psy_audio_WAVEFORMS_SINUS);				
			}
			break;
		case XM_SAMPLER_CMD_E_PANBRELLO_WAVE:
			if (parameter <= psy_audio_WAVEFORMS_RANDOM) {
				psy_audio_xmsamplerchannel_setpanbrellotype(self,
					(psy_audio_WaveForms)parameter);
			} else {
				psy_audio_xmsamplerchannel_setpanbrellotype(self,
					psy_audio_WAVEFORMS_SINUS);				
			}
			break;
		case XM_SAMPLER_CMD_E_TREMOLO_WAVE:
			if (parameter <= psy_audio_WAVEFORMS_RANDOM) {
				psy_audio_xmsamplerchannel_settremolotype(self,
					(psy_audio_WaveForms)parameter);
			} else {
				psy_audio_xmsamplerchannel_settremolotype(self,
					psy_audio_WAVEFORMS_SINUS);
			}			
			break;
		default:
			break;
		}
		break;
	case XM_SAMPLER_CMD_MIDI_MACRO:
		if (parameter < 0x80)
		{
			realSet = self->m_MIDI_Set;
			realValue = parameter;
		} else
		{
			realSet = psy_audio_xmsampler_getmap(self->m_pSampler, parameter - 0x80).mode;
			realValue = psy_audio_xmsampler_getmap(self->m_pSampler, parameter - 0x80).value;			
		}
		switch (realSet)
		{
		case 0:
			self->m_Cutoff = realValue;
			if (voice)
			{
				if (psy_audio_xmsamplervoice_filtertype(voice) == F_NONE) {
					psy_audio_xmsamplervoice_setfiltertype(voice,
						self->m_DefaultFilterType);
				}
				psy_audio_xmsamplervoice_setcutoff(voice,
					self->m_Cutoff);
			}
			break;
		case 1:
			self->m_Ressonance = realValue;
			if (voice)
			{
				if (psy_audio_xmsamplervoice_filtertype(voice) == F_NONE) {
					psy_audio_xmsamplervoice_setfiltertype(voice,
						self->m_DefaultFilterType);
				}				
				psy_audio_xmsamplervoice_setressonance(voice,
					self->m_Ressonance);
			}
			break;
		case 2:
			//Set filter mode. OpenMPT only says 0..F lowpass and 10..1F highpass.
			// It also has a macro default setup where 0 and 8 set the lowpass and 10 an 18 set the highpass
			// From there, I adapted the following table for Psycle.
			if (realValue < 0x20)
			{
				if (realValue < 4) { //0..3
					self->m_DefaultFilterType = F_ITLOWPASS;
				} else if (realValue < 6) { //4..5
					self->m_DefaultFilterType = F_LOWPASS12;
				} else if (realValue < 8) { //6..7
					self->m_DefaultFilterType = F_BANDPASS12;
				} else if (realValue < 0xC) { //8..B
					self->m_DefaultFilterType = F_MPTLOWPASSE;
				} else if (realValue < 0xE) { //C..D
					self->m_DefaultFilterType = F_LOWPASS12E;
				} else if (realValue < 0x10) { //E..F
					self->m_DefaultFilterType = F_BANDPASS12E;
				} else if (realValue < 0x14) { //10..13
					self->m_DefaultFilterType = F_MPTHIGHPASSE;
				} else if (realValue < 0x16) { //14..15
					self->m_DefaultFilterType = F_HIGHPASS12;
				} else if (realValue < 0x18) { //16..17
					self->m_DefaultFilterType = F_BANDREJECT12;
				} else if (realValue < 0x1C) { //18..1B
					self->m_DefaultFilterType = F_MPTHIGHPASSE;
				} else if (realValue < 0x1E) { //1C..1D
					self->m_DefaultFilterType = F_HIGHPASS12E;
				} else { // 1E..1F
					self->m_DefaultFilterType = F_BANDREJECT12E;
				}
				if (voice) {
					psy_audio_xmsamplervoice_setfiltertype(voice,
						self->m_DefaultFilterType);
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	// 2nd Check. Commands that require a voice.
	slidval = 0;
	if (voice)
	{
		switch (volcmd & 0xF0)
		{
		case XM_SAMPLER_CMD_VOL_VOLUME0:
		case XM_SAMPLER_CMD_VOL_VOLUME1:
		case XM_SAMPLER_CMD_VOL_VOLUME2:
		case XM_SAMPLER_CMD_VOL_VOLUME3:
			psy_audio_xmsamplervoice_setvolume(voice, volcmd << 1);
			break;
		case XM_SAMPLER_CMD_VOL_VOLSLIDEUP:
			psy_audio_xmsamplerchannel_volumeslide(self, (volcmd & 0x0F) << 4);
			break;
		case XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN:
			psy_audio_xmsamplerchannel_volumeslide(self, volcmd & 0x0F);			
			break;
		case XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP:
			voice->m_VolumeSlideSpeed = (volcmd & 0x0F) << 1;
			psy_audio_xmsamplervoice_volumeslide(voice);
			break;
		case XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN:
			voice->m_VolumeSlideSpeed = -((volcmd & 0x0F) << 1);
			psy_audio_xmsamplervoice_volumeslide(voice);
			break;
		//	/*				case CMD_VOL::VOL_VIBRATO_SPEED:
		//						Vibrato(volcmd&0x0F,0); //\todo: vibrato_speed does not activate the vibrato if it isn't running.
		//						break;
		//	*/

		case XM_SAMPLER_CMD_VOL_VIBRATO:
			psy_audio_xmsamplerchannel_vibrato(self, 0, (volcmd & 0x0F) << 2);
			break;
		case XM_SAMPLER_CMD_VOL_TONEPORTAMENTO:
			// Portamento to (Gx) affects the memory for Gxx and has the equivalent
			// slide given by this table:
			// SlideTable      DB      1, 4, 8, 16, 32, 64, 96, 128, 255
			if ((volcmd & 0x0F) == 0) slidval = 0;
			else if ((volcmd & 0x0F) == 1)  slidval = 1;
			else if ((volcmd & 0x0F) < 9) slidval = powf(2.0f, volcmd & 0x0F);
			else slidval = 255;			
			psy_audio_xmsamplerchannel_pitchslide(self,
				// up ?
				psy_audio_xmsamplervoice_period(voice) >
				psy_audio_xmsamplervoice_notetoperiod(voice, 
					psy_audio_xmsamplerchannel_note(self), TRUE),
				// speed
				slidval,
				// note
				psy_audio_xmsamplerchannel_note(self));
			break;
		case XM_SAMPLER_CMD_VOL_PITCH_SLIDE_DOWN:
			// Pitch slide up/down affect E/F/(G)'s memory - a Pitch slide
			// up/down of x is equivalent to a normal slide by x*4
			psy_audio_xmsamplerchannel_pitchslide(self,
				FALSE, (volcmd & 0x0F) << 2,
				psy_audio_NOTECOMMANDS_EMPTY);
			break;
		case XM_SAMPLER_CMD_VOL_PITCH_SLIDE_UP:
			psy_audio_xmsamplerchannel_pitchslide(self,
				TRUE, (volcmd & 0x0F) << 2,
				psy_audio_NOTECOMMANDS_EMPTY);
			break;
		default:
			break;
		}

		switch (cmd)
		{
			// Class A: Voice ( They can apply to an already playing voice, or a new coming one).
		case XM_SAMPLER_CMD_VOLUME:
			psy_audio_xmsamplervoice_setvolume(voice, parameter);
			break;
		case XM_SAMPLER_CMD_SET_ENV_POSITION: {
			const psy_dsp_Envelope* env;

			env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_amplitudeenvelope(voice));
			if (psy_dsp_envelope_isenabled(env)) {
				xmenvelopecontroller_setpositioninsamples(
					psy_audio_xmsamplervoice_amplitudeenvelope(voice),
					parameter * psy_audio_machine_samplespertick(self->m_pSampler));
			}
			env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_panenvelope(voice));
			if (psy_dsp_envelope_isenabled(env)) {
				xmenvelopecontroller_setpositioninsamples(
					psy_audio_xmsamplervoice_panenvelope(voice),
					parameter * psy_audio_machine_samplespertick(self->m_pSampler));
			}
			env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_pitchenvelope(voice));
			if (psy_dsp_envelope_isenabled(env)) {
				xmenvelopecontroller_setpositioninsamples(
					psy_audio_xmsamplervoice_pitchenvelope(voice),
					parameter * psy_audio_machine_samplespertick(self->m_pSampler));
			}
			env = xmenvelopecontroller_envelope(psy_audio_xmsamplervoice_filterenvelope(voice));
			if (psy_dsp_envelope_isenabled(env)) {
				xmenvelopecontroller_setpositioninsamples(
					psy_audio_xmsamplervoice_filterenvelope(voice),
					(int)(parameter * psy_audio_machine_samplespertick(self->m_pSampler)));
			}			
			break; }
		case XM_SAMPLER_CMD_EXTENDED:
			switch (parameter & 0xF0)
			{
			case XM_SAMPLER_CMD_E9:
				switch (parameter & 0x0F)
				{
				case XM_SAMPLER_CMD_E9_PLAY_FORWARD:
					psy_audio_wavedatacontroller_changeloopdirection(
						&voice->m_WaveDataController,
						psy_audio_LOOPDIRECTION_FORWARD);
					break;
				case XM_SAMPLER_CMD_E9_PLAY_BACKWARD:
					if (psy_audio_wavedatacontroller_position(&voice->m_WaveDataController) == 0) {
						psy_audio_wavedatacontroller_setposition(&voice->m_WaveDataController,
							psy_audio_wavedatacontroller_length(&voice->m_WaveDataController) - 1);								
					}
					psy_audio_wavedatacontroller_changeloopdirection(
						&voice->m_WaveDataController,
						psy_audio_LOOPDIRECTION_BACKWARD);
					psy_audio_wavedatacontroller_changeloopdirection(
						&voice->m_WaveDataController,
						psy_audio_LOOPDIRECTION_FORWARD);
					break;
				}
				break;
			case XM_SAMPLER_CMD_EE:
				switch (parameter & 0x0F)
				{
					case XM_SAMPLER_CMD_EE_SETNOTECUT:
						psy_audio_xmsamplervoice_setnna(voice,
							psy_audio_NNA_STOP);
						break;
					case XM_SAMPLER_CMD_EE_SETNOTECONTINUE:
						psy_audio_xmsamplervoice_setnna(voice,
							psy_audio_NNA_CONTINUE);
						break;
					case XM_SAMPLER_CMD_EE_SETNOTEOFF:
						psy_audio_xmsamplervoice_setnna(voice,
							psy_audio_NNA_NOTEOFF);
						break;
					case XM_SAMPLER_CMD_EE_SETNOTEFADE:
						psy_audio_xmsamplervoice_setnna(voice,
							psy_audio_NNA_FADEOUT);
						break;
					case XM_SAMPLER_CMD_EE_VOLENVOFF:
						xmenvelopecontroller_stop(
							psy_audio_xmsamplervoice_amplitudeenvelope(voice));					
						break;
					case XM_SAMPLER_CMD_EE_VOLENVON:
						xmenvelopecontroller_start(
							psy_audio_xmsamplervoice_amplitudeenvelope(voice));					
						break;
					case XM_SAMPLER_CMD_EE_PANENVOFF:
						xmenvelopecontroller_stop(
							psy_audio_xmsamplervoice_panenvelope(voice));
						break;
					case XM_SAMPLER_CMD_EE_PANENVON:
						xmenvelopecontroller_start(
							psy_audio_xmsamplervoice_panenvelope(voice));					
						break;
					case XM_SAMPLER_CMD_EE_PITCHENVON:
						xmenvelopecontroller_stop(
							psy_audio_xmsamplervoice_pitchenvelope(voice));					
						break;
					case XM_SAMPLER_CMD_EE_PITCHENVOFF:
						xmenvelopecontroller_start(
							psy_audio_xmsamplervoice_pitchenvelope(voice));
						break;
				}
				break;
			case XM_SAMPLER_CMD_E_DELAYED_NOTECUT:
				psy_audio_xmsamplerchannel_notecuttick(self, parameter & 0x0F);
				break;
			}
			break;

			// Class B Channel ( Just like Class A, but remember its old value if it is called again with  00 as parameter  )

		case XM_SAMPLER_CMD_PORTAMENTO_UP:
			psy_audio_xmsamplerchannel_pitchslide(self, TRUE, parameter, psy_audio_NOTECOMMANDS_EMPTY);
			break;
		case XM_SAMPLER_CMD_PORTAMENTO_DOWN:
			psy_audio_xmsamplerchannel_pitchslide(self, FALSE, parameter, psy_audio_NOTECOMMANDS_EMPTY);
			break;
		case XM_SAMPLER_CMD_PORTA2NOTE:
			psy_audio_xmsamplerchannel_pitchslide(self,
				psy_audio_xmsamplervoice_period(voice) > psy_audio_xmsamplervoice_notetoperiod(voice,
					psy_audio_xmsamplerchannel_note(self), TRUE), parameter, psy_audio_xmsamplerchannel_note(self));
			break;
		case XM_SAMPLER_CMD_VOLUMESLIDE:
			psy_audio_xmsamplerchannel_volumeslide(self, parameter);
			break;
		case XM_SAMPLER_CMD_TONEPORTAVOL:
			psy_audio_xmsamplerchannel_volumeslide(self, parameter);			
			psy_audio_xmsamplerchannel_pitchslide(self,
				psy_audio_xmsamplervoice_period(voice) > psy_audio_xmsamplervoice_notetoperiod(voice,
					psy_audio_xmsamplerchannel_note(self), TRUE), 0, psy_audio_xmsamplerchannel_note(self));
			break;
		case XM_SAMPLER_CMD_VIBRATOVOL:
			psy_audio_xmsamplerchannel_volumeslide(self, parameter);
			psy_audio_xmsamplerchannel_vibrato(self, 0, 0);
			break;
		case XM_SAMPLER_CMD_VIBRATO:
			psy_audio_xmsamplerchannel_vibrato(self, ((parameter >> 4) & 0x0f), (parameter & 0x0f) << 2);
			break;
		case XM_SAMPLER_CMD_FINE_VIBRATO:
			psy_audio_xmsamplerchannel_vibrato(self, ((parameter >> 4) & 0x0f), (parameter & 0x0f));
			break;
		case XM_SAMPLER_CMD_TREMOR:
			psy_audio_xmsamplerchannel_tremorparam(self, parameter);
			break;
		case XM_SAMPLER_CMD_TREMOLO:
			psy_audio_xmsamplerchannel_tremolo(self, (parameter >> 4) & 0x0F, (parameter & 0x0F));
			break;
		case XM_SAMPLER_CMD_RETRIG:
			psy_audio_xmsamplerchannel_retrigger(self, parameter);
			break;
		case XM_SAMPLER_CMD_PANBRELLO:
			psy_audio_xmsamplerchannel_panbrello(self, (parameter >> 4) & 0x0F, (parameter & 0x0F));
			break;
		case XM_SAMPLER_CMD_ARPEGGIO:
			psy_audio_xmsamplerchannel_arpeggio(self, parameter);
			break;			
		}
	}	
	//3rd check: It is not needed that the voice is playing, but it applies to the last instrument.
	if (/*psy_audio_xmsamplerchannel_instrumentno(self) != psy_audio_NOTECOMMANDS_INST_EMPTY*/ 
		psy_audio_xmsamplerchannel_instrumentno(self) != 255 &&
			cmd == XM_SAMPLER_CMD_EXTENDED && (parameter & 0xF0) == XM_SAMPLER_CMD_EE)
	{
		switch (parameter & 0x0F)
		{
		case XM_SAMPLER_CMD_EE_BACKGROUNDNOTECUT:
			psy_audio_xmsamplerchannel_stopbackgroundnotes(
				self, psy_audio_NNA_STOP);
			break;
		case XM_SAMPLER_CMD_EE_BACKGROUNDNOTEOFF:
			psy_audio_xmsamplerchannel_stopbackgroundnotes(
				self, psy_audio_NNA_NOTEOFF);			
			break;
		case XM_SAMPLER_CMD_EE_BACKGROUNDNOTEFADE:
			psy_audio_xmsamplerchannel_stopbackgroundnotes(
				self, psy_audio_NNA_FADEOUT);			
			break;
		}
	}
}

// Add Here those commands that have an effect each tracker tick ( 1 and onwards) .
		// tick "0" is worked on in channel.SetEffect();
void psy_audio_xmsamplerchannel_performfx(psy_audio_XMSamplerChannel* self)
{
	int i;	
	
	for (i = 0; i < psy_audio_xmsampler_numvoices(self->m_pSampler); i++)
	{
		if (psy_audio_xmsamplervoice_channelnum(psy_audio_xmsampler_rvoice(self->m_pSampler, i)) == self->m_Index &&
				psy_audio_xmsamplervoice_isplaying(psy_audio_xmsampler_rvoice(self->m_pSampler, i))) {			
			psy_audio_xmsamplervoice_tick(psy_audio_xmsampler_rvoice(self->m_pSampler, i));
		}
	}
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) // Effects that need a voice to be active.
	{
		if (psy_audio_xmsamplervoice_isautovibrato(psy_audio_xmsamplerchannel_foregroundvoice(self)))
		{
			psy_audio_xmsamplervoice_doautovibrato(psy_audio_xmsamplerchannel_foregroundvoice(self));			
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_PITCHSLIDE)
		{
			psy_audio_xmsamplervoice_pitchslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_SLIDE2NOTE)
		{
			psy_audio_xmsamplervoice_slide2note(psy_audio_xmsamplerchannel_foregroundvoice(self));			
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_VIBRATO)
		{
			psy_audio_xmsamplervoice_vibrato(psy_audio_xmsamplerchannel_foregroundvoice(self));			
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_TREMOLO)
		{
			psy_audio_xmsamplervoice_tremolo(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_PANBRELLO)
		{
			psy_audio_xmsamplervoice_panbrello(psy_audio_xmsamplerchannel_foregroundvoice(self));			
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_TREMOR)
		{
			psy_audio_xmsamplervoice_tremor(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_VOLUMESLIDE)
		{
			psy_audio_xmsamplervoice_volumeslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_NOTECUT)
		{
			psy_audio_xmsamplerchannel_notecut(self);
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_ARPEGGIO)
		{
			psy_audio_xmsamplervoice_updatespeed(psy_audio_xmsamplerchannel_foregroundvoice(self));			
		}
		if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_RETRIG)
		{
			psy_audio_xmsamplerchannel_setlastvoicepanfactor(self,
				psy_audio_xmsamplervoice_panfactor(
					psy_audio_xmsamplerchannel_foregroundvoice(self)));
			if (self->m_RetrigOperation == 1)
			{
				int tmp;				

				tmp = psy_audio_xmsamplervoice_volume(
					psy_audio_xmsamplerchannel_foregroundvoice(self)) +
					self->m_RetrigVol;
				if (tmp < 0) tmp = 0;
				else if (tmp > 128) tmp = 128;
				psy_audio_xmsamplerchannel_setlastvoicevolume(self, tmp);
			} else if (self->m_RetrigOperation == 2)
			{
				int tmp;
				
				tmp = psy_audio_xmsamplervoice_volume(
					psy_audio_xmsamplerchannel_foregroundvoice(self)) *
					self->m_RetrigVol;
				if (tmp < 0) tmp = 0;
				else if (tmp > 128) tmp = 128;
				psy_audio_xmsamplerchannel_setlastvoicevolume(self, tmp);
			}

			if (psy_dsp_envelope_isenabled(
				xmenvelopecontroller_envelope(
					psy_audio_xmsamplervoice_amplitudeenvelope(
						psy_audio_xmsamplerchannel_foregroundvoice(self))))) {
				psy_audio_xmsamplerchannel_setlastampenvelopeposinsamples(
					self, xmenvelopecontroller_getpositioninsamples(
						psy_audio_xmsamplervoice_amplitudeenvelope(
							psy_audio_xmsamplerchannel_foregroundvoice(self))));				
			}
			if (psy_dsp_envelope_isenabled(
				xmenvelopecontroller_envelope(
					psy_audio_xmsamplervoice_panenvelope(
						psy_audio_xmsamplerchannel_foregroundvoice(self))))) {
				psy_audio_xmsamplerchannel_setlastpanenvelopeposinsamples(
					self, xmenvelopecontroller_getpositioninsamples(
						psy_audio_xmsamplervoice_panenvelope(
							psy_audio_xmsamplerchannel_foregroundvoice(self))));
			}
			if (psy_dsp_envelope_isenabled(
				xmenvelopecontroller_envelope(
					psy_audio_xmsamplervoice_filterenvelope(
						psy_audio_xmsamplerchannel_foregroundvoice(self))))) {
				psy_audio_xmsamplerchannel_setlastfilterenvelopeposinsamples(
					self, xmenvelopecontroller_getpositioninsamples(
						psy_audio_xmsamplervoice_filterenvelope(
							psy_audio_xmsamplerchannel_foregroundvoice(self))));
			}
			if (psy_dsp_envelope_isenabled(
				xmenvelopecontroller_envelope(
					psy_audio_xmsamplervoice_pitchenvelope(
						psy_audio_xmsamplerchannel_foregroundvoice(self))))) {
				psy_audio_xmsamplerchannel_setlastpitchenvelopeposinsamples(
					self, xmenvelopecontroller_getpositioninsamples(
						psy_audio_xmsamplervoice_pitchenvelope(
							psy_audio_xmsamplerchannel_foregroundvoice(self))));
			}
			psy_audio_xmsamplervoice_retrig(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	}
	if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_CHANNELVOLSLIDE)
	{
		psy_audio_xmsamplerchannel_channelvolumeslide(self);
	}
	if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_PANSLIDE)
	{
		psy_audio_xmsamplerchannel_panningslide(self);
	}
	if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_NOTEDELAY)
	{
		if (psy_audio_xmsampler_currenttick(self->m_pSampler) == self->m_NoteCutTick)
		{
			psy_List* ite;

			for (ite = self->m_DelayedNote; ite != NULL; psy_list_next(&ite)) {
				psy_audio_xmsampler_tick(self->m_pSampler, self->m_Index,
					ite->entry);				
			}
			psy_list_deallocate(&self->m_DelayedNote, NULL);			
		}
	}
	if (psy_audio_xmsamplerchannel_effectflags(self) & XM_SAMPLER_EFFECT_GLOBALVOLSLIDE)
	{
		psy_audio_xmsampler_setslidevolume(self->m_pSampler,
			self->m_GlobalVolSlideSpeed);		
	}	
}

void psy_audio_xmsamplerchannel_globalvolslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->m_GlobalVolSlideMem == 0) return;
		speed = self->m_GlobalVolSlideMem;
	} else self->m_GlobalVolSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide up
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_GLOBALVOLSLIDE;
		self->m_GlobalVolSlideSpeed = speed;
		if (speed == 0xF) {
			psy_audio_xmsampler_setslidevolume(self->m_pSampler,
				self->m_GlobalVolSlideSpeed);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide down
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_GLOBALVOLSLIDE;
		self->m_GlobalVolSlideSpeed = -speed;
		if (speed == 0xF) {
			psy_audio_xmsampler_setslidevolume(self->m_pSampler,
				self->m_GlobalVolSlideSpeed);			
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide up
		self->m_GlobalVolSlideSpeed = GETSLIDEUPVAL(speed);
		psy_audio_xmsampler_setslidevolume(self->m_pSampler,
			self->m_GlobalVolSlideSpeed);		
	} else if ((speed & 0xF0) == 0xF0) { // FineSlide down
		self->m_GlobalVolSlideSpeed = -GETSLIDEDOWNVAL(speed);
		psy_audio_xmsampler_setslidevolume(self->m_pSampler,
			self->m_GlobalVolSlideSpeed);
	}
}

void psy_audio_xmsamplerchannel_panningslidespeed(
	psy_audio_XMSamplerChannel* self, int speed)
{
	if (speed == 0) {
		if (self->m_PanSlideMem == 0) return;
		speed = self->m_PanSlideMem;
	} else self->m_PanSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide Left
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_PANSLIDE;
		self->m_PanSlideSpeed = -speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_xmsamplerchannel_panningslide(self);		
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Right
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_PANSLIDE;
		self->m_PanSlideSpeed = speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_xmsamplerchannel_panningslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide left
		self->m_PanSlideSpeed = -(GETSLIDEUPVAL(speed)) / 64.0f;
		psy_audio_xmsamplerchannel_panningslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide right
		self->m_PanSlideSpeed = GETSLIDEDOWNVAL(speed) / 64.0f;
		psy_audio_xmsamplerchannel_panningslide(self);
	}
}

void psy_audio_xmsamplerchannel_channelvolumeslidespeed(
	psy_audio_XMSamplerChannel* self, int speed)
{
	if (speed == 0) {
		if (self->m_ChanVolSlideMem == 0) return;
		speed = self->m_ChanVolSlideMem;
	} else self->m_ChanVolSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide up
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_CHANNELVOLSLIDE;
		self->m_ChanVolSlideSpeed = speed / 64.0f;
		if (speed == 0xF) psy_audio_xmsamplerchannel_channelvolumeslide(self);
	} else if (ISSLIDEDOWN(speed)) { // Slide down
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_CHANNELVOLSLIDE;
		self->m_ChanVolSlideSpeed = -speed / 64.0f;
		if (speed == 0xF) psy_audio_xmsamplerchannel_channelvolumeslide(self);
	} else if (ISFINESLIDEUP(speed)) { // FineSlide up
		self->m_ChanVolSlideSpeed = (GETSLIDEUPVAL(speed)) / 64.0f;
		psy_audio_xmsamplerchannel_channelvolumeslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide down
		self->m_ChanVolSlideSpeed = -GETSLIDEDOWNVAL(speed) / 64.0f;
		psy_audio_xmsamplerchannel_channelvolumeslide(self);
	}
}

void psy_audio_xmsamplerchannel_pitchslide(psy_audio_XMSamplerChannel* self,
	bool bUp, int speed, int note)
{
	if (speed == 0) {
		if (self->m_PitchSlideMem == 0) return;
		speed = self->m_PitchSlideMem;
	} else self->m_PitchSlideMem = speed & 0xff;
	if (speed < 0xE0 || note != psy_audio_NOTECOMMANDS_EMPTY)	// Portamento , Fine porta ("f0", and Extra fine porta "e0" ) (*)
	{															// Porta to note does not have Fine.
		speed <<= 2;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
				psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PitchSlideSpeed = bUp ? -speed : speed;
		}
		if (note != psy_audio_NOTECOMMANDS_EMPTY)
		{
			if (note != psy_audio_NOTECOMMANDS_RELEASE) {
 				if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
					psy_audio_xmsamplerchannel_foregroundvoice(self)->m_Slide2NoteDestPeriod =
						psy_audio_xmsamplervoice_notetoperiod(psy_audio_xmsamplerchannel_foregroundvoice(self), note, TRUE);
				}
				self->m_EffectFlags |= XM_SAMPLER_EFFECT_SLIDE2NOTE;
			}
		} else self->m_EffectFlags |= XM_SAMPLER_EFFECT_PITCHSLIDE;
	} else if (speed < 0xF0) {
		speed = speed & 0xf;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PitchSlideSpeed = bUp ? -speed : speed;
			psy_audio_xmsamplervoice_pitchslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	} else {
		speed = (speed & 0xf) << 2;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PitchSlideSpeed = bUp ? -speed : speed;
			psy_audio_xmsamplervoice_pitchslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	}
}

void psy_audio_xmsamplerchannel_volumeslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->m_VolumeSlideMem == 0) return;
		speed = self->m_VolumeSlideMem;
	} else self->m_VolumeSlideMem = speed;

	if (ISSLIDEUP(speed)) { // Slide Up
		speed = GETSLIDEUPVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_VOLUMESLIDE;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = speed << 1;
			if (speed == 0xF) {
				psy_audio_xmsamplervoice_volumeslide(psy_audio_xmsamplerchannel_foregroundvoice(self));				
			}
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Down
		speed = GETSLIDEDOWNVAL(speed);
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_VOLUMESLIDE;
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = -(speed << 1);
				if (speed == 0xF) {
					psy_audio_xmsamplervoice_volumeslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
				}
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide Up
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = GETSLIDEUPVAL(speed) << 1;
			psy_audio_xmsamplervoice_volumeslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide Down
		if (psy_audio_xmsamplerchannel_foregroundvoice(self))
		{
			psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VolumeSlideSpeed = -(GETSLIDEDOWNVAL(speed) << 1);
			psy_audio_xmsamplervoice_volumeslide(psy_audio_xmsamplerchannel_foregroundvoice(self));
		}
	}
}

void psy_audio_xmsamplerchannel_tremorparam(psy_audio_XMSamplerChannel* self,
	int parameter)
{
	if (parameter == 0) {
		if (self->m_TremorMem == 0) return;
		parameter = self->m_TremorMem;
	} else self->m_TremorMem = parameter;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorOnTicks = ((parameter >> 4) & 0xF) + 1;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorOffTicks = (parameter & 0xF) + 1;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorTickChange = psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremorOnTicks;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_TREMOR;
}
void psy_audio_xmsamplerchannel_vibrato(psy_audio_XMSamplerChannel* self,
	int speed, int depth)
{
	if (depth == 0) {
		if (self->m_VibratoDepthMem == 0) return;
		depth = self->m_VibratoDepthMem;
	} else self->m_VibratoDepthMem = depth;

	if (speed == 0) {
		if (self->m_VibratoSpeedMem == 0) return;
		speed = self->m_VibratoSpeedMem;
	} else self->m_VibratoSpeedMem = speed;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VibratoSpeed = speed << 2;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_VibratoDepth = depth;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_VIBRATO;

}// XMSampler::Voice::Vibrato(const int depth,const int speed) ------------------------

void psy_audio_xmsamplerchannel_tremolo(psy_audio_XMSamplerChannel* self,
	int speed, int depth)
{
	if (depth == 0) {
		if (self->m_TremoloDepthMem == 0) return;
		depth = self->m_TremoloDepthMem;
	} else self->m_TremoloDepthMem = depth;

	if (speed == 0) {
		if (self->m_TremoloSpeedMem == 0) return;
		speed = self->m_TremoloSpeedMem;
	} else self->m_TremoloSpeedMem = speed;

	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremoloSpeed = speed << 2;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_TremoloDepth = depth;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_TREMOLO;
}

void psy_audio_xmsamplerchannel_panbrello(psy_audio_XMSamplerChannel* self,
	int speed, int depth)
{
	if (depth == 0) {
		if (self->m_PanbrelloDepthMem == 0) return;
		depth = self->m_PanbrelloDepthMem;
	} else self->m_PanbrelloDepthMem = depth;

	if (speed == 0) {
		if (self->m_PanbrelloSpeedMem == 0) return;
		speed = self->m_PanbrelloSpeedMem;
	} else self->m_PanbrelloSpeedMem = speed;

	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PanbrelloSpeed = speed << 2;
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_PanbrelloDepth = depth;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_PANBRELLO;
}

void psy_audio_xmsamplerchannel_arpeggio(psy_audio_XMSamplerChannel* self,
	int param)
{
	if (param != 0)
	{
		self->m_ArpeggioMem = param;
	} else param = self->m_ArpeggioMem;
	if (psy_audio_xmsamplerchannel_foregroundvoice(self))
	{		
		self->m_ArpeggioPeriod[0] =
			psy_audio_xmsamplervoice_notetoperiod(
				psy_audio_xmsamplerchannel_foregroundvoice(self),
				psy_audio_xmsamplerchannel_note(self) + ((param & 0xf0) >> 4),
				TRUE);
		self->m_ArpeggioPeriod[1] = 
			psy_audio_xmsamplervoice_notetoperiod(
				psy_audio_xmsamplerchannel_foregroundvoice(self),
				psy_audio_xmsamplerchannel_note(self) + (param & 0xf),
				TRUE);			
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_ARPEGGIO;
}

void psy_audio_xmsamplerchannel_retrigger(psy_audio_XMSamplerChannel* self,
	int parameter)
{
	int ticks, volumeModifier;
	int effretVol, effretMode;

	if (parameter == 0)
	{
		ticks = (self->m_RetrigMem & 0x0F);
		volumeModifier = (self->m_RetrigMem >> 4) & 0x0F;
	} else {
		ticks = (parameter & 0x0F);
		volumeModifier = (parameter >> 4) & 0x0F;
		self->m_RetrigMem = parameter;
	}
	switch (volumeModifier)
	{
	case 1://fallthrouhg
	case 2://fallthrouhg
	case 3://fallthrouhg
	case 4://fallthrouhg
	case 5: effretVol = -(int)pow(2., volumeModifier - 1); effretMode = 1; break;
	case 6: effretVol = 0.66666666f;	 effretMode = 2; break;
	case 7: effretVol = 0.5f;			 effretMode = 2; break;

	case 9://fallthrouhg
	case 10://fallthrouhg
	case 11://fallthrouhg
	case 12://fallthrouhg
	case 13: effretVol = (int)pow(2., volumeModifier - 9); effretMode = 1; break;
	case 14: effretVol = 1.5f; effretMode = 2; break;
	case 15: effretVol = 2.0f; effretMode = 2; break;

	case 0://fallthrouhg
	case 8:	//fallthrouhg
	default: effretVol = 0; effretMode = 0; break;
	}
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplerchannel_foregroundvoice(self)->m_RetrigTicks = ticks != 0 ? ticks : 1;
	}
	self->m_RetrigVol = effretVol;
	self->m_RetrigOperation = effretMode;
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_RETRIG;
}

void psy_audio_xmsamplerchannel_notecuttick(psy_audio_XMSamplerChannel* self,
	int ntick)
{
	self->m_NoteCutTick = ntick;
	if (ntick == 0) {
		if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
			psy_audio_xmsamplervoice_setvolume(
				psy_audio_xmsamplerchannel_foregroundvoice(self), 0);
		}		
		return;
	}
	self->m_EffectFlags |= XM_SAMPLER_EFFECT_NOTECUT;
}

void psy_audio_xmsamplerchannel_delayednote(psy_audio_XMSamplerChannel* self,
	psy_audio_PatternEvent data)
{
	psy_audio_PatternEvent* ev;

	if (data.cmd == XM_SAMPLER_CMD_EXTENDED && (data.parameter & 0xf0) == XM_SAMPLER_CMD_E_NOTE_DELAY) {
		self->m_NoteCutTick = data.parameter & 0x0f;
		data.cmd = XM_SAMPLER_CMD_NONE;
		data.parameter = 0;
		self->m_EffectFlags |= XM_SAMPLER_EFFECT_NOTEDELAY;
	}
	ev = psy_audio_patternevent_clone(&data);
	psy_list_append(&self->m_DelayedNote, ev);
}

void psy_audio_xmsamplerchannel_panningslide(psy_audio_XMSamplerChannel* self)
{
	self->m_PanFactor += self->m_PanSlideSpeed;
	if (self->m_PanFactor < 0.0f) {
		self->m_PanFactor = 0.0f;
	} else if (self->m_PanFactor > 1.0f) {
		self->m_PanFactor = 1.0f;
	}
	if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplervoice_setpanfactor(
			psy_audio_xmsamplerchannel_foregroundvoice(self), self->m_PanFactor);
	}	
}

void psy_audio_xmsamplerchannel_channelvolumeslide(psy_audio_XMSamplerChannel* self)
{
	self->m_Volume += self->m_ChanVolSlideSpeed;
	if (self->m_Volume < 0.0f) {
		self->m_Volume = 0.0f;
	} else if (self->m_Volume > 1.0f) {
		self->m_Volume = 1.0f;
	}
}

void psy_audio_xmsamplerchannel_notecut(psy_audio_XMSamplerChannel* self)
{
	if (psy_audio_xmsampler_currenttick(self->m_pSampler) == self->m_NoteCutTick)
	{
		if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
			psy_audio_xmsamplervoice_setvolume(
				psy_audio_xmsamplerchannel_foregroundvoice(self), 0);
		}
		self->m_EffectFlags &= ~XM_SAMPLER_EFFECT_NOTECUT;
	}
}

void psy_audio_xmsamplerchannel_stopbackgroundnotes(psy_audio_XMSamplerChannel* self,
	psy_audio_NewNoteAction action)
{
	int current;

	for (current = 0; current < psy_audio_xmsampler_numvoices(self->m_pSampler); current++)
	{
		if (psy_audio_xmsamplervoice_channelnum(psy_audio_xmsampler_rvoice(self->m_pSampler, current)) == self->m_Index &&
			psy_audio_xmsamplervoice_isplaying(psy_audio_xmsampler_rvoice(self->m_pSampler, current)))
		{
			switch (action)
			{
			case psy_audio_NNA_NOTEOFF:
				psy_audio_xmsamplervoice_noteoff(
					psy_audio_xmsampler_rvoice(self->m_pSampler, current));				
				break;
			case psy_audio_NNA_FADEOUT:
				psy_audio_xmsamplervoice_notefadeout(
					psy_audio_xmsampler_rvoice(self->m_pSampler, current));				
				break;
			case psy_audio_NNA_STOP:
				psy_audio_xmsamplervoice_noteofffast(
					psy_audio_xmsampler_rvoice(self->m_pSampler, current));				
				break;
			}
		}
	}
}

double psy_audio_xmsamplerchannel_arpeggioperiod(const psy_audio_XMSamplerChannel* self)
{
	int arpi = psy_audio_xmsampler_currenttick(self->m_pSampler) % 3;
	if (arpi >= 1) {
		return self->m_ArpeggioPeriod[arpi - 1];
	} else {
		return self->m_Period;
	}
}

bool  psy_audio_xmsamplerchannel_load(psy_audio_XMSamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	char chan[8];
	int32_t size = 0;
	int32_t temp32;

	psyfile_read(songfile->file, chan, 4);
	chan[4] = '\0';
	psyfile_read(songfile->file, &size, sizeof(int32_t));
	if (strcmp(chan, "CHAN")) {
		return FALSE;
	}
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	///< (0..200)   &0x100 = Mute.
	self->m_ChannelDefVolume = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	//<  0..200 .  &0x100 = Surround.
	self->m_DefaultPanFactor = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultCutoff = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultRessonance = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultFilterType = (psy_dsp_FilterType)temp32;
	return TRUE;
}

void psy_audio_xmsamplerchannel_save(psy_audio_XMSamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	int size;
	
	size = 5 * sizeof(int);
	psyfile_write(songfile->file, "CHAN", 4);
	psyfile_write_int32(songfile->file, size);
	psyfile_write_int32(songfile->file, self->m_ChannelDefVolume);
	psyfile_write_int32(songfile->file, self->m_DefaultPanFactor);
	psyfile_write_int32(songfile->file, self->m_DefaultCutoff);
	psyfile_write_int32(songfile->file, self->m_DefaultRessonance);		
	psyfile_write_uint32(songfile->file, (uint32_t)self->m_DefaultFilterType);
}

void psy_audio_xmsamplerchannel_filtertype_tweak(psy_audio_XMSamplerChannel* self,
	psy_audio_ChoiceMachineParam* sender, float value)
{
	psy_audio_xmsamplerchannel_setdefaultfiltertype(self,
		(psy_dsp_FilterType)(int)((filter_numfilters() - 1)* value));
}

void psy_audio_xmsamplerchannel_filtertype_normvalue(psy_audio_XMSamplerChannel* self,
	psy_audio_ChoiceMachineParam* sender, float* rv)
{
	*rv = (float)psy_audio_xmsamplerchannel_defaultfiltertype(self) /
		((float)filter_numfilters() - 1);
}

void psy_audio_xmsamplerchannel_defaultpan_tweak(psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float value)
{
	if (self->m_pSampler->channeldisplay == 0) {
		psy_audio_xmsamplerchannel_setdefaultpanfactorfloat(self, value, FALSE);		
	} else {
		psy_audio_xmsamplerchannel_setpanfactor(self, value);		
	}	
}

void psy_audio_xmsamplerchannel_defaultpan_normvalue(psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	if (self->m_pSampler->channeldisplay == 0) {
		*rv = psy_audio_xmsamplerchannel_defaultpanfactorfloat(self);
	} else {
		*rv = psy_audio_xmsamplerchannel_panfactor(self);
	}	
}

void psy_audio_xmsamplerchannel_slider_tweak(psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float value)
{
	if (self->m_pSampler->channeldisplay == 0) {
		psy_audio_xmsamplerchannel_setdefaultvolumefloat(self, value, TRUE);		
	} else if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		psy_audio_xmsamplerchannel_setvolume(self, value);
	}
}

void psy_audio_xmsamplerchannel_slider_normvalue(psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	if (self->m_pSampler->channeldisplay == 0) {
		*rv = psy_audio_xmsamplerchannel_defaultvolumefloat(self);
	} else if (psy_audio_xmsamplerchannel_foregroundvoice(self)) {
		*rv = psy_audio_xmsamplerchannel_volume(self);
	}
}

void psy_audio_xmsamplerchannel_defaultismute_tweak(
	psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float value)
{
	if (value == 0.f) {
		psy_audio_xmsamplerchannel_setdefaultismute(self, FALSE);
	} else {
		psy_audio_xmsamplerchannel_setdefaultismute(self, TRUE);
	}
}

void psy_audio_xmsamplerchannel_defaultismute_normvalue(
	psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	if (psy_audio_xmsamplerchannel_defaultismute(self)) {
		*rv = 1.f;
	} else {
		*rv = 0.f;
	}
}

void psy_audio_xmsamplerchannel_defaultissurround_tweak(
	psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float value)
{
	if (value == 0.f) {
		psy_audio_xmsamplerchannel_setdefaultissurround(self, FALSE);
	} else {
		psy_audio_xmsamplerchannel_setdefaultissurround(self, TRUE);
	}
}

void psy_audio_xmsamplerchannel_defaultissurround_normvalue(
	psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	if (psy_audio_xmsamplerchannel_defaultissurround(self)) {
		*rv = 1.f;
	} else {
		*rv = 0.f;
	}
}
