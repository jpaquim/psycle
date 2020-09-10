// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsamplerchannel.h"

#include "samplerdefs.h"
#include "machineparam.h"
#include "songio.h"
#include "plugin_interface.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "../../detail/portable.h"
#include "../../detail/trace.h"
#include "constants.h"


static const char E8VolMap[16] = { 0,4,9,13,17,21,26,30,34,38,43,47,51,55,60,64 };

// SamplerChannel (master index: UINTPTR_MAX)

static void psy_audio_xmsamplerchannel_level_normvalue(psy_audio_XMSamplerChannel*,
	psy_audio_IntMachineParam* sender, float* rv);
// getters
INLINE float psy_audio_xmsamplerchannel_defaultpanfactorfloat(
	psy_audio_XMSamplerChannel* self)
{
	return (self->defaultpanfactor & 0xFF) / 200.0f;
}

// effects
static void psy_audio_xmsamplerchannel_setvolumeslide(psy_audio_XMSamplerChannel* self,
	int speed);
static void psy_audio_xmsamplerchannel_setchannelvolumeslide(psy_audio_XMSamplerChannel*,
	int speed);
static void psy_audio_xmsamplerchannel_perform_channelvolumeslide(psy_audio_XMSamplerChannel*);
static void psy_audio_xmsamplerchannel_setpanningslide(psy_audio_XMSamplerChannel*,
	int speed);
static void psy_audio_xmsamplerchannel_performpanningslide(psy_audio_XMSamplerChannel*);
static void psy_audio_xmsamplerchannel_setvibrato(psy_audio_XMSamplerChannel*, int speed, int depth);
static void psy_audio_xmsamplerchannel_setpitchslide(psy_audio_XMSamplerChannel*,
	bool bUp, int speed, int note);
static void psy_audio_xmsamplerchannel_settremolo(psy_audio_XMSamplerChannel*, int speed, int depth);
static void psy_audio_xmsamplerchannel_setpanbrello(psy_audio_XMSamplerChannel*, int speed, int depth);
static void  psy_audio_xmsamplerchannel_setretrigger(psy_audio_XMSamplerChannel*, const int parameter);
static void  psy_audio_xmsamplerchannel_performretrigger(psy_audio_XMSamplerChannel*);

void psy_audio_xmsamplerchannel_init(psy_audio_XMSamplerChannel* self, uintptr_t index)
{
	char text[127];

	self->volume = 1.f;
	self->panfactor = (psy_dsp_amp_t) 0.5f;
	self->channeldefvolume =  1.f;	
	self->defaultpanfactor = 100;
	self->defaultcutoff = 127;
	self->defaultressonance = 0;
	self->defaultfiltertype = F_NONE;
	self->effects = NULL;
	self->index = index;
	psy_audio_xmsamplerchannel_restore(self);
	if (index == UINTPTR_MAX) {
		psy_snprintf(text, 127, "%s", "Master");
	} else {
		psy_snprintf(text, 127, "Channel %d", (int)index);
	}
	psy_audio_infomachineparam_init(&self->param_channel, text, "", MPF_SMALL);
	psy_audio_intmachineparam_init(&self->filter_cutoff,
		"", "", MPF_STATE | MPF_SMALL, &self->defaultcutoff, 0, 200);
	psy_audio_intmachineparam_init(&self->filter_res,
		"", "", MPF_STATE | MPF_SMALL, &self->defaultressonance, 0, 200);
	psy_audio_intmachineparam_init(&self->pan,
		"", "", MPF_STATE | MPF_SMALL, &self->defaultpanfactor, 0, 200);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "", MPF_SLIDER | MPF_SMALL, &self->channeldefvolume);
	psy_audio_volumemachineparam_setmode(&self->slider_param, psy_audio_VOLUME_LINEAR);
	psy_audio_volumemachineparam_setrange(&self->slider_param, 0, 200);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		psy_audio_xmsamplerchannel_level_normvalue);	
}

void psy_audio_xmsamplerchannel_dispose(psy_audio_XMSamplerChannel* self)
{
	psy_audio_infomachineparam_dispose(&self->param_channel);
	psy_audio_intmachineparam_dispose(&self->filter_cutoff);
	psy_audio_intmachineparam_dispose(&self->filter_res);
	psy_audio_intmachineparam_dispose(&self->pan);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
	psy_list_free(self->effects);
	self->effects = NULL;
}

