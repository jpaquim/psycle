/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "mixer.h"
// local
#include "machines.h"
#include "player.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
#include "wire.h"
#include "constants.h"
// dsp
#include <operations.h>
#include <dsptypes.h>
#include <convert.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"

static void mixer_describeeditname(psy_audio_Mixer*, char* text, uintptr_t slot);
static uintptr_t numreturncolumns(psy_audio_Mixer*);

// InputLabelParam
static int inputlabelparam_type(psy_audio_InputLabelParam* self)
{
	return MPF_INFOLABEL | MPF_SMALL;
}

static int inputlabelparam_name(psy_audio_InputLabelParam*, char* text);
static int inputlabelparam_describe(psy_audio_InputLabelParam*, char* text);

static MachineParamVtable inputlabelparam_vtable;
static bool inputlabelparam_vtable_initialized = FALSE;

static void inputlabelparam_vtable_init(psy_audio_InputLabelParam* self)
{
	if (!inputlabelparam_vtable_initialized) {
		inputlabelparam_vtable = *(self->machineparam.vtable);
		inputlabelparam_vtable.describe = (fp_machineparam_describe)
			inputlabelparam_describe;
		inputlabelparam_vtable.name = (fp_machineparam_name)
			inputlabelparam_name;
		inputlabelparam_vtable.type = (fp_machineparam_type)
			inputlabelparam_type;
		inputlabelparam_vtable_initialized = TRUE;
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
	if (self->channel) {
		psy_snprintf(text, 128, "Input %d", (int)self->channel->id + 1);
	} else {
		psy_snprintf(text, 128, "Input %d", (int)self->machineparam.param0);
	}
	return 1;
}

int inputlabelparam_describe(psy_audio_InputLabelParam* self, char* text)
{
	if (self->channel) {
		mixer_describeeditname(self->channel->mixer, text,
			self->channel->inputslot);
	} else {
		psy_snprintf(text, 128, "inactive");
	}
	return 1;
}

// SendLabelParam
static int sendreturnlabelparam_type(psy_audio_SendReturnLabelParam* self)
{
	return MPF_INFOLABEL | MPF_SMALL;
}

static int sendreturnlabelparam_name(psy_audio_SendReturnLabelParam*,
	char* text);
static int sendreturnlabelparam_describe(psy_audio_SendReturnLabelParam*,
	char* text);

static MachineParamVtable sendreturnlabelparam_vtable;
static bool sendreturnlabelparam_vtable_initialized = 0;

static void sendreturnlabelparam_vtable_init(psy_audio_SendReturnLabelParam* self)
{
	if (!sendreturnlabelparam_vtable_initialized) {
		sendreturnlabelparam_vtable = *(self->machineparam.vtable);
		sendreturnlabelparam_vtable.describe = (fp_machineparam_describe)
			sendreturnlabelparam_describe;
		sendreturnlabelparam_vtable.name = (fp_machineparam_name)
			sendreturnlabelparam_name;
		sendreturnlabelparam_vtable.type = (fp_machineparam_type)
			sendreturnlabelparam_type;
		sendreturnlabelparam_vtable_initialized = TRUE;
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
	if (self->channel->fx) {
		psy_snprintf(text, 128, "%s", psy_audio_machine_editname(self->channel->fx));
	} else {
		psy_snprintf(text, 128, "%s", "");
	}
	return 1;
}

// RouteMachineParam
static int routemachineparam_type(psy_audio_RouteMachineParam* self) { return MPF_SWITCH | MPF_SMALL; }
static void routemachineparam_tweak(psy_audio_RouteMachineParam* self, float val);
static float routemachineparam_normvalue(psy_audio_RouteMachineParam* self);
static int routemachineparam_name(psy_audio_RouteMachineParam*, char* text);

static MachineParamVtable routemachineparam_vtable;
static bool routemachineparam_vtable_initialized = FALSE;

static void routemachineparam_vtable_init(psy_audio_RouteMachineParam* self)
{
	if (!routemachineparam_vtable_initialized) {
		routemachineparam_vtable = *(self->machineparam.vtable);
		routemachineparam_vtable.normvalue = (fp_machineparam_normvalue)
			routemachineparam_normvalue;
		routemachineparam_vtable.tweak = (fp_machineparam_tweak)
			routemachineparam_tweak;
		routemachineparam_vtable.type = (fp_machineparam_type)
			routemachineparam_type;
		routemachineparam_vtable.name = (fp_machineparam_name)
			routemachineparam_name;
		routemachineparam_vtable_initialized = TRUE;
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
	if (self->send < numreturncolumns(self->mixer)) {
		if (val > 0.f) {
			psy_table_insert(&self->channel->sendsto, self->send, (void*)(uintptr_t)TRUE);
		} else {
			psy_table_remove(&self->channel->sendsto, self->send);
		}
	} else
	if (self->send == numreturncolumns(self->mixer)) {
		self->channel->mastersend = (val > 0.f) ? TRUE : FALSE;
	}
}

float routemachineparam_normvalue(psy_audio_RouteMachineParam* self)
{
	if (self->send < numreturncolumns(self->mixer)) {
		return psy_table_exists(&self->channel->sendsto, self->send) ? 1.f : 0.f;
	} else 
	if (self->send == numreturncolumns(self->mixer)) {
		return self->channel->mastersend ? 1.f : 0.f;
	}
	return 0.f;
}

int routemachineparam_name(psy_audio_RouteMachineParam* self, char* text)
{
	psy_snprintf(text, 128, "%s",
		(self->send < numreturncolumns(self->mixer) ? "Route" : "Master"));
	return 1;
}

const psy_audio_MachineInfo* psy_audio_mixer_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		GENERATOR | 32 | 64,
		psy_audio_MACHMODE_GENERATOR,
		"Mixer"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"Mixer",
		"Psycledelics",
		"help",
		psy_audio_MIXER,
		0,
		0,
		"",
		"",
		"",
		psy_INDEX_INVALID,
		""
	};
	return &macinfo;
}
// virtual prototypes
static const psy_audio_MachineInfo* info(psy_audio_Mixer* self)
{
	return psy_audio_mixer_info();
}
static void psy_audio_mixer_dispose(psy_audio_Mixer*);
static void psy_audio_mixer_dispose_channels(psy_audio_Mixer*);
static void psy_audio_mixer_dispose_sends(psy_audio_Mixer*);
static void psy_audio_mixer_dispose_returns(psy_audio_Mixer*);
static void psy_audio_mixer_dispose_legacywires(psy_audio_Mixer*);

static int mixer_mode(psy_audio_Mixer* self) { return psy_audio_MACHMODE_FX; }
static uintptr_t numinputs(psy_audio_Mixer* self) { return 2; }
static uintptr_t numoutputs(psy_audio_Mixer* self) { return 2; }
static psy_audio_Buffer* mix(psy_audio_Mixer*, uintptr_t slot, uintptr_t amount,
	psy_audio_MachineSockets*, psy_audio_Machines*, psy_audio_Player*);
static void work(psy_audio_Mixer*, psy_audio_BufferContext*);
static int loadspecific(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static int savespecific(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void postload(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void postloadinputchannels(psy_audio_Mixer*, psy_audio_SongFile* songfile, uintptr_t slot);
static void postreturnchannels(psy_audio_Mixer*, psy_audio_SongFile* songfile, uintptr_t slot);
static void psy_audio_mixer_clearlegacywires(psy_audio_Mixer*);
static void onconnected(psy_audio_Mixer*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void ondisconnected(psy_audio_Mixer*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
// Parameter
static psy_audio_MachineParam* parameter(psy_audio_Mixer*, uintptr_t param);
static psy_audio_MachineParam* tweakparameter(psy_audio_Mixer*, uintptr_t param);
static uintptr_t numparameters(psy_audio_Mixer*);
static uintptr_t numtweakparameters(psy_audio_Mixer*);
static uintptr_t numparametercols(psy_audio_Mixer*);
static uintptr_t paramstrobe(const psy_audio_Mixer*);
static void paramcoords(psy_audio_Mixer* self, uintptr_t param, uintptr_t* col, uintptr_t* row);
static psy_dsp_amp_range_t amprange(psy_audio_Mixer* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static void mixer_describepanning(char* text, float pan);

// private methods
static uintptr_t mastercolumn(psy_audio_Mixer* self) { return 1; }
static uintptr_t inputcolumn(psy_audio_Mixer* self)
{
	return mastercolumn(self) + 1;
}

static uintptr_t numinputcolumns(psy_audio_Mixer* self)
{
	uintptr_t rv = 0;

	if (self->maxinput == 0 && !psy_table_exists(&self->inputs, 0)) {
		rv = 0;
	} else {
		rv = self->maxinput + 1;
	}
	return rv;
}

static uintptr_t returncolumn(psy_audio_Mixer* self)
{
	return inputcolumn(self) + numinputcolumns(self);
}

uintptr_t numreturncolumns(psy_audio_Mixer* self)
{
	uintptr_t rv = 0;

	if (self->maxreturn == 0 && !psy_table_exists(&self->returns, 0)) {
		rv = 0;
	} else {
		rv = self->maxreturn + 1;
	}
	return rv;
}

static uintptr_t freeinputchannel(psy_audio_Mixer*);
static uintptr_t freereturnchannel(psy_audio_Mixer*);
static uintptr_t inputmax(psy_audio_Mixer*);
static uintptr_t returnmax(psy_audio_Mixer* self);
static void preparemix(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void mixinputs(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount,
	psy_audio_Player*);
static void workreturns(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount,
	psy_audio_Player*);
static void workreturn(psy_audio_Mixer* self, psy_audio_Machines* machines,
	psy_audio_ReturnChannel*,
	uintptr_t amount,
	psy_audio_Player*);
static void mixreturnstomaster(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void levelmaster(psy_audio_Mixer*, uintptr_t amount);
// D/W
static void masterchannel_dw_describe(psy_audio_MasterChannel*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);
// Pan
static void channel_pan_describe(psy_audio_MachineParam*,
	psy_audio_MachineParam* sender, int* active, char* text);
// Slider/Level
static void masterchannel_level_normvalue(psy_audio_MasterChannel*,
	psy_audio_CustomMachineParam* sender, float* rv);
// Send Vol
static void inputchannel_sendvol_tweak(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, float value);
static void inputchannel_sendvol_normvalue(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, float* rv);
static void inputchannel_sendvol_describe(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, int* active, char* txt);
// D/W
static void inputchannel_dw_describe(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, int* active, char* txt);
// Gain
static void inputchannel_gain_tweak(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, float value);
static void inputchannel_gain_normvalue(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, float* rv);
static void inputchannel_gain_describe(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, int* active, char* txt);
// Slider/Level
static void inputchannel_level_normvalue(psy_audio_InputChannel*,
	psy_audio_CustomMachineParam* sender, float* rv);
// ReturnChannel
static void returnchannel_init(psy_audio_ReturnChannel*,
	psy_audio_Mixer* mixer, uintptr_t id, uintptr_t fxslot);
static psy_audio_ReturnChannel* returnchannel_allocinit(psy_audio_Mixer*,
	uintptr_t id, uintptr_t fxslot);
static void returnchannel_dispose(psy_audio_ReturnChannel*);
static void returnchannel_computepath(psy_audio_ReturnChannel*);
static psy_audio_Buffer* returnchannel_firstbuffer(psy_audio_ReturnChannel*);
// Slider/Level
static void returnchannel_level_normvalue(psy_audio_ReturnChannel*,
	psy_audio_CustomMachineParam* sender, float* rv);

// Send
void psy_audio_mixersend_init(psy_audio_MixerSend* self, uintptr_t slot)
{
	self->slot = slot;
	self->inputconvol = (psy_dsp_amp_t) 1.f;
}

void psy_audio_mixersend_dispose(psy_audio_MixerSend* self, uintptr_t slot)
{

}

psy_audio_MixerSend* psy_audio_mixersend_alloc(void)
{
	return (psy_audio_MixerSend*) malloc(sizeof(psy_audio_MixerSend));
}


psy_audio_MixerSend* psy_audio_mixersend_allocinit(uintptr_t slot)
{
	psy_audio_MixerSend* rv;

	rv = psy_audio_mixersend_alloc();
	if (rv) {
		psy_audio_mixersend_init(rv, slot);
	}
	return rv;
}

// MasterChannel
// implementation
void masterchannel_init(psy_audio_MasterChannel* self, psy_audio_Mixer* mixer, const char* name, const char* label)
{
	self->mixer = mixer;
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
		"Level", "Level", MPF_SLIDER | MPF_SMALL, &self->volume);	
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_LEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		masterchannel_level_normvalue);	
}

void masterchannel_dispose(psy_audio_MasterChannel* self)
{
	psy_audio_infomachineparam_dispose(&self->info_param);
	psy_audio_floatmachineparam_dispose(&self->dw_param);
	psy_audio_gainmachineparam_dispose(&self->gain_param);
	psy_audio_floatmachineparam_dispose(&self->pan_param);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
	psy_table_dispose(&self->sendvols);
}

psy_audio_MasterChannel* masterchannel_allocinit(psy_audio_Mixer* mixer,
	const char* name, const char* label)
{
	psy_audio_MasterChannel* rv;

	rv = (psy_audio_MasterChannel*) malloc(sizeof(psy_audio_MasterChannel));
	if (rv) {
		masterchannel_init(rv, mixer, name, label);
	}
	return rv;
}

void masterchannel_dw_describe(psy_audio_MasterChannel* self,
	psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	psy_snprintf(txt, 20, "%d%%", (int)(psy_audio_machineparam_normvalue(
		psy_audio_custommachineparam_base(sender)) * 100));
	*active = 1;
}

void channel_pan_describe(psy_audio_MachineParam* self,
	psy_audio_MachineParam* sender, int* active, char* txt)
{
	mixer_describepanning(txt, psy_audio_machineparam_normvalue(sender));
	*active = 1;
}

void masterchannel_level_normvalue(psy_audio_MasterChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	psy_audio_Buffer* memory;

	memory = psy_audio_machine_buffermemory(psy_audio_mixer_base(self->mixer));
	if (memory) {
		*rv = psy_audio_buffer_rmsdisplay(memory);
	} else {
		*rv = 0.f;
	}
}

// Input Channel
void inputchannel_init(psy_audio_InputChannel* self, uintptr_t id,
	psy_audio_Mixer* mixer,
	uintptr_t inputslot)
{
	assert(self);

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
	self->dryonly = 0;
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
		"Level", "Level", MPF_SLIDER | MPF_SMALL, &self->volume);	
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_LEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		inputchannel_level_normvalue);	
}

void inputchannel_dispose(psy_audio_InputChannel* self)
{
	assert(self);

	psy_audio_inputlabelparam_dispose(&self->info_param);
	psy_audio_custommachineparam_dispose(&self->sendvol_param);
	psy_audio_floatmachineparam_dispose(&self->mix_param);
	psy_audio_custommachineparam_dispose(&self->gain_param);
	psy_audio_floatmachineparam_dispose(&self->pan_param);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);		
	psy_table_dispose(&self->sendvols);
}

psy_audio_InputChannel* inputchannel_allocinit(psy_audio_Mixer* mixer, uintptr_t id,
	uintptr_t inputslot)
{
	psy_audio_InputChannel* rv;

	assert(mixer);

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

	assert(self);

	scaled = (int)(value * 0xFF);
	psy_table_insert(&self->sendvols, sender->row, (void*)(uintptr_t) scaled);
}

void inputchannel_sendvol_normvalue(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	assert(self);

	*rv = (uintptr_t)psy_table_at(&self->sendvols,
		sender->row) / (float)0xFF;
}

void inputchannel_sendvol_describe(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	psy_dsp_amp_t sendvol;

	assert(self);

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
	assert(self);

	psy_snprintf(txt, 20, "%d%%", (int)(self->drymix * 100));
	*active = 1;
}

void inputchannel_gain_tweak(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float value)
{
	assert(self);

	psy_audio_connections_setwirevolume(&psy_audio_machine_machines(
		psy_audio_mixer_base(self->mixer))->connections,
		psy_audio_wire_make(
			self->inputslot,
			psy_audio_machine_slot(psy_audio_mixer_base(self->mixer))),
		value * value * 4.f);
}

void inputchannel_gain_normvalue(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	assert(self);

	*rv = (float)sqrt(psy_audio_connections_wirevolume(
		&psy_audio_machine_machines(psy_audio_mixer_base(self->mixer))->connections,
		psy_audio_wire_make(
			self->inputslot,
			psy_audio_machine_slot(psy_audio_mixer_base(self->mixer))))) * 0.5f;
}

void inputchannel_gain_describe(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, int* active, char* txt)
{
	float db;
	
	assert(self);

	db = (psy_dsp_amp_t) (20 * log10(inputchannel_wirevolume(self)));
	psy_snprintf(txt, 10, "%.2f dB", db);
	*active = 1;	
}

void inputchannel_level_normvalue(psy_audio_InputChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{	
	psy_audio_Machines* machines;
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	assert(self);

	*rv = 0.f;
	machines = psy_audio_machine_machines(psy_audio_mixer_base(self->mixer));
	machine = psy_audio_machines_at(machines, self->inputslot);
	if (machine) {
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			psy_dsp_amp_t temp;

			temp = ((int)(50.0f * log10((double)inputchannel_wirevolume(self) *
				(double)self->volume))) / 97.f;
			*rv = psy_audio_buffer_rmsdisplay(memory) + temp;
		}
	}
}

psy_dsp_amp_t inputchannel_wirevolume(psy_audio_InputChannel* self)
{
	psy_dsp_amp_t rv;

	assert(self);	

	rv = psy_audio_connections_wirevolume(
		&psy_audio_machine_machines(psy_audio_mixer_base(self->mixer))->connections,
		psy_audio_wire_make(
			self->inputslot,
			psy_audio_machine_slot(psy_audio_mixer_base(self->mixer))));
	return rv;
}

bool inputchannel_issoloed(const psy_audio_InputChannel* self)
{
	return self->mixer->inputsolo == self->id;
}

bool psy_audio_inputchannel_active(const psy_audio_InputChannel* self)
{
	return (self->mixer->inputsolo == psy_INDEX_INVALID ||
		self->mixer->inputsolo == self->id) && !self->mute;
}

bool inputchannel_ismute(const psy_audio_InputChannel* self)
{
	return self->mute;
}

// Return Channel
void returnchannel_init(psy_audio_ReturnChannel* self,
	struct psy_audio_Mixer* mixer,
	uintptr_t id,
	uintptr_t fxslot)
{
	self->id = id;
	self->fxslot = fxslot;
	self->fx = 0;
	self->path = 0;
	self->mute = 0;
	self->panning = 0.5f;
	self->volume = 1.f;
	self->mastersend = 1;
	self->mixer = mixer;
	psy_table_init(&self->sendsto);	
	psy_audio_sendreturnlabelparam_init(&self->sendlabel_param, self, TRUE);
	psy_audio_sendreturnlabelparam_init(&self->returnlabel_param, self, FALSE);
	psy_audio_custommachineparam_init(&self->send_param,
		"", "", MPF_INFOLABEL | MPF_SMALL, 0, 100);
	psy_audio_custommachineparam_init(&self->info_param,
		"", "", MPF_INFOLABEL | MPF_SMALL, 0, 100);
	psy_audio_floatmachineparam_init(&self->pan_param,
		"Pan", "Pan", MPF_STATE | MPF_SMALL, &self->panning, 0, 100);
	psy_signal_connect(&self->pan_param.machineparam.signal_describe, self,
		channel_pan_describe);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Level", "Level", MPF_SLIDER | MPF_SMALL, &self->volume);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_LEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		returnchannel_level_normvalue);		
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
	psy_audio_routemachineparam_dispose(&self->route_param);
	psy_table_dispose(&self->sendsto);
	if (self->path) {
		psy_list_free(self->path);
	}
}

psy_audio_ReturnChannel* returnchannel_allocinit(psy_audio_Mixer* mixer,
	uintptr_t id, uintptr_t fxslot)
{
	psy_audio_ReturnChannel* rv;

	rv = (psy_audio_ReturnChannel*) malloc(sizeof(psy_audio_ReturnChannel));
	if (rv) {
		returnchannel_init(rv, mixer, id, fxslot);
	}
	return rv;
}

bool psy_audio_returnchannel_active(const psy_audio_ReturnChannel* self)
{
	return (self->mixer->returnsolo == psy_INDEX_INVALID ||
		self->mixer->returnsolo == self->id) && !self->mute;
}

void returnchannel_computepath(psy_audio_ReturnChannel* self)
{
	MachineList* path;
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(psy_audio_mixer_base(self->mixer));
	path = psy_audio_compute_path(machines, self->fxslot, FALSE);
	if (self->path) {
		psy_list_free(self->path);
	}
	self->path = path;	
	for (path = self->path; path != 0; path = path->next) {
		uintptr_t slot;
		
		slot = (uintptr_t)path->entry;
		if (slot == psy_INDEX_INVALID) {
			continue;
		}
		psy_audio_machines_addmixersend(machines, slot);
	}
}

psy_audio_Buffer* returnchannel_firstbuffer(psy_audio_ReturnChannel* self)
{
	psy_audio_Buffer* rv = 0;

	if (!self->path) {
		returnchannel_computepath(self);
	}
	if (self->path) {
		uintptr_t slot;

		slot = (uintptr_t)(self->path->entry);
		if (slot != psy_INDEX_INVALID) {
			psy_audio_Machine* firstmachine;
			psy_audio_Machines* machines;

			machines = psy_audio_machine_machines(
				psy_audio_mixer_base(self->mixer));
			if (machines) {
				firstmachine = psy_audio_machines_at(machines, slot);
				if (firstmachine) {
					rv = psy_audio_machines_outputs(machines, slot);
				}
			}
		}
	}
	return rv;
}

void returnchannel_level_normvalue(psy_audio_ReturnChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	psy_audio_Machines* machines;
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	*rv = 0.f;
	machines = psy_audio_machine_machines(psy_audio_mixer_base(self->mixer));
	machine = psy_audio_machines_at(machines, self->fxslot);
	if (machine) {
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			psy_dsp_amp_t temp;

			temp = ((int)(50.0f * log10(/*(double)inputchannel_wirevolume(self)*/
				(double)self->volume))) / 97.f;
			*rv = psy_audio_buffer_rmsdisplay(memory) + temp;
		}
	}
}

// Mixer
// paramview
static void mixer_solo_tweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_solo_normvalue(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float* rv);
static void mixer_mute_tweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_mute_normvalue(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float* rv);
static void mixer_dry_tweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_dry_normvalue(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float* rv);
static void mixer_wet_tweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_wet_normvalue(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float* rv);
// pattern
static void mixer_solo_patterntweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_mute_patterntweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_dry_patterntweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);
static void mixer_returngrid_tweak(psy_audio_Mixer*,
	psy_audio_IntMachineParam* sender, float value);

static MachineVtable vtable;
static bool vtable_initialized = FALSE;
static fp_machine_work work_super = 0;
static fp_machine_postload postload_super = 0;

static void vtable_init(psy_audio_Mixer* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		work_super = vtable.work;
		postload_super = vtable.postload;
		vtable.info =
			(fp_machine_info)
			info;
		vtable.numinputs =
			(fp_machine_numinputs)
			numinputs;
		vtable.numoutputs =
			(fp_machine_numoutputs)
			numoutputs;
		vtable.dispose =
			(fp_machine_dispose)
			psy_audio_mixer_dispose;
		vtable.mode =
			(fp_machine_mode)
			mixer_mode;
		vtable.mix =
			(fp_machine_mix)
			mix;
		vtable.work =
			(fp_machine_work)
			work;
		vtable.parameter =
			(fp_machine_parameter)
			parameter;
		vtable.tweakparameter =
			(fp_machine_parameter)
			tweakparameter;
		vtable.numparameters =
			(fp_machine_numparameters)
			numparameters;
		vtable.numtweakparameters =
			(fp_machine_numtweakparameters)
			numtweakparameters;
		vtable.numparametercols =
			(fp_machine_numparametercols)
			numparametercols;
		vtable.paramstrobe =
			(fp_machine_paramstrobe)
			paramstrobe;
		vtable.loadspecific =
			(fp_machine_loadspecific)
			loadspecific;
		vtable.savespecific =
			(fp_machine_savespecific)
			savespecific;
		vtable.postload =
			(fp_machine_postload)
			postload;
		vtable.amprange =
			(fp_machine_amprange)
			amprange;
		vtable_initialized = TRUE;
	}
	self->custommachine.machine.vtable = &vtable;
}

void psy_audio_mixer_init(psy_audio_Mixer* self, psy_audio_MachineCallback* callback)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	psy_audio_Machines* machines;

	psy_audio_custommachine_init(&self->custommachine, callback);
	vtable_init(self);	
	psy_table_init(&self->inputs);
	self->maxinput = 0;
	psy_table_init(&self->sends);
	psy_table_init(&self->returns);
	self->maxreturn = 0;
	psy_table_init(&self->legacyreturn_);
	psy_table_init(&self->legacysend_);
	// psycle-mfc uses -1 instead of psy_INDEX_INVALID
	self->inputsolo = psy_INDEX_INVALID;
	self->returnsolo = psy_INDEX_INVALID;	
	self->strobe = 0;
	machines = psy_audio_machine_machines(base);
	psy_signal_connect(&machines->connections.signal_connected, self,
		onconnected);
	psy_signal_connect(&machines->connections.signal_disconnected, self,
		ondisconnected);
	masterchannel_init(&self->master, self, "Master Out", "");
	psy_audio_machine_seteditname(base, "Mixer");
	psy_audio_custommachineparam_init(&self->blank_param, "", "", MPF_NULL | MPF_SMALL, 0, 0);
	psy_audio_inputlabelparam_init(&self->blank_inputinfo_param, NULL);
	psy_audio_custommachineparam_init(&self->ignore_param, "-", "-", MPF_IGNORE | MPF_SMALL, 0, 0);
	//psy_audio_custommachineparam_init(&self->route_param, "Route", "Route", MPF_SWITCH | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->routemaster_param, "Master", "Master", MPF_SWITCH | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->label_mix_param, "Mix", "Mix", MPF_INFOLABEL | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->label_gain_param, "Gain", "Gain", MPF_INFOLABEL | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->label_pan_param, "Pan", "Pan", MPF_INFOLABEL | MPF_SMALL, 0, 0);
	psy_audio_custommachineparam_init(&self->label_ch_input_param, "Ch. Input", "Ch. Input",
		MPF_INFOLABEL | MPF_SMALL | MPF_BOTTOM, 0, 0);
	// paramview parameters
	// solo
	psy_audio_intmachineparam_init(&self->solo_param, "S", "S", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	self->solo_param.machineparam.isslidergroup = TRUE;
	psy_signal_connect(&self->solo_param.machineparam.signal_tweak, self,
		mixer_solo_tweak);
	psy_signal_connect(&self->solo_param.machineparam.signal_normvalue, self,
		mixer_solo_normvalue);	
	// mute
	psy_audio_intmachineparam_init(&self->mute_param, "M", "M", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	psy_signal_connect(&self->mute_param.machineparam.signal_tweak, self,
		mixer_mute_tweak);
	psy_signal_connect(&self->mute_param.machineparam.signal_normvalue, self,
		mixer_mute_normvalue);
	self->mute_param.machineparam.isslidergroup = TRUE;	
	// dry
 	psy_audio_intmachineparam_init(&self->dry_param, "D", "D", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	self->dry_param.machineparam.isslidergroup = TRUE;
	psy_signal_connect(&self->dry_param.machineparam.signal_tweak, self,
		mixer_dry_tweak);
	psy_signal_connect(&self->dry_param.machineparam.signal_normvalue, self,
		mixer_dry_normvalue);
	// wet
	psy_audio_intmachineparam_init(&self->wet_param, "W", "W", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	self->wet_param.machineparam.isslidergroup = TRUE;
	psy_signal_connect(&self->wet_param.machineparam.signal_tweak, self,
		mixer_wet_tweak);
	psy_signal_connect(&self->wet_param.machineparam.signal_normvalue, self,
		mixer_wet_normvalue);
	// pattern tweak
	// solo
	psy_audio_intmachineparam_init(&self->solo_tweak_param, "S", "S", MPF_CHECK | MPF_SMALL, NULL, 0, 0xFF);
	psy_signal_connect(&self->solo_tweak_param.machineparam.signal_tweak, self,
		mixer_solo_patterntweak);
	// mute
	psy_audio_intmachineparam_init(&self->mute_tweak_param, "M", "M", MPF_CHECK | MPF_SMALL, NULL, 0, 0xFF);
	psy_signal_connect(&self->mute_tweak_param.machineparam.signal_tweak, self,
		mixer_mute_patterntweak);
	// dry
	psy_audio_intmachineparam_init(&self->dry_tweak_param, "D", "D", MPF_CHECK | MPF_SMALL, NULL, 0, 0xFF);
	psy_signal_connect(&self->dry_tweak_param.machineparam.signal_tweak, self,
		mixer_dry_patterntweak);	
	// returngrid
	psy_audio_intmachineparam_init(&self->returngrid_tweak_param, "M", "M", MPF_CHECK | MPF_SMALL, NULL, 0, 0xFF);
	psy_signal_connect(&self->returngrid_tweak_param.machineparam.signal_tweak, self,
		mixer_returngrid_tweak);
}

void psy_audio_mixer_dispose(psy_audio_Mixer* self)
{
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(psy_audio_mixer_base(self));
	if (machines) {
		psy_signal_disconnect(&machines->connections.signal_connected, self,
			onconnected);
		psy_signal_disconnect(&machines->connections.signal_disconnected, self,
			ondisconnected);
	}
	psy_audio_mixer_dispose_channels(self);
	psy_audio_mixer_dispose_sends(self);
	psy_audio_mixer_dispose_returns(self);
	psy_audio_mixer_dispose_legacywires(self);
	psy_audio_custommachineparam_dispose(&self->blank_param);
	psy_audio_inputlabelparam_dispose(&self->blank_inputinfo_param);
	psy_audio_custommachineparam_dispose(&self->ignore_param);
	psy_audio_custommachineparam_dispose(&self->routemaster_param);
	psy_audio_custommachineparam_dispose(&self->label_mix_param);
	psy_audio_custommachineparam_dispose(&self->label_gain_param);
	psy_audio_custommachineparam_dispose(&self->label_pan_param);
	psy_audio_custommachineparam_dispose(&self->label_ch_input_param);
	psy_audio_intmachineparam_dispose(&self->solo_param);
	psy_audio_intmachineparam_dispose(&self->mute_param);
	psy_audio_intmachineparam_dispose(&self->dry_param);
	psy_audio_intmachineparam_dispose(&self->wet_param);
	psy_audio_intmachineparam_dispose(&self->solo_tweak_param);	
	psy_audio_intmachineparam_dispose(&self->mute_tweak_param);
	psy_audio_intmachineparam_dispose(&self->dry_tweak_param);	
	psy_audio_intmachineparam_dispose(&self->returngrid_tweak_param);
	psy_audio_custommachine_dispose(&self->custommachine);
}

void psy_audio_mixer_dispose_channels(psy_audio_Mixer* self)
{	
	psy_table_dispose_all(&self->inputs, (psy_fp_disposefunc)
		inputchannel_dispose);
}

void psy_audio_mixer_dispose_sends(psy_audio_Mixer* self)
{	
	psy_table_dispose_all(&self->sends, (psy_fp_disposefunc)NULL);
}

void psy_audio_mixer_dispose_returns(psy_audio_Mixer* self)
{
	psy_table_dispose_all(&self->returns, (psy_fp_disposefunc)
		returnchannel_dispose);	
}

void psy_audio_mixer_dispose_legacywires(psy_audio_Mixer* self)
{
	psy_audio_mixer_clearlegacywires(self);
	psy_table_dispose(&self->legacyreturn_);
	psy_table_dispose(&self->legacysend_);
}

psy_audio_Buffer* mix(psy_audio_Mixer* self, uintptr_t slot, uintptr_t amount,
	psy_audio_MachineSockets* connected_machine_sockets,
	psy_audio_Machines* machines,
	psy_audio_Player* player)
{
	preparemix(self, machines, amount);
	mixinputs(self, machines, amount, player);
	workreturns(self, machines, amount, player);
	mixreturnstomaster(self, machines, amount);
	levelmaster(self, amount);
	return self->master.buffer;
}

void work(psy_audio_Mixer* self, psy_audio_BufferContext* bc)
{
	work_super(&self->custommachine.machine, bc);
}

void preparemix(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{
	psy_TableIterator iter;

	self->master.buffer = psy_audio_machines_outputs(machines,
		psy_audio_machine_slot(psy_audio_mixer_base(self)));
	psy_audio_buffer_clearsamples(self->master.buffer, amount);
	for (iter = psy_table_begin(&self->returns);
			!psy_tableiterator_equal(&iter, psy_table_end());
			psy_tableiterator_inc(&iter)) {
		psy_audio_ReturnChannel* channel;
		psy_audio_Buffer* buffer;

		channel = (psy_audio_ReturnChannel*)psy_tableiterator_value(&iter);
		returnchannel_computepath(channel);
		buffer = returnchannel_firstbuffer(channel); 
		psy_audio_buffer_clearsamples(buffer, amount);
		buffer->preventmixclear = TRUE;		
		channel->buffer = psy_audio_machines_outputs(machines, channel->fxslot);
		channel->fx = psy_audio_machines_at(machines, channel->fxslot);		
	}
}

void mixinputs(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount, psy_audio_Player* player)
{
	psy_TableIterator input_iter;

	for (input_iter = psy_table_begin(&self->inputs);
			!psy_tableiterator_equal(&input_iter, psy_table_end());
			psy_tableiterator_inc(&input_iter)) {
		psy_audio_InputChannel* input;
		psy_TableIterator sendvol_iter;

		input = (psy_audio_InputChannel*)psy_tableiterator_value(&input_iter);
		if (input && psy_audio_inputchannel_active(input)) {			
			input->buffer = psy_audio_machines_outputs(machines, input->inputslot);
			// mix input to master
			if (!input->wetonly) {
				psy_audio_buffer_addsamples(self->master.buffer, input->buffer,
					amount, input->volume * input->drymix *
					inputchannel_wirevolume(input));
			}
			// mix input to return channel chain start
			if (input->dryonly) {
				continue;
			}
			for (sendvol_iter = psy_table_begin(&input->sendvols);
					!psy_tableiterator_equal(&sendvol_iter, psy_table_end());
					psy_tableiterator_inc(&sendvol_iter)) {
				psy_dsp_amp_t sendvol;
				
				sendvol = ((uintptr_t)psy_tableiterator_value(&sendvol_iter)) /
					(psy_dsp_amp_t) 0xFF;
				if (sendvol > 0.f) {
					psy_audio_ReturnChannel* returnchannel;

					returnchannel = psy_audio_mixer_return(self,
						psy_tableiterator_key(&sendvol_iter));
					if (returnchannel && psy_audio_returnchannel_active(returnchannel)) {
						psy_audio_Buffer* buffer;
												
						buffer = returnchannel_firstbuffer(returnchannel);
						if (buffer) {
							psy_audio_buffer_addsamples(buffer, input->buffer,
								amount, sendvol);
						}
					}					
				}
			}
		}
	}
}

void mixreturnstomaster(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{
	psy_TableIterator iter;

	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
		psy_tableiterator_inc(&iter)) {
		psy_audio_ReturnChannel* channel;

		channel = psy_tableiterator_value(&iter);
		if (channel && channel->mastersend && psy_audio_returnchannel_active(channel)) {
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

					sendto = psy_audio_mixer_return(self, psy_tableiterator_key(&sendsto_iter));					
					if (sendto && sendto->buffer) {
							psy_audio_buffer_addsamples(sendto->buffer, channel->buffer, amount,
								channel->volume);						
					}
				}
			}
		}
	}
}

void workreturn(psy_audio_Mixer* self, psy_audio_Machines* machines,
	psy_audio_ReturnChannel* channel,
	uintptr_t amount,
	psy_audio_Player* player)
{	
	if (channel->path) {
		uintptr_t slot;
		MachineList* path;
		
		for (path = channel->path; path != 0; path = path->next) {
			slot = (uintptr_t) path->entry;
			if (slot == psy_INDEX_INVALID) {
				// delimits the machines that could be processed parallel
				// todo: add thread functions
				continue;
			}
			psy_audio_player_work_machine(player, amount, slot);
		}
	}
	channel->buffer = psy_audio_machines_outputs(machines, channel->fxslot);
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
			machine = psy_audio_machines_at(machines, outputslot);
			if (machine) {
				if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR ||
					!psy_audio_machines_is_connect_as_mixersend(machines)) {
					uintptr_t inputnum;

					inputnum = freeinputchannel(self);
					psy_audio_mixer_insertchannel(self, inputnum,
						inputchannel_allocinit(self, inputnum, outputslot));
				} else {
					MachineList* path;
					psy_audio_MixerSend* send;

					send = psy_audio_mixer_insertsend(self, numreturncolumns(self),
						psy_audio_mixersend_allocinit(outputslot));
					psy_audio_mixer_insertreturn(self, numreturncolumns(self),
						returnchannel_allocinit(self, numreturncolumns(self), outputslot));
					path = psy_audio_compute_path(machines, outputslot, FALSE);
					if (path) {
						// work fx chain
						for (; path != 0; path = path->next) {
							uintptr_t slot;

							slot = (size_t)path->entry;
							if (slot == psy_INDEX_INVALID) {
								// delimits the machines that could be processed parallel
								// todo: add thread functions
								continue;
							}
							psy_audio_machines_addmixersend(machines, slot);
						}
						psy_list_free(path);
					}
				}
			}
		}
	}
}

