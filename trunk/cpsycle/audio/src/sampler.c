// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampler.h"
// audio
#include "constants.h"
#include "instruments.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "songio.h"
#include "samples.h"
// dsp
#include <noteperiods.h>
#include <operations.h>
// std
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
// platform
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

static int psy_audio_sampler_getdelta(psy_audio_WaveForms wavetype,
	int wavepos)
{
	switch (wavetype) {
		case psy_audio_WAVEFORMS_SAWDOWN:
			return m_FineRampDownData[wavepos];
			break;
		case psy_audio_WAVEFORMS_SAWUP:
			return m_FineRampDownData[0xFF - wavepos];
			break;
		case psy_audio_WAVEFORMS_SQUARE:
			return m_FineSquareTable[wavepos];
			break;
		case psy_audio_WAVEFORMS_RANDOM:
			return m_RandomTable[wavepos];
			break;
		case psy_audio_WAVEFORMS_SINUS:
		default:
			return m_FineSineData[wavepos];
			break;
	}
}

static void generateaudio(psy_audio_Sampler*, psy_audio_BufferContext*);
static void seqtick(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void newline(psy_audio_Sampler*);
static psy_List* sequencerinsert(psy_audio_Sampler*, psy_List* events);
static void stop(psy_audio_Sampler*);
static const psy_audio_MachineInfo* info(psy_audio_Sampler*);
static uintptr_t numparametercols(psy_audio_Sampler*);
static uintptr_t numparameters(psy_audio_Sampler*);
static psy_audio_MachineParam* parameter(psy_audio_Sampler*,
	uintptr_t param);
static void dispose(psy_audio_Sampler*);
static void disposecmds(psy_audio_Sampler*);
static void disposevoices(psy_audio_Sampler*);
static void disposechannels(psy_audio_Sampler*);
static void disposeparameters(psy_audio_Sampler*);
static int alloc_voice(psy_audio_Sampler*);
static void releaseallvoices(psy_audio_Sampler*);
static psy_audio_SamplerVoice* activevoice(psy_audio_Sampler*,
	uintptr_t channel);
static void releasevoices(psy_audio_Sampler*, uintptr_t channel);
static void nnavoices(psy_audio_Sampler*, uintptr_t channel);
static void removeunusedvoices(psy_audio_Sampler* self);
static uintptr_t numinputs(psy_audio_Sampler*);
static uintptr_t numoutputs(psy_audio_Sampler*);
static void loadspecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static psy_audio_SamplerChannel* sampler_channel(psy_audio_Sampler*,
	uintptr_t channelnum);
static psy_audio_InstrumentIndex currslot(psy_audio_Sampler*,
	uintptr_t channel, const psy_audio_PatternEvent*);
// configuration
static void psy_audio_sampler_initps1(psy_audio_Sampler*);
static void psy_audio_sampler_initsampulse(psy_audio_Sampler*);
static void psy_audio_sampler_addcmd(psy_audio_Sampler*, int cmd,
	int patternid, int mask);
static void psy_audio_sampler_updatecmdmap(psy_audio_Sampler*);

static psy_audio_MachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64 | MACH_SUPPORTS_INSTRUMENTS,
	MACHMODE_GENERATOR,
	"Sampler"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampler",
	"Psycledelics",
	"help",	
	MACH_XMSAMPLER,
	NULL,		// NO MODULPATH
	0,			// shellidx	
	SAMPLERHELP	// help text
};

const psy_audio_MachineInfo* psy_audio_sampler_info(void)
{
	return &macinfo;
}

static void psy_audio_sampler_initparameters(psy_audio_Sampler*);
static void resamplingmethod_tweak(psy_audio_Sampler*,
	psy_audio_ChoiceMachineParam* sender, float value);
static void psy_audio_sampler_ontimertick(psy_audio_Sampler*);
static void psy_audio_sampler_ontimerwork(psy_audio_Sampler*,
	psy_audio_BufferContext*);

static MachineVtable sampler_vtable;
static int sampler_vtable_initialized = 0;

static void sampler_vtable_init(psy_audio_Sampler* self)
{
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_sampler_base(self)->vtable);
		sampler_vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
		sampler_vtable.seqtick = (fp_machine_seqtick)seqtick;
		sampler_vtable.newline = (fp_machine_newline)newline;
		sampler_vtable.sequencerinsert = (fp_machine_sequencerinsert)
			sequencerinsert;
		sampler_vtable.stop = (fp_machine_stop)stop;
		sampler_vtable.info = (fp_machine_info)info;
		sampler_vtable.dispose = (fp_machine_dispose)dispose;
		sampler_vtable.numinputs = (fp_machine_numinputs)numinputs;
		sampler_vtable.numoutputs = (fp_machine_numoutputs)numoutputs;
		sampler_vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		sampler_vtable.savespecific = (fp_machine_savespecific)savespecific;
		sampler_vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		sampler_vtable.numparameters = (fp_machine_numparameters)numparameters;
		sampler_vtable.parameter = (fp_machine_parameter)parameter;
		sampler_vtable_initialized = 1;
	}
}

