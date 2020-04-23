// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mixer.h"
#include "machines.h"
#include "player.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"

#include <operations.h>
#include <dsptypes.h>
#include <convert.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static void mixer_describeeditname(psy_audio_Mixer*, char* text, uintptr_t slot);

// InputLabelParam
static int inputlabelparam_type(psy_audio_InputLabelParam* self) { return MPF_INFOLABEL | MPF_SMALL; }
static int inputlabelparam_name(psy_audio_InputLabelParam*, char* text);
static int inputlabelparam_describe(psy_audio_InputLabelParam*, char* text);

static MachineParamVtable inputlabelparam_vtable;
static int inputlabelparam_vtable_initialized = 0;

static void inputlabelparam_vtable_init(psy_audio_InputLabelParam* self)
{
	if (!inputlabelparam_vtable_initialized) {
		inputlabelparam_vtable = *(self->machineparam.vtable);
		inputlabelparam_vtable.describe = (fp_machineparam_describe)inputlabelparam_describe;
		inputlabelparam_vtable.name = (fp_machineparam_name)inputlabelparam_name;
		inputlabelparam_vtable.type = (fp_machineparam_type)inputlabelparam_type;
		inputlabelparam_vtable_initialized = 1;
	}
}

void psy_audio_inputlabelparam_init(psy_audio_InputLabelParam* self,
	struct psy_audio_InputChannel* channel)
{
	psy_audio_machineparam_init(&self->machineparam);
	inputlabelparam_vtable_init(self);
	self->machineparam.vtable = &inputlabelparam_vtable;
	self->channel = channel;
}

