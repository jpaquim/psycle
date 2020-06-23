// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "samplerchannel.h"

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


// SamplerChannel (master index: UINTPTR_MAX)

static void psy_audio_samplerchannel_level_normvalue(psy_audio_SamplerChannel*,
	psy_audio_IntMachineParam* sender, float* rv);
// getters
static INLINE float psy_audio_samplerchannel_defaultpanfactorfloat(
	psy_audio_SamplerChannel* self)
{
	return (self->defaultpanfactor & 0xFF) / 200.0f;
}

// effects
static void psy_audio_samplerchannel_setvolumeslide(psy_audio_SamplerChannel* self,
	int speed);
static void psy_audio_samplerchannel_setchannelvolumeslide(psy_audio_SamplerChannel*,
	int speed);
static void psy_audio_samplerchannel_perform_channelvolumeslide(psy_audio_SamplerChannel*);
static void psy_audio_samplerchannel_setpanningslide(psy_audio_SamplerChannel*,
	int speed);
static void psy_audio_samplerchannel_performpanningslide(psy_audio_SamplerChannel*);
static void psy_audio_samplerchannel_setvibrato(psy_audio_SamplerChannel*, int speed, int depth);
static void psy_audio_samplerchannel_settremolo(psy_audio_SamplerChannel*, int speed, int depth);
static void psy_audio_samplerchannel_setpanbrello(psy_audio_SamplerChannel*, int speed, int depth);
static void  psy_audio_samplerchannel_setretrigger(psy_audio_SamplerChannel*, const int parameter);
static void  psy_audio_samplerchannel_performretrigger(psy_audio_SamplerChannel*);

void psy_audio_samplerchannel_init(psy_audio_SamplerChannel* self, uintptr_t index)
{
	char text[127];

	self->volume = 1.f;
	self->panfactor = (psy_dsp_amp_t) 0.5f;
	self->channeldefvolume =  1.f;	
	self->defaultpanfactor = 100;
	self->m_DefaultCutoff = 127;
	self->m_DefaultRessonance = 0;
	self->defaultfiltertype = F_NONE;
	self->effects = NULL;
	self->index = index;
	psy_audio_samplerchannel_restore(self);
	if (index == UINTPTR_MAX) {
		psy_snprintf(text, 127, "%s", "Master");
	} else {
		psy_snprintf(text, 127, "Channel %d", (int)index);
	}
	psy_audio_infomachineparam_init(&self->param_channel, text, "", MPF_SMALL);
	psy_audio_intmachineparam_init(&self->filter_cutoff,
		"", "", MPF_STATE | MPF_SMALL, &self->m_DefaultCutoff, 0, 200);
	psy_audio_intmachineparam_init(&self->filter_res,
		"", "", MPF_STATE | MPF_SMALL, &self->m_DefaultRessonance, 0, 200);
	psy_audio_intmachineparam_init(&self->pan,
		"", "", MPF_STATE | MPF_SMALL, &self->defaultpanfactor, 0, 200);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "", MPF_SLIDER | MPF_SMALL, &self->channeldefvolume);
	psy_audio_volumemachineparam_setmode(&self->slider_param, psy_audio_VOLUME_LINEAR);
	psy_audio_volumemachineparam_setrange(&self->slider_param, 0, 200);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		psy_audio_samplerchannel_level_normvalue);	
}

void psy_audio_samplerchannel_dispose(psy_audio_SamplerChannel* self)
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

void psy_audio_samplerchannel_restore(psy_audio_SamplerChannel* self)
{
	self->volume = self->channeldefvolume;
	self->panfactor = psy_audio_samplerchannel_defaultpanfactorfloat(self);
	psy_audio_samplerchannel_effectinit(self);	
}

void psy_audio_samplerchannel_newline(psy_audio_SamplerChannel* self)
{
	psy_list_free(self->effects);
	self->effects = NULL;
}