void psy_audio_sampler_init(psy_audio_Sampler* self,
	psy_audio_MachineCallback callback)
{
	custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_sampler_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_sampler_base(self), "XMSampler");
	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;
	self->voices = 0;
	self->resamplerquality = RESAMPLERTYPE_LINEAR;
	self->defaultspeed = 1;
	self->maxvolume = 0xFF;
	self->panpersistent = 0;
	self->xmsamplerload = 0;
	self->basec = 48;
	self->clipmax = (psy_dsp_amp_t)4.0f;
	self->instrumentbank = 0;
	self->samplerowcounter = 0;
	self->amigaslides = FALSE;
	self->usefilters = TRUE;
	self->channelbank = 0;
	self->panningmode = psy_audio_PANNING_LINEAR;
	self->samplerowcounter = 0;
	psy_table_init(&self->channels);
	psy_table_init(&self->lastinst);
	psy_audio_sampler_initparameters(self);
	self->cmds = NULL;
	psy_table_init(&self->cmdmap);
	psy_audio_sampler_initsampulse(self);
	if (self->xmsamplerload == FALSE) {
		self->instrumentbank = 0;
	}
	psy_audio_samplerticktimer_init(&self->ticktimer,
		self, // callback context (sampler)
		psy_audio_sampler_ontimertick,
		psy_audio_sampler_ontimerwork);
}

void psy_audio_sampler_initparameters(psy_audio_Sampler* self)
{
	uintptr_t i;

	psy_audio_custommachineparam_init(&self->param_general,
		"General", "General", MPF_HEADER | MPF_SMALL, 0, 0);
	psy_audio_intmachineparam_init(&self->param_numvoices,
		"limit voices", "limit voices", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->numvoices, 1, 64);
	psy_audio_choicemachineparam_init(&self->param_resamplingmethod,
		"resample", "resample", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->resamplerquality, 0, 3);
	psy_signal_connect(&self->param_resamplingmethod.machineparam.signal_tweak, self,
		resamplingmethod_tweak);
	for (i = 0; i < RESAMPLERTYPE_NUMRESAMPLERS; ++i) {
		psy_audio_choicemachineparam_setdescription(&self->param_resamplingmethod, i,
			psy_dsp_multiresampler_name((ResamplerType)i));
	}
	psy_audio_choicemachineparam_init(&self->param_defaultspeed,
		"Default Speed", "Default Speed", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->defaultspeed,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 0,
		"played by C3");
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 1,
		"played by C4");
	psy_audio_choicemachineparam_init(&self->param_amigaslides,
		"slide", "slide", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->amigaslides,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_amigaslides, 0,
		"linear");
	psy_audio_choicemachineparam_setdescription(&self->param_amigaslides, 1,
		"Amiga");
	psy_audio_choicemachineparam_init(&self->param_usefilters,
		"use filters", "use filters", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->usefilters,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_usefilters, 0,
		"disabled");
	psy_audio_choicemachineparam_setdescription(&self->param_usefilters, 1,
		"enabled");
	psy_audio_choicemachineparam_init(&self->param_panningmode,
		"panning", "panning", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->panningmode,
		0, 2);
	psy_audio_choicemachineparam_setdescription(&self->param_panningmode, 0,
		"linear"); // (Cross)
	psy_audio_choicemachineparam_setdescription(&self->param_panningmode, 1,
		"2-sliders"); // ft2
	psy_audio_choicemachineparam_setdescription(&self->param_panningmode, 2,
		"equal pwr"); // power
	psy_audio_custommachineparam_init(&self->param_channels, "Channels",
		"Channels", MPF_HEADER | MPF_SMALL, 0, 0);
	psy_audio_choicemachineparam_init(&self->param_channelview,
		"display", "display", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->channelbank,
		0, 7);
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 0,
		"00 - 07");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 1,
		"08 - 15");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 2,
		"16 - 23");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 3,
		"24 - 31");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 4,
		"32 - 39");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 5,
		"40 - 47");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 6,
		"48 - 55");
	psy_audio_choicemachineparam_setdescription(&self->param_channelview, 7,
		"56 - 64");
	psy_audio_intmachineparam_init(&self->param_maxvolume,
		"Max volume", "Max volume", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->maxvolume,
		0, 255);
	psy_audio_intmachineparam_setmask(&self->param_maxvolume, "%0X");
	psy_audio_choicemachineparam_init(&self->param_panpersistent,
		"Pan Persistence", "Pan Persistence", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->panpersistent,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_panpersistent, 0,
		"reset on new note");
	psy_audio_choicemachineparam_setdescription(&self->param_panpersistent, 1,
		"keep on channel");
	psy_audio_intmachineparam_init(&self->param_instrumentbank,
		"instr. bank", "instr. bank", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->instrumentbank,
		0, 1);
	psy_audio_custommachineparam_init(&self->param_blank, "", "",
		MPF_NULL | MPF_SMALL, 0, 0);
	psy_audio_infomachineparam_init(&self->param_filter_cutoff,
		"Filter Cutoff", "", MPF_SMALL);
	psy_audio_infomachineparam_init(&self->param_filter_res,
		"Filter Q (Res)", "", MPF_SMALL);
	psy_audio_infomachineparam_init(&self->param_pan, "Panning", "",
		MPF_SMALL);
	psy_audio_samplerchannel_init(&self->masterchannel, UINTPTR_MAX);
	psy_audio_custommachineparam_init(&self->ignore_param, "-", "-",
		MPF_IGNORE | MPF_SMALL, 0, 0);
}