void psy_audio_inputlabelparam_dispose(psy_audio_InputLabelParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

int inputlabelparam_name(psy_audio_InputLabelParam* self, char* text)
{
	psy_snprintf(text, 128, "Input %d", (int)self->channel->id + 1);	
	return 1;
}

int inputlabelparam_describe(psy_audio_InputLabelParam* self, char* text)
{
	mixer_describeeditname(self->channel->mixer, text, self->channel->inputslot);
	return 1;
}


// SendLabelParam
static int sendreturnlabelparam_type(psy_audio_SendReturnLabelParam* self) { return MPF_INFOLABEL | MPF_SMALL; }
static int sendreturnlabelparam_name(psy_audio_SendReturnLabelParam*, char* text);
static int sendreturnlabelparam_describe(psy_audio_SendReturnLabelParam*, char* text);

static MachineParamVtable sendreturnlabelparam_vtable;
static int sendreturnlabelparam_vtable_initialized = 0;

static void sendreturnlabelparam_vtable_init(psy_audio_SendReturnLabelParam* self)
{
	if (!sendreturnlabelparam_vtable_initialized) {
		sendreturnlabelparam_vtable = *(self->machineparam.vtable);
		sendreturnlabelparam_vtable.describe = (fp_machineparam_describe)sendreturnlabelparam_describe;
		sendreturnlabelparam_vtable.name = (fp_machineparam_name)sendreturnlabelparam_name;
		sendreturnlabelparam_vtable.type = (fp_machineparam_type)sendreturnlabelparam_type;
		sendreturnlabelparam_vtable_initialized = 1;
	}
}

void psy_audio_sendreturnlabelparam_init(psy_audio_SendReturnLabelParam* self,
	struct psy_audio_ReturnChannel* channel, bool displaysend)
{
	psy_audio_machineparam_init(&self->machineparam);
	sendreturnlabelparam_vtable_init(self);
	self->machineparam.vtable = &sendreturnlabelparam_vtable;
	self->channel = channel;
	self->displaysend = displaysend;
}

void psy_audio_sendreturnlabelparam_dispose(psy_audio_SendReturnLabelParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

int sendreturnlabelparam_name(psy_audio_SendReturnLabelParam* self, char* text)
{
	if (self->displaysend) {
		psy_snprintf(text, 128, "Send %d", (int)self->channel->id + 1);
	} else {
		psy_snprintf(text, 128, "Return %d", (int)self->channel->id + 1);
	}
	return 1;
}

int sendreturnlabelparam_describe(psy_audio_SendReturnLabelParam* self, char* text)
{
	psy_snprintf(text, 128, "%s", psy_audio_machine_editname(self->channel->fx));
	return 1;
}


// DryWetMixParam
static void drywetmixmachineparam_tweak(psy_audio_DryWetMixMachineParam*, float val);
static float drywetmixmachineparam_normvalue(psy_audio_DryWetMixMachineParam*);
static void drywetmixmachineparam_range(psy_audio_DryWetMixMachineParam*,
	intptr_t* minval, intptr_t* maxval);

static MachineParamVtable drywetmixmachineparam_vtable;
static int drywetmixmachineparam_vtable_initialized = 0;

static void drywetmixmachineparam_vtable_init(psy_audio_DryWetMixMachineParam* self)
{
	if (!drywetmixmachineparam_vtable_initialized) {
		drywetmixmachineparam_vtable = *(self->machineparam.vtable);
		drywetmixmachineparam_vtable.tweak = (fp_machineparam_tweak)drywetmixmachineparam_tweak;
		drywetmixmachineparam_vtable.normvalue = (fp_machineparam_normvalue)drywetmixmachineparam_normvalue;
		drywetmixmachineparam_vtable.range = (fp_machineparam_range)drywetmixmachineparam_range;
		drywetmixmachineparam_vtable_initialized = 1;
	}
}

void psy_audio_drywetmixmachineparam_init(psy_audio_DryWetMixMachineParam* self,
	int32_t* mute, int32_t* dryonly, int32_t* wetonly)
{
	psy_audio_machineparam_init(&self->machineparam);
	drywetmixmachineparam_vtable_init(self);
	self->machineparam.vtable = &drywetmixmachineparam_vtable;
	self->mute = mute;
	self->dryonly = dryonly;
	self->wetonly = wetonly;
}

void psy_audio_drywetmixmachineparam_dispose(psy_audio_DryWetMixMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

void drywetmixmachineparam_tweak(psy_audio_DryWetMixMachineParam* self, float val)
{
	// drywetmix. (0 normal, 1 dryonly, 2 wetonly  3 mute)
	int scaled;

	scaled = (int)(val * 3);
	switch (scaled) {
	case 0:
		*self->mute = 0;
		*self->dryonly = 0;
		*self->wetonly = 0;
		break;
	case 1:
		*self->dryonly = 1;
		*self->mute = 0;
		*self->wetonly = 0;
		break;
	case 2:
		*self->wetonly = 1;
		*self->mute = 0;
		*self->dryonly = 0;
		break;
	case 3:
		*self->mute = 1;
		*self->dryonly = 0;
		*self->wetonly = 0;
		break;
	default:
		break;
	}
}

float drywetmixmachineparam_normvalue(psy_audio_DryWetMixMachineParam* self)
{
	// drywetmix. (0 normal, 1 dryonly, 2 wetonly  3 mute)	
	if (*self->mute == 0 && *self->dryonly == 1 && *self->wetonly == 0) {
		return 1.f / 3;
	} else
		if (*self->mute == 0 && *self->dryonly == 0 && *self->wetonly == 1) {
			return 2.f / 3;
		} else
			if (*self->mute == 1 && *self->dryonly == 0 && *self->wetonly == 0) {
				return 3.f / 3;
			}
		return 0.f;
}

void drywetmixmachineparam_range(psy_audio_DryWetMixMachineParam* self,
	intptr_t* minval, intptr_t* maxval)
{
	*minval = 0;
	*maxval = 3;
}

// RouteMachineParam
static int routemachineparam_type(psy_audio_RouteMachineParam* self) { return MPF_SWITCH | MPF_SMALL; }
static void routemachineparam_tweak(psy_audio_RouteMachineParam* self, float val);
static float routemachineparam_normvalue(psy_audio_RouteMachineParam* self);
static int routemachineparam_name(psy_audio_RouteMachineParam*, char* text);

static MachineParamVtable routemachineparam_vtable;
static int routemachineparam_vtable_initialized = 0;

static void routemachineparam_vtable_init(psy_audio_RouteMachineParam* self)
{
	if (!routemachineparam_vtable_initialized) {
		routemachineparam_vtable = *(self->machineparam.vtable);
		routemachineparam_vtable.normvalue = (fp_machineparam_normvalue) routemachineparam_normvalue;
		routemachineparam_vtable.tweak = (fp_machineparam_tweak) routemachineparam_tweak;
		routemachineparam_vtable.type = (fp_machineparam_type) routemachineparam_type;
		routemachineparam_vtable.name = (fp_machineparam_name)routemachineparam_name;
		routemachineparam_vtable_initialized = 1;
	}
}

void psy_audio_routemachineparam_init(psy_audio_RouteMachineParam* self,
	psy_audio_ReturnChannel* channel, psy_audio_Mixer* mixer, uintptr_t send)
{
	psy_audio_machineparam_init(&self->machineparam);
	routemachineparam_vtable_init(self);
	self->machineparam.vtable = &routemachineparam_vtable;
	self->channel = channel;
	self->mixer = mixer;
	self->send = send;
}

void psy_audio_routemachineparam_dispose(psy_audio_RouteMachineParam* self)
{
	psy_audio_machineparam_dispose(&self->machineparam);
}

void routemachineparam_tweak(psy_audio_RouteMachineParam* self, float val)
{
	if (self->send < self->mixer->sends.count) {
		if (val > 0.f) {
			psy_table_insert(&self->channel->sendsto, self->send,
				self->channel);
		} else {
			psy_table_remove(&self->channel->sendsto, self->send);
		}
	} else
		if (self->send == self->mixer->sends.count) {
			self->channel->mastersend = val > 0.f ? TRUE : FALSE;
		}
}

float routemachineparam_normvalue(psy_audio_RouteMachineParam* self)
{
	if (self->send < self->mixer->sends.count) {
		return psy_table_exists(&self->channel->sendsto, self->send) ? 1.f : 0.f;
	} else 
	if (self->send == self->mixer->sends.count) {
		return self->channel->mastersend ? 1.f : 0.f;
	}
	return 0.f;
}

int routemachineparam_name(psy_audio_RouteMachineParam* self, char* text)
{
	psy_snprintf(text, 128, "%s",
		(self->send < self->mixer->sends.count) ? "Route" : "Master");
	return 1;
}

const psy_audio_MachineInfo* mixer_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		GENERATOR | 32 | 64,
		MACHMODE_GENERATOR,
		"Mixer"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"Mixer",
		"Psycledelics",
		"help",
		MACH_MIXER
	};
	return &macinfo;
}
// virtual prototypes
static const psy_audio_MachineInfo* info(psy_audio_Mixer* self)
{
	return mixer_info();
}
static void mixer_dispose(psy_audio_Mixer*);
static int mixer_mode(psy_audio_Mixer* self) { return MACHMODE_FX; }
static uintptr_t numinputs(psy_audio_Mixer* self) { return 2; }
static uintptr_t numoutputs(psy_audio_Mixer* self) { return 2; }
static psy_audio_Buffer* mix(psy_audio_Mixer*, uintptr_t slot, uintptr_t amount,
	psy_audio_MachineSockets*, psy_audio_Machines*, psy_audio_Player*);
static void work(psy_audio_Mixer*, psy_audio_BufferContext*);
static void loadspecific(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void postload(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void onconnected(psy_audio_Mixer*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void ondisconnected(psy_audio_Mixer*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
// Parameter
static psy_audio_MachineParam* parameter(psy_audio_Mixer*, uintptr_t param);
static psy_audio_MachineParam* tweakparameter(psy_audio_Mixer*, uintptr_t param);
static void patterntweak(psy_audio_Mixer* self, uintptr_t param, float val);
static uintptr_t numparameters(psy_audio_Mixer*);
static uintptr_t numtweakparameters(psy_audio_Mixer*);
static uintptr_t numparametercols(psy_audio_Mixer*);
static void paramcoords(psy_audio_Mixer* self, uintptr_t param, uintptr_t* col, uintptr_t* row);
static psy_dsp_amp_range_t amprange(psy_audio_Mixer* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static void mixer_describepanning(char* text, float pan);

// private methods
static psy_audio_WireSocketEntry* wiresocketentry(psy_audio_Mixer*, uintptr_t input);
static void insertinputchannels(psy_audio_Mixer*, uintptr_t num, psy_audio_Machines* machines);
static uintptr_t mastercolumn(psy_audio_Mixer* self) { return 1; }
static uintptr_t inputcolumn(psy_audio_Mixer* self)
{
	return mastercolumn(self) + 1;
}
static uintptr_t returncolumn(psy_audio_Mixer* self)
{
	return inputcolumn(self) + psy_table_size(&self->inputs);
}
static void preparemix(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void mixinputs(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount,
	psy_audio_Player*);
static void workreturns(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount,
	psy_audio_Player*);
static void workreturn(psy_audio_Mixer* self, psy_audio_Machines* machines,
	psy_audio_ReturnChannel*,
	uintptr_t amount,
	psy_audio_Player*);
static void mixreturns(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void levelmaster(psy_audio_Mixer*, uintptr_t amount);
// MasterChannel
static void masterchannel_init(psy_audio_MasterChannel*, uintptr_t inputslot, const char* name, const char* label);
static psy_audio_MasterChannel* masterchannel_allocinit(uintptr_t inputslot, const char* name, const char* label);
static void masterchannel_dispose(psy_audio_MasterChannel*);
// D/W
static void masterchannel_dw_describe(psy_audio_MasterChannel*, psy_audio_CustomMachineParam* sender, int* active, char* txt);
// Pan
static void channel_pan_describe(psy_audio_MachineParam* self, psy_audio_MachineParam* sender, int* active, char* txt);
// Slider/Level
static void masterchannel_level_normvalue(psy_audio_MasterChannel* self, psy_audio_CustomMachineParam* sender, float* rv);
// InputChannel
static void inputchannel_init(psy_audio_InputChannel*, uintptr_t id, psy_audio_Mixer*, uintptr_t inputslot);
static psy_audio_InputChannel* inputchannel_allocinit(psy_audio_Mixer* , uintptr_t id, uintptr_t inputslot);
static void inputchannel_dispose(psy_audio_InputChannel*);
// Send Vol
static void inputchannel_sendvol_tweak(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, float value);
static void inputchannel_sendvol_normvalue(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, float* rv);
static void inputchannel_sendvol_describe(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, int* active, char* txt);
// D/W
static void inputchannel_dw_describe(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, int* active, char* txt);
// Gain
static void inputchannel_gain_tweak(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, float value);
static void inputchannel_gain_normvalue(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, float* rv);
static void inputchannel_gain_describe(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, int* active, char* txt);
// Slider/Level
static void inputchannel_level_normvalue(psy_audio_InputChannel*, psy_audio_CustomMachineParam* sender, float* rv);
// ReturnChannel
static void returnchannel_init(psy_audio_ReturnChannel*, struct psy_audio_Mixer* mixer, uintptr_t id, uintptr_t fxslot, const char* name, const char* label);
static psy_audio_ReturnChannel* returnchannel_allocinit(uintptr_t id, uintptr_t fxslot, struct psy_audio_Mixer* mixer, const char* name, const char* label);
static void returnchannel_dispose(psy_audio_ReturnChannel*);
// Slider/Level
static void returnchannel_level_normvalue(psy_audio_ReturnChannel*, psy_audio_CustomMachineParam* sender, float* rv);

void masterchannel_init(psy_audio_MasterChannel* self, uintptr_t inputslot, const char* name, const char* label)
{
	self->inputslot = inputslot;
	self->drymix = 1.0f;
	self->dryonly = 0;
	self->mute = 0;
	self->panning = 0.5f;
	psy_table_init(&self->sendvols);
	self->volume = 1.0f;
	self->gain = 1.f;
	self->wetonly = 0;
	self->volumedisplay = 0.f;
	psy_audio_infomachineparam_init(&self->info_param, name, label, MPF_SMALL);
	psy_audio_floatmachineparam_init(&self->dw_param,
		"D/W", "D/W", MPF_STATE | MPF_SMALL, &self->drymix, 0, 0x1000);	
	psy_signal_connect(&self->dw_param.machineparam.signal_describe, self, masterchannel_dw_describe);
	psy_audio_gainmachineparam_init(&self->gain_param,
		"Gain", "Gain", MPF_STATE | MPF_SMALL, &self->gain, 0, 0x400);
	psy_audio_floatmachineparam_init(&self->pan_param,
		"Pan", "Pan", MPF_STATE | MPF_SMALL, &self->panning, 0, 0x100);
	psy_signal_connect(&self->pan_param.machineparam.signal_describe, self, channel_pan_describe);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "Volume", MPF_SLIDER | MPF_SMALL, &self->volume);	
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		masterchannel_level_normvalue);
	psy_audio_intmachineparam_init(&self->solo_param,
		"S", "S", MPF_SLIDERCHECK | MPF_SMALL, NULL, 0, 1);
	psy_audio_intmachineparam_init(&self->mute_param,
		"M", "M", MPF_SLIDERCHECK | MPF_SMALL, &self->mute, 0, 1);
	psy_audio_intmachineparam_init(&self->dryonly_param,
		"D", "D", MPF_SLIDERCHECK | MPF_SMALL, &self->dryonly, 0, 1);
	psy_audio_intmachineparam_init(&self->wetonly_param,
		"W", "W", MPF_SLIDERCHECK | MPF_SMALL, &self->wetonly, 0, 1);
	psy_audio_drywetmixmachineparam_init(&self->drywetmix_param, &self->mute,
		&self->dryonly, &self->wetonly);
}

void masterchannel_dispose(psy_audio_MasterChannel* self)
{
	psy_audio_infomachineparam_dispose(&self->info_param);
	psy_audio_floatmachineparam_dispose(&self->dw_param);
	psy_audio_gainmachineparam_dispose(&self->gain_param);
	psy_audio_floatmachineparam_dispose(&self->pan_param);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
	psy_audio_intmachineparam_dispose(&self->solo_param);
	psy_audio_intmachineparam_dispose(&self->mute_param);		
	psy_audio_intmachineparam_dispose(&self->dryonly_param);
	psy_audio_intmachineparam_dispose(&self->wetonly_param);
	psy_audio_drywetmixmachineparam_dispose(&self->drywetmix_param);
	psy_table_dispose(&self->sendvols);
}

psy_audio_MasterChannel* masterchannel_allocinit(uintptr_t inputslot, const char* name, const char* label)
{
	psy_audio_MasterChannel* rv;

	rv = (psy_audio_MasterChannel*) malloc(sizeof(psy_audio_MasterChannel));
	if (rv) {
		masterchannel_init(rv, inputslot, name, label);
	}
	return rv;
}

void masterchannel_dw_describe(psy_audio_MasterChannel* self, psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	psy_snprintf(txt, 20, "%d%%", (int)(psy_audio_machineparam_normvalue(
		psy_audio_custommachineparam_base(sender)) * 100));
	*active = 1;
}

void channel_pan_describe(psy_audio_MachineParam* self, psy_audio_MachineParam* sender, int* active, char* txt)
{
	mixer_describepanning(txt, psy_audio_machineparam_normvalue(sender));
	*active = 1;
}

void masterchannel_level_normvalue(psy_audio_MasterChannel* self, psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = (self->buffer) ? self->buffer->volumedisplay : 0.f;
}

// Mixer Channel
void inputchannel_init(psy_audio_InputChannel* self, uintptr_t id, psy_audio_Mixer* mixer,
	uintptr_t inputslot)
{
	self->id = id;
	self->mixer = mixer;
	self->inputslot = inputslot;
	self->drymix = 1.0f;
	self->dryonly = 0;
	self->mute = 0;
	self->panning = 0.5f;
	psy_table_init(&self->sendvols);
	self->volume = 1.0f;
	self->gain = 1.f;
	self->wetonly = 0;
	self->volumedisplay = 0.f;
	self->buffer = 0;
	psy_audio_inputlabelparam_init(&self->info_param, self);
	psy_audio_floatmachineparam_init(&self->mix_param,
		"Mix", "Mix", MPF_STATE | MPF_SMALL, &self->drymix, 0, 0xFF);
	psy_signal_connect(&self->mix_param.machineparam.signal_describe, self,
		inputchannel_dw_describe);
	psy_audio_custommachineparam_init(&self->sendvol_param,
		"Send", "Send", MPF_STATE | MPF_SMALL, 0, 0xFF);
	psy_signal_connect(&self->sendvol_param.machineparam.signal_tweak, self,
		inputchannel_sendvol_tweak);
	psy_signal_connect(&self->sendvol_param.machineparam.signal_normvalue, self,
		inputchannel_sendvol_normvalue);
	psy_signal_connect(&self->sendvol_param.machineparam.signal_describe, self,
		inputchannel_sendvol_describe);
	psy_audio_custommachineparam_init(&self->gain_param,
		"Gain", "Gain", MPF_STATE | MPF_SMALL, 0, 0x400);
	self->gain_param.index = id;
	psy_signal_connect(&self->gain_param.machineparam.signal_tweak, self,
		inputchannel_gain_tweak);
	psy_signal_connect(&self->gain_param.machineparam.signal_normvalue, self,
		inputchannel_gain_normvalue);
	psy_signal_connect(&self->gain_param.machineparam.signal_describe, self,
		inputchannel_gain_describe);
	psy_audio_floatmachineparam_init(&self->pan_param,
		"Pan", "Pan", MPF_STATE | MPF_SMALL, &self->panning, 0, 0x100);
	psy_signal_connect(&self->pan_param.machineparam.signal_describe,
		self, channel_pan_describe);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "Volume", MPF_SLIDER | MPF_SMALL, &self->volume);	
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		inputchannel_level_normvalue);
	psy_audio_intmachineparam_init(&self->solo_param,
		"S", "S", MPF_SLIDERCHECK | MPF_SMALL, NULL, 0, 1);
	psy_audio_intmachineparam_init(&self->mute_param,
		"M", "M", MPF_SLIDERCHECK | MPF_SMALL, &self->mute, 0, 1);
	psy_audio_intmachineparam_init(&self->dryonly_param,
		"D", "D", MPF_SLIDERCHECK | MPF_SMALL, &self->dryonly, 0, 1);
	psy_audio_intmachineparam_init(&self->wetonly_param,
		"W", "W", MPF_SLIDERCHECK | MPF_SMALL, &self->wetonly, 0, 1);
	psy_audio_drywetmixmachineparam_init(&self->drywetmix_param, &self->mute,
		&self->dryonly, &self->wetonly);	
}

void inputchannel_dispose(psy_audio_InputChannel* self)
{
	psy_audio_inputlabelparam_dispose(&self->info_param);
	psy_audio_custommachineparam_dispose(&self->sendvol_param);
	psy_audio_floatmachineparam_dispose(&self->mix_param);
	psy_audio_custommachineparam_dispose(&self->gain_param);
	psy_audio_floatmachineparam_dispose(&self->pan_param);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
	psy_audio_intmachineparam_dispose(&self->solo_param);
	psy_audio_intmachineparam_dispose(&self->mute_param);
	psy_audio_intmachineparam_dispose(&self->dryonly_param);
	psy_audio_intmachineparam_dispose(&self->wetonly_param);
	psy_audio_drywetmixmachineparam_dispose(&self->drywetmix_param);
	psy_table_dispose(&self->sendvols);
}

psy_audio_InputChannel* inputchannel_allocinit(psy_audio_Mixer* mixer, uintptr_t id,
	uintptr_t inputslot)
{
	psy_audio_InputChannel* rv;

	rv = (psy_audio_InputChannel*) malloc(sizeof(psy_audio_InputChannel));
	if (rv) {
		inputchannel_init(rv, id, mixer, inputslot);
	}
	return rv;
}

void inputchannel_sendvol_tweak(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float value)
{
	int scaled;

	scaled = (int)(value * 0xFF);
	psy_table_insert(&self->sendvols, sender->row, (void*)(uintptr_t) scaled);
}

void inputchannel_sendvol_normvalue(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = (uintptr_t)psy_table_at(&self->sendvols,
		sender->row) / (float)0xFF;
}

void inputchannel_sendvol_describe(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	psy_dsp_amp_t sendvol;

	sendvol = psy_audio_machineparam_normvalue(
		psy_audio_custommachineparam_base(sender));
	if (sendvol == 0.0f) {
		strcpy(txt, "Off");
	} else {
		sprintf(txt, "%.0f%%", sendvol * 100.0f);
	}
	*active = 1;
}

void inputchannel_dw_describe(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	psy_snprintf(txt, 20, "%d%%", (int)(self->drymix * 100));
	*active = 1;
}

void inputchannel_gain_tweak(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float value)
{
	psy_audio_WireSocketEntry* input_entry;

	input_entry = wiresocketentry(self->mixer, sender->index);
	if (input_entry) {
		input_entry->volume = value * value * 4.f;
	}
}

void inputchannel_gain_normvalue(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	psy_audio_WireSocketEntry* input_entry;

	input_entry = wiresocketentry(self->mixer, sender->index);
	if (input_entry) {
		*rv = (float)sqrt(input_entry->volume) * 0.5f;
	} else {
		*rv = 0.f;
	}
}

void inputchannel_gain_describe(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	psy_audio_WireSocketEntry* input_entry;

	input_entry = wiresocketentry(self->mixer, sender->index);
	if (input_entry) {
		float db;

		db = (psy_dsp_amp_t) (20 * log10(input_entry->volume));
		psy_snprintf(txt, 10, "%.2f dB", db);
		*active = 1;
	} else {
		*active = 0;
	}	
}

void inputchannel_level_normvalue(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = (self->buffer) ? self->buffer->volumedisplay : 0.f;
}

void returnchannel_init(psy_audio_ReturnChannel* self,
	struct psy_audio_Mixer* mixer,
	uintptr_t id,
	uintptr_t fxslot,
	const char* name, const char* label)
{
	self->id = id;
	self->fxslot = fxslot;
	self->mute = 0;
	self->panning = 0.5f;
	self->volume = 1.f;
	self->mastersend = 1;
	psy_table_init(&self->sendsto);	
	psy_audio_sendreturnlabelparam_init(&self->sendlabel_param, self, TRUE);
	psy_audio_sendreturnlabelparam_init(&self->returnlabel_param, self, FALSE);
	psy_audio_custommachineparam_init(&self->send_param,
		"Send", label, MPF_INFOLABEL | MPF_SMALL, 0, 100);
	psy_audio_custommachineparam_init(&self->info_param,
		name, label, MPF_INFOLABEL | MPF_SMALL, 0, 100);
	psy_audio_floatmachineparam_init(&self->pan_param,
		"Pan", "Pan", MPF_STATE | MPF_SMALL, &self->panning, 0, 100);
	psy_signal_connect(&self->pan_param.machineparam.signal_describe, self,
		channel_pan_describe);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "Volume", MPF_SLIDER | MPF_SMALL, &self->volume);	
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		returnchannel_level_normvalue);
	psy_audio_intmachineparam_init(&self->solo_param,
		"S", "S", MPF_SLIDERCHECK | MPF_SMALL, NULL, 0, 1);
	psy_audio_intmachineparam_init(&self->mute_param,
		"M", "M", MPF_SLIDERCHECK | MPF_SMALL, &self->mute, 0, 1);
	psy_audio_routemachineparam_init(&self->route_param, self, mixer, 0);
}

void returnchannel_dispose(psy_audio_ReturnChannel* self)
{
	psy_audio_sendreturnlabelparam_dispose(&self->sendlabel_param);
	psy_audio_sendreturnlabelparam_dispose(&self->returnlabel_param);
	psy_audio_custommachineparam_dispose(&self->info_param);
	psy_audio_floatmachineparam_dispose(&self->pan_param);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
	psy_audio_intmachineparam_dispose(&self->solo_param);
	psy_audio_intmachineparam_dispose(&self->mute_param);
	psy_audio_routemachineparam_dispose(&self->route_param);
	psy_table_dispose(&self->sendsto);
}

psy_audio_ReturnChannel* returnchannel_allocinit(uintptr_t id, uintptr_t fxslot,
	struct psy_audio_Mixer* mixer,
	const char* name, const char* label)
{
	psy_audio_ReturnChannel* rv;

	rv = (psy_audio_ReturnChannel*)malloc(sizeof(psy_audio_ReturnChannel));
	if (rv) {
		returnchannel_init(rv, mixer, id, fxslot, name, label);
	}
	return rv;
}

void returnchannel_slider_tweak(psy_audio_ReturnChannel* self,
	psy_audio_CustomMachineParam* sender, float value)
{
	self->volume = value * value;
}

void returnchannel_slider_normvalue(psy_audio_ReturnChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = (float)sqrt(self->volume);
}

void returnchannel_level_normvalue(psy_audio_ReturnChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = (self->buffer) ? self->buffer->volumedisplay : 0.f;
}

// Mixer

static MachineVtable vtable;
static int vtable_initialized = 0;
static fp_machine_work work_super = 0;

static void vtable_init(psy_audio_Mixer* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		work_super = vtable.work;
		vtable.info = (fp_machine_info)info;
		vtable.numinputs = (fp_machine_numinputs)numinputs;
		vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		vtable.dispose = (fp_machine_dispose)mixer_dispose;
		vtable.mode = (fp_machine_mode)mixer_mode;
		vtable.mix = (fp_machine_mix)mix;
		vtable.work = (fp_machine_work)work;
		vtable.parameter = (fp_machine_parameter) parameter;
		vtable.tweakparameter = (fp_machine_parameter) tweakparameter;
		vtable.numparameters = (fp_machine_numparameters)numparameters;
		vtable.numtweakparameters = (fp_machine_numtweakparameters)numtweakparameters;
		vtable.numparametercols = (fp_machine_numparametercols)numparametercols;
		vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		vtable.savespecific = (fp_machine_savespecific)savespecific;
		vtable.postload = (fp_machine_postload)postload;
		vtable.amprange = (fp_machine_amprange)amprange;
		vtable_initialized = 1;
	}
}

void mixer_init(psy_audio_Mixer* self, MachineCallback callback)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	psy_audio_Machines* machines;

	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	psy_table_init(&self->inputs);
	psy_table_init(&self->sends);
	psy_table_init(&self->returns);
	machines = psy_audio_machine_machines(base);
	psy_signal_connect(&machines->connections.signal_connected, self,
		onconnected);
	psy_signal_connect(&machines->connections.signal_disconnected, self,
		ondisconnected);
	masterchannel_init(&self->master, 0, "Master Out", "");
	self->mastervolumedisplay = 0;
	psy_audio_machine_seteditname(base, "Mixer");
	psy_audio_custommachineparam_init(&self->blank_param, "", "", MPF_NULL | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->ignore_param, "-", "-", MPF_IGNORE | MPF_SMALL, 0, 0);
	//psy_audio_custommachineparam_init(&self->route_param, "Route", "Route", MPF_SWITCH | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->routemaster_param, "Master", "Master", MPF_SWITCH | MPF_SMALL, 0, 0);
}