uintptr_t freeinputchannel(psy_audio_Mixer* self)
{
	uintptr_t rv = 0;

	while (psy_table_exists(&self->inputs, rv)) ++rv;
	return rv;
}

uintptr_t freereturnchannel(psy_audio_Mixer* self)
{
	uintptr_t rv = 0;

	while (psy_table_exists(&self->returns, rv)) ++rv;
	return rv;
}

void ondisconnected(psy_audio_Mixer* self, psy_audio_Connections* connections,
	uintptr_t outputslot, uintptr_t inputslot)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	if (inputslot == psy_audio_machine_slot(psy_audio_mixer_base(self))) {
		psy_audio_Machine* machine;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(base);
		machine = psy_audio_machines_at(machines, outputslot);
		if (psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) {
			psy_TableIterator it;

			for (it = psy_table_begin(&self->inputs);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				psy_audio_InputChannel* channel;

				channel = (psy_audio_InputChannel*)psy_tableiterator_value(&it);
				if (channel->inputslot == outputslot) {
					psy_audio_mixer_discardchannel(self, psy_tableiterator_key(&it));
					break;
				}
			}
		} else {
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
					sendto = psy_audio_mixer_return(self, psy_tableiterator_key(&sendsto_iter));
					psy_tableiterator_inc(&sendsto_iter);
					if (sendto) {
						if (sendto->fxslot == outputslot) {
							psy_table_remove(&channel->sendsto, key);
						}
					}
				}
			}
			for (it = psy_table_begin(&self->sends);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {
				uintptr_t sendslot;
				uintptr_t c;
				psy_audio_MixerSend* send;

				c = psy_tableiterator_key(&it);
				send = (psy_audio_MixerSend*)psy_tableiterator_value(&it);
				sendslot = (send) ? send->slot : 0;
				if (sendslot == outputslot) {
					psy_audio_ReturnChannel* returnchannel;
					psy_audio_mixer_discardsend(self, c);
					returnchannel = psy_table_at(&self->returns, c);
					psy_table_remove(&self->returns, c);
					if (returnchannel) {
						MachineList* path;

						path = psy_audio_compute_path(machines, returnchannel->fxslot, FALSE);
						if (path) {							
							for (; path != 0; path = path->next) {
								uintptr_t slot;

								slot = (size_t)path->entry;
								if (slot == psy_INDEX_INVALID) {
									// delimits the machines that could be processed parallel
									// todo: add thread functions
									continue;
								}
								psy_audio_machines_removemixersend(machines, slot);
							}
							psy_list_free(path);
						}
						returnchannel_dispose(returnchannel);
						free(returnchannel);
					}
					self->maxreturn = returnmax(self);
					break;
				}
			}
		}
	}
}