void psy_audio_samplerchannel_level_normvalue(psy_audio_SamplerChannel* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	/*if (self->index == UINTPTR_MAX) {
		psy_audio_Buffer* memory;

		memory = psy_audio_machine_buffermemory(
			psy_audio_sampler_base(self->sampler));
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

void psy_audio_samplerchannel_work(psy_audio_SamplerChannel* self,
	psy_audio_BufferContext* bc)
{
	if (self->index == UINTPTR_MAX) {
		psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->channeldefvolume);
	} else {
		psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->volume);
	}
}

// Channel Effects
void psy_audio_samplerchannel_effectinit(psy_audio_SamplerChannel* self)
{
	psy_list_free(self->effects);
	self->effects = NULL;
	// volume slide
	self->volumeslidemem = 0;
	// channel volume slide
	self->chanvolslidespeed = 0.0f;
	self->chanvolslidemem = 0;
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

void psy_audio_samplerchannel_seteffect(psy_audio_SamplerChannel* self,
	const psy_audio_PatternEvent* ev)
{
	switch (ev->cmd) {
		case SAMPLER_CMD_VOLUMESLIDE:
			psy_audio_samplerchannel_setvolumeslide(self, ev->parameter);
		break;
		case SAMPLER_CMD_SET_CHANNEL_VOLUME:
			self->volume = (psy_dsp_amp_t)((ev->parameter < 64)
			? (ev->parameter / 64.0f)
			: 1.0f);
			break;
		case SAMPLER_CMD_CHANNEL_VOLUMESLIDE:
			psy_audio_samplerchannel_setchannelvolumeslide(self, ev->parameter);
			break;		
		case SAMPLER_CMD_PANNING:
			self->panfactor = ev->parameter / (psy_dsp_amp_t) 255;		
			break;
		case SAMPLER_CMD_PANNINGSLIDE:
			psy_audio_samplerchannel_setpanningslide(self, ev->parameter);
			break;
		case SAMPLER_CMD_VIBRATO:
			psy_audio_samplerchannel_setvibrato(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F) << 2);
			break;
		case SAMPLER_CMD_FINE_VIBRATO:
			psy_audio_samplerchannel_setvibrato(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case SAMPLER_CMD_TREMOLO:
			psy_audio_samplerchannel_settremolo(self,
				((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;		
		case SAMPLER_CMD_PANBRELLO:
			psy_audio_samplerchannel_setpanbrello(self,
			((ev->parameter >> 4) & 0x0F),
				(ev->parameter & 0x0F));
			break;
		case SAMPLER_CMD_RETRIG:
			psy_audio_samplerchannel_setretrigger(self,
				ev->parameter);
			break;
		default:
			break;
	}
}

void psy_audio_samplerchannel_addeffect(psy_audio_SamplerChannel* self, int cmd)
{
	psy_list_append(&self->effects, (void*)(uintptr_t)cmd);
}

void psy_audio_samplerchannel_performfx(psy_audio_SamplerChannel* self)
{
	psy_List* p;

	for (p = self->effects; p != NULL; psy_list_next(&p)) {
		int effect = (int)psy_list_entry(p);
		
		switch (effect) {
			case SAMPLER_EFFECT_CHANNELVOLSLIDE:
				psy_audio_samplerchannel_perform_channelvolumeslide(self);
				break;
			case SAMPLER_EFFECT_PANSLIDE:				
				psy_audio_samplerchannel_performpanningslide(self);
				break;
			default:
				break;
		}
	}	
}

void psy_audio_samplerchannel_setvolumeslide(psy_audio_SamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->volumeslidemem == 0) return;
		speed = self->volumeslidemem;
	} else self->volumeslidemem = speed;
	psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_CHANNELVOLSLIDE);
}

void psy_audio_samplerchannel_setchannelvolumeslide(psy_audio_SamplerChannel* self,
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
		psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_CHANNELVOLSLIDE);
		self->chanvolslidespeed = speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_samplerchannel_perform_channelvolumeslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide down
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_CHANNELVOLSLIDE);
		self->chanvolslidespeed = -speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_samplerchannel_perform_channelvolumeslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide up
		self->chanvolslidespeed = (GETSLIDEUPVAL(speed)) / 64.0f;
		psy_audio_samplerchannel_perform_channelvolumeslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide down
		self->chanvolslidespeed = -GETSLIDEDOWNVAL(speed) / 64.0f;
		psy_audio_samplerchannel_perform_channelvolumeslide(self);
	}
}

void psy_audio_samplerchannel_perform_channelvolumeslide(psy_audio_SamplerChannel* self)
{
	self->volume += self->chanvolslidespeed;
	if (self->volume < 0.0f) {
		self->volume = 0.0f;
	} else if (self->volume > 1.0f) {
		self->volume = 1.0f;
	}
}

void psy_audio_samplerchannel_setpanningslide(psy_audio_SamplerChannel* self,
	int speed)
{
	if (speed == 0) {
		if (self->panslidemem == 0) return;
		speed = self->panslidemem;
	} else self->panslidemem = speed;

	if (ISSLIDEUP(speed)) { // Slide Left
		speed = GETSLIDEUPVAL(speed);
		psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_PANSLIDE);
		self->panslidespeed = -speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_samplerchannel_performpanningslide(self);
		}
	} else if (ISSLIDEDOWN(speed)) { // Slide Right
		speed = GETSLIDEDOWNVAL(speed);
		psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_PANSLIDE);
		self->panslidespeed = speed / 64.0f;
		if (speed == 0xF) {
			psy_audio_samplerchannel_performpanningslide(self);
		}
	} else if (ISFINESLIDEUP(speed)) { // FineSlide left
		self->panslidespeed = -(GETSLIDEUPVAL(speed)) / 64.0f;
		psy_audio_samplerchannel_performpanningslide(self);
	} else if (ISFINESLIDEDOWN(speed)) { // FineSlide right
		self->panslidespeed = GETSLIDEDOWNVAL(speed) / 64.0f;
		psy_audio_samplerchannel_performpanningslide(self);
	}
}

void psy_audio_samplerchannel_performpanningslide(psy_audio_SamplerChannel* self)
{
	self->panfactor += self->panslidespeed;
	if (self->panfactor < 0.0f) {
		self->panfactor = 0.0f;
	} else if (self->panfactor > 1.0f) {
		self->panfactor = 1.0f;
	}
}

void psy_audio_samplerchannel_setvibrato(psy_audio_SamplerChannel* self, int speed, int depth)
{	
	if (depth == 0) {
		if (self->vibratodepthmem == 0) return;
		depth = self->vibratodepthmem;
	} else self->vibratodepthmem = depth;

	if (speed == 0) {
		if (self->vibratospeedmem == 0) return;
		speed = self->vibratospeedmem;
	} else self->vibratospeedmem = speed;	
	psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_VIBRATO);
}

void psy_audio_samplerchannel_settremolo(psy_audio_SamplerChannel* self, int speed, int depth)
{
	if (depth == 0) {
		if (self->tremolodepthmem == 0) return;
		depth = self->tremolodepthmem;
	} else self->tremolodepthmem = depth;

	if (speed == 0) {
		if (self->tremolospeedmem == 0) return;
		speed = self->tremolospeedmem;
	} else self->tremolospeedmem = speed;
	psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_TREMOLO);
}

void psy_audio_samplerchannel_setpanbrello(psy_audio_SamplerChannel* self, int speed, int depth)
{
	if (depth == 0) {
		if (self->panbrellodepthmem == 0) return;
		depth = self->panbrellodepthmem;
	} else self->panbrellodepthmem = depth;

	if (speed == 0) {
		if (self->panbrellospeedmem == 0) return;
		speed = self->panbrellospeedmem;
	} else self->panbrellospeedmem = speed;
	psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_PANBRELLO);
}

void  psy_audio_samplerchannel_setretrigger(psy_audio_SamplerChannel* self, 
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
	psy_audio_samplerchannel_addeffect(self, SAMPLER_EFFECT_RETRIG);
	
}

bool  psy_audio_samplerchannel_load(psy_audio_SamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	char chan[8];
	int32_t size = 0;
	int32_t temp32;
	psyfile_read(songfile->file, chan, 4); chan[4] = '\0';
	psyfile_read(songfile->file, &size, sizeof(int32_t));
	if (strcmp(chan, "CHAN")) return FALSE;

	psyfile_read(songfile->file, &temp32, sizeof(temp32));///< (0..200)   &0x100 = Mute.
	self->channeldefvolume = temp32 / 200.f;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));//<  0..200 .  &0x100 = Surround.
	self->defaultpanfactor = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultCutoff = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	self->m_DefaultRessonance = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	// self->m_DefaultFilterType = (FilterType)temp32;
	return TRUE;
}

void psy_audio_samplerchannel_save(psy_audio_SamplerChannel* self,
	psy_audio_SongFile* songfile)
{
	int size = 5 * sizeof(int);
	psyfile_write(songfile->file, "CHAN", 4);
	psyfile_write_int32(songfile->file, size);
	psyfile_write_int32(songfile->file, (int32_t)(self->channeldefvolume * 200));
	psyfile_write_int32(songfile->file, self->defaultpanfactor);
	psyfile_write_int32(songfile->file, self->m_DefaultCutoff);
	psyfile_write_int32(songfile->file, self->m_DefaultRessonance);	
	psyfile_write_uint32(songfile->file, self->defaultfiltertype);	
}