void mixer_dispose(psy_audio_Mixer* self)
{
	{ // dispose inputs
		psy_TableIterator it;

		for (it = psy_table_begin(&self->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_InputChannel* channel;
			psy_audio_Machine* machine;

			channel = (psy_audio_InputChannel*)psy_tableiterator_value(&it);
			machine = machines_at(psy_audio_machine_machines(
				&self->custommachine.machine),
				channel->inputslot);
			inputchannel_dispose(channel);
			free(channel);
		}
		psy_table_dispose(&self->inputs);
	}
	psy_table_dispose(&self->sends);
	{ // dispose returns
		psy_TableIterator it;

		for (it = psy_table_begin(&self->returns);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
			psy_audio_ReturnChannel* channel;

			channel = (psy_audio_ReturnChannel*)psy_tableiterator_value(&it);
			returnchannel_dispose(channel);
			free(channel);
		}
		psy_table_dispose(&self->returns);
	}
	psy_audio_custommachineparam_dispose(&self->blank_param);
	psy_audio_custommachineparam_dispose(&self->ignore_param);
	//psy_audio_custommachineparam_dispose(&self->route_param);
	psy_audio_custommachineparam_dispose(&self->routemaster_param);	
	custommachine_dispose(&self->custommachine);	
}

psy_audio_Buffer* mix(psy_audio_Mixer* self, uintptr_t slot, uintptr_t amount,
	psy_audio_MachineSockets* connected_machine_sockets,
	psy_audio_Machines* machines,
	psy_audio_Player* player)
{
	preparemix(self, machines, amount);
	mixinputs(self, machines, amount, player);
	workreturns(self, machines, amount, player);
	mixreturns(self, machines, amount);	
	levelmaster(self, amount);
	return self->master.buffer;
}

void work(psy_audio_Mixer* self, psy_audio_BufferContext* bc)
{
	work_super(&self->custommachine.machine, bc);
	self->mastervolumedisplay = psy_audio_buffercontext_volumedisplay(bc);
}

void preparemix(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{
	psy_TableIterator iter;

	self->master.buffer = machines_outputs(machines,
		psy_audio_machine_slot(psy_audio_mixer_base(self)));
	psy_audio_buffer_clearsamples(self->master.buffer, amount);
	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
		psy_tableiterator_inc(&iter)) {
		psy_audio_ReturnChannel* channel;

		channel = (psy_audio_ReturnChannel*)psy_tableiterator_value(&iter);
		channel->buffer = machines_outputs(machines, channel->fxslot);
		channel->fx = machines_at(machines, channel->fxslot);
		if (channel->buffer) {
			psy_audio_buffer_clearsamples(channel->buffer, amount);
		}
	}
}

void mixinputs(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount, psy_audio_Player* player)
{
	psy_TableIterator input_iter;
	psy_Table clearsend;

	psy_table_init(&clearsend);
	for (input_iter = psy_table_begin(&self->inputs);
			!psy_tableiterator_equal(&input_iter, psy_table_end());
			psy_tableiterator_inc(&input_iter)) {
		psy_audio_InputChannel* input;
		psy_TableIterator sendvol_iter;

		input = (psy_audio_InputChannel*)psy_tableiterator_value(&input_iter);
		if (input) {
			psy_audio_WireSocketEntry* input_entry;
			psy_dsp_amp_t wirevol = 1.f;

			input->buffer = machines_outputs(machines, input->inputslot);
			input_entry = wiresocketentry(self, psy_tableiterator_key(&input_iter));
			if (input_entry) {
				wirevol = input_entry->volume;
			}
			psy_audio_buffer_addsamples(self->master.buffer, input->buffer,
				amount, input->volume * input->drymix * wirevol);
			for (sendvol_iter = psy_table_begin(&input->sendvols);
				!psy_tableiterator_equal(&sendvol_iter, psy_table_end());
				psy_tableiterator_inc(&sendvol_iter)) {
				uintptr_t sendvol;
				uintptr_t sendchannelslot;
				uintptr_t sendmachineslot;
				MachineList* chainpath;

				sendvol = (uintptr_t)psy_tableiterator_value(&sendvol_iter);
				if (sendvol >= 0) {
					sendchannelslot = psy_tableiterator_key(&sendvol_iter);
					sendmachineslot = (uintptr_t)psy_table_at(&self->sends, sendchannelslot);
					chainpath = compute_path(machines, sendmachineslot, FALSE);
					if (chainpath) {
						uintptr_t slot;
						psy_audio_Buffer* sendbuffer;

						// mix inputs to start of fx chain
						slot = (uintptr_t)chainpath->entry;
						sendbuffer = machines_outputs(machines, slot);
						if (sendvol > 0) {
							if (!psy_table_exists(&clearsend, sendchannelslot)) {
								psy_audio_buffer_clearsamples(sendbuffer, amount);
								psy_table_insert(&clearsend, sendchannelslot, NULL);
							}
							psy_audio_buffer_addsamples(sendbuffer, input->buffer,
								amount, (sendvol / (psy_dsp_amp_t)0xFF));
							sendbuffer->preventmixclear = TRUE;
						}
						psy_list_free(chainpath);
					}
				}
			}
		}
	}
	psy_table_dispose(&clearsend);
}

void mixreturns(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{
	psy_TableIterator iter;

	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
		psy_tableiterator_inc(&iter)) {
		psy_audio_ReturnChannel* channel;

		channel = psy_tableiterator_value(&iter);
		if (channel && channel->mastersend) {
			psy_audio_buffer_addsamples(self->master.buffer, channel->buffer,
				amount, channel->volume);
		}
	}
}

void workreturns(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount, psy_audio_Player* player)
{
	psy_TableIterator iter;

	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
		psy_tableiterator_inc(&iter)) {
		psy_audio_ReturnChannel* channel;

		channel = psy_tableiterator_value(&iter);
		if (channel && channel->fx && channel->buffer) {
			psy_List* events = 0;
			psy_TableIterator sendsto_iter;

			workreturn(self, machines, channel, amount, player);
			//	buffer_pan(fxbuffer, fx->panning(fx), amount);
			//	buffer_pan(fxbuffer, channel->panning, amount);
			if (channel->sendsto.count >= 0) {
				for (sendsto_iter = psy_table_begin(&channel->sendsto);
					!psy_tableiterator_equal(&sendsto_iter, psy_table_end());
					psy_tableiterator_inc(&sendsto_iter)) {
					psy_audio_ReturnChannel* sendto;

					sendto = (psy_audio_ReturnChannel*)psy_tableiterator_value(&sendsto_iter);
					if (sendto) {
						if (sendto->buffer) {
							psy_audio_buffer_addsamples(sendto->buffer, channel->buffer, amount,
								channel->volume);
						}
					}
				}
			}
		}
	}
}