uintptr_t inputmax(psy_audio_Mixer* self)
{
	psy_TableIterator it;
	uintptr_t rv = 0;

	for (it = psy_table_begin(&self->inputs);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		
		if (rv < psy_tableiterator_key(&it)) {
			rv = psy_tableiterator_key(&it);
		}
	}
	return rv;
}

uintptr_t returnmax(psy_audio_Mixer* self)
{
	psy_TableIterator it;
	uintptr_t rv = 0;

	for (it = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {

		if (rv < psy_tableiterator_key(&it)) {
			rv = psy_tableiterator_key(&it);
		}
	}
	return rv;
}

void mixer_describeeditname(psy_audio_Mixer* self, char* text, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(psy_audio_machine_machines(&self->custommachine.machine),
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
	return (uintptr_t)(numparametercols(self) * (10 + numreturncolumns(self)));
}

uintptr_t numtweakparameters(psy_audio_Mixer* self)
{
	return 0xFF; // psy_audio_machine_numparameters(psy_audio_mixer_base(self));
}

uintptr_t numparametercols(psy_audio_Mixer* self)
{
	return returncolumn(self) + numreturncolumns(self);
}

uintptr_t paramstrobe(const psy_audio_Mixer* self)
{
	return self->strobe;
}

psy_audio_MachineParam* parameter(psy_audio_Mixer* self, uintptr_t param)
{
	uintptr_t col;
	uintptr_t row;

	paramcoords(self, param, &col, &row);
	if (col < mastercolumn(self)) {
		// Description column (col 0)
		if (row > 0 && row <= numreturncolumns(self)) {
			psy_audio_ReturnChannel* channel;

			channel = psy_audio_mixer_return(self, row - 1);
			if (channel) {
				return psy_audio_sendreturnlabelparam_base(&channel->sendlabel_param);
			}
		} else if (row > numreturncolumns(self) && row <= numreturncolumns(self) + 3) {
			if (row == numreturncolumns(self) + 1) {
				return &self->label_mix_param.machineparam;
			} else if (row == numreturncolumns(self) + 2) {
				return &self->label_gain_param.machineparam;
			} else if (row == numreturncolumns(self) + 3) {
				return &self->label_pan_param.machineparam;
			}
		} else if (row > numreturncolumns(self) + 3 && row < numreturncolumns(self) + 9) {
			if (row == numreturncolumns(self) + 8) {				
				return &self->label_ch_input_param.machineparam;
			}
			return &self->ignore_param.machineparam;
		} else {
			return &self->blank_param.machineparam;
		}		
	} else if (col == mastercolumn(self)) {
		// Master Column
		psy_audio_MasterChannel* channel;
		uintptr_t index;

		index = col - inputcolumn(self);
		channel = &self->master;
		if (row == 0) {
			return &channel->info_param.machineparam;
		} else if (row == numreturncolumns(self) + 1) {
			return &channel->dw_param.machineparam;
		} else if (row == numreturncolumns(self) + 2) {
			return &channel->gain_param.machineparam;
		} else if (row == numreturncolumns(self) + 3) {
			return &channel->pan_param.machineparam;
		} else if (row == numreturncolumns(self) + 4) {
			return &channel->slider_param.machineparam;
		} else if (row == numreturncolumns(self) + 5) {
			return &channel->level_param.machineparam;
		} else if (row == numreturncolumns(self) + 6) {
			return &self->ignore_param.machineparam;
		} else if (row == numreturncolumns(self) + 7) {
			return &self->ignore_param.machineparam;
		} else if (row == numreturncolumns(self) + 8) {
			return &self->ignore_param.machineparam;
		} else if (row == numreturncolumns(self) + 9) {
			return &self->ignore_param.machineparam;
		} else if (row == numreturncolumns(self) + 10) {
			return &self->ignore_param.machineparam;
		} else {
			return &self->blank_param.machineparam;
		}
	} else if (col < returncolumn(self)) {
		psy_audio_InputChannel* channel;
		uintptr_t index;

		index = col - inputcolumn(self);
		channel = psy_audio_mixer_channel(self, index);
		if (!channel) {			
			if (row > numreturncolumns(self) + 3 && row < numreturncolumns(self) + 9) {
				return &self->ignore_param.machineparam;
			} else {
				if (row == 0) {
					self->blank_inputinfo_param.machineparam.param0 = index + 1;
					return &self->blank_inputinfo_param.machineparam;
				}
				return &self->blank_param.machineparam;
			}			
		}
		if (row == 0) {			
			return &channel->info_param.machineparam;
		} else if (row > 0 && row <= numreturncolumns(self)) {
			channel->sendvol_param.row = row - 1;
			return &channel->sendvol_param.machineparam;
		} else if (row == numreturncolumns(self) + 1) {
			return &channel->mix_param.machineparam;
		} else if (row == numreturncolumns(self) + 2) {
			return &channel->gain_param.machineparam;
		} else if (row == numreturncolumns(self) + 3) {
			return &channel->pan_param.machineparam;
		} else if (row == numreturncolumns(self) + 4) {
			return &channel->slider_param.machineparam;
		} else if (row == numreturncolumns(self) + 5) {
			return &channel->level_param.machineparam;
		} else if (row == numreturncolumns(self) + 6) {
			self->solo_param.machineparam.param0 = index;
			self->solo_param.machineparam.param1 = psy_INDEX_INVALID;
			return &self->solo_param.machineparam;
		} else if (row == numreturncolumns(self) + 7) {
			self->mute_param.machineparam.param0 = index;
			self->mute_param.machineparam.param1 = psy_INDEX_INVALID;
			return &self->mute_param.machineparam;
		} else if (row == numreturncolumns(self) + 8) {
			self->dry_param.machineparam.param0 = index;
			self->dry_param.machineparam.param1 = psy_INDEX_INVALID;
			return &self->dry_param.machineparam;			
		} else if (row == numreturncolumns(self) + 9) {
			self->wet_param.machineparam.param0 = index;
			self->wet_param.machineparam.param1 = psy_INDEX_INVALID;
			return &self->wet_param.machineparam;			
		} else if (row == numreturncolumns(self) + 10) {
			return &self->ignore_param.machineparam;
		} else {
			return &self->blank_param.machineparam;
		}
	} else if (col < returncolumn(self) + numreturncolumns(self)) {
		psy_audio_ReturnChannel* channel;
		uintptr_t index;

		index = col - returncolumn(self);
		channel = psy_audio_mixer_return(self, index);
		if (channel) {
			if (row == 0) {
				return psy_audio_sendreturnlabelparam_base(&channel->returnlabel_param);
			} else if (row > 1 && row < numreturncolumns(self) + 1) {
				if (index <= row - 2) {
					channel->route_param.send = row - 1;
					return &channel->route_param.machineparam;
				} else {
					return &self->blank_param.machineparam;
				}
			} else if (row == numreturncolumns(self) + 1) {
				channel->route_param.send = row - 1;
				return &channel->route_param.machineparam;
			} else if (row == numreturncolumns(self) + 3) {
				return &channel->pan_param.machineparam;
			} else if (row == numreturncolumns(self) + 4) {
				return &channel->slider_param.machineparam;
			} else if (row == numreturncolumns(self) + 5) {
				return &channel->level_param.machineparam;
			} else if (row == numreturncolumns(self) + 6) {
				self->solo_param.machineparam.param0 = psy_INDEX_INVALID;
				self->solo_param.machineparam.param1 = index;
				return &self->solo_param.machineparam;
			} else if (row == numreturncolumns(self) + 7) {
				self->mute_param.machineparam.param0 = psy_INDEX_INVALID;
				self->mute_param.machineparam.param1 = index;
				return &self->mute_param.machineparam;				
			} else if (row >= numreturncolumns(self) + 8) {
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

	uintptr_t digitx0;
	uintptr_t digit0x;
	
	digitx0 = param / 16;
	digit0x = param % 16;
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
		} else if (digit0x <= 0x0C) {
			// 1..C->Input volumes
			psy_audio_InputChannel* channel;

			channel = psy_audio_mixer_channel(self, param - 1);
			if (channel) {
				rv = psy_audio_volumemachineparam_base(&channel->slider_param);
			}
		} else if (digit0x == 0x0D) {
			// D->master drywetmix.
			// rv = psy_audio_drywetmixmachineparam_base(&self->master.drywetmix_param);
			rv = psy_audio_floatmachineparam_base(&self->master.dw_param);
		} else if (digit0x == 0x0E) {
			//  E->master gain.
			rv = psy_audio_gainmachineparam_base(&self->master.gain_param);
		} else if (digit0x == 0x0F) {
			// F->master pan.
			rv = psy_audio_floatmachineparam_base(&self->master.pan_param);
		}
	} else if (digitx0 >= 0x1 && digitx0 <= 0xC) {
		psy_audio_InputChannel* channel;

		channel = psy_audio_mixer_channel(self, digitx0 - 1);
		if (channel) {
			if (digit0x == 0x0) {
				//  0->input wet mix.
				rv = psy_audio_floatmachineparam_base(&channel->mix_param);
			} else if (digit0x >= 0x1 && digit0x <= 0xC) {
				// input send amout to the send x.
				channel->sendvol_param.index = digitx0 - 1;
				rv = psy_audio_custommachineparam_base(&channel->sendvol_param);
			} else if (digit0x == 0xD) {
				// D->input drywetmix. (0 normal, 1 dryonly, 2 wetonly  3 mute)
				self->dry_tweak_param.machineparam.param0 = digitx0 - 1;
				rv = &self->dry_tweak_param.machineparam;				
			} else if (digit0x == 0xE) {
				// E->input gain.
				rv = psy_audio_custommachineparam_base(&channel->gain_param);
			} else if (digit0x == 0xF) {
				// F->input panning.
				rv = psy_audio_floatmachineparam_base(&channel->pan_param);
			}
		}
	} else if (digitx0 == 0xD) {
		if (digit0x == 0) {
			return &self->solo_tweak_param.machineparam;
		} else {
			self->returngrid_tweak_param.machineparam.param0 = digit0x;
			return &self->returngrid_tweak_param.machineparam;
		}
	} else if (digitx0 == 0xE) {
		// 1..C -> return volumes
		if (digit0x >= 0x1 && digit0x <= 0xC) {
			psy_audio_ReturnChannel* channel;

			channel = psy_audio_mixer_return(self, digitx0 - 1);
			if (channel) {
				// rv = &channel->volume_param
			}
		}
	} else if (digitx0 == 0xF) {
		// 1..C -> return panning
		if (digit0x >= 0x1 && digit0x <= 0xC) {
			psy_audio_ReturnChannel* channel;

			channel = psy_audio_mixer_return(self, digitx0 - 1);
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

int loadspecific(psy_audio_Mixer* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t filesize;
	int32_t numins = 0;
	uint32_t numrets = 0;
	uint32_t i;
	int32_t solocolumn;
	int status;

	if ((status = psyfile_read(songfile->file, &filesize, sizeof(filesize)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &solocolumn, sizeof(solocolumn)))) {
		return status;
	}	
	if (solocolumn == -1) {		
		self->inputsolo = psy_INDEX_INVALID;
		self->returnsolo = psy_INDEX_INVALID;
	} else if (solocolumn >= 0 && solocolumn < 0x0D) {
		self->inputsolo = (uintptr_t)solocolumn;
	} else {
		self->returnsolo = (uintptr_t)solocolumn - 0x0D;
	}	
	if ((status = psyfile_read(songfile->file, &self->master.volume, sizeof(float)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &self->master.gain, sizeof(float)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &self->master.drymix, sizeof(float)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &numins, sizeof(int32_t)))) {
		return status;
	}
	if ((status = psyfile_read(songfile->file, &numrets, sizeof(int32_t)))) {
		return status;
	}
	self->custommachine.slot = slot;
	
	for (i = 0; i < (uint32_t) numins; ++i) {
		psy_audio_InputChannel* channel;
		unsigned int j;
		float temp;
		uint8_t temp8;

		channel = psy_audio_mixer_insertchannel(self, i,
			inputchannel_allocinit(self, i, -1));		
		for (j = 0; j < numrets; ++j) {
			float send = 0.0f;

			if ((status = psyfile_read(songfile->file, &send, sizeof(float)))) {
				return status;
			}
			if (channel) {
				psy_table_insert(&channel->sendvols, j, (void*)(intptr_t)(send * 0xFF));
			}
		}
		if ((status = psyfile_read(songfile->file, &temp, sizeof(float)))) {
			return status;
		}
		channel->volume = temp;		
		if ((status = psyfile_read(songfile->file, &temp, sizeof(float)))) {
			return status;
		}
		channel->panning = temp;		
		if ((status = psyfile_read(songfile->file, &temp, sizeof(float)))) {
			return status;
		}
		channel->drymix = temp;		
		if ((status = psyfile_read(songfile->file, &temp8, sizeof(uint8_t)))) {
			return status;
		}
		channel->mute = temp8;		
		if ((status = psyfile_read(songfile->file, &temp8, sizeof(uint8_t)))) {
			return status;
		}
		channel->dryonly = temp8;		
		if ((status = psyfile_read(songfile->file, &temp8, sizeof(uint8_t)))) {
			return status;
		}
		channel->wetonly = temp8;		
	}
	if (numins > 0) {
		self->maxinput = numins - 1;
	}
	
	for (i = 0; i < numrets; ++i) {
		unsigned int j;
		psy_audio_ReturnChannel* channel;

		channel = psy_audio_mixer_insertreturn(self, i,
			returnchannel_allocinit(self, i, -1));
		{
			int inputmachine;
			float inputconvol;
			float wiremultiplier;

			// Incoming (Return) connections psy_audio_Machine number
			if ((status = psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine)))) {
				return status;
			}
			// volume value for the current return wire. Range 0.0..1.0. (As
			// opposed to the standard wires)
			if ((status = psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol)))) {
				return status;
			}
			// Ignore. (value to divide returnVolume for work. The reason is
			// because natives output at -32768.0f..32768.0f range )
			if ((status = psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier)))) {
				return status;
			}
			//bugfix on 1.10.1 alpha
			if (inputconvol > 8.0f) {
				inputconvol /= 32768.f;
			}
			psy_table_insert(&self->legacyreturn_, i,
				psy_audio_legacywire_allocinit_all(
					inputmachine, TRUE, inputconvol,
					wiremultiplier, (int)slot, TRUE
			));
			psy_audio_mixer_insertsend(self, i,
				psy_audio_mixersend_allocinit(inputmachine));
			channel->fxslot = inputmachine;			
			psy_table_insert(&songfile->song->machines.connections.sends,
				inputmachine, (void*)1);
		}
		{			
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			
			// Outgoing (Send) connections psy_audio_Machine number
			if ((status = psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine)))) {

			}
			// volume value for the current send wire. Range 0.0..1.0. (As
			// opposed to the standard wires)
			if ((status = psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol)))) {
				return status;
			}
			// Ignore. (value to divide returnVolume for work. The reason is
			// because natives output at -32768.0f..32768.0f range )
			if ((status = psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier)))) {
				return status;
			}
			// bugfix on 1.10.1 alpha
			if (inputconvol > 0.f && inputconvol < 0.0002f) {
				inputconvol *= 32768.f;
			}					
			psy_table_insert(&self->legacysend_, i,
				psy_audio_legacywire_allocinit_all(
					inputmachine, TRUE, inputconvol,
					wiremultiplier, (int)slot, TRUE
			));
		}
		for (j = 0; j < numrets; ++j) {
			uint8_t send = 0;

			if ((status = psyfile_read(songfile->file, &send, sizeof(uint8_t)))) {
				return status;
			}
			if (send != 0) {
				psy_table_insert(&channel->sendsto, j, (void*)(uintptr_t)TRUE);
			}
		}
		{
			// Return(i)
			unsigned char mastersend;
			float volume;
			float panning;
			uint8_t mute;

			if ((status = psyfile_read(songfile->file, &mastersend, sizeof(uint8_t)))) {
				return status;
			}
			if ((status = psyfile_read(songfile->file, &volume, sizeof(float)))) {
				return status;
			}
			if ((status = psyfile_read(songfile->file, &panning, sizeof(float)))) {
				return status;
			}
			if ((status = psyfile_read(songfile->file, &mute, sizeof(uint8_t)))) {
				return status;
			}
			if (channel) {
				channel->mastersend = mastersend;
				channel->volume = volume;
				channel->panning = panning;
				channel->mute = mute;
			}
		}
	}
	if (numrets > 0) {
		self->maxreturn = numrets - 1;
	}
	return PSY_OK;
}