void psy_audio_xmsamplerchannel_restore(psy_audio_XMSamplerChannel* self)
{
	self->volume = self->channeldefvolume;
	self->panfactor = psy_audio_xmsamplerchannel_defaultpanfactorfloat(self);
	psy_audio_xmsamplerchannel_effectinit(self);	
}

void psy_audio_xmsamplerchannel_newline(psy_audio_XMSamplerChannel* self)
{
	psy_list_free(self->effects);
	self->effects = NULL;
}

void psy_audio_xmsamplerchannel_level_normvalue(psy_audio_XMSamplerChannel* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	/*if (self->index == UINTPTR_MAX) {
		psy_audio_Buffer* memory;

		memory = psy_audio_machine_buffermemory(
			psy_audio_xmsampler_base(self->sampler));
		if (memory) {
			*rv = psy_audio_buffer_rmsdisplay(memory);
		} else {
			*rv = 0.f;
		}
	} else {
		*rv = 0.f;
	}*/
	* rv = 0;
}

void psy_audio_xmsamplerchannel_work(psy_audio_XMSamplerChannel* self,
	psy_audio_BufferContext* bc)
{
	if (self->index == UINTPTR_MAX) {
		psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->channeldefvolume);
	} else {
		psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->volume);
	}
}

// Channel Effects
void psy_audio_xmsamplerchannel_effectinit(psy_audio_XMSamplerChannel* self)
{
	psy_list_free(self->effects);
	self->effects = NULL;
	// volume slide
	self->volumeslidemem = 0;
	// channel volume slide
	self->chanvolslidespeed = 0.0f;
	self->chanvolslidemem = 0;
	// pitchslide
	self->pitchslidespeed = 0.0f;
	self->pitchslidemem = 0;
	// pan
	self->panslidespeed = 0.0f;
	self->panslidemem = 0;
	// vibrato
	self->vibratotype = psy_audio_WAVEFORMS_SINUS;
	self->vibratodepthmem = 0;
	self->vibratospeedmem = 0;
	// tremor
	self->tremorontime = 0;
	self->tremorofftime = 0;
	// tremelo
	self->tremolotype = psy_audio_WAVEFORMS_SINUS;
	self->tremolospeed = 0;
	self->tremolodepth = 0;
	self->tremolodelta = 0;
	self->tremolopos = 0;
	self->tremolodepthmem = 0;
	self->tremolospeedmem = 0;
	// panbrello
	self->panbrellotype = psy_audio_WAVEFORMS_SINUS;
	self->panbrellospeed = 0;
	self->panbrellodepth = 0;
	self->panbrellodelta = 0;
	self->panbrellopos = 0;	
	self->panbrellodepthmem = 0;
	self->panbrellospeedmem = 0;
	// retrig
	self->retrigoperation = 0;
	self->retrigvol = 0;
	// appreggio
	self->arpeggioperiod[0] = 0.0;
	self->arpeggioperiod[1] = 0.0;
}