void workreturn(psy_audio_Mixer* self, psy_audio_Machines* machines,
	psy_audio_ReturnChannel* returnchannel,
	uintptr_t amount,
	psy_audio_Player* player)
{
	MachineList* path;

	path = compute_path(machines, returnchannel->fxslot, FALSE);
	if (path) {
		uintptr_t slot;
				
		for (; path != 0; path = path->next) {
			slot = (uintptr_t) path->entry;
			if (slot == NOMACHINE_INDEX) {
				// delimits the machines that could be processed parallel
				// todo: add thread functions
				continue;
			}
			player_workmachine(player, amount, slot);
		}
		psy_list_free(path);
	}
	returnchannel->buffer = machines_outputs(machines, returnchannel->fxslot);
}

void levelmaster(psy_audio_Mixer* self, uintptr_t amount)
{
	dsp.mul(self->master.buffer->samples[0], amount, self->master.volume);
	dsp.mul(self->master.buffer->samples[1], amount, self->master.volume);
}

void onconnected(psy_audio_Mixer* self, psy_audio_Connections* connections,
	uintptr_t outputslot, uintptr_t inputslot)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	if (inputslot == psy_audio_machine_slot(psy_audio_mixer_base(self))) {
		if (outputslot != psy_audio_machine_slot(psy_audio_mixer_base(self))) {
			psy_audio_Machine* machine;
			psy_audio_Machines* machines;

			machines = psy_audio_machine_machines(base);
			machine = machines_at(machines, outputslot);
			if (psy_audio_machine_mode(machine) == MACHMODE_GENERATOR ||
					!machines_isconnectasmixersend(machines)) {
				psy_audio_InputChannel* input;
				
				input = inputchannel_allocinit(self, self->inputs.count, outputslot);
				psy_table_insert(&self->inputs, self->inputs.count, (void*) input);
			}
			else {
				MachineList* path;
				char title[128];
				char label[128];
				
				psy_table_insert(&self->sends, self->sends.count, (void*)outputslot);
				psy_snprintf(title, 128, "Return %u", (unsigned int)self->returns.count + 1);
				mixer_describeeditname(self, label, outputslot);
				psy_table_insert(&self->returns, self->returns.count,
					returnchannel_allocinit(self->returns.count, outputslot, self, title, label));
				path = compute_path(machines, outputslot, FALSE);
				if (path) {
					// work fx chain
					for (; path != 0; path = path->next) {
						uintptr_t slot;

						slot = (size_t)path->entry;
						if (slot == NOMACHINE_INDEX) {
							// delimits the machines that could be processed parallel
							// todo: add thread functions
							continue;
						}
						machines_addmixersend(machines, slot);
					}
					psy_list_free(path);
				}
			}
		}
	}
}