int savespecific(psy_audio_Mixer* self, struct psy_audio_SongFile* songfile,
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
	int32_t solocolumn;
	psy_audio_InputChannel emptyinput;
	psy_audio_ReturnChannel emptyreturn;
	int status;

	size = (sizeof(solocolumn) + sizeof(volume_) + sizeof(drywetmix_) + sizeof(gain_) +
		2 * sizeof(uint32_t));
	size += (3 * sizeof(float) + 3 * sizeof(unsigned char) + (uint32_t)self->sends.count * sizeof(float)) * (uint32_t)self->inputs.count;
	size += (2 * sizeof(float) + 2 * sizeof(unsigned char) + (uint32_t)self->sends.count * sizeof(unsigned char) + 2 * sizeof(float) + sizeof(uint32_t)) * (uint32_t)self->returns.count;
	size += (2 * sizeof(float) + sizeof(uint32_t)) * (uint32_t)self->sends.count;
	if ((status = psyfile_write(songfile->file, &size, sizeof(size)))) {
		return status;
	}
	if (self->returnsolo == psy_INDEX_INVALID && self->inputsolo == psy_INDEX_INVALID) {
		solocolumn = -1;
	} else {
		if (self->inputsolo != psy_INDEX_INVALID) {
			solocolumn = (int32_t)self->inputsolo;
		} else {
			solocolumn = (int32_t)self->returnsolo + 0xD;
		}
	}
	if ((status = psyfile_write(songfile->file, &solocolumn, sizeof(solocolumn)))) {
		return status;
	}
	if ((status = psyfile_write(songfile->file, &self->master.volume, sizeof(float)))) {
		return status;
	}
	if ((status = psyfile_write(songfile->file, &self->master.gain, sizeof(float)))) {
		return status;
	}
	if ((status = psyfile_write(songfile->file, &self->master.drymix, sizeof(float)))) {
		return status;
	}
	numins = (uint32_t)numinputcolumns(self);
	numrets = (uint32_t)numreturncolumns(self);
	psyfile_write(songfile->file, &numins, sizeof(int32_t));
	psyfile_write(songfile->file, &numrets, sizeof(int32_t));
	inputchannel_init(&emptyinput, 0, self, 0);
	for (i = 0; i < (uint32_t) numins; ++i) {
		psy_audio_InputChannel* channel;		

		channel = psy_audio_mixer_channel(self, i);
		if (!channel) {
			channel = &emptyinput;
		}
		for (j = 0; j < self->sends.count; ++j) {
			float sendvol;

			sendvol = (int)(intptr_t)psy_table_at(&channel->sendvols, j) / (psy_dsp_amp_t) 0xFF;
			if ((status = psyfile_write(songfile->file, &sendvol, sizeof(float)))) {
				return status;
			}
		}
		if ((status = psyfile_write(songfile->file, &channel->volume, sizeof(float)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->panning, sizeof(float)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->drymix, sizeof(float)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->mute, sizeof(uint8_t)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->dryonly, sizeof(uint8_t)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->wetonly, sizeof(uint8_t)))) {
			return status;
		}
	}
	returnchannel_init(&emptyreturn, self, 0, 0);
	for (i = 0; i < (uint32_t) numrets; ++i) {
		float volume, volMultiplier;
		uint32_t wMacIdx;
		psy_audio_ReturnChannel* channel;

		channel = psy_audio_mixer_return(self, i);
		if (!channel) {
			channel = &emptyreturn;
		}
		//Returning machines and values
		//const psy_audio_Wire& wireRet = Return(i).GetWire();
		//wMacIdx = (wireRet.Enabled()) ? wireRet.GetSrcMachine()._macIndex : -1;
		//volume = wireRet.GetVolume();
		volMultiplier = 1.0f; // wireRet.GetVolMultiplier();
		// Incoming connections psy_audio_Machine number
		if ((status = psyfile_write_int32(songfile->file, (int32_t)channel->fxslot))) {
			return status;
		}
		// Incoming connections psy_audio_Machine vol
		if ((status = psyfile_write_float(songfile->file, channel->volume))) {
			return status;
		}
		// Value to multiply input_convol[] to have a 0.0...1.0 range
		if ((status = psyfile_write_float(songfile->file, volMultiplier))) {
			return status;
		}
		//Sending machines and values
		if (psy_table_exists(&channel->sendsto, i)) {
			psy_audio_ReturnChannel* sendto;

			sendto = psy_audio_mixer_return(self, i);
			if (sendto) {
				wMacIdx = (uint32_t)sendto->fxslot;
				volume = sendto->volume;
			} else {
				wMacIdx = -1;
				volume = 1.f;
			}
		}
		else {
			wMacIdx = -1;
			volume = 1.0f;
		}
		volMultiplier = 1.0f;
		// send connections psy_audio_Machine number
		if ((status = psyfile_write(songfile->file, &wMacIdx, sizeof(int)))) {
			return status;
		}
		// send connections psy_audio_Machine vol
		if ((status = psyfile_write(songfile->file, &volume, sizeof(float)))) {
			return status;
		}
		// Value to multiply input_convol[] to have a 0.0...1.0 range
		if ((status = psyfile_write(songfile->file, &volMultiplier, sizeof(float)))) {
			return status;
		}

		//Rewiring of returns to sends and mix values
		for (j = 0; j < numreturncolumns(self); j++)
		{
			unsigned char send;

			send = psy_table_exists(&channel->sendsto, j);
			psyfile_write(songfile->file, &send, sizeof(uint8_t));
		}
		if ((status = psyfile_write(songfile->file, &channel->mastersend, sizeof(uint8_t)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->volume, sizeof(float)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->panning, sizeof(float)))) {
			return status;
		}
		if ((status = psyfile_write(songfile->file, &channel->mute, sizeof(uint8_t)))) {
			return status;
		}
	}
	inputchannel_dispose(&emptyinput);
	returnchannel_dispose(&emptyreturn);
	return PSY_OK;
}