void dispose(psy_audio_Sampler* self)
{		
	disposeparameters(self);
	disposevoices(self);
	disposechannels(self);
	disposecmds(self);
	psy_table_dispose(&self->cmdmap);
	custommachine_dispose(&self->custommachine);		
}

void disposeparameters(psy_audio_Sampler* self)
{
	psy_audio_custommachineparam_dispose(&self->param_general);
	psy_audio_intmachineparam_dispose(&self->param_numvoices);
	psy_audio_choicemachineparam_dispose(&self->param_resamplingmethod);
	psy_audio_choicemachineparam_dispose(&self->param_defaultspeed);
	psy_audio_choicemachineparam_dispose(&self->param_amigaslides);
	psy_audio_choicemachineparam_dispose(&self->param_usefilters);
	psy_audio_choicemachineparam_dispose(&self->param_panningmode);
	psy_audio_choicemachineparam_dispose(&self->param_channelview);
	psy_audio_intmachineparam_dispose(&self->param_maxvolume);
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
	psy_audio_custommachineparam_dispose(&self->param_channels);
	psy_audio_custommachineparam_dispose(&self->param_blank);
	psy_audio_infomachineparam_dispose(&self->param_filter_cutoff);
	psy_audio_infomachineparam_dispose(&self->param_filter_res);
	psy_audio_infomachineparam_dispose(&self->param_pan);
	psy_audio_custommachineparam_dispose(&self->ignore_param);
}

void disposevoices(psy_audio_Sampler* self)
{
	psy_list_deallocate(&self->voices, (psy_fp_disposefunc)
		psy_audio_samplervoice_dispose);
}

void disposechannels(psy_audio_Sampler* self)
{
	psy_table_disposeall(&self->channels, (psy_fp_disposefunc)
		psy_audio_samplerchannel_dispose);
	psy_audio_samplerchannel_dispose(&self->masterchannel);
}

void disposecmds(psy_audio_Sampler* self)
{
	psy_list_deallocate(&self->cmds, (psy_fp_disposefunc)
		psy_audio_samplercmd_dispose);	
}

psy_audio_Sampler* psy_audio_sampler_alloc(void)
{
	return (psy_audio_Sampler*)malloc(sizeof(psy_audio_Sampler));
}

psy_audio_Sampler* psy_audio_sampler_allocinit(psy_audio_MachineCallback callback)
{
	psy_audio_Sampler* rv;

	rv = psy_audio_sampler_alloc();
	if (rv) {
		psy_audio_sampler_init(rv, callback);
	}
	return rv;
}

psy_audio_SamplerChannel* sampler_channel(psy_audio_Sampler* self, uintptr_t track)
{
	psy_audio_SamplerChannel* rv;

	rv = psy_table_at(&self->channels, track);
	if (rv == NULL) {
		rv = malloc(sizeof(psy_audio_SamplerChannel));
		psy_audio_samplerchannel_init(rv, track);
		psy_table_insert(&self->channels, track, (void*) rv);
	}
	return rv;
}

void generateaudio(psy_audio_Sampler* self, psy_audio_BufferContext* bc)
{	
	self->samplerowcounter += bc->numsamples;
	psy_audio_samplerticktimer_update(&self->ticktimer, bc->numsamples, bc);		
	removeunusedvoices(self);	
	psy_audio_samplerchannel_work(&self->masterchannel, bc);
}

