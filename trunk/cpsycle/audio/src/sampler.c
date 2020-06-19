// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampler.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "instruments.h"
#include "samples.h"
#include "songio.h"
#include <noteperiods.h>
#include <operations.h>
#include <linear.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"
#include "constants.h"

// Version for the sampler machine data.
// The instruments and sample bank versions are saved with the song chunk
// versioning
#define XMSAMPLER_VERSION 0x00010002
// Version zero was the development version (no format freeze). Version one is
// the published one.
#define XMSAMPLER_VERSION_ONE 0x00010000
// Version Sampler PS1
#define SAMPLERVERSION 0x00000003

// Sampler Configuration
void psy_audio_samplercmd_init_all(psy_audio_SamplerCmd* self,
	int id, int patternid, int mask)
{
	self->id = id;
	self->patternid = patternid;
	self->mode = mask;
}

void psy_audio_samplercmd_dispose(psy_audio_SamplerCmd* self)
{	
}

psy_audio_SamplerCmd* psy_audio_samplercmd_alloc(void)
{
	return (psy_audio_SamplerCmd*)malloc(sizeof(psy_audio_SamplerCmd));
}

psy_audio_SamplerCmd* psy_audio_samplercmd_allocinit_all(int id,
	int patternid, int mask)
{
	psy_audio_SamplerCmd* rv;
	
	rv = psy_audio_samplercmd_alloc();
	if (rv) {
		psy_audio_samplercmd_init_all(rv, id, patternid, mask);
	}
	return rv;
}

// SamplerMasterChannel
static void psy_audio_samplermasterchannel_init(psy_audio_SamplerMasterChannel*,
	psy_audio_Sampler*);
static void psy_audio_samplermasterchannel_dispose(psy_audio_SamplerMasterChannel*);
// Slider/Level
static void psy_audio_samplermasterchannel_level_normvalue(psy_audio_SamplerMasterChannel*,
	psy_audio_CustomMachineParam* sender, float* rv);
// processing
void psy_audio_samplermasterchannel_work(psy_audio_SamplerMasterChannel*,
	psy_audio_BufferContext*);

void psy_audio_samplermasterchannel_init(psy_audio_SamplerMasterChannel* self,
	psy_audio_Sampler* sampler)
{	
	self->sampler = sampler;
	self->volume = 1.f;
	psy_audio_infomachineparam_init(&self->param_channel, "Master", "", MPF_SMALL);	
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "", MPF_SLIDER | MPF_SMALL, &self->volume);
	psy_audio_volumemachineparam_setmode(&self->slider_param, psy_audio_VOLUME_LINEAR);
	psy_audio_volumemachineparam_setrange(&self->slider_param, 0, 127);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
		psy_audio_samplermasterchannel_level_normvalue);
}

void psy_audio_samplermasterchannel_dispose(psy_audio_SamplerMasterChannel* self)
{
	psy_audio_infomachineparam_dispose(&self->param_channel);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
}

void psy_audio_samplermasterchannel_level_normvalue(psy_audio_SamplerMasterChannel* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	psy_audio_Buffer* memory;

	memory = psy_audio_machine_buffermemory(psy_audio_sampler_base(self->sampler));
	if (memory) {
		*rv = psy_audio_buffer_rmsdisplay(memory);
	} else {
		*rv = 0.f;
	}
}

void psy_audio_samplermasterchannel_work(psy_audio_SamplerMasterChannel* self,
	psy_audio_BufferContext* bc)
{
	psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->volume);
}

// SamplerChannel
static void psy_audio_samplerchannel_init(psy_audio_SamplerChannel*, uintptr_t index,
	psy_audio_Sampler* sampler);
static void psy_audio_samplerchannel_dispose(psy_audio_SamplerChannel*);
static void psy_audio_samplerchannel_seteffect(psy_audio_SamplerChannel*,
	const psy_audio_PatternEvent*);

void psy_audio_samplerchannel_init(psy_audio_SamplerChannel* self, uintptr_t index,
	psy_audio_Sampler* sampler)
{
	char text[127];

	self->sampler = sampler;
	self->volume = 1.f;
	self->panfactor = (psy_dsp_amp_t) 0.5f;
	self->m_ChannelDefVolume = 200;	
	self->m_DefaultPanFactor = 100;
	self->m_DefaultCutoff = 127;
	self->m_DefaultRessonance = 0;
	self->defaultfiltertype = F_NONE;
	psy_snprintf(text, 127, "Channel %d", (int)index);
	psy_audio_infomachineparam_init(&self->param_channel, text, "", MPF_SMALL);
	psy_audio_intmachineparam_init(&self->filter_cutoff,
		"", "", MPF_STATE | MPF_SMALL, &self->m_DefaultCutoff, 0, 200);
	psy_audio_intmachineparam_init(&self->filter_res,
		"", "", MPF_STATE | MPF_SMALL, &self->m_DefaultRessonance, 0, 200);
	psy_audio_intmachineparam_init(&self->pan,
		"", "", MPF_STATE | MPF_SMALL, &self->m_DefaultPanFactor, 0, 200);
	psy_audio_volumemachineparam_init(&self->slider_param,
		"Volume", "", MPF_SLIDER | MPF_SMALL, &self->volume);
	psy_audio_volumemachineparam_setmode(&self->slider_param, psy_audio_VOLUME_LINEAR);
	psy_audio_volumemachineparam_setrange(&self->slider_param, 0, 127);
	psy_audio_intmachineparam_init(&self->level_param,
		"Level", "Level", MPF_SLIDERLEVEL | MPF_SMALL, NULL, 0, 100);
	//psy_signal_connect(&self->level_param.machineparam.signal_normvalue, self,
	//	inputchannel_level_normvalue);
}

void psy_audio_samplerchannel_dispose(psy_audio_SamplerChannel* self)
{
	psy_audio_infomachineparam_dispose(&self->param_channel);
	psy_audio_intmachineparam_dispose(&self->filter_cutoff);
	psy_audio_intmachineparam_dispose(&self->filter_res);
	psy_audio_intmachineparam_dispose(&self->pan);
	psy_audio_volumemachineparam_dispose(&self->slider_param);
	psy_audio_intmachineparam_dispose(&self->level_param);
}