void postload(psy_audio_Mixer* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	psy_TableIterator it;

	postload_super(psy_audio_mixer_base(self), songfile, slot);
	postloadinputchannels(self, songfile, slot);
	postreturnchannels(self, songfile, slot);

	for (it = psy_table_begin(&self->sends);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		uintptr_t c;
		MachineList* path;
		uintptr_t sendslot;
		psy_audio_MixerSend* send;

		c = psy_tableiterator_key(&it);
		send = (psy_audio_MixerSend*)psy_tableiterator_value(&it);
		sendslot = (send) ? send->slot : 0;
		psy_audio_machines_disconnect(psy_audio_machine_machines(
			&self->custommachine.machine),
			psy_audio_wire_make(slot, sendslot));
		path = psy_audio_compute_path(psy_audio_machine_machines(
			&self->custommachine.machine), sendslot, FALSE);
		if (path) {
			// work fx chain
			for (; path != 0; path = path->next) {				
				sendslot = (size_t)path->entry;
				if (sendslot == psy_INDEX_INVALID) {
					// delimits the machines that could be processed parallel					
					continue;
				}
				psy_audio_machines_disconnect(psy_audio_machine_machines(
					&self->custommachine.machine),
					psy_audio_wire_make(slot, sendslot));
				psy_audio_machines_addmixersend(psy_audio_machine_machines(
					&self->custommachine.machine), sendslot);				
			}
			psy_list_free(path);
		}
	}
	psy_signal_connect(&songfile->song->machines.connections.signal_connected,
		self, onconnected);
	psy_signal_connect(&songfile->song->machines.connections.signal_disconnected,
		self, ondisconnected);
}