void ondisconnected(psy_audio_Mixer* self, psy_audio_Connections* connections,
	uintptr_t outputslot, uintptr_t inputslot)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	if (inputslot == psy_audio_machine_slot(psy_audio_mixer_base(self))) {
		psy_audio_Machine* machine;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(base);
		machine = machines_at(machines, outputslot);
		if (psy_audio_machine_mode(machine) == MACHMODE_GENERATOR) {
			psy_TableIterator it;
			int c = 0;

			for (it = psy_table_begin(&self->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it), ++c) {
				psy_audio_InputChannel* channel;

				channel = (psy_audio_InputChannel*)psy_tableiterator_value(&it);
				if (channel->inputslot == outputslot) {
					psy_table_remove(&self->inputs, c);
					inputchannel_dispose(channel);
					free(channel);
					break;
				}
			}
		}
		else {
			psy_TableIterator it;

			for (it = psy_table_begin(&self->returns);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_TableIterator sendsto_iter;
				psy_audio_ReturnChannel* channel;

				channel = psy_table_at(&self->returns, psy_tableiterator_key(&it));
				sendsto_iter = psy_table_begin(&channel->sendsto);
				while (!psy_tableiterator_equal(&sendsto_iter, psy_table_end())) {
					psy_audio_ReturnChannel* sendto;
					uintptr_t key;

					key = psy_tableiterator_key(&sendsto_iter);
					sendto = (psy_audio_ReturnChannel*)psy_tableiterator_value(&sendsto_iter);
					psy_tableiterator_inc(&sendsto_iter);
					if (sendto) {
						if (sendto->fxslot == outputslot) {
							psy_table_remove(&channel->sendsto, key);
						}
					}
				}
			}
			for (it = psy_table_begin(&self->sends);
				!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
				uintptr_t sendslot;
				uintptr_t c;

				c = psy_tableiterator_key(&it);
				sendslot = (uintptr_t)psy_tableiterator_value(&it);
				if (sendslot == outputslot) {
					psy_audio_ReturnChannel* returnchannel;
					psy_table_remove(&self->sends, c);
					returnchannel = psy_table_at(&self->returns, c);
					psy_table_remove(&self->returns, c);
					if (returnchannel) {
						MachineList* path;

						path = compute_path(machines, returnchannel->fxslot, FALSE);
						if (path) {							
							for (; path != 0; path = path->next) {
								uintptr_t slot;

								slot = (size_t)path->entry;
								if (slot == NOMACHINE_INDEX) {
									// delimits the machines that could be processed parallel
									// todo: add thread functions
									continue;
								}
								machines_removemixersend(machines, slot);
							}
							psy_list_free(path);
						}
						returnchannel_dispose(returnchannel);
						free(returnchannel);
					}
					break;
				}
			}
		}
	}
}