void psy_audio_sampler_ontimertick(psy_audio_Sampler* self)
{
	psy_List* p;
	uintptr_t c = 0;
	psy_TableIterator it;

	// first notify channels
	for (it = psy_table_begin(&self->channels);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_SamplerChannel* channel;

		channel = (psy_audio_SamplerChannel*)psy_tableiterator_value(&it);
		// SetEffect is called by seqtick
		if (psy_audio_samplerticktimer_tickcount(&self->ticktimer) != 0) {
			psy_audio_samplerchannel_performfx(channel);			
		}
	}
	// secondly notify voices
	for (p = self->voices; p != NULL && c < self->numvoices; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)p->entry;
		if (psy_audio_samplerticktimer_tickcount(&self->ticktimer) != 0) {
			psy_audio_samplervoice_performfx(voice);
		}
	}
}

void psy_audio_sampler_ontimerwork(psy_audio_Sampler* self,
	psy_audio_BufferContext* bc)
{
	psy_List* p;
	uintptr_t c = 0;
	
	// psy_audio_buffer_clearsamples(bc->output, bc->numsamples);
	for (p = self->voices; p != NULL && c < self->numvoices;
			psy_list_next(&p), ++c) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)p->entry;
		if (psy_audio_samplervoice_isplaying(voice)) {
			psy_audio_samplervoice_work(voice, bc->output,
				bc->numsamples);
		}
	}
}

void seqtick(psy_audio_Sampler* self, uintptr_t channelnum,
	const psy_audio_PatternEvent* ev)
{		
	psy_audio_SamplerVoice* voice = 0;
	psy_audio_SamplerChannel* channel = 0;	
	psy_audio_PatternEvent event;

	assert(ev);
	if (ev == NULL) {
		return;
	}
	event = *ev;
	if (event.cmd == SAMPLER_CMD_EXTENDED) {
		if ((event.parameter & 0xF0) == SAMPLER_CMD_E_NOTE_DELAY) {
			// skip for now and reinsert in sequencerinsert
			// with delayed offset
			return;
		}
	}	
	channel = sampler_channel(self, channelnum);
	if (channel) {
		psy_audio_samplerchannel_seteffect(channel, &event);
	}
	if (event.note == NOTECOMMANDS_RELEASE) {
		releasevoices(self, channelnum);
		return;
	}
	if (event.note < NOTECOMMANDS_RELEASE) {
		nnavoices(self, channelnum);
	} else {
		voice = activevoice(self, channelnum);
	}
	if (!voice) {
		psy_audio_Instrument* instrument;
		
		instrument = instruments_at(psy_audio_machine_instruments(
			psy_audio_sampler_base(self)),
			currslot(self, channelnum, &event));
		if (instrument) {
			voice = psy_audio_samplervoice_allocinit(self, instrument,
				channel,
				channelnum,
				psy_audio_machine_samplerate(psy_audio_sampler_base(self)));
			psy_list_append(&self->voices, voice);
		}
	}	
	if (voice) {
		psy_audio_samplervoice_seqtick(voice, &event,
			1 / psy_audio_machine_beatspersample(
				psy_audio_sampler_base(self)));
	}
}

void newline(psy_audio_Sampler* self)
{
	psy_List* p;
	psy_TableIterator it;
	
	self->samplerowcounter = 0;
	// first notify channels
	for (it = psy_table_begin(&self->channels);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_SamplerChannel* channel;

		channel = (psy_audio_SamplerChannel*)psy_tableiterator_value(&it);		
		psy_audio_samplerchannel_newline(channel);		
	}
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;	
		if (psy_audio_samplervoice_isplaying(voice)) {
			psy_audio_samplervoice_newline(voice);
		}
	}
	psy_audio_samplerticktimer_reset(&self->ticktimer,	
		(uintptr_t)
		psy_audio_machine_samplespertick(psy_audio_sampler_base(self)));
}

void stop(psy_audio_Sampler* self)
{
	psy_TableIterator it;

	releaseallvoices(self);
	// first notify channels
	for (it = psy_table_begin(&self->channels);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_SamplerChannel* channel;

		channel = (psy_audio_SamplerChannel*)psy_tableiterator_value(&it);
		psy_audio_samplerchannel_restore(channel);
	}
}

psy_audio_InstrumentIndex currslot(psy_audio_Sampler* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	int rv;

	if (event->inst != NOTECOMMANDS_EMPTY) {
		psy_table_insert(&self->lastinst, channel, (void*)(uintptr_t)event->inst);
		rv = event->inst;
	} else
	if (psy_table_exists(&self->lastinst, channel)) {
		rv = (int)(uintptr_t) psy_table_at(&self->lastinst, channel);
	} else { 
		rv = NOTECOMMANDS_EMPTY;
	}
	return instrumentindex_make(self->instrumentbank, rv);
}