void postloadinputchannels(psy_audio_Mixer* self, psy_audio_SongFile* songfile, uintptr_t slot)
{
	uintptr_t c;
	psy_Table* legacywiretable;

	legacywiretable = psy_audio_legacywires_at(songfile->legacywires, slot);
	if (!legacywiretable) {
		return;
	}

	for (c = 0; c < numinputcolumns(self); ++c) {
		psy_audio_LegacyWire* wire;
		psy_audio_Machine* inputmachine;
		uintptr_t f;

		wire = psy_table_at(legacywiretable, c);
		if (!wire) {
			psy_audio_mixer_discardchannel(self, c);
			continue;
		}
		//load bugfix: Ensure no duplicate wires could be created.
		for (f = 0; f < c; f++) {
			psy_audio_LegacyWire* legacywire;

			legacywire = psy_table_at(legacywiretable, f);
			if (!legacywire) {
				continue;
			}
			if (wire->input_con && legacywire->input_con &&
				wire->input_machine == legacywire->input_machine) {
				wire->input_con = FALSE;
			}
		}
		inputmachine = psy_audio_machines_at(&songfile->song->machines, wire->input_machine);
		if (wire->input_con
			&& wire->input_machine >= 0 && wire->input_machine < MAX_MACHINES
			&& slot != wire->input_machine && inputmachine)
		{
			//Do not create the hidden wire from mixer send to the send machine.
			int outWire = psy_audio_legacywires_findlegacyoutput(
				songfile->legacywires, wire->input_machine, (int)slot);
			if (outWire != -1) {	
				psy_audio_InputChannel* channel;				

				//if (wire.pinMapping.size() > 0) {
				//	inWires[c].ConnectSource(*_pMachine[wire.input_machine], 0
				//		, FindLegacyOutput(_pMachine[wire.input_machine], _macIndex)
				//		, &wire.pinMapping);
				//} else {
				//	inWires[c].ConnectSource(*_pMachine[wire.input_machine], 0
				//		, FindLegacyOutput(_pMachine[wire.input_machine], _macIndex));
				//}
				//while (wire->input_convol * wire->wire_multiplier > 8.0f) { //psycle 1.10.1 alpha bugfix
					//wire->input_convol /= 32768.f;
				//}
				//while (wire->input_convol > 0.f && wire->input_convol * wire->wire_multiplier < 0.0002f) { //psycle 1.10.1 alpha bugfix
					//wire->input_convol *= 32768.f;
				//}
				//inWires[c].SetVolume(wire.input_convol * wire.wire_multiplier);
				psy_audio_machines_connect(&songfile->song->machines,
					psy_audio_wire_make(wire->input_machine, slot));
				psy_audio_connections_setwirevolume(&songfile->song->machines.connections,
					psy_audio_wire_make(wire->input_machine, slot),
					wire->input_convol * wire->wire_multiplier);
				channel = psy_audio_mixer_channel(self, c);
				channel->inputslot = wire->input_machine;
					
			}
		} else {
			psy_audio_mixer_discardchannel(self, c);
		}
	}
}