void patterntweak(psy_audio_Mixer* self, uintptr_t numparam, float value)
{
	/*psy_audio_Machine* base = (psy_audio_Machine*)self;
	uintptr_t channelindex = numparam / 16;
	uintptr_t param = numparam % 16;

	if (channelindex == 0) {
		if (param == 0) {
			if (value >= 0x1000) {
				self->master.volume = 1.0f;
			}
			else
				if (value == 0) {
					self->master.volume = 0.0f;
				}
				else {
					psy_dsp_amp_t dbs = (value / 42.67f) - 96.0f;
					self->master.volume = psy_dsp_convert_db_to_amp(dbs);
				}
		}
		else
			if (param == 13) {
				self->master.drymix = (value >= 0x100)
					? 1.0f
					: ((machine_parametervalue_scaled(&self->custommachine.machine,
						param, value) & 0xFF) / 256.f);
			}
			else
				if (param == 14) {
					self->master.gain = (value >= 1024)
						? 4.0f
						: ((machine_parametervalue_scaled(&self->custommachine.machine,
							param, value) & 0x3FF) / 256.0f);
				}
				else {
					psy_audio_machine_setpanning(base, (machine_parametervalue_scaled(&self->custommachine.machine,
						param, value) >> 1) / 256.f);
				}
	}
	else
		// Inputs
		if (channelindex <= self->inputs.count) {
			psy_audio_InputChannel* channel;

			channel = psy_table_at(&self->inputs, channelindex - 1);
			if (channel) {
				if (param == 0) {
					channel->drymix = (value == 256)
						? 1.0f
						: ((machine_parametervalue_scaled(
							&self->custommachine.machine,
							param, value) & 0xFF) / 256.0f);
				}
				else
					if (param <= 12) {
						if (param - 1 < self->sends.count) {
							psy_table_insert(&channel->sendvols, param - 1,
								(void*)(uintptr_t)machine_parametervalue_scaled(&self->custommachine.machine, param,
									(machine_parametervalue_scaled(&self->custommachine.machine,
										param, value) == 256) ? 1.0f :
								((machine_parametervalue_scaled(&self->custommachine.machine,
									param, value) & 0xFF) / 256.0f))
								);
						}
					}
					else
						if (param == 13) {
							channel->mute = value == 3;
							channel->wetonly = value == 2;
							channel->dryonly = value == 1;
						}
						else
							if (param == 14) {
								psy_audio_WireSocketEntry* input_entry;

								input_entry = wiresocketentry(self, channelindex - 1);
								if (input_entry) {
									float val = (machine_parametervalue_scaled(&self->custommachine.machine,
										param, value) >= 1024) ? 4.0f : ((machine_parametervalue_scaled(&self->custommachine.machine,
											param, value) & 0x3FF) / 256.0f);
									input_entry->volume = val;
								}
							}
							else {
								if (value >= 0x1000) {
									channel->volume = 1.0f;
								}
								else
									if (value == 0) {
										channel->volume = 0.0f;
									}
									else {
										psy_dsp_amp_t dbs = (value / 42.67f) - 96.0f;
										channel->volume = psy_dsp_convert_db_to_amp(dbs);
									}
							}
			}
		}*/
}

void mixer_describeeditname(psy_audio_Mixer* self, char* text, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = machines_at(psy_audio_machine_machines(&self->custommachine.machine),
		slot);
	if (machine) {
		psy_snprintf(text, 128, "%s",
			psy_audio_machine_editname(machine));
	}
	else {
		psy_snprintf(text, 128, "%s", "");
	}
}

void mixer_describepanning(char* text, float pan)
{
	if (pan == 0.f) {
		strcpy(text, "left");
	} else
	if (pan == 1.f) {
		strcpy(text, "right");
	} else
	if (pan == 0.5f) {
		strcpy(text, "center");
	} else {
		sprintf(text, "%.0f%%", (float)((int)(pan * 100.f)));
	}
}

uintptr_t numparameters(psy_audio_Mixer* self)
{
	return (uintptr_t)(numparametercols(self) * (10 + psy_table_size(&self->sends)));
}

static uintptr_t numtweakparameters(psy_audio_Mixer* self)
{
	return psy_audio_machine_numparameters(psy_audio_mixer_base(self));
}

uintptr_t numparametercols(psy_audio_Mixer* self)
{
	return returncolumn(self) + self->returns.count;
}

psy_audio_MachineParam* parameter(psy_audio_Mixer* self, uintptr_t param)
{
	uintptr_t col;
	uintptr_t row;

	paramcoords(self, param, &col, &row);
	if (col < mastercolumn(self)) {
		if (row > 0 && row <= psy_table_size(&self->sends)) {
			psy_audio_ReturnChannel* channel;

			channel = (psy_audio_ReturnChannel*)psy_table_at(&self->returns, row - 1);
			if (channel) {
				return psy_audio_sendreturnlabelparam_base(&channel->sendlabel_param);
			}
		} else
		if (row > self->sends.count + 3 && row < self->sends.count + 9) {
			return &self->ignore_param.machineparam;
		} else {
			return &self->blank_param.machineparam;
		}		
	} else
	if (col == mastercolumn(self)) {
		psy_audio_MasterChannel* channel;
		uintptr_t index;

		index = col - inputcolumn(self);
		channel = &self->master;
		if (row == 0) {
			return &channel->info_param.machineparam;
		} else
		if (row == self->sends.count + 1) {
			return &channel->dw_param.machineparam;
		} else
		if (row == self->sends.count + 2) {
			return &channel->gain_param.machineparam;
		} else
		if (row == self->sends.count + 3) {
			return &channel->pan_param.machineparam;
		} else
		if (row == self->sends.count + 4) {
			return &channel->slider_param.machineparam;
		} else
		if (row == self->sends.count + 5) {
			return &channel->level_param.machineparam;
		} else
		if (row == self->sends.count + 6) {
			return &channel->solo_param.machineparam;
		} else
		if (row == self->sends.count + 7) {
			return &channel->mute_param.machineparam;
		} else
		if (row == self->sends.count + 8) {
			return &channel->dryonly_param.machineparam;
		} else
		if (row == self->sends.count + 9) {
			return &channel->wetonly_param.machineparam;
		} else
		if (row == self->sends.count + 10) {
			return &self->ignore_param.machineparam;
		} else {
			return &self->blank_param.machineparam;
		}
	} else
	if (col < returncolumn(self)) {
		psy_audio_InputChannel* channel;
		uintptr_t index;

		index = col - inputcolumn(self);
		channel = (psy_audio_InputChannel*)psy_table_at(&self->inputs, index);
		if (row == 0) {			
			return &channel->info_param.machineparam;
		} else
		if (row > 0 && row <= psy_table_size(&self->sends)) {
			channel->sendvol_param.row = row - 1;
			return &channel->sendvol_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 1) {
			return &channel->mix_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 2) {
			return &channel->gain_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 3) {
			return &channel->pan_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 4) {
			return &channel->slider_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 5) {
			return &channel->level_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 6) {
			return &channel->solo_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 7) {
			return &channel->mute_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 8) {
			return &channel->dryonly_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 9) {
			return &channel->wetonly_param.machineparam;
		} else
		if (row == psy_table_size(&self->sends) + 10) {
			return &self->ignore_param.machineparam;
		} else {
			return &self->blank_param.machineparam;
		}
	} else
	if (col < returncolumn(self) + psy_table_size(&self->returns)) {
		psy_audio_ReturnChannel* channel;
		uintptr_t index;

		index = col - returncolumn(self);
		channel = (psy_audio_ReturnChannel*)psy_table_at(&self->returns, index);
		if (channel) {
			if (row == 0) {
				return psy_audio_sendreturnlabelparam_base(&channel->returnlabel_param);
			} else
			if (row > 1 && row < self->sends.count + 1) {
				if (index <= row - 2) {
					channel->route_param.send = row - 1;
					return &channel->route_param.machineparam;
				} else {
					return &self->blank_param.machineparam;
				}
			} else
			if (row == self->sends.count + 1) {
				channel->route_param.send = row - 1;
				return &channel->route_param.machineparam;
			} else
			if (row == self->sends.count + 3) {
				return &channel->pan_param.machineparam;
			} else
			if (row == self->sends.count + 4) {
				return &channel->slider_param.machineparam;
			} else
			if (row == self->sends.count + 5) {
				return &channel->level_param.machineparam;
			} else
			if (row == self->sends.count + 6) {
				return &channel->solo_param.machineparam;
			} else
			if (row == self->sends.count + 7) {
				return &channel->mute_param.machineparam;
			} else
			if (row >= self->sends.count + 8) {
				return &self->ignore_param.machineparam;
			} else {
				return &self->blank_param.machineparam;
			}
		} else {
			return &self->blank_param.machineparam;
		}
	} else {
		return &self->blank_param.machineparam;
	}
	return NULL;
}