void psy_audio_xmsamplerchannel_seteffect(psy_audio_XMSamplerChannel* self,
	const psy_audio_PatternEvent* ev)
{
	int realset = 0;
	int	realvalue = 0;
	int volcmd = 0;
	
	//1st check: Channel ( They can appear without an existing playing note and are persistent when a new one comes)
	switch (volcmd & 0xF0)
	{
		case SAMPLER_CMD_VOL_PANNING:
			psy_audio_xmsamplerchannel_setpanfactor(self,
				(volcmd & 0x0F) / 15.0f);
			break;
		case SAMPLER_CMD_VOL_PANSLIDELEFT:
			// this command is actually fine pan slide
			psy_audio_xmsamplerchannel_setpanningslide(self,
				(volcmd & 0x0F) << 4);
			break;
		case SAMPLER_CMD_VOL_PANSLIDERIGHT:
			// this command is actually fine pan slide
			psy_audio_xmsamplerchannel_setpanningslide(self,
				volcmd & 0x0F);
			break;
		case SAMPLER_CMD_VOL_VIBRATO:
			psy_audio_xmsamplerchannel_setvibrato(self,
				0, (volcmd & 0x0F) << 2);
			break;
		default:
			break;
	}
	
	switch (ev->cmd) {
		case SAMPLER_CMD_PANNING:
			self->surround = FALSE;
			self->panfactor = ev->parameter / (psy_dsp_amp_t)255;
		break;
		case SAMPLER_CMD_SET_CHANNEL_VOLUME:
			self->volume = (psy_dsp_amp_t)((ev->parameter < 64)
				? (ev->parameter / 64.0f)
				: 1.0f);
			break;
		case SAMPLER_CMD_PANNINGSLIDE:
			psy_audio_xmsamplerchannel_setpanningslide(self, ev->parameter);
			break;
		case SAMPLER_CMD_CHANNEL_VOLUMESLIDE:
			psy_audio_xmsamplerchannel_setchannelvolumeslide(self, ev->parameter);
			break;
		case SAMPLER_CMD_SET_GLOBAL_VOLUME:			
			// m_pSampler->GlobalVolume(parameter < 0x80 ? parameter : 0x80);
			break;
		case SAMPLER_CMD_VOLUMESLIDE:
			psy_audio_xmsamplerchannel_setvolumeslide(self, ev->parameter);
			break;
		case SAMPLER_CMD_GLOBAL_VOLUME_SLIDE:
			break;		
		case SAMPLER_CMD_EXTENDED:
			switch (ev->parameter & 0xF0) {
				case SAMPLER_CMD_E9:
					switch (ev->parameter & 0x0F) {
						case SAMPLER_CMD_E9_SURROUND_OFF:
							self->surround = FALSE;
							break;
						case SAMPLER_CMD_E9_SURROUND_ON:
							self->surround = TRUE;
							break;
						case SAMPLER_CMD_E9_REVERB_OFF:
							break;
						case SAMPLER_CMD_E9_REVERB_FORCE:
							break;
						case SAMPLER_CMD_E9_STANDARD_SURROUND:
							break;
						case SAMPLER_CMD_E9_QUAD_SURROUND:
							break;
						case SAMPLER_CMD_E9_GLOBAL_FILTER:
							break;
						case SAMPLER_CMD_E9_LOCAL_FILTER:
							break;
						default:
							break;
						}
					break;
				case SAMPLER_CMD_E_SET_PAN:
					psy_audio_xmsamplerchannel_setpanfactor(self,
						E8VolMap[(ev->parameter & 0xf)] / 64.0f);
					break;
				case SAMPLER_CMD_E_SET_MIDI_MACRO:
					//\todo : implement. For now, it maps directly to internal Filter commands
					self->midi_set = ev->parameter & 0x0F;
					break;
				case SAMPLER_CMD_E_GLISSANDO_TYPE:
					self->grissando = ev->parameter != 0;
					break;
				case SAMPLER_CMD_E_VIBRATO_WAVE:
					if (ev->parameter <= psy_audio_WAVEFORMS_RANDOM) {
						self->vibratotype = (psy_audio_WaveForms)(ev->parameter);
					} else { self->vibratotype = psy_audio_WAVEFORMS_SINUS; }
					break;
				case SAMPLER_CMD_E_PANBRELLO_WAVE:
					if (ev->parameter <= psy_audio_WAVEFORMS_RANDOM) {
						self->panbrellotype = (psy_audio_WaveForms)(ev->parameter);
					} else { self->panbrellotype = psy_audio_WAVEFORMS_SINUS; }
					break;
				case SAMPLER_CMD_E_TREMOLO_WAVE:
					if (ev->parameter <= psy_audio_WAVEFORMS_RANDOM) {
						self->tremolotype = (psy_audio_WaveForms)(ev->parameter);
					} else { self->tremolotype = psy_audio_WAVEFORMS_SINUS; }
					break;
				default:
				break;
			}
			break;
		case SAMPLER_CMD_MIDI_MACRO:
			if (ev->parameter < 0x80) {
				realset = self->midi_set;
				realvalue = ev->parameter;
			} else {
				//realset = m_pSampler->GetMap(ev->parameter - 0x80).mode;
				//realvalue = m_pSampler->GetMap(ev->parameter - 0x80).value;
			}
			switch (realset) {
				case 0:
					self->cutoff = realvalue;
					//if (voice)
					//{
					//	if (voice->FilterType() == F_NONE) voice->FilterType(self->defaultfiltertype);
					//	voice->CutOff(m_Cutoff);
					//}
					break;
				case 1:
					self->ressonance = realvalue;
					//if (voice)
					//{
					//	if (voice->FilterType() == F_NONE) voice->FilterType(m_DefaultFilterType);
					//	voice->Ressonance(m_Ressonance);
					//}
					break;
				case 2:
					//Set filter mode. OpenMPT only says 0..F lowpass and 10..1F highpass.
					// It also has a macro default setup where 0 and 8 set the lowpass and 10 an 18 set the highpass
					// From there, I adapted the following table for Psycle.
					if (realvalue < 0x20) {
						if (realvalue < 4) { //0..3
							self->defaultfiltertype = F_ITLOWPASS;
						} else if (realvalue < 6) { //4..5
							self->defaultfiltertype = F_LOWPASS12;
						} else if (realvalue < 8) { //6..7
							self->defaultfiltertype = F_BANDPASS12;
						} else if (realvalue < 0xC) { //8..B
							self->defaultfiltertype = F_MPTLOWPASSE;
						} else if (realvalue < 0xE) { //C..D
							self->defaultfiltertype = F_LOWPASS12E;
						} else if (realvalue < 0x10) { //E..F
							self->defaultfiltertype = F_BANDPASS12E;
						} else if (realvalue < 0x14) { //10..13
							self->defaultfiltertype = F_MPTHIGHPASSE;
						} else if (realvalue < 0x16) { //14..15
							self->defaultfiltertype = F_HIGHPASS12;
						} else if (realvalue < 0x18) { //16..17
							self->defaultfiltertype = F_BANDREJECT12;
						} else if (realvalue < 0x1C) { //18..1B
							self->defaultfiltertype = F_MPTHIGHPASSE;
						} else if (realvalue < 0x1E) { //1C..1D
							self->defaultfiltertype = F_HIGHPASS12E;
						} else { // 1E..1F
							self->defaultfiltertype = F_BANDREJECT12E;
						}
						// if (voice) { voice->FilterType(m_DefaultFilterType); }
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	switch (ev->cmd) {
		case SAMPLER_CMD_VIBRATO:
			psy_audio_xmsamplerchannel_setvibrato(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F) << 2);
			break;
		case SAMPLER_CMD_FINE_VIBRATO:
			psy_audio_xmsamplerchannel_setvibrato(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case SAMPLER_CMD_TREMOLO:
			psy_audio_xmsamplerchannel_settremolo(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case SAMPLER_CMD_PANBRELLO:
			psy_audio_xmsamplerchannel_setpanbrello(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case SAMPLER_CMD_RETRIG:
			psy_audio_xmsamplerchannel_setretrigger(self,
				ev->parameter);
			break;
		default:
			break;
	}
}

void psy_audio_xmsamplerchannel_addeffect(psy_audio_XMSamplerChannel* self, int cmd)
{
	psy_list_append(&self->effects, (void*)(uintptr_t)cmd);
}

void psy_audio_xmsamplerchannel_performfx(psy_audio_XMSamplerChannel* self)
{
	psy_List* p;

	for (p = self->effects; p != NULL; psy_list_next(&p)) {
		int effect = (int)(intptr_t)psy_list_entry(p);
		
		switch (effect) {
			case SAMPLER_EFFECT_CHANNELVOLSLIDE:
				psy_audio_xmsamplerchannel_perform_channelvolumeslide(self);
				break;
			case SAMPLER_EFFECT_PANSLIDE:				
				psy_audio_xmsamplerchannel_performpanningslide(self);
				break;
			default:
				break;
		}
	}	
}

void psy_audio_xmsamplerchannel_setvolumeslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->volumeslidemem == 0) return;
		speed = self->volumeslidemem;
	} else self->volumeslidemem = speed;
	psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_CHANNELVOLSLIDE);
}

void psy_audio_xmsamplerchannel_setchannelvolumeslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->chanvolslidemem == 0) {
			return;
		}
		speed = self->chanvolslidemem;
	} else self->chanvolslidemem = speed;

	if (ISSLIDEUP(speed)) { // Slide up
		speed = GETSLIDEUPVAL(speed);
		psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_CHANNELVOLSLIDE);
		self->chanvolslidespeed = speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_xmsamplerchannel_perform_channelvolumeslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide down
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_CHANNELVOLSLIDE);
		self->chanvolslidespeed = -speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_xmsamplerchannel_perform_channelvolumeslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide up
		self->chanvolslidespeed = (GETSLIDEUPVAL(speed)) / 64.0f;
		psy_audio_xmsamplerchannel_perform_channelvolumeslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide down
		self->chanvolslidespeed = -GETSLIDEDOWNVAL(speed) / 64.0f;
		psy_audio_xmsamplerchannel_perform_channelvolumeslide(self);
	}
}