void postreturnchannels(psy_audio_Mixer* self, psy_audio_SongFile* songfile, uintptr_t slot)
{
	uintptr_t j;

	for (j = 0; j <= numreturncolumns(self); ++j)
	{
		psy_audio_Machine* inputmachine;
		psy_audio_LegacyWire* wire = (psy_audio_LegacyWire*) psy_table_at(&self->legacyreturn_, j);
		if (!wire) {
			psy_audio_mixer_discardreturn(self, j);
			continue;
		}
		//inputCon is not used in legacyReturn.
		inputmachine = psy_audio_machines_at(&songfile->song->machines, wire->input_machine);
		if (wire->input_machine >= 0 && wire->input_machine < MAX_MACHINES
			&& slot != wire->input_machine && inputmachine)
		{
			psy_audio_ReturnChannel* channel;
			psy_audio_LegacyWire* wire2;

			channel = psy_audio_mixer_return(self, j);
			if (channel) {
				channel->fxslot = wire->input_machine;
			}
			psy_audio_machines_connect(&songfile->song->machines,
				psy_audio_wire_make(wire->input_machine, slot));
			psy_audio_connections_setwirevolume(&songfile->song->machines.connections,
				psy_audio_wire_make(wire->input_machine, slot),
				wire->input_convol * wire->wire_multiplier);
			/*if (wire.pinMapping.size() > 0) {
				Return(j).GetWire().ConnectSource(*_pMachine[wire.input_machine], 1
					, FindLegacyOutput(_pMachine[wire.input_machine], _macIndex)
					, &wire.pinMapping);
			} else {
				Return(j).GetWire().ConnectSource(*_pMachine[wire.input_machine], 1
					, FindLegacyOutput(_pMachine[wire.input_machine], _macIndex));
			}
			Return(j).GetWire().SetVolume(wire.input_convol);
			*/
			wire2 = (psy_audio_LegacyWire*) psy_table_at(&self->legacysend_, j);
			if (wire2) {
				psy_audio_MixerSend* send;

				send = psy_audio_mixer_Send(self, j);
				if (send) {
					send->inputconvol = wire2->input_convol;
				}				
				//if (wire2.pinMapping.size() > 0) {
					//Send(j).ChangeMapping(wire2.pinMapping);
				//}
			}
		} else{
			psy_audio_mixer_discardreturn(self, j);
		}
	}
	psy_audio_mixer_clearlegacywires(self);
}