void releaseallvoices(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;		
		psy_audio_samplervoice_release(voice);		
	}
}

void releasevoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_samplervoice_release(voice);
		}
	}
}

void nnavoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_samplervoice_nna(voice);
		}
	}
}

psy_audio_SamplerVoice* activevoice(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_audio_SamplerVoice* rv = NULL;	
	psy_List* p = NULL;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel && voice->amplitudeenvelope.stage != ENV_RELEASE
				&& voice->amplitudeenvelope.stage != ENV_OFF) {
			rv = voice;
			break;
		}
	}
	return rv;
}

void removeunusedvoices(psy_audio_Sampler* self)
{
	psy_List* p;
	psy_List* q;
		
	for (p = self->voices; p != NULL; p = q) {
		psy_audio_SamplerVoice* voice;

		q = p->next;
		voice = (psy_audio_SamplerVoice*) p->entry;				
		if (voice->amplitudeenvelope.stage == ENV_OFF ||
				!psy_audio_samplervoice_isplaying(voice)) {
			psy_audio_samplervoice_dispose(voice);
			free(voice);
			psy_list_remove(&self->voices, p);
		}			
	}
}

const psy_audio_MachineInfo* info(psy_audio_Sampler* self)
{	
	return &macinfo;
}

uintptr_t numparameters(psy_audio_Sampler* self)
{
	return numparametercols(self) * 8;
}

uintptr_t numparametercols(psy_audio_Sampler* self)
{
	return 10;
}

#define CHANNELROW 2

psy_audio_MachineParam* parameter(psy_audio_Sampler* self,
	uintptr_t param)
{
	uintptr_t col;
	uintptr_t row;
	uintptr_t rows;

	rows = numparameters(self) / numparametercols(self);
	row = param % rows;
	col = param / rows;
	if (col == 0) {
		switch (row) {
		case 0: return &self->param_general.machineparam; break;
		case 1: return &self->param_channels.machineparam; break;
		case CHANNELROW + 1: return &self->param_filter_cutoff.machineparam; break;
		case CHANNELROW + 2: return &self->param_filter_res.machineparam; break;
		case CHANNELROW + 3: return &self->param_pan.machineparam;  break;
		case CHANNELROW + 5: return &self->ignore_param.machineparam;  break;
		default:
			return &self->param_blank.machineparam; break;
			break;
		}
	} else
	if (col == 9) {
		switch (row) {		
		case CHANNELROW + 0: return &self->masterchannel.param_channel.machineparam; break;
		case CHANNELROW + 4: return &self->masterchannel.slider_param.machineparam; break;
		case CHANNELROW + 5: return &self->masterchannel.level_param.machineparam; break;
		default:
			return &self->param_blank.machineparam;
			break;
		}
	} else if (row == 0) {
		switch (col) {		
		case 1: return &self->param_resamplingmethod.machineparam; break;
		case 2: return &self->param_numvoices.machineparam; break;	
		case 3: return &self->param_amigaslides.machineparam;  break;
		case 4: return &self->param_usefilters.machineparam;  break;
		case 5: return &self->param_panningmode.machineparam;  break;
		case 6: return &self->param_defaultspeed.machineparam;  break;		
		case 7: return &self->param_maxvolume.machineparam; break;
		case 8: return &self->param_panpersistent.machineparam; break;
		case 9: return &self->param_instrumentbank.machineparam; break;
		default:
			return &self->param_blank.machineparam; break;
			break;
		}
	} else if (row == 1) {
		switch (col) {		
		case 1: return &self->param_channelview.machineparam; break;
		default:
			return &self->param_blank.machineparam; break;
			break;
		}
	} else {	
		psy_audio_SamplerChannel* channel;
		
		channel = sampler_channel(self, col - 1 + self->channelbank * 8);
		if (channel) {
			switch (row) {
				case CHANNELROW + 0:
				return &channel->param_channel.machineparam;
					break;
				case CHANNELROW + 1:
					return &channel->filter_cutoff.machineparam;
					break;
				case CHANNELROW + 2:
					return &channel->filter_res.machineparam;
					break;
				case CHANNELROW + 3:
					return &channel->pan.machineparam;
					break;
				case CHANNELROW + 4:
					return &channel->slider_param.machineparam;
					break;
				case CHANNELROW + 5:
				return &channel->level_param.machineparam;
					break;
				case CHANNELROW + 6:
					return &self->ignore_param.machineparam;
					break;
				default:
					return &self->param_blank.machineparam; break;
				break;
			}
		}
	}
	return NULL;
}