void psy_audio_samplerchannel_seteffect(psy_audio_SamplerChannel* self,
	const psy_audio_PatternEvent* ev)
{
	switch (ev->cmd) {
		case SAMPLER_CMD_SET_CHANNEL_VOLUME:
			self->volume = (psy_dsp_amp_t) ((ev->parameter < 64)
				? (ev->parameter / 64.0f)
				: 1.0f);
		break;
		case SAMPLER_CMD_PANNING:
			self->panfactor = ev->parameter / (psy_dsp_amp_t) 255;
		break;
		default:
		;
	}
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
	self->m_ChannelDefVolume = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));//<  0..200 .  &0x100 = Surround.
	self->m_DefaultPanFactor = temp32;
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
	psyfile_write_int32(songfile->file, self->m_ChannelDefVolume);
	psyfile_write_int32(songfile->file, self->m_DefaultPanFactor);
	psyfile_write_int32(songfile->file, self->m_DefaultCutoff);
	psyfile_write_int32(songfile->file, self->m_DefaultRessonance);	
	psyfile_write_uint32(songfile->file, self->defaultfiltertype);	
}

// Sampler
static void generateaudio(psy_audio_Sampler*, psy_audio_BufferContext*);
static void seqtick(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void newline(psy_audio_Sampler*);
static psy_List* sequencerinsert(psy_audio_Sampler*, psy_List* events);
static void stop(psy_audio_Sampler*);
static const psy_audio_MachineInfo* info(psy_audio_Sampler*);
// Parameters
static uintptr_t numparametercols(psy_audio_Sampler*);
static uintptr_t numparameters(psy_audio_Sampler*);
static psy_audio_MachineParam* parameter(psy_audio_Sampler*,
	uintptr_t param);
static void dispose(psy_audio_Sampler*);
static void disposecmds(psy_audio_Sampler*);
static int alloc_voice(psy_audio_Sampler*);
static void releaseallvoices(psy_audio_Sampler*);
static psy_audio_SamplerVoice* activevoice(psy_audio_Sampler*, uintptr_t channel);
static void releasevoices(psy_audio_Sampler*, uintptr_t channel);
static void nnavoices(psy_audio_Sampler*, uintptr_t channel);
static void removeunusedvoices(psy_audio_Sampler* self);
static uintptr_t numinputs(psy_audio_Sampler*);
static uintptr_t numoutputs(psy_audio_Sampler*);
static void loadspecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);