void psy_audio_mixer_clearlegacywires(psy_audio_Mixer* self)
{
	{ // return
		psy_table_dispose_all(&self->legacyreturn_, (psy_fp_disposefunc)NULL);	
		psy_table_init(&self->legacyreturn_);
	}
	{ // send
		psy_table_dispose_all(&self->legacysend_, (psy_fp_disposefunc) NULL);
		psy_table_init(&self->legacysend_);
	}
}

psy_audio_InputChannel* psy_audio_mixer_insertchannel(psy_audio_Mixer* self, uintptr_t idx,
	psy_audio_InputChannel* input)
{
	psy_audio_mixer_discardchannel(self, idx);
	psy_table_insert(&self->inputs, idx, input);
	self->maxinput = inputmax(self);
	++self->strobe;
	return input;
}

psy_audio_ReturnChannel* psy_audio_mixer_insertreturn(psy_audio_Mixer* self, uintptr_t idx,
	psy_audio_ReturnChannel* retchan)
{
	psy_audio_mixer_discardreturn(self, idx);
	psy_table_insert(&self->returns, idx,
		retchan);
	self->maxreturn = returnmax(self);
	++self->strobe;
	return retchan;
}

psy_audio_MixerSend* psy_audio_mixer_insertsend(psy_audio_Mixer* self, uintptr_t idx,
	psy_audio_MixerSend* send)
{
	psy_audio_mixer_discardsend(self, idx);
	psy_table_insert(&self->sends, idx, (void*)send);
	++self->strobe;
	return send;
}

void psy_audio_mixer_discardchannel(psy_audio_Mixer* self, uintptr_t idx)
{
	psy_audio_InputChannel* channel;

	channel = (psy_audio_InputChannel*) psy_table_at(&self->inputs, idx);
	if (channel) {
		inputchannel_dispose(channel);
		free(channel);
		psy_table_remove(&self->inputs, idx);
		self->maxinput = inputmax(self);
		++self->strobe;
	}
}

void psy_audio_mixer_discardreturn(psy_audio_Mixer* self, uintptr_t idx)
{
	psy_audio_ReturnChannel* channel;

	channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, idx);
	if (channel) {
		returnchannel_dispose(channel);
		free(channel);
		psy_table_remove(&self->returns, idx);
		self->maxreturn = returnmax(self);
		++self->strobe;
	}
}

void psy_audio_mixer_discardsend(psy_audio_Mixer* self, uintptr_t idx)
{
	psy_audio_MixerSend* send;

	send = (psy_audio_MixerSend*) psy_table_at(&self->sends, idx);
	if (send) {
		// mixersend_dispose(send);
		free(send);
		psy_table_remove(&self->sends, idx);
		++self->strobe;
	}
}

// tweak invoked by the paramview ui
void mixer_solo_tweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{	
	if (value == 0) {
		self->inputsolo = psy_INDEX_INVALID;
		self->returnsolo = psy_INDEX_INVALID;
	} else {			
		self->inputsolo = sender->machineparam.param0;
		self->returnsolo = sender->machineparam.param1;			
	}	
}

void mixer_solo_normvalue(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float* rv)
{	
	if (sender == &self->solo_param && (self->inputsolo != psy_INDEX_INVALID ||
			self->returnsolo != psy_INDEX_INVALID)) {
		*rv = (sender->machineparam.param0 == self->inputsolo &&
			   sender->machineparam.param1 == self->returnsolo)
			? 1.f
			: 0.f;
		return;
	}
	*rv = 0.f;
}

// tweak invoked by the paramview ui
void mixer_mute_tweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{		
	if (sender->machineparam.param0 != psy_INDEX_INVALID) {
		psy_audio_InputChannel* channel;

		channel = psy_audio_mixer_channel(self, 
			sender->machineparam.param0);
		if (channel) {
			channel->mute = value != 0.0;
		}
	} else {
		psy_audio_ReturnChannel* channel;

		channel = psy_audio_mixer_return(self,
			sender->machineparam.param1);
		if (channel) {
			channel->mute = value != 0.0;
		}
	}	
}

void mixer_mute_normvalue(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float* rv)
{	
	if (sender->machineparam.param0 != psy_INDEX_INVALID) {
		psy_audio_InputChannel* channel;

		channel = psy_audio_mixer_channel(self,
			sender->machineparam.param0);
		if (channel) {
			if (self->returnsolo != psy_INDEX_INVALID) {
				*rv = 1.f;
			} else if (self->inputsolo == psy_INDEX_INVALID) {
				*rv = (channel->mute) ? 1.f : 0.f;
			} else if (self->inputsolo == sender->machineparam.param0) {					
				*rv = 0.f;
			} else if (self->inputsolo != sender->machineparam.param0) {
				*rv = 1.f;
			}
		}
	} else if (sender->machineparam.param1 != psy_INDEX_INVALID) {
		psy_audio_ReturnChannel* channel;

		channel = psy_audio_mixer_return(self,
			sender->machineparam.param1);
		if (channel) {				
			if (self->returnsolo == psy_INDEX_INVALID) {
				*rv = (channel->mute) ? 1.f : 0.f;
			} else if (self->returnsolo == sender->machineparam.param1) {
				*rv = 0.f;
			} else if (self->returnsolo != sender->machineparam.param1) {
				*rv = 1.f;
			}
		}
	}		
	return;
}

void mixer_dry_tweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{
	psy_audio_InputChannel* channel;

	channel = psy_audio_mixer_channel(self, sender->machineparam.param0);
	if (channel) {		
		channel->dryonly = value != 0.f;
		if (channel->dryonly) {
			channel->wetonly = FALSE;
			channel->mute = FALSE;
		}
	}
}

void mixer_dry_normvalue(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	psy_audio_InputChannel* channel;

	channel = psy_audio_mixer_channel(self,
		sender->machineparam.param0);
	if (channel) {
		*rv = (channel->dryonly) ? 1.f : 0.f;
		return;
	}
	*rv = 0.f;
}

void mixer_wet_tweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{
	psy_audio_InputChannel* channel;

	channel = psy_audio_mixer_channel(self, sender->machineparam.param0);
	if (channel) {
		channel->wetonly = value != 0.f;
		if (channel->wetonly) {
			channel->dryonly = FALSE;
			channel->mute = FALSE;
		}
	}
}

void mixer_wet_normvalue(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	psy_audio_InputChannel* channel;

	channel = psy_audio_mixer_channel(self,
		sender->machineparam.param0);
	if (channel) {		
		*rv = (channel->wetonly) ? 1.f : 0.f;
		return;
	}
	*rv = 0.f;	
}

// pattern tweak
void mixer_solo_patterntweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{	
	// tweak invoked by the pattern
	uintptr_t scaled;

	scaled = (uintptr_t)(value * 0xFF);
	if (scaled == 0) {
		self->inputsolo = psy_INDEX_INVALID;
		self->returnsolo = psy_INDEX_INVALID;
	} else if (scaled < 0xD) {
		self->inputsolo = scaled - 1;
		self->returnsolo = psy_INDEX_INVALID;
	} else {
		self->inputsolo = psy_INDEX_INVALID;
		self->returnsolo = scaled - 0xD;
	}
}

void mixer_mute_patterntweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{	
	if (sender->machineparam.param0 != psy_INDEX_INVALID) {
		psy_audio_InputChannel* channel;

		channel = psy_audio_mixer_channel(self,
			sender->machineparam.param0);
		if (channel) {
			channel->mute = value != 0.0;
		}
	} else {
		psy_audio_ReturnChannel* channel;

		channel = psy_audio_mixer_return(self,
			sender->machineparam.param1);
		if (channel) {
			channel->mute = value != 0.0;
		}
	}
}

void mixer_dry_patterntweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{
	psy_audio_InputChannel* channel;
	uintptr_t scaled;

	scaled = (uintptr_t)(value * 0xFF);
	channel = psy_audio_mixer_channel(self,
		sender->machineparam.param0);
	if (channel) {
		channel->dryonly = FALSE;
		channel->wetonly = FALSE;
		channel->mute = FALSE;
		if (scaled == 0) {			
		} else if (scaled == 1) {
			channel->dryonly = TRUE;
		} else if (scaled == 2) {
			channel->wetonly = TRUE;
		} else if (scaled == 3) {
			channel->mute = TRUE;
		}		
	}
}

// tweak invoked by the pattern
void mixer_returngrid_tweak(psy_audio_Mixer* self,
	psy_audio_IntMachineParam* sender, float value)
{
	psy_audio_ReturnChannel* channel;
	uintptr_t scaled;

	scaled = (uintptr_t)(value * 0xFF);	
	channel = psy_audio_mixer_return(self, sender->machineparam.param0 - 1);
	if (channel) {
		uintptr_t i;
		// the return grid array grid represents:
		// bit0 -> mute, bit 1..12 routing to send. bit 13 -> route to master
		channel->mute = (scaled & 1) ? TRUE : FALSE;
		psy_table_clear(&channel->sendsto);		
		for (i = sender->machineparam.param0; i < 11; ++i) {
			if (scaled & (2 << i)) {
				psy_table_insert(&channel->sendsto, i, (void*)(uintptr_t)TRUE);
			}				
		}
		channel->mastersend = (scaled & (1 << 13)) ? TRUE : FALSE;		
	}	
}