void resamplingmethod_tweak(psy_audio_Sampler* self,
	psy_audio_ChoiceMachineParam* sender, float value)
{	
	psy_audio_sampler_setresamplerquality(self,
		(int)(value * (sender->maxval - sender->minval) + 0.5f) +
			sender->minval);	
}

uintptr_t numinputs(psy_audio_Sampler* self)
{
	return 0;
}

uintptr_t numoutputs(psy_audio_Sampler* self)
{
	return 2;
}

static int alteRand(int x) { return (x * rand()) / 32768; }

psy_List* sequencerinsert(psy_audio_Sampler* self, psy_List* events)
{
	psy_List* p;
	psy_List* insert = 0;

	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* event;

		entry = p->entry;
		event = patternentry_front(entry);
		if (event->cmd == SAMPLER_CMD_EXTENDED) {
			if ((event->parameter & 0xf0) == SAMPLER_CMD_E_DELAYED_NOTECUT) {
				psy_audio_PatternEntry* noteoff;

				// This means there is always 6 ticks per row whatever number of rows.
				//_triggerNoteOff = (Global::player().SamplesPerRow()/6.f)*(ite->_parameter & 0x0f);
				noteoff = patternentry_allocinit();
				patternentry_front(noteoff)->note = NOTECOMMANDS_RELEASE;
				patternentry_front(noteoff)->mach = patternentry_front(entry)->mach;
				noteoff->delta += /*entry->offset*/ + (event->parameter & 0x0f) / 6.f *
					psy_audio_machine_currbeatsperline(
						psy_audio_sampler_base(self));
				psy_list_append(&insert, noteoff);
			} else 
			if ((event->parameter & 0xF0) == SAMPLER_CMD_E_NOTE_DELAY) {
				psy_audio_PatternEntry* newentry;
				psy_audio_PatternEvent* ev;
				int numticks;

				newentry = patternentry_clone(entry);
				ev = patternentry_front(newentry);
				numticks = event->parameter & 0x0f;
				ev->cmd = 0;
				ev->parameter = 0;
				newentry->delta += numticks * psy_audio_machine_beatspertick(
					psy_audio_sampler_base(self));				
				psy_list_append(&insert, newentry);
			}
		}
	}
	return insert;
}