void psy_audio_xmsamplerchannel_perform_channelvolumeslide(psy_audio_XMSamplerChannel* self)
{
	self->volume += self->chanvolslidespeed;
	if (self->volume < 0.0f) {
		self->volume = 0.0f;
	} else if (self->volume > 1.0f) {
		self->volume = 1.0f;
	}
}

void psy_audio_xmsamplerchannel_setpanningslide(psy_audio_XMSamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->panslidemem == 0) return;
		speed = self->panslidemem;
	} else self->panslidemem = speed;

	if (ISSLIDEUP(speed)) { // Slide Left
		speed = GETSLIDEUPVAL(speed);
		psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_PANSLIDE);
		self->panslidespeed = -speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_xmsamplerchannel_performpanningslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Right
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_PANSLIDE);
		self->panslidespeed = speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_xmsamplerchannel_performpanningslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide left
		self->panslidespeed = -(GETSLIDEUPVAL(speed)) / 64.0f;
		psy_audio_xmsamplerchannel_performpanningslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide right
		self->panslidespeed = GETSLIDEDOWNVAL(speed) / 64.0f;
		psy_audio_xmsamplerchannel_performpanningslide(self);
	}
}

void psy_audio_xmsamplerchannel_performpanningslide(psy_audio_XMSamplerChannel* self)
{
	self->panfactor += self->panslidespeed;
	if (self->panfactor < 0.0f) {
		self->panfactor = 0.0f;
	} else if (self->panfactor > 1.0f) {
		self->panfactor = 1.0f;
	}
}