static void disposechannels(psy_audio_Sampler*);
static psy_audio_SamplerChannel* sampler_channel(psy_audio_Sampler*, uintptr_t channelnum);
static psy_audio_InstrumentIndex currslot(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void psy_audio_sampler_initps1(psy_audio_Sampler*);
static void psy_audio_sampler_initsampulse(psy_audio_Sampler*);
static void psy_audio_sampler_addcmd(psy_audio_Sampler*, int cmd,
	int patternid, int mask);
static void psy_audio_sampler_updatecmdmap(psy_audio_Sampler* self);
static psy_audio_SamplerCmd* psy_audio_sampler_cmd(psy_audio_Sampler*, int patternid);

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
	0,
	0,
	// help text
	""
	"Track Commands :""\n"
	"	01xx : Portamento Up(Fx : fine, Ex : Extra fine)""\n"
	"	02xx : Portamento Down(Fx : fine, Ex : Extra fine)""\n"
	"	03xx : Tone Portamento""\n"
	"	04xy : Vibrato with speed y and depth x""\n"
	"	05xx : Continue Portamento and Volume Slide with speed xx""\n"
	"	06xx : Continue Vibrato and Volume Slide with speed xx""\n"
	"	07xx : Tremolo""\n"
	"	08xx : Pan. 0800 Left 08FF right""\n"
	"	09xx: Panning slide x0 Left, 0x Right""\n"
	"	0Axx: Channel Volume, 00 = Min, 40 = Max""\n"
	"	0Bxx : Channel VolSlide x0 Up(xF fine), 0x Down(Fx Fine)""\n"
	"	0Cxx: Volume(0C80 : 100 %)""\n"
	"	0Dxx : Volume Slide x0 Up(xF fine), 0x Down(Fx Fine)""\n"
	"	0Exy: Extended(see below).""\n"
	"	0Fxx : Filter.""\n"
	"	10xy : Arpeggio with note, note + x and note + y""\n"
	"	11xy : Retrig note after y ticks""\n"
	"	14xx : Fine Vibrato with speed y and depth x""\n"
	"	17xy : Tremor Effect(ontime x, offtime y)""\n"
	"	18xx : Panbrello""\n"
	"	19xx : Set Envelope position(in ticks)""\n"
	"	1Cxx : Global Volume, 00 = Min, 80 = Max""\n"
	"	1Dxx : Global Volume Slide x0 Up(xF fine), 0x Down(Fx Fine)""\n"
	"	1Exx: Send xx to volume colum(see below)""\n"
	"	9xxx : Sample Offset x * 256"
	"""\n"
	"Extended Commands :""\n"
	"	30 / 1 : Glissando mode Off / on""\n"
	"	4x : Vibrato Wave""\n"
	"	5x : Panbrello Wave""\n"
	"	7x : Tremolo Wave""\n"
	"	Waves : 0 : Sinus, 1 : Square""\n"
	"	2 : Ramp Up, 3 : Ramp Down, 4 : Random""\n"
	"	8x : Panning""\n"
	"	90 : Surround Off""\n"
	"	91 : Surround On""\n"
	"	9E : Play Forward""\n"
	"	9F : Play Backward""\n"
	"	Cx : Delay NoteCut by x ticks""\n"
	"	Dx : Delay New Note by x ticks""\n"
	"	E0 : Notecut background notes""\n"
	"	E1 : Noteoff background notes""\n"
	"	E2 : NoteFade background notes""\n"
	"	E3 : Set NNA NoteCut for this voice""\n"
	"	E4 : Set NNA NoteContinue for this voice""\n"
	"	E5 : Set NNA Noteoff for this voice""\n"
	"	E6 : Set NNA NoteFade for this channel""\n"
	"	E7 / 8 : Disable / Enable Volume Envelope""\n"
	"	E9 / A : Disable / Enable Pan Envelope""\n"
	"	EB / C : Disable / Enable Pitch / Filter Envelope""\n"
	"	Fx : Set Filter Mode.""\n"
	"	""\n"
	"Volume Column : ""\n"
	"	00..3F : Set volume to x * 2""\n"
	"	4x : Volume slide up""\n"
	"	5x : Volume slide down""\n"
	"	6x : Fine Volslide up""\n"
	"	7x : Fine Volslide down""\n"
	"	8x : Panning(0:Left, F : Right)""\n"
	"	9x : PanSlide Left""\n"
	"	Ax : PanSlide Right""\n"
	"	Bx : Vibrato""\n"
	"	Cx : TonePorta""\n"
	"	Dx : Pitch slide up""\n"
	"	Ex : Pitch slide down""\n"
};

const psy_audio_MachineInfo* psy_audio_sampler_info(void)
{
	return &macinfo;
}

// TickTimer
static void psy_audio_samplerticktimer_dowork(psy_audio_SamplerTickTimer*,
	uintptr_t numsamples, psy_audio_BufferContext*, uintptr_t offset);

void psy_audio_samplerticktimer_init(psy_audio_SamplerTickTimer* self,
	void* context,
	fp_samplerticktimer_ontick tick,
	fp_samplerticktimer_onwork work)
{	
	self->context = context;
	self->tick = tick;
	self->work = work;
	self->samplesprotick = 882;
	self->counter = 0;
	self->tickcount = 0;
}

void psy_audio_samplerticktimer_reset(psy_audio_SamplerTickTimer* self,
	uintptr_t samplesprotick)
{
	self->samplesprotick = samplesprotick;
	self->counter = 0;
	self->tickcount = 0;
}

void psy_audio_samplerticktimer_update(psy_audio_SamplerTickTimer* self,
	uintptr_t numsamples, psy_audio_BufferContext* bc)
{	
	uintptr_t j = 0;
	uintptr_t lastpos = 0;
	uintptr_t amount;

	amount = numsamples;
	for (; j < numsamples; ++j) {
		if (self->counter == 0) {
			uintptr_t worknum;

			worknum = j - lastpos;
			if (worknum) {
				psy_audio_samplerticktimer_dowork(self, worknum, bc, lastpos);
				amount -= worknum;
				lastpos = j;
			}
			self->tick(self->context);			
			++self->tickcount;
		}
		if (self->counter >= self->samplesprotick) {
			self->counter = 0;
		} else {
			++self->counter;
		}
	}
	if (amount) {
		psy_audio_samplerticktimer_dowork(self, amount, bc, lastpos);		
	}
}

void psy_audio_samplerticktimer_dowork(psy_audio_SamplerTickTimer* self,
	uintptr_t amount, psy_audio_BufferContext* bc, uintptr_t offset)
{
	uintptr_t restorenumsamples;
	uintptr_t restoreoffset;

	restorenumsamples = psy_audio_buffercontext_numsamples(bc);
	restoreoffset = bc->output->offset;
	psy_audio_buffercontext_setnumsamples(bc, amount);
	psy_audio_buffercontext_setoffset(bc, restoreoffset + offset);
	self->work(self->context, bc);
	psy_audio_buffercontext_setnumsamples(bc, restorenumsamples);
	psy_audio_buffercontext_setoffset(bc, restoreoffset);
}


// Sampler
static void resamplingmethod_tweak(psy_audio_Sampler*,
	psy_audio_ChoiceMachineParam* sender, float value);
static void psy_audio_sampler_ontimertick(psy_audio_Sampler*);
static void psy_audio_sampler_ontimerwork(psy_audio_Sampler*, psy_audio_BufferContext*);

static MachineVtable sampler_vtable;
static int sampler_vtable_initialized = 0;

static void sampler_vtable_init(psy_audio_Sampler* self)
{
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_sampler_base(self)->vtable);
		sampler_vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
		sampler_vtable.seqtick = (fp_machine_seqtick)seqtick;
		sampler_vtable.newline = (fp_machine_newline)newline;
		sampler_vtable.sequencerinsert = (fp_machine_sequencerinsert)sequencerinsert;
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

void psy_audio_sampler_init(psy_audio_Sampler* self, psy_audio_MachineCallback callback)
{
	uintptr_t i;

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
	self->samplecounter = 0;
	self->amigaslides = FALSE;
	self->usefilters = TRUE;
	self->channelbank = 0;
	self->panningmode = psy_audio_PANNING_LINEAR;
	psy_audio_samplerticktimer_init(&self->ticktimer,
		self,
		psy_audio_sampler_ontimertick,
		psy_audio_sampler_ontimerwork);
	psy_table_init(&self->channels);
	psy_table_init(&self->lastinst);
	psy_audio_custommachineparam_init(&self->param_general, "General", "General",
		MPF_HEADER | MPF_SMALL, 0, 0);
	psy_audio_intmachineparam_init(&self->param_numvoices,
		"limit voices", "limit voices", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->numvoices,
		1, 64);
	psy_audio_choicemachineparam_init(&self->param_resamplingmethod,
		"resample", "resample", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->resamplerquality,
		0, 3);
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
	psy_audio_infomachineparam_init(&self->param_filter_cutoff, "Filter Cutoff", "", MPF_SMALL);
	psy_audio_infomachineparam_init(&self->param_filter_res, "Filter Q (Res)", "", MPF_SMALL);
	psy_audio_infomachineparam_init(&self->param_pan, "Panning", "", MPF_SMALL);
	psy_audio_samplermasterchannel_init(&self->masterchannel, self);
	psy_audio_custommachineparam_init(&self->ignore_param, "-", "-", MPF_IGNORE | MPF_SMALL, 0, 0);
	self->cmds = NULL;
	psy_table_init(&self->cmdmap);
	psy_audio_sampler_initsampulse(self);
	if (self->xmsamplerload == FALSE) {
		self->instrumentbank = 0;
	}
}

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
		SAMPLER_CMD_TREMOLO,
		SAMPLER_CMD_TREMOLO,
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
		SAMPLER_CMD_TONEPORTAVOL,
		SAMPLER_CMD_TONEPORTAVOL,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_VIBRATOVOL,
		SAMPLER_CMD_VIBRATOVOL,
		psy_audio_SAMPLERCMDMODE_TICK);
	psy_audio_sampler_addcmd(self,
		SAMPLER_CMD_TREMOLO,
		SAMPLER_CMD_TREMOLO,
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
	psy_audio_sampler_updatecmdmap(self);
}

void psy_audio_sampler_addcmd(psy_audio_Sampler* self, int cmdid, int patternid, int mask)
{		
	psy_list_append(&self->cmds, (void*)psy_audio_samplercmd_allocinit_all(cmdid,
		patternid, mask));
}

psy_audio_SamplerCmd* psy_audio_sampler_cmd(psy_audio_Sampler* self, int patternid)
{	
	return (psy_audio_SamplerCmd*)psy_table_at(&self->cmdmap, patternid);
}

void psy_audio_sampler_updatecmdmap(psy_audio_Sampler* self)
{
	psy_List* p;

	psy_table_clear(&self->cmdmap);	
	for (p = self->cmds; p != NULL; p = p->next) {
		psy_audio_SamplerCmd* cmd;
		cmd = (psy_audio_SamplerCmd*)(p->entry);
		psy_table_insert(&self->cmdmap, (uintptr_t)cmd->patternid, cmd);
	}
}

void dispose(psy_audio_Sampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		psy_audio_samplervoice_dispose(voice);		
		free(voice);
	}
	psy_list_free(self->voices);
	self->voices = 0;
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
	disposechannels(self);
	custommachine_dispose(&self->custommachine);
	disposecmds(self);
	psy_table_dispose(&self->cmdmap);
}

void disposechannels(psy_audio_Sampler* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->channels);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_SamplerChannel* channel;

		channel = (psy_audio_SamplerChannel*) psy_tableiterator_value(&it);
		psy_audio_samplerchannel_dispose(channel);
		free(channel);
	}
	psy_table_dispose(&self->channels);
	psy_audio_samplermasterchannel_dispose(&self->masterchannel);
}

void disposecmds(psy_audio_Sampler* self)
{
	psy_List* p;

	for (p = self->cmds; p != NULL; p = p->next) {		
		psy_audio_samplercmd_dispose((psy_audio_SamplerCmd*)p->entry);
		free(p->entry);
	}
	psy_list_free(self->cmds);
	self->cmds = NULL;
}

psy_audio_Sampler* psy_audio_sampler_alloc(void)
{
	return (psy_audio_Sampler*) malloc(sizeof(psy_audio_Sampler));
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
		psy_audio_samplerchannel_init(rv, track, self);
		psy_table_insert(&self->channels, track, (void*) rv);
	}
	return rv;
}

void generateaudio(psy_audio_Sampler* self, psy_audio_BufferContext* bc)
{	
	self->samplecounter += bc->numsamples;
	psy_audio_samplerticktimer_update(&self->ticktimer, bc->numsamples, bc);		
	removeunusedvoices(self);	
	psy_audio_samplermasterchannel_work(&self->masterchannel, bc);
}

void psy_audio_sampler_ontimertick(psy_audio_Sampler* self)
{
	psy_List* p;
	uintptr_t c = 0;
	
	for (p = self->voices; p != NULL && c < self->numvoices; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)p->entry;
		psy_audio_samplervoice_tick(voice);
	}
}

void psy_audio_sampler_ontimerwork(psy_audio_Sampler* self, psy_audio_BufferContext* bc)
{
	psy_List* p;
	uintptr_t c = 0;
	
	// psy_audio_buffer_clearsamples(bc->output, bc->numsamples);
	for (p = self->voices; p != NULL && c < self->numvoices; p = p->next, ++c) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)p->entry;
		psy_audio_samplervoice_work(voice, bc->output, bc->numsamples);
	}
}

void seqtick(psy_audio_Sampler* self, uintptr_t channelnum,
	const psy_audio_PatternEvent* event)
{		
	psy_audio_SamplerVoice* voice = 0;
	psy_audio_SamplerChannel* channel = 0;

	if (event->cmd == SAMPLER_CMD_EXTENDED) {
		if ((event->parameter & 0xF0) == SAMPLER_CMD_E_NOTE_DELAY) {
			// skip for now and reinsert in sequencerinsert
			// with delayed offset
			return;
		}
	}

	channel = sampler_channel(self, channelnum);
	if (channel) {
		psy_audio_samplerchannel_seteffect(channel, event);
	}
	if (event->note == NOTECOMMANDS_RELEASE) {
		releasevoices(self, channelnum);
		return;
	}
	if (event->note < NOTECOMMANDS_RELEASE) {
		nnavoices(self, channelnum);
	} else {
		voice = activevoice(self, channelnum);
	}
	if (!voice) {		
		psy_audio_Instrument* instrument;
		
		instrument = instruments_at(psy_audio_machine_instruments(
			psy_audio_sampler_base(self)),
			currslot(self, channelnum, event));
		if (instrument) {
			voice = psy_audio_samplervoice_allocinit(self, instrument,
				channel,
				channelnum,
				psy_audio_machine_samplerate(psy_audio_sampler_base(self)));
			psy_list_append(&self->voices, voice);
		}
	}	
	if (voice) {
		psy_audio_samplervoice_seqtick(voice, event,
			1 / psy_audio_machine_beatspersample(
				psy_audio_sampler_base(self)));
	}
}

void newline(psy_audio_Sampler* self)
{
	psy_List* p;
	
	self->samplecounter = 0;
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		voice->effcmd.id = SAMPLER_CMD_NONE;
	}
	psy_audio_samplerticktimer_reset(&self->ticktimer,	
		(uintptr_t)
		psy_audio_machine_samplespertick(psy_audio_sampler_base(self)));	
}

void stop(psy_audio_Sampler* self)
{
	releaseallvoices(self);
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
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;		
		psy_audio_samplervoice_release(voice);		
	}
}

void releasevoices(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; p = p->next) {
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
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_samplervoice_nna(voice);
		}
	}
}

psy_audio_SamplerVoice* activevoice(psy_audio_Sampler* self, uintptr_t channel)
{
	psy_audio_SamplerVoice* rv = 0;	
	psy_List* p = 0;
	
	for (p = self->voices; p != NULL; p = p->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;
		if (voice->channelnum == channel && voice->env.stage != ENV_RELEASE
				&& voice->env.stage != ENV_OFF) {
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
		if (voice->env.stage == ENV_OFF) {
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

// psy_audio_SamplerVoice

static void psy_audio_samplervoice_updatespeed(psy_audio_SamplerVoice*);
static void psy_audio_samplervoice_updateiteratorspeed(psy_audio_SamplerVoice*,
	psy_audio_SampleIterator*);
static void psy_audio_samplervoice_initfilter(psy_audio_SamplerVoice*,
	psy_audio_Instrument* instrument);
static psy_dsp_amp_t psy_audio_samplervoice_workfilter(psy_audio_SamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, psy_dsp_amp_t* filterenv, uintptr_t pos);
static void psy_audio_samplervoice_currvolume(psy_audio_SamplerVoice* self,
	psy_audio_Sample* sample, psy_dsp_amp_t* svol, psy_dsp_amp_t* lvol,
	psy_dsp_amp_t* rvol);
psy_dsp_amp_t psy_audio_samplervoice_unprocessed_wavedata(psy_audio_SamplerVoice*,
	psy_audio_SampleIterator* it, uintptr_t channel);
static psy_dsp_amp_t psy_audio_samplervoice_processenvelopes(psy_audio_SamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_dsp_amp_t* env, psy_dsp_amp_t* filterenv,
	psy_dsp_amp_t svol, psy_dsp_amp_t lvol, psy_dsp_amp_t rvol);
static void psy_audio_samplervoice_adddatatosamplerbuffer(psy_audio_SamplerVoice*,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_audio_Buffer* output);
static int alteRand(int x) { return (x * rand()) / 32768; }

void psy_audio_samplervoice_init(psy_audio_SamplerVoice* self,
	psy_audio_Sampler* sampler,
	psy_audio_Samples* samples,
	psy_audio_Instrument* instrument,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate,
	int resamplingmethod,
	int maxvolume) 
{	
	filter_init_samplerate(&self->_filter, samplerate);
	self->sampler = sampler;
	self->samples = samples;
	self->instrument = instrument;
	self->channelnum = channelnum;
	self->channel = channel;
	self->usedefaultvolume = 1;
	self->vol = 1.f;
	self->pan = 0.5f;
	self->dopan = 0;
	self->positions = 0;
	self->effcmd = psy_audio_samplercmd_make(SAMPLER_CMD_NONE, 0, 0);
	self->effval = 0;
	self->dooffset = 0;
	self->maxvolume = maxvolume;
	self->positions = 0;
	self->stopping = FALSE;
	self->period = 0;
	if (instrument) {
		psy_dsp_adsr_init(&self->env, &instrument->volumeenvelope, samplerate);
		psy_dsp_adsr_init(&self->filterenv, &instrument->filterenvelope, samplerate);	
	} else {
		psy_dsp_adsr_initdefault(&self->env, samplerate);
		psy_dsp_adsr_initdefault(&self->filterenv, samplerate);
	}		
	psy_dsp_multiresampler_init(&self->resampler);
	psy_dsp_multiresampler_settype(&self->resampler,
		resamplingmethod);
	psy_audio_samplervoice_initfilter(self, instrument);
}

void psy_audio_samplervoice_initfilter(psy_audio_SamplerVoice* self,
	psy_audio_Instrument* instrument)
{
	if (instrument) {
		filter_settype(&self->_filter, instrument->filtertype);
		filter_setressonance(&self->_filter,
			(instrument->_RRES)
			? alteRand((int)(instrument->filterres * 127))
			: (int)(instrument->filterres * 127));
		self->_cutoff = (instrument->_RCUT)
			? alteRand((int)(instrument->filtercutoff * 127))
			: (int)instrument->filtercutoff * 127;
		self->_coModify = (instrument->filtermodamount - 0.5f) * 255.f;
		filter_setcutoff(&self->_filter, self->_cutoff);
	}
}

void psy_audio_samplervoice_reset(psy_audio_SamplerVoice* self)
{
	self->effcmd = psy_audio_samplercmd_make(SAMPLER_CMD_NONE, 0, 0);
	self->stopping = FALSE;
	psy_dsp_adsr_reset(&self->env);
	psy_dsp_adsr_reset(&self->filterenv);	
}

void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice* self)
{
	psy_audio_samplervoice_clearpositions(self);
	self->positions = 0;	
}

psy_audio_SamplerVoice* psy_audio_samplervoice_alloc(void)
{
	return (psy_audio_SamplerVoice*) malloc(sizeof(psy_audio_SamplerVoice));
}

psy_audio_SamplerVoice* psy_audio_samplervoice_allocinit(psy_audio_Sampler* sampler,
	psy_audio_Instrument* instrument,
	psy_audio_SamplerChannel* channel,
	uintptr_t channelnum,
	uintptr_t samplerate)
{
	psy_audio_SamplerVoice* rv;

	rv = psy_audio_samplervoice_alloc();
	if (rv) {
		psy_audio_samplervoice_init(rv,
			sampler,
			psy_audio_machine_samples(psy_audio_sampler_base(sampler)),
			instrument,
			channel,
			channelnum,
			samplerate,
			sampler->resamplerquality,
			sampler->maxvolume);
	}
	return rv;
}

void psy_audio_samplervoice_tick(psy_audio_SamplerVoice* self)
{
	switch (self->effcmd.id) {
		case SAMPLER_CMD_VOLUMESLIDE:
			if (self->effval < 0x10) {
				// Slide Down  [0 + Slide speed]
				self->vol -= (self->effval & 0x0F) / (psy_dsp_amp_t)self->maxvolume;
			} else {
				// Slide Up  [Slide speed + 0]
				int val;

				val = ((self->effval & 0xF0) >> 4);
				self->vol += (val)  / (psy_dsp_amp_t)self->maxvolume;
			}
			break;		
		case SAMPLER_CMD_PORTAMENTO_UP: {
			/*double factor;
			psy_List* p;

			factor = 1.0 / (12.0 * psy_audio_machine_ticksperbeat(
				psy_audio_sampler_base(self->sampler)));
			for (p = self->positions; p != NULL; p = p->next) {
				psy_audio_SampleIterator* iterator;

				iterator = (psy_audio_SampleIterator*)p->entry;
				iterator->speed *= pow(2.0, self->effval * factor);
				iterator->speedinternal *= pow(2.0, self->effval * factor);
				self = self;
			}*/
			self->period -= self->effval;
			break;
		}
		case SAMPLER_CMD_PORTAMENTO_DOWN: {
			/*double factor;
			psy_List* p;
			
			factor = 1.0 / (12.0 * psy_audio_machine_ticksperbeat(
				psy_audio_sampler_base(self->sampler)));
			for (p = self->positions; p != NULL; p = p->next) {
				psy_audio_SampleIterator* iterator;

				iterator = (psy_audio_SampleIterator*)p->entry;
				iterator->speed *= pow(2.0, -self->effval * factor);
			}*/
			self->period += self->effval;
			break;
		}
		default:
		break;
	}
	psy_audio_samplervoice_updatespeed(self);
}

void psy_audio_samplervoice_seqtick(psy_audio_SamplerVoice* self,
	const psy_audio_PatternEvent* event, double samplesprobeat)
{
	self->dopan = 0;	
	psy_audio_SamplerCmd* cmd;	
	
	cmd = psy_audio_sampler_cmd(self->sampler, event->cmd);
	if (cmd != NULL) {
		switch (cmd->id) {
		case SAMPLER_CMD_VOLUME:
			self->usedefaultvolume = 0;
			self->vol = event->parameter /
				(psy_dsp_amp_t)self->maxvolume;

			break;
		case SAMPLER_CMD_PANNING:
			self->dopan = 1;
			self->pan = event->parameter / (psy_dsp_amp_t)255;
			break;
		case SAMPLER_CMD_OFFSET:
			self->dooffset = 1;
			self->offset = event->parameter;
			break;
		case SAMPLER_CMD_PORTAMENTO_UP: {			
			self->effval = event->parameter;
			self->effcmd = *cmd;
			break;
		}
		case  SAMPLER_CMD_PORTAMENTO_DOWN: {
			self->effval = event->parameter;
			self->effcmd = *cmd;
			break;
		}
		case SAMPLER_CMD_PORTA2NOTE: {
			
		}
		case SAMPLER_CMD_VOLUMESLIDE:
			self->effval = event->parameter;
			self->effcmd = *cmd;
		break;
		default:
			break;
		}
	}
	if (event->note < NOTECOMMANDS_RELEASE) {		
		psy_audio_samplervoice_noteon(self, event, samplesprobeat);
	}
}

void psy_audio_samplervoice_noteon(psy_audio_SamplerVoice* self, const psy_audio_PatternEvent* event,
	double samplesprobeat)
{	
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;

	psy_audio_samplervoice_initfilter(self, self->instrument);
	psy_audio_samplervoice_clearpositions(self);
	entries = psy_audio_instrument_entriesintersect(self->instrument,
		event->note, 127, 0);
	for (p = entries; p != NULL; p = p->next) {
		psy_audio_InstrumentEntry* entry;
		
		entry = (psy_audio_InstrumentEntry*) p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			psy_audio_SampleIterator* iterator;			
			
			iterator = psy_audio_sampleiterator_alloc();
			*iterator = psy_audio_sample_begin(sample);
			iterator->resampler_data =
				psy_dsp_multiresampler_base(&self->resampler)->vtable->getresamplerdata(
					psy_dsp_multiresampler_base(&self->resampler));
			psy_list_append(&self->positions, iterator);
			if (self->instrument->loop && self->sampler) {
				psy_dsp_beat_t bpl;
				double totalsamples;
				
				bpl = psy_audio_machine_currbeatsperline(
					psy_audio_sampler_base(self->sampler));		
				totalsamples = samplesprobeat * bpl * self->instrument->lines;
				psy_audio_sampleiterator_setspeed(iterator,
					sample->numframes / (double)totalsamples);
			} else {
				if (self->sampler->amigaslides) {
					self->period = (int)psy_dsp_notetoamigaperiod(event->note,
						sample->samplerate,
						sample->tune +
						NOTECOMMANDS_MIDDLEC - self->sampler->basec,
						sample->finetune);
				} else {
					self->period = (int)psy_dsp_notetoperiod(event->note, sample->tune +
						NOTECOMMANDS_MIDDLEC - self->sampler->basec,
						sample->finetune);
				}
				psy_audio_samplervoice_updateiteratorspeed(self, iterator);
			}
			psy_audio_sampleiterator_play(iterator);
			psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
				&self->resampler),
				iterator->resampler_data,
				iterator->speed * 1/ 4294967296.0f);
		}
	}	
	psy_list_free(entries);	
	if (self->positions) {		
		psy_dsp_adsr_start(&self->env);		
		psy_dsp_adsr_start(&self->filterenv);
	}
	if (!self->dopan && self->instrument->randompan) {
		self->dopan = 1; 
		self->pan = rand() / (psy_dsp_amp_t) 32768.f;
	}	
}

void psy_audio_samplervoice_updatespeed(psy_audio_SamplerVoice* self)
{
	if (self->positions && self->env.stage != ENV_OFF) {
		psy_List* p;

		for (p = self->positions; p != NULL; p = p->next) {
			psy_audio_SampleIterator* it;

			it = (psy_audio_SampleIterator*)p->entry;
			psy_audio_samplervoice_updateiteratorspeed(self, it);
		}
	}
}

void psy_audio_samplervoice_updateiteratorspeed(psy_audio_SamplerVoice* self,
	psy_audio_SampleIterator* it)
{
	int period;
	double speed;
	
	period = self->period;
	speed = self->sampler->amigaslides
		? psy_dsp_amigaperiodtospeed(period,
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)),
			0)
		: psy_dsp_periodtospeed(period,
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)),
			it->sample->samplerate, 0);
	//\todo: Attention, AutoVibrato always use linear slides with IT, but in FT2 it depends on amigaslides switch.	
	// speed *= pow(2.0, ((-AutoVibratoAmount()) / 768.0));	
	psy_audio_sampleiterator_setspeed(it, speed);	
}

void psy_audio_samplervoice_noteon_frequency(psy_audio_SamplerVoice* self, double frequency)
{
	psy_audio_Sample* sample;
	psy_List* entries;
	psy_List* p;

	psy_audio_samplervoice_clearpositions(self);
	entries = psy_audio_instrument_entriesintersect(self->instrument,
		0, 0, frequency);
	for (p = entries; p != NULL; p = p->next) {
		psy_audio_InstrumentEntry* entry;

		entry = (psy_audio_InstrumentEntry*)p->entry;
		sample = psy_audio_samples_at(self->samples, entry->sampleindex);
		if (sample) {
			psy_audio_SampleIterator* iterator;

			iterator = psy_audio_sampleiterator_alloc();
			*iterator = psy_audio_sample_begin(sample);
			iterator->resampler_data =
				psy_dsp_multiresampler_base(&self->resampler)->vtable->getresamplerdata(
					psy_dsp_multiresampler_base(&self->resampler));
			psy_list_append(&self->positions, iterator);
			psy_audio_sampleiterator_setspeed(iterator, frequency / 440);
			psy_audio_sampleiterator_play(iterator);
			psy_dsp_resampler_setspeed(psy_dsp_multiresampler_base(
				&self->resampler),
				iterator->resampler_data,
				iterator->speed * 1 / 4294967296.0f);
		}
	}
	psy_list_free(entries);
	if (self->positions) {
		psy_dsp_adsr_start(&self->env);
		psy_dsp_adsr_start(&self->filterenv);
	}
	if (!self->dopan && self->instrument->randompan) {
		self->dopan = 1;
		self->pan = rand() / (psy_dsp_amp_t)32768.f;
	}
}

void psy_audio_samplervoice_clearpositions(psy_audio_SamplerVoice* self)
{
	psy_List* p;

	for (p = self->positions; p != NULL; p = p->next) {
		psy_audio_SampleIterator* iterator;

		iterator = (psy_audio_SampleIterator*)p->entry;
		psy_dsp_multiresampler_base(&self->resampler)->vtable->disposeresamplerdata(
			psy_dsp_multiresampler_base(&self->resampler),
			iterator->resampler_data);
		psy_audio_sampleiterator_dispose(iterator);
		free(iterator);
	}
	psy_list_free(self->positions);
	self->positions = 0;
}

void psy_audio_samplervoice_nna(psy_audio_SamplerVoice* self)
{
	if (self->instrument) {
		switch (psy_audio_instrument_nna(self->instrument)) {
			case psy_audio_NNA_STOP:
				psy_audio_samplervoice_fastnoteoff(self);
			break;
			case psy_audio_NNA_NOTEOFF:
				psy_audio_samplervoice_noteoff(self);
			break;
			case psy_audio_NNA_CONTINUE:				
			break;
			default:
				// note cut
				psy_audio_samplervoice_fastnoteoff(self);
			break;
		}
	}
}

void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice* self)
{
	psy_dsp_adsr_release(&self->env);
	psy_dsp_adsr_release(&self->filterenv);
	self->stopping = TRUE;
}

void psy_audio_samplervoice_fastnoteoff(psy_audio_SamplerVoice* self)
{
	psy_dsp_adsr_fastrelease(&self->env);
	psy_dsp_adsr_fastrelease(&self->filterenv);
	self->stopping = TRUE;
}

void psy_audio_samplervoice_work(psy_audio_SamplerVoice* self,
	psy_audio_Buffer* output, uintptr_t amount)
{
	if (self->positions && self->env.stage != ENV_OFF) {
		psy_List* p;
		psy_dsp_amp_t* env;
		psy_dsp_amp_t* filterenv;
		uintptr_t i;		
		
		env = malloc(amount * sizeof(psy_dsp_amp_t));
		for (i = 0; i < amount; ++i) {
			psy_dsp_adsr_tick(&self->env);
			env[i] = self->env.value;
		}
		filterenv = NULL;
		if (filter_type(&self->_filter) != F_NONE) {
			filterenv = malloc(amount * sizeof(psy_dsp_amp_t));
			for (i = 0; i < amount; ++i) {
				psy_dsp_adsr_tick(&self->filterenv);
				filterenv[i] = self->filterenv.value;
			}
		}

		for (p = self->positions; p != NULL; p = p->next) {
			psy_audio_SampleIterator* position;
			psy_dsp_amp_t svol;
			psy_dsp_amp_t rvol;
			psy_dsp_amp_t lvol;
			uintptr_t dstpos;
			uintptr_t numsamples;
			
			position = (psy_audio_SampleIterator*)p->entry;
			if (self->dooffset) {
				psy_audio_sampleiterator_dooffset(position, self->offset);
			}			
			psy_audio_samplervoice_currvolume(self, position->sample, &svol,
				&lvol, &rvol);
			dstpos = 0;
			numsamples = amount;
			while (numsamples) {
				uintptr_t channel;
				intptr_t diff;

				intptr_t nextsamples = min(psy_audio_sampleiterator_prework(position,
					numsamples, FALSE), numsamples);
				numsamples -= nextsamples;
				while (nextsamples)
				{
					for (channel = 0; channel < psy_audio_buffer_numchannels(&position->sample->channels) &&
							channel < psy_audio_buffer_numchannels(output); ++channel) {
						psy_dsp_amp_t val;

						val = psy_audio_samplervoice_unprocessed_wavedata(self,
							position, channel);
						val = psy_audio_samplervoice_processenvelopes(self,
							channel, val, dstpos, env, filterenv, svol, lvol, rvol);
						psy_audio_samplervoice_adddatatosamplerbuffer(self, channel,
							val, dstpos, output);
					}					
					++dstpos;
					nextsamples--;
					diff = psy_audio_sampleiterator_inc(position);
					position->m_pL += diff;
					if (psy_audio_buffer_numchannels(&position->sample->channels) > 1) {
						position->m_pR += diff;
					}
				}				
				psy_audio_sampleiterator_postwork(position);
				if (!psy_audio_sampleiterator_playing(position)) {
					psy_audio_samplervoice_reset(self);
					break;
				}
			}
			if (psy_audio_buffer_mono(&position->sample->channels)) {
				psy_audio_buffer_make_monoaureal(output, amount);				
			}			
		}		
		free(env);
		free(filterenv);
	}
	self->dooffset = 0;
}

psy_dsp_amp_t psy_audio_samplervoice_unprocessed_wavedata(psy_audio_SamplerVoice* self,
	psy_audio_SampleIterator* it, uintptr_t channel)
{
	psy_dsp_amp_t* src;

	src = psy_audio_buffer_at(&it->sample->channels, channel);	
	return psy_dsp_resampler_work_float_unchecked(
		psy_dsp_multiresampler_base(&self->resampler),
		(channel == 0) ? it->m_pL : it->m_pR,
		it->pos.LowPart,
		it->resampler_data);
}

void psy_audio_samplervoice_setresamplerquality(psy_audio_SamplerVoice* self, 
	ResamplerType quality)
{
	psy_dsp_multiresampler_settype(&self->resampler, quality);
}

psy_dsp_amp_t psy_audio_samplervoice_processenvelopes(psy_audio_SamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_dsp_amp_t* env, psy_dsp_amp_t* filterenv,
	psy_dsp_amp_t svol, psy_dsp_amp_t lvol, psy_dsp_amp_t rvol)
{
	psy_dsp_amp_t rv;
	psy_dsp_amp_t volume;

	rv = psy_audio_samplervoice_workfilter(self, channel, input, filterenv, pos);
	volume = env[pos] * self->instrument->globalvolume;
	if (filterenv) {
		volume *= filterenv[pos];
	}
	if (channel == 0) {
		volume *= lvol;
	} else if (channel == 1) {
		volume *= rvol;
	} else {
		volume *= svol;
	}
	rv *= volume;
	return rv;
}

void psy_audio_samplervoice_adddatatosamplerbuffer(psy_audio_SamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, uintptr_t pos,
	psy_audio_Buffer* output)
{
	psy_dsp_amp_t* dst;

	dst = psy_audio_buffer_at(output, channel);
	if (dst) {
		dst[pos] += input;
	}
}

void psy_audio_samplervoice_currvolume(psy_audio_SamplerVoice* self,
	psy_audio_Sample* sample, psy_dsp_amp_t* svol, psy_dsp_amp_t* lvol,
	psy_dsp_amp_t* rvol)
{
	psy_dsp_amp_t cvol;
	psy_dsp_amp_t panning;

	*svol = psy_audio_samplervoice_volume(self, sample) *
		//(self->usedefaultvolume || self->effcmd.id == SAMPLER_CMD_VOLUMESLIDE)
			//? sample->defaultvolume 
			self->vol;
	cvol = self->channel ? self->channel->volume : (psy_dsp_amp_t)1.f;
	if (self->channel && self->sampler->panpersistent) {
		panning = self->channel->panfactor;
	} else {
		panning = self->dopan ? self->pan : sample->panfactor;
	}
	*rvol = panning * *svol * cvol;
	*lvol = ((psy_dsp_amp_t)1.f - panning) * *svol * cvol;
	//FT2 Style (Two slides) mode, but with max amp = 0.5.
	if (*rvol > self->sampler->clipmax) {
		*rvol = (psy_dsp_amp_t)self->sampler->clipmax;
	}
	if (*lvol > self->sampler->clipmax) {
		*lvol = (psy_dsp_amp_t)self->sampler->clipmax;
	}
}

psy_dsp_amp_t psy_audio_samplervoice_workfilter(psy_audio_SamplerVoice* self,
	uintptr_t channel, psy_dsp_amp_t input, psy_dsp_amp_t* filterenv, uintptr_t pos)
{		
	if (filter_type(&self->_filter) != F_NONE)
	{
		int newcutoff = (int)(self->_cutoff + (filterenv[pos] * self->_coModify + 0.5f));
		if (newcutoff < 0) {
			newcutoff = 0;
		} else if (newcutoff > 127) {
			newcutoff = 127;
		}
		filter_setcutoff(&self->_filter, newcutoff);		
		return input = self->_filter.vtable->work(&self->_filter, input);
	}
	return input;
}

void psy_audio_samplervoice_release(psy_audio_SamplerVoice* self)
{
	self->effcmd.id = SAMPLER_CMD_NONE;
	psy_dsp_adsr_release(&self->env);	
	psy_dsp_adsr_release(&self->filterenv);
}

void psy_audio_samplervoice_fastrelease(psy_audio_SamplerVoice* self)
{
	self->effcmd.id = SAMPLER_CMD_NONE;
	psy_dsp_adsr_fastrelease(&self->env);	
	psy_dsp_adsr_fastrelease(&self->filterenv);
}

psy_List* sequencerinsert(psy_audio_Sampler* self, psy_List* events)
{
	psy_List* p;
	psy_List* insert = 0;

	for (p = events; p != NULL; p = p->next) {
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
			bool m_bAmigaSlides;// Using Linear or Amiga Slides.
			bool m_UseFilters;
			int32_t m_GlobalVolume;
			int32_t m_PanningMode;

			// numSubtracks
			psyfile_read(songfile->file, &temp, sizeof(temp));
			self->numvoices = temp;
			// quality
			psyfile_read(songfile->file, &temp, sizeof(temp));
			switch (temp)
			{
				case 2:	psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_SPLINE); break;
				case 3:	psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_SINC); break;
				case 0:	psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_ZERO_ORDER); break;
				case 1:
				default:
					psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_LINEAR);
				break;
			}

			for (i = 0; i < 128; ++i) {
				psyfile_read(songfile->file, &zxxMap[i].mode, sizeof(zxxMap[i].mode));
				psyfile_read(songfile->file, &zxxMap[i].value, sizeof(zxxMap[i].value));
			}

			psyfile_read(songfile->file, &m_bAmigaSlides, sizeof(m_bAmigaSlides));
			self->amigaslides = m_bAmigaSlides;
			psyfile_read(songfile->file, &m_UseFilters, sizeof(m_UseFilters));
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
				case 2:	psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_SPLINE); break;
				case 3:	psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_SINC); break;
				case 0:	psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_ZERO_ORDER); break;
				case 1:
				default:
					psy_audio_sampler_setresamplerquality(self, RESAMPLERTYPE_LINEAR);
				break;
			}
			if (size > 3 * sizeof(unsigned int))
			{
				unsigned int internalversion;
				psyfile_read(songfile->file, &internalversion, sizeof(internalversion));
				if (internalversion >= 1) {
					uint8_t defaultC4;

					psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4)); // correct A4 frequency.
					psy_audio_sampler_defaultC4(self, defaultC4 != 0);
				}
				if (internalversion >= 2) {
					unsigned char slidemode;
					psyfile_read(songfile->file, &slidemode, sizeof(slidemode)); // correct slide.
					// LinearSlide(slidemode);
				}
				if (internalversion >= 3) {
					uint32_t instrbank;
					psyfile_read(songfile->file, &instrbank, sizeof(instrbank)); // instrument bank.
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
	for (int i = 0; i < 128; i++) {
		psyfile_write_int32(songfile->file, 0); // zxxMap[i].mode);
		psyfile_write_int32(songfile->file, 0); // zxxMap[i].value);
	}
	psyfile_write_uint8(songfile->file, (uint8_t)(self->amigaslides != 0));
	psyfile_write_uint8(songfile->file, self->usefilters);
	psyfile_write_int32(songfile->file, 128);
	psyfile_write_int32(songfile->file, psy_audio_PANNING_LINEAR);

	for (int i = 0; i < MAX_TRACKS; i++) {		
		psy_audio_samplerchannel_save(sampler_channel(self, i),
			songfile);
	}

	psyfile_write_uint32(songfile->file, (uint32_t)self->instrumentbank);
	size_t endpos = psyfile_getpos(songfile->file);
	psyfile_seek(songfile->file, filepos);
	size = (uint32_t)(endpos - filepos - sizeof(size));
	psyfile_write_uint32(songfile->file, size);
	psyfile_seek(songfile->file, endpos);
}

void psy_audio_sampler_setresamplerquality(psy_audio_Sampler* self,
	ResamplerType quality)
{
	psy_List* it;
	
	for (it = self->voices; it != NULL; it = it->next) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)it->entry;
		psy_audio_samplervoice_setresamplerquality(voice, quality);
	}
}