// fileio
void loadspecific(psy_audio_Sampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	if (self->xmsamplerload) {
		int32_t temp;
		bool wrongState = FALSE;
		uint32_t filevers;
		size_t filepos;
		uint32_t size=0;

		psy_audio_sampler_initsampulse(self);		
		psyfile_read(songfile->file, &size,sizeof(size));
		filepos = psyfile_getpos(songfile->file);
		psyfile_read(songfile->file, &filevers, sizeof(filevers));
			
		// Check higher bits of version (AAAABBBB). 
		// different A, incompatible, different B, compatible
 		if ( (filevers&0xFFFF0000) == XMSAMPLER_VERSION_ONE )
		{
			ZxxMacro zxxMap[128];
			int i;
			uint8_t m_bAmigaSlides;// Using Linear or Amiga Slides.
			uint8_t m_UseFilters;
			int32_t m_GlobalVolume;
			int32_t m_PanningMode;
			ResamplerType resamplertype;

			// numSubtracks
			psyfile_read(songfile->file, &temp, sizeof(temp));
			self->numvoices = temp;
			// quality
			psyfile_read(songfile->file, &temp, sizeof(temp));			
			switch (temp) {
				case 2: resamplertype = RESAMPLERTYPE_SPLINE; break;
				case 3: resamplertype = RESAMPLERTYPE_SINC; break;
				case 0:	resamplertype = RESAMPLERTYPE_ZERO_ORDER; break;
				case 1:
				default:
					resamplertype = RESAMPLERTYPE_LINEAR;
					break;
			}
			psy_audio_sampler_setresamplerquality(self, resamplertype);
			for (i = 0; i < 128; ++i) {
				psyfile_read(songfile->file, &zxxMap[i].mode, sizeof(zxxMap[i].mode));
				psyfile_read(songfile->file, &zxxMap[i].value, sizeof(zxxMap[i].value));
			}

			psyfile_read(songfile->file, &m_bAmigaSlides, sizeof(m_bAmigaSlides));
			self->amigaslides = m_bAmigaSlides;
			psyfile_read(songfile->file, &m_UseFilters, sizeof(m_UseFilters));
			self->usefilters = m_UseFilters;
			psyfile_read(songfile->file, &m_GlobalVolume, sizeof(m_GlobalVolume));
			psyfile_read(songfile->file, &m_PanningMode, sizeof(m_PanningMode));
			self->masterchannel.volume = m_GlobalVolume / 127.f;
			for (i = 0; i < MAX_TRACKS; ++i) {				
				psy_audio_samplerchannel_load(sampler_channel(self, i),
					songfile);
			}

			if ((filevers & XMSAMPLER_VERSION & 0x0000FFFF) >= 0x02) {
				uint32_t temp32;

				psyfile_read(songfile->file, &temp32, sizeof(temp32));
				self->instrumentbank = temp32;
			}
		} else {
			wrongState = TRUE;
		}
		if (!wrongState) {
			return;
		} else {
			psyfile_seek(songfile->file, filepos + size);
			return; // FALSE;
		}
	} else {
		uint32_t size;
		int resamplertype;
		
		psy_audio_sampler_initps1(self);
		// LinearSlide(false);
		size = 0;
		psyfile_read(songfile->file, &size, sizeof(size));
		if (size)
		{
			/// Version 0
			int32_t temp;
			psyfile_read(songfile->file, &temp, sizeof(temp)); // numSubtracks
			self->numvoices = temp;
			psyfile_read(songfile->file, &temp, sizeof(temp)); // quality		
			switch (temp) {
				case 2: resamplertype = RESAMPLERTYPE_SPLINE; break;
				case 3: resamplertype = RESAMPLERTYPE_SINC; break;
				case 0:	resamplertype = RESAMPLERTYPE_ZERO_ORDER; break;
				case 1:
				default:
					resamplertype = RESAMPLERTYPE_LINEAR;
					break;
			}
			psy_audio_sampler_setresamplerquality(self, resamplertype);			
			if (size > 3 * sizeof(unsigned int))
			{
				unsigned int internalversion;
				psyfile_read(songfile->file, &internalversion,
					sizeof(internalversion));
				if (internalversion >= 1) {
					uint8_t defaultC4;

					// correct A4 frequency.
					psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4));
					psy_audio_sampler_defaultC4(self, defaultC4 != 0);
				}
				if (internalversion >= 2) {
					unsigned char slidemode;
					// correct slide.
					psyfile_read(songfile->file, &slidemode, sizeof(slidemode));
					// LinearSlide(slidemode);
				}
				if (internalversion >= 3) {
					uint32_t instrbank;
					// instrument bank.
					psyfile_read(songfile->file, &instrbank, sizeof(instrbank));
					self->instrumentbank = instrbank;
				}
			}
		}
	}
}

void savespecific(psy_audio_Sampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	int32_t temp;
	int i;
	size_t endpos;

	// we cannot calculate the size previous to save, so we write a placeholder
	// and seek back to write the correct value.
	uint32_t size = 0;
	size_t filepos = psyfile_getpos(songfile->file);
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write_uint32(songfile->file, XMSAMPLER_VERSION);
	psyfile_write_uint32(songfile->file, self->numvoices);
	switch (self->resamplerquality)
	{
	case RESAMPLERTYPE_ZERO_ORDER: temp = 0; break;
	case RESAMPLERTYPE_SPLINE: temp = 2; break;
	case RESAMPLERTYPE_SINC: temp = 3; break;
	case RESAMPLERTYPE_LINEAR: //fallthrough
	default: temp = 1;
	}
	psyfile_write_int32(songfile->file, temp); // quality
	//TODO: zxxMap cannot be edited right now.
	for (i = 0; i < 128; i++) {
		psyfile_write_int32(songfile->file, 0); // zxxMap[i].mode);
		psyfile_write_int32(songfile->file, 0); // zxxMap[i].value);
	}
	psyfile_write_uint8(songfile->file, (uint8_t)(self->amigaslides != 0));
	psyfile_write_uint8(songfile->file, (uint8_t) self->usefilters);
	psyfile_write_int32(songfile->file, 128);
	psyfile_write_int32(songfile->file, psy_audio_PANNING_LINEAR);
	for (i = 0; i < MAX_TRACKS; i++) {		
		psy_audio_samplerchannel_save(sampler_channel(self, i),
			songfile);
	}
	psyfile_write_uint32(songfile->file, (uint32_t)self->instrumentbank);
	endpos = psyfile_getpos(songfile->file);
	psyfile_seek(songfile->file, filepos);
	size = (uint32_t)(endpos - filepos - sizeof(size));
	psyfile_write_uint32(songfile->file, size);
	psyfile_seek(songfile->file, endpos);
}

void psy_audio_sampler_setresamplerquality(psy_audio_Sampler* self,
	ResamplerType quality)
{
	psy_List* it;
	
	for (it = self->voices; it != NULL; psy_list_next(&it)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)(it->entry);
		psy_audio_samplervoice_setresamplerquality(voice, quality);
	}
}