void psy_audio_xmsamplerchannel_setvibrato(psy_audio_XMSamplerChannel* self, int speed, int depth)
{	
	if (depth == 0) {
		if (self->vibratodepthmem == 0) return;
		depth = self->vibratodepthmem;
	} else self->vibratodepthmem = depth;

	if (speed == 0) {
		if (self->vibratospeedmem == 0) return;
		speed = self->vibratospeedmem;
	} else self->vibratospeedmem = speed;	
	psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_VIBRATO);
}

void psy_audio_xmsamplerchannel_setpitchslide(psy_audio_XMSamplerChannel* self,
	bool bUp, int speed, int note)
{
	return;

	if (speed == 0) {
		if (self->pitchslidemem == 0) return;
		speed = self->pitchslidemem;
	} else self->pitchslidemem = speed & 0xff;
	if (speed < 0xE0 || note != NOTECOMMANDS_EMPTY)	// Portamento , Fine porta ("f0", and Extra fine porta "e0" ) (*)
	{									// Porta to note does not have Fine.
		speed <<= 2;
		//if (ForegroundVoice()) { ForegroundVoice()->m_PitchSlideSpeed = bUp ? -speed : speed; }
		if (note != NOTECOMMANDS_EMPTY)
		{
			if (note != NOTECOMMANDS_RELEASE) {
			//	if (ForegroundVoice()) { ForegroundVoice()->m_Slide2NoteDestPeriod = ForegroundVoice()->NoteToPeriod(note); }
				psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_SLIDE2NOTE);
			}
		} else {
			psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_PITCHSLIDE);
		}
	} else if (speed < 0xF0) {
		speed = speed & 0xf;
		// if (ForegroundVoice())
		// {
			//ForegroundVoice()->m_PitchSlideSpeed = bUp ? -speed : speed;
			//ForegroundVoice()->PitchSlide();
		//}
	} else {
		speed = (speed & 0xf) << 2;
		//if (ForegroundVoice())
		//{
			//ForegroundVoice()->m_PitchSlideSpeed = bUp ? -speed : speed;
			//ForegroundVoice()->PitchSlide();
		//}
	}
}