psy_audio_MachineParam* tweakparameter(psy_audio_Mixer* self, uintptr_t param)
{
	psy_audio_MachineParam* rv = NULL;

	int digitx0 = param / 16;
	int digit0x = param % 16;
// [0x]:
//     0->Master volume
//     1..C->Input volumes
//     D->master drywetmix.
//     E->master gain.
//     F->master pan.
// [1x..Cx]:
//     0->input wet mix.
//     1..C->input send amout to the send x.
//     D->input drywetmix. (0 normal, 1 dryonly, 2 wetonly  3 mute)
//     E->input gain.
//     F->input panning.
// [Dx]:
//     0->Solo channel.
//     1..C -> return grid. // the return grid array grid represents: bit0 -> mute, bit 1..12 routing to send. bit 13 -> route to master
// [Ex] :
//     1..C -> return volumes
// [Fx] :
//     1..C -> return panning
	if (digitx0 == 0) {
		if (digit0x == 0) {
			// 0->Master volume
			rv = psy_audio_volumemachineparam_base(&self->master.slider_param);
		} else
		if (digit0x <= 0x0C) {
			// 1..C->Input volumes
			psy_audio_InputChannel* channel;

			channel = (psy_audio_InputChannel*)psy_table_at(&self->inputs, param - 1);
			if (channel) {
				rv = psy_audio_volumemachineparam_base(&channel->slider_param);
			}
		} else
		if (digit0x == 0x0D) {
			// D->master drywetmix.
			// rv = psy_audio_drywetmixmachineparam_base(&self->master.drywetmix_param);
			rv = psy_audio_floatmachineparam_base(&self->master.dw_param);
		} else
		if (digit0x == 0x0E) {
			//  E->master gain.
			rv = psy_audio_gainmachineparam_base(&self->master.gain_param);
		} else
		if (digit0x == 0x0F) {
			// F->master pan.
			rv = psy_audio_floatmachineparam_base(&self->master.pan_param);
		}
	} else
	if (digitx0 >= 0x1 && digitx0 <= 0xC) {
		psy_audio_InputChannel* channel;

		channel = (psy_audio_InputChannel*)psy_table_at(&self->inputs, digitx0 - 1);
		if (channel) {
			if (digit0x == 0x0) {
				//  0->input wet mix.
				rv = psy_audio_floatmachineparam_base(&channel->mix_param);
			} else
			if (digit0x >= 0x1 && digit0x <= 0xC) {
				// input send amout to the send x.
				channel->sendvol_param.index = digitx0 - 1;
				rv = psy_audio_custommachineparam_base(&channel->sendvol_param);
			} else
			if (digit0x == 0xD) {
				// D->input drywetmix. (0 normal, 1 dryonly, 2 wetonly  3 mute)
				rv = psy_audio_drywetmixmachineparam_base(&channel->drywetmix_param);
			} else
			if (digit0x == 0xE) {
				// E->input gain.
				rv = psy_audio_custommachineparam_base(&channel->gain_param);
			} else
			if (digit0x == 0xF) {
				// F->input panning.
				rv = psy_audio_floatmachineparam_base(&channel->pan_param);
			}
		}
	} else
	if (digitx0 == 0xE) {
		// 1..C -> return volumes
		if (digit0x >= 0x1 && digit0x <= 0xC) {
			psy_audio_ReturnChannel* channel;

			channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, digitx0 - 1);			
			if (channel) {

				// rv = &channel->volume_param
			}
		}
	} else
	if (digitx0 == 0xF) {
		// 1..C -> return panning
		if (digit0x >= 0x1 && digit0x <= 0xC) {
			psy_audio_ReturnChannel* channel;

			channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, digitx0 - 1);			
			if (channel) {
				rv = psy_audio_floatmachineparam_base(&channel->pan_param);
			}
		}
	}
	return rv;
}

void paramcoords(psy_audio_Mixer* self, uintptr_t param, uintptr_t* col, uintptr_t* row)
{
	uintptr_t rows;

	rows = numparameters(self) / numparametercols(self);
	*row = param % rows;
	*col = param / rows;
}

psy_audio_WireSocketEntry* wiresocketentry(psy_audio_Mixer* self, uintptr_t input)
{
	psy_audio_WireSocketEntry* rv = 0;
	psy_audio_MachineSockets* sockets;
	WireSocket* p;
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(base);
	sockets = connections_at(&machines->connections,
		psy_audio_machine_slot(psy_audio_mixer_base(self)));
	if (sockets) {
		uintptr_t c = 0;

		for (p = sockets->inputs; p != 0 && c != input; p = p->next, ++c);
		if (p) {
			rv = (psy_audio_WireSocketEntry*) p->entry;
		}
	}
	return rv;
}