// Sampler Configurations
void psy_audio_sampler_initps1(psy_audio_Sampler* self)
{
	// Old version had default C4 as false
	psy_audio_sampler_defaultC4(self, FALSE);
	self->instrumentbank = 0;
	disposecmds(self);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PORTAMENTO_UP,
		SAMPLER_CMD_PORTAMENTO_UP,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PORTAMENTO_DOWN,
		SAMPLER_CMD_PORTAMENTO_DOWN,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PORTA2NOTE,
		SAMPLER_CMD_PORTA2NOTE,
		psy_audio_SAMPLERCMDMODE_TICK |
		psy_audio_SAMPLERCMDMODE_MEM0);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VIBRATO,
		SAMPLER_CMD_VIBRATO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TONEPORTAVOL,
		SAMPLER_CMD_TONEPORTAVOL,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VIBRATOVOL,
		SAMPLER_CMD_VIBRATOVOL,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_FINE_VIBRATO,
		SAMPLER_CMD_FINE_VIBRATO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TREMOR,
		SAMPLER_CMD_TREMOR,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TREMOLO,
		SAMPLER_CMD_TREMOLO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PANBRELLO,
		SAMPLER_CMD_PANBRELLO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PANNING,
		SAMPLER_CMD_PANNING,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_SET_CHANNEL_VOLUME,
		SAMPLER_CMD_SET_CHANNEL_VOLUME,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VOLUMESLIDE,
		SAMPLER_CMD_VOLUMESLIDE,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PANNINGSLIDE,
		SAMPLER_CMD_PANNINGSLIDE,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_CHANNEL_VOLUMESLIDE,
		SAMPLER_CMD_CHANNEL_VOLUMESLIDE,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_RETRIG,
		SAMPLER_CMD_RETRIG,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_updatecmdmap(self);
}

void psy_audio_sampler_initsampulse(psy_audio_Sampler* self)
{
	psy_audio_sampler_defaultC4(self, TRUE);
	self->panpersistent = TRUE;
	self->instrumentbank = 1;
	disposecmds(self);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PORTAMENTO_UP,
		SAMPLER_CMD_PORTAMENTO_UP,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PORTAMENTO_DOWN,
		SAMPLER_CMD_PORTAMENTO_DOWN,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PORTA2NOTE,
		SAMPLER_CMD_PORTA2NOTE,
		psy_audio_SAMPLERCMDMODE_TICK |
		psy_audio_SAMPLERCMDMODE_MEM0);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VIBRATO,
		SAMPLER_CMD_VIBRATO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_FINE_VIBRATO,
		SAMPLER_CMD_FINE_VIBRATO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TONEPORTAVOL,
		SAMPLER_CMD_TONEPORTAVOL,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VIBRATOVOL,
		SAMPLER_CMD_VIBRATOVOL,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TREMOR,
		SAMPLER_CMD_TREMOR,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TREMOLO,
		SAMPLER_CMD_TREMOLO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PANBRELLO,
		SAMPLER_CMD_PANBRELLO,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PANNING,
		SAMPLER_CMD_PANNING,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_SET_CHANNEL_VOLUME,
		SAMPLER_CMD_SET_CHANNEL_VOLUME,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VOLUMESLIDE,
		SAMPLER_CMD_VOLUMESLIDE,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_PANNINGSLIDE,
		SAMPLER_CMD_PANNINGSLIDE,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_CHANNEL_VOLUMESLIDE,
		SAMPLER_CMD_CHANNEL_VOLUMESLIDE,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_RETRIG,
		SAMPLER_CMD_RETRIG,
		psy_audio_SAMPLERCMDMODE_PERS);
	psy_audio_sampler_updatecmdmap(self);
}

void psy_audio_sampler_addcmd(psy_audio_Sampler* self, int cmdid,
	int patternid, int mask)
{
	psy_list_append(&self->cmds,
		(void*)psy_audio_samplercmd_allocinit_all(cmdid, patternid, mask));
}

psy_audio_SamplerCmd* psy_audio_sampler_cmd(psy_audio_Sampler* self,
	int patternid)
{
	return (psy_audio_SamplerCmd*)psy_table_at(&self->cmdmap, patternid);
}

void psy_audio_sampler_updatecmdmap(psy_audio_Sampler* self)
{
	psy_List* p;

	psy_table_clear(&self->cmdmap);
	for (p = self->cmds; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerCmd* cmd;
		cmd = (psy_audio_SamplerCmd*)psy_list_entry(p);
		psy_table_insert(&self->cmdmap, (uintptr_t)cmd->patternid, cmd);
	}
}