void psy_audio_xmsamplerchannel_settremolo(psy_audio_XMSamplerChannel* self, int speed, int depth)
{
	if (depth == 0) {
		if (self->tremolodepthmem == 0) return;
		depth = self->tremolodepthmem;
	} else self->tremolodepthmem = depth;

	if (speed == 0) {
		if (self->tremolospeedmem == 0) return;
		speed = self->tremolospeedmem;
	} else self->tremolospeedmem = speed;
	psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_TREMOLO);
}

void psy_audio_xmsamplerchannel_setpanbrello(psy_audio_XMSamplerChannel* self, int speed, int depth)
{
	if (depth == 0) {
		if (self->panbrellodepthmem == 0) return;
		depth = self->panbrellodepthmem;
	} else self->panbrellodepthmem = depth;

	if (speed == 0) {
		if (self->panbrellospeedmem == 0) return;
		speed = self->panbrellospeedmem;
	} else self->panbrellospeedmem = speed;
	psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_PANBRELLO);
}

void  psy_audio_xmsamplerchannel_setretrigger(psy_audio_XMSamplerChannel* self, 
	const int parameter)
{
	int ticks, volumemodifier;
	int effretvol, effretmode;

	if (parameter == 0)
	{
		ticks = (self->retrigmem & 0x0f);
		volumemodifier = (self->retrigmem >> 4) & 0x0f;
	} else {
		ticks = (parameter & 0x0F);
		volumemodifier = (parameter >> 4) & 0x0f;
		self->retrigmem = parameter;
	}
	switch (volumemodifier)
	{
	case 1://fallthrouhg
	case 2://fallthrouhg
	case 3://fallthrouhg
	case 4://fallthrouhg
	case 5: effretvol = -(int)pow(2., volumemodifier - 1); effretmode = 1; break;
	case 6: effretvol = 0.66666666f;	 effretmode = 2; break;
	case 7: effretvol = 0.5f;			 effretmode = 2; break;

	case 9://fallthrouhg
	case 10://fallthrouhg
	case 11://fallthrouhg
	case 12://fallthrouhg
	case 13: effretvol = (int)pow(2., volumemodifier - 9); effretmode = 1; break;
	case 14: effretvol = 1.5f; effretmode = 2; break;
	case 15: effretvol = 2.0f; effretmode = 2; break;

	case 0://fallthrouhg
	case 8:	//fallthrouhg
	default: effretvol = 0; effretmode = 0; break;
	}
	self->ticks = ticks;
	self->retrigvol = effretvol;
	self->retrigoperation = effretmode;
	psy_audio_xmsamplerchannel_addeffect(self, SAMPLER_EFFECT_RETRIG);
	
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
	self->channeldefvolume = temp32 / 200.f;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	//<  0..200 .  &0x100 = Surround.
	self->defaultpanfactor = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->defaultcutoff = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->defaultressonance = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->defaultfiltertype = (FilterType)temp32;
	return TRUE;
}

void psy_audio_xmsamplerchannel_save(psy_audio_XMSamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	int size = 5 * sizeof(int);
	psyfile_write(songfile->file, "CHAN", 4);
	psyfile_write_int32(songfile->file, size);
	psyfile_write_int32(songfile->file, (int32_t)(self->channeldefvolume * 200));
	psyfile_write_int32(songfile->file, self->defaultpanfactor);
	psyfile_write_int32(songfile->file, self->defaultcutoff);
	psyfile_write_int32(songfile->file, self->defaultressonance);
	psyfile_write_uint32(songfile->file, self->defaultfiltertype);	
}