void loadspecific(psy_audio_Mixer* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t filesize;
	int32_t numins = 0;
	uint32_t numrets = 0;
	uint32_t i;

	psyfile_read(songfile->file, &filesize, sizeof(filesize));
	psyfile_read(songfile->file, &self->solocolumn, sizeof(self->solocolumn));
	psyfile_read(songfile->file, &self->master.volume, sizeof(float));
	psyfile_read(songfile->file, &self->master.gain, sizeof(float));
	psyfile_read(songfile->file, &self->master.drymix, sizeof(float));
	psyfile_read(songfile->file, &numins, sizeof(int32_t));
	psyfile_read(songfile->file, &numrets, sizeof(int32_t));
	self->custommachine.slot = slot;
	if (numins > 0) insertinputchannels(self, numins, &songfile->song->machines);
	//	if (numrets > 0) InsertReturn(numrets - 1);
	//	if (numrets > 0) InsertSend(numrets-1, NULL);
	for (i = 0; i < psy_table_size(&self->inputs); ++i) {
		psy_audio_InputChannel* channel;
		unsigned int j;

		channel = (psy_audio_InputChannel*)psy_table_at(&self->inputs, i);
		for (j = 0; j < numrets; ++j) {
			float send = 0.0f;
			psyfile_read(songfile->file, &send, sizeof(float));
			psy_table_insert(&channel->sendvols, j, (void*)(intptr_t)(send * 0xFF));
		}
		psyfile_read(songfile->file, &channel->volume, sizeof(float));
		psyfile_read(songfile->file, &channel->panning, sizeof(float));
		psyfile_read(songfile->file, &channel->drymix, sizeof(float));
		psyfile_read(songfile->file, &channel->mute, sizeof(unsigned char));
		psyfile_read(songfile->file, &channel->dryonly, sizeof(unsigned char));
		psyfile_read(songfile->file, &channel->wetonly, sizeof(unsigned char));
	}
	//legacyReturn_.resize(numrets);
	//legacySend_.resize(numrets);
	for (i = 0; i < numrets; ++i) {
		psy_audio_ReturnChannel* channel;
		char title[128];
		char label[128];

		psy_snprintf(title, 128, "Return %u", (unsigned int)i + 1);
		mixer_describeeditname(self, label, -1);
		channel = returnchannel_allocinit(i, -1, self, title, label);
		psy_table_insert(&self->returns, i, channel);
	}

	for (i = 0; i < numrets; ++i) {
		unsigned int j;
		psy_audio_ReturnChannel* channel;

		channel = (psy_audio_ReturnChannel*)psy_table_at(&self->returns, i);
		{
			// LegacyWire& leg = legacyReturn_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;

			psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine));	// Incoming (Return) connections psy_audio_Machine number
			psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol));	// /volume value for the current return wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 8.0f) { //bugfix on 1.10.1 alpha
				inputconvol /= 32768.f;
			}
			psy_table_insert(&self->sends, i, (void*)(intptr_t)inputmachine);
			channel->fxslot = inputmachine;
			psy_table_insert(&songfile->song->machines.connections.sends, inputmachine, (void*)1);
		}
		{
			// LegacyWire& leg2 = legacySend_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;

			psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine));	// Outgoing (Send) connections psy_audio_Machine number
			psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol));	//volume value for the current send wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 0.f && inputconvol < 0.0002f) { //bugfix on 1.10.1 alpha
				inputconvol *= 32768.f;
			}
		}
		for (j = 0; j < numrets; j++) {
			unsigned char send = 0;
			psyfile_read(songfile->file, &send, sizeof(unsigned char));
			if (send) {
				psy_table_insert(&channel->sendsto, j,
					psy_table_at(&self->returns, j));
			}
		}
		{
			// Return(i)
			unsigned char mastersend;
			float volume;
			float panning;
			unsigned char mute;

			psyfile_read(songfile->file, &mastersend, sizeof(unsigned char));
			psyfile_read(songfile->file, &volume, sizeof(float));
			psyfile_read(songfile->file, &panning, sizeof(float));
			psyfile_read(songfile->file, &mute, sizeof(unsigned char));
			channel->mastersend = mastersend;
			channel->volume = volume;
			channel->panning = panning;
			channel->mute = mute;
		}
	}
	psy_signal_connect(&songfile->song->machines.connections.signal_connected,
		self, onconnected);
	psy_signal_connect(&songfile->song->machines.connections.signal_disconnected,
		self, ondisconnected);
	// return true;
}

void savespecific(psy_audio_Mixer* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	float volume_;
	float drywetmix_;
	float gain_;
	int32_t numins;
	int32_t numrets;
	uint32_t size;
	uint32_t i;
	uint32_t j;

	size = (sizeof(self->solocolumn) + sizeof(volume_) + sizeof(drywetmix_) + sizeof(gain_) +
		2 * sizeof(uint32_t));
	size += (3 * sizeof(float) + 3 * sizeof(unsigned char) + self->sends.count * sizeof(float)) * self->inputs.count;
	size += (2 * sizeof(float) + 2 * sizeof(unsigned char) + self->sends.count * sizeof(unsigned char) + 2 * sizeof(float) + sizeof(uint32_t)) * self->returns.count;
	size += (2 * sizeof(float) + sizeof(uint32_t)) * self->sends.count;
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &self->solocolumn, sizeof(self->solocolumn));
	psyfile_write(songfile->file, &self->master.volume, sizeof(float));
	psyfile_write(songfile->file, &self->master.gain, sizeof(float));
	psyfile_write(songfile->file, &self->master.drymix, sizeof(float));
	numins = self->inputs.count;
	numrets = self->returns.count;
	psyfile_write(songfile->file, &numins, sizeof(int32_t));
	psyfile_write(songfile->file, &numrets, sizeof(int32_t));
	for (i = 0; i < (uint32_t)(self->inputs.count); ++i) {
		psy_audio_InputChannel* channel;

		channel = (psy_audio_InputChannel*)psy_table_at(&self->inputs, i);
		for (j = 0; j < self->sends.count; ++j) {
			float sendvol;

			sendvol = (int)(intptr_t)psy_table_at(&channel->sendvols, j) / (psy_dsp_amp_t) 0xFF;
			psyfile_write(songfile->file, &sendvol, sizeof(float));
		}
		psyfile_write(songfile->file, &channel->volume, sizeof(float));
		psyfile_write(songfile->file, &channel->panning, sizeof(float));
		psyfile_write(songfile->file, &channel->drymix, sizeof(float));
		psyfile_write(songfile->file, &channel->mute, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->dryonly, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->wetonly, sizeof(unsigned char));
	}
	for (i = 0; i < self->returns.count; ++i) {
		float volume, volMultiplier;
		uint32_t wMacIdx;
		psy_audio_ReturnChannel* channel;

		channel = psy_table_at(&self->returns, i);
		//Returning machines and values
		//const psy_audio_Wire& wireRet = Return(i).GetWire();
		//wMacIdx = (wireRet.Enabled()) ? wireRet.GetSrcMachine()._macIndex : -1;
		//volume = wireRet.GetVolume();
		volMultiplier = 1.0f; // wireRet.GetVolMultiplier();
		psyfile_write(songfile->file, &channel->fxslot, sizeof(int32_t));	// Incoming connections psy_audio_Machine number
		psyfile_write(songfile->file, &channel->volume, sizeof(float));	// Incoming connections psy_audio_Machine vol
		psyfile_write(songfile->file, &volMultiplier, sizeof(float));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		//Sending machines and values
		if (psy_table_exists(&channel->sendsto, i)) {
			psy_audio_ReturnChannel* sendto;

			sendto = psy_table_at(&channel->sendsto, i);
			wMacIdx = sendto->fxslot;
			volume = sendto->volume;
		}
		else {
			wMacIdx = -1;
			volume = 1.0f;
		}
		volMultiplier = 1.0f;
		psyfile_write(songfile->file, &wMacIdx, sizeof(int));	// send connections psy_audio_Machine number
		psyfile_write(songfile->file, &volume, sizeof(float));	// send connections psy_audio_Machine vol
		psyfile_write(songfile->file, &volMultiplier, sizeof(float));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range

		//Rewiring of returns to sends and mix values
		for (j = 0; j < self->sends.count; j++)
		{
			unsigned char send;

			send = psy_table_exists(&channel->sendsto, j);
			psyfile_write(songfile->file, &send, sizeof(unsigned char));
		}
		psyfile_write(songfile->file, &channel->mastersend, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->volume, sizeof(float));
		psyfile_write(songfile->file, &channel->panning, sizeof(float));
		psyfile_write(songfile->file, &channel->mute, sizeof(unsigned char));
	}
}

void postload(psy_audio_Mixer* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->sends);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		uintptr_t c;
		MachineList* path;
		uintptr_t sendslot;

		c = psy_tableiterator_key(&it);
		sendslot = (uintptr_t)psy_tableiterator_value(&it);
		machines_disconnect(psy_audio_machine_machines(
			&self->custommachine.machine), slot, sendslot);
		path = compute_path(psy_audio_machine_machines(
			&self->custommachine.machine), sendslot, FALSE);
		if (path) {
			// work fx chain
			for (; path != 0; path = path->next) {				
				sendslot = (size_t)path->entry;
				if (sendslot == NOMACHINE_INDEX) {
					// delimits the machines that could be processed parallel					
					continue;
				}
				machines_disconnect(psy_audio_machine_machines(
					&self->custommachine.machine), slot, sendslot);
				machines_addmixersend(psy_audio_machine_machines(
					&self->custommachine.machine), sendslot);				
			}
			psy_list_free(path);
		}
	}
}

void insertinputchannels(psy_audio_Mixer* self, uintptr_t num, psy_audio_Machines* machines)
{
	psy_audio_WireSocketEntry* rv = 0;
	psy_audio_MachineSockets* sockets;
	WireSocket* p;

	sockets = connections_at(&machines->connections,
		psy_audio_machine_slot(psy_audio_mixer_base(self)));
	if (sockets) {
		int c = 0;

		for (p = sockets->inputs; p != 0 && c != num; p = p->next, ++c) {
			psy_audio_WireSocketEntry* entry;		

			entry = (psy_audio_WireSocketEntry*)p->entry;			
			psy_table_insert(&self->inputs, c,
				inputchannel_allocinit(self, c, entry->slot));
		}
	}
}
