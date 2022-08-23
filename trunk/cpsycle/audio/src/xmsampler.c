// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsampler.h"
// local
#include "constants.h"
#include "instruments.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "songio.h"
#include "samples.h"
// dsp
#include <valuemapper.h>
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

static int psy_audio_xmsampler_getdelta(psy_audio_WaveForms wavetype,
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

static psy_audio_MachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64 | psy_audio_SUPPORTS_INSTRUMENTS,
	psy_audio_MACHMODE_GENERATOR,
	"Sampulse"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampulse",
	"Psycledelics",
	"help",
	psy_audio_XMSAMPLER,
	NULL,		// NO MODULPATH
	0,			// shellidx	
	SAMPLERHELP,	// help text
	"Sampler with the essence of FastTracker II and Impulse Tracker II.",	// desc
	"Sampler"		// category
};

// prototypes
static void dispose(psy_audio_XMSampler*);
static void disposeparameters(psy_audio_XMSampler*);
static void disposevoices(psy_audio_XMSampler*);
static void disposechannels(psy_audio_XMSampler*);
static const psy_audio_MachineInfo* xmsampler_info(psy_audio_XMSampler*);
static void psy_audio_xmsampler_clearmulticmdmem(psy_audio_XMSampler*);
static uintptr_t psy_audio_xmsampler_numinputs(psy_audio_XMSampler* self) { return 0; }
static uintptr_t psy_audio_xmsampler_numoutputs(psy_audio_XMSampler* self) { return 2; }
static void psy_audio_xmsampler_newline(psy_audio_XMSampler*);
static int loadspecificchunk(psy_audio_XMSampler*, psy_audio_SongFile*,
	uintptr_t slot);
static int savespecificchunk(psy_audio_XMSampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void psy_audio_xmsampler_on_timertick(psy_audio_XMSampler*);
static void psy_audio_xmsampler_on_timerwork(psy_audio_XMSampler*,
	psy_audio_BufferContext*);
static void generateaudio(psy_audio_XMSampler*, psy_audio_BufferContext*);
static void psy_audio_xmsampler_stop(psy_audio_XMSampler*);
static uintptr_t numparametercols(psy_audio_XMSampler*);
static uintptr_t numparameters(psy_audio_XMSampler*);
static uintptr_t numtweakparameters(psy_audio_XMSampler*);
static psy_audio_MachineParam* parameter(psy_audio_XMSampler*,
	uintptr_t param);
static psy_audio_MachineParam* tweakparameter(psy_audio_XMSampler*,
	uintptr_t param);
static void ontweakzxxvalue(psy_audio_XMSampler*,
	psy_audio_IntMachineParam* sender, float value);
static void psy_audio_xmsampler_initparameters(psy_audio_XMSampler*);
static void resamplingmethod_tweak(psy_audio_XMSampler*,
	psy_audio_ChoiceMachineParam* sender, float value);
static void psy_audio_xmsampler_setresamplerquality(psy_audio_XMSampler*,
	psy_dsp_ResamplerQuality);
static void display_tweak(psy_audio_XMSampler*,
	psy_audio_IntMachineParam* sender, float value);
static void display_normvalue(psy_audio_XMSampler*,
	psy_audio_IntMachineParam* sender, float* rv);
static void masterlevel_normvalue(psy_audio_XMSampler* self,
	psy_audio_CustomMachineParam* sender, float* rv);
static uintptr_t paramstrobe(const psy_audio_XMSampler*);

const psy_audio_MachineInfo* psy_audio_xmsampler_info(void)
{
	return &macinfo;
}

static MachineVtable sampler_vtable;
static bool sampler_vtable_initialized = FALSE;

static void sampler_vtable_init(psy_audio_XMSampler* self)
{
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_xmsampler_base(self)->vtable);
		sampler_vtable.dispose = (fp_machine_dispose)dispose;
		sampler_vtable.numinputs = (fp_machine_numinputs)psy_audio_xmsampler_numinputs;
		sampler_vtable.numoutputs = (fp_machine_numoutputs)psy_audio_xmsampler_numoutputs;
		sampler_vtable.info = (fp_machine_info)xmsampler_info;
		sampler_vtable.newline = (fp_machine_newline)psy_audio_xmsampler_newline;
		sampler_vtable.seqtick = (fp_machine_seqtick)psy_audio_xmsampler_tick;
		sampler_vtable.loadspecific = (fp_machine_loadspecific)loadspecificchunk;
		sampler_vtable.savespecific = (fp_machine_savespecific)savespecificchunk;
		sampler_vtable.generateaudio = (fp_machine_generateaudio)generateaudio;	
		sampler_vtable.stop = (fp_machine_stop)psy_audio_xmsampler_stop;				
		sampler_vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		sampler_vtable.numparameters = (fp_machine_numparameters)numparameters;
		sampler_vtable.numtweakparameters = (fp_machine_numparameters)numtweakparameters;
		sampler_vtable.parameter = (fp_machine_parameter)parameter;
		sampler_vtable.tweakparameter = (fp_machine_parameter)tweakparameter;
		sampler_vtable.paramstrobe = (fp_machine_paramstrobe)paramstrobe;
		sampler_vtable_initialized = TRUE;
	}
}

int psy_audio_xmsampler_init(psy_audio_XMSampler* self,
	psy_audio_MachineCallback* callback)
{
	int status;
	int i;

	status = PSY_OK;
	psy_audio_custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_xmsampler_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_xmsampler_base(self), "XMSampler");

	self->_numVoices = XM_SAMPLER_MAX_POLYPHONY;
	self->playingVoices = 0;
	self->multicmdMem = NULL;	
	self->m_sampleRate = (int)
		psy_audio_machine_samplerate(psy_audio_xmsampler_base(self));
	self->m_bAmigaSlides = FALSE;
	self->m_UseFilters = TRUE;
	self->m_GlobalVolume = 128;
	self->m_PanningMode = psy_audio_PANNING_LINEAR;	
	self->channelbank = 0;
	self->instrumentbank = 1;
	self->strobe = 0;

	for (i = 0; i < XM_SAMPLER_MAX_POLYPHONY; i++)
	{
		psy_audio_xmsamplervoice_init(&self->m_Voices[i]);
		psy_audio_xmsamplervoice_setpsampler(&self->m_Voices[i], self);		
		psy_audio_xmsamplervoice_reset(&self->m_Voices[i]);
	}

	for (i = 0; i < MAX_TRACKS; i++)
	{
		// mfc-psycle (implicit constructor call)
		psy_audio_xmsamplerchannel_init(&self->m_Channel[i]);
		psy_audio_xmsamplerchannel_setpsampler(&self->m_Channel[i], self);
		psy_audio_xmsamplerchannel_setindex(&self->m_Channel[i], i);
		psy_audio_xmsamplerchannel_initparamview(&self->m_Channel[i]);
	}
	
	psy_audio_xmsampler_setresamplerquality(self,
		psy_dsp_RESAMPLERQUALITY_LINEAR);

	psy_audio_ticktimer_init(&self->ticktimer,
		self, // callback context (sampler)
		(fp_samplerticktimer_ontick)psy_audio_xmsampler_on_timertick,
		(fp_samplerticktimer_onwork)psy_audio_xmsampler_on_timerwork);
	
	psy_audio_xmsampler_initparameters(self);
	return status;
}

void dispose(psy_audio_XMSampler* self)
{
	psy_audio_xmsampler_clearmulticmdmem(self);
	disposeparameters(self);
	disposevoices(self);
	disposechannels(self);	
	psy_audio_custommachine_dispose(&self->custommachine);
}

void disposevoices(psy_audio_XMSampler* self)
{
	int i;

	for (i = 0; i < XM_SAMPLER_MAX_POLYPHONY; i++)
	{
		psy_audio_xmsamplervoice_dispose(&self->m_Voices[i]);
	}
}

void disposechannels(psy_audio_XMSampler* self)
{
	int i;

	for (i = 0; i < MAX_TRACKS; i++)
	{
		psy_audio_xmsamplerchannel_dispose(&self->m_Channel[i]);
	}
}


void psy_audio_xmsampler_setsamplerate(psy_audio_XMSampler* self,
	int sr)
{	
	int i;

	self->m_sampleRate = sr;
	for (i = 0; i < self->_numVoices; i++)
	{
		if (psy_audio_xmsamplervoice_isplaying(&self->m_Voices[i])) {
			//\todo
			//update envelopes
			//
		}
	}
}

void psy_audio_xmsampler_on_timertick(psy_audio_XMSampler* self)
{			
	int channel;

	// Do the Tick jump.
	for (channel = 0; channel < MAX_TRACKS; channel++)
	{		
		psy_audio_xmsamplerchannel_performfx(
			psy_audio_xmsampler_rchannel(self, channel));
	}
	self->playingVoices = psy_audio_xmsampler_getplayingvoices(self);
}

void generateaudio(psy_audio_XMSampler* self, psy_audio_BufferContext* bc)
{	
	psy_audio_ticktimer_update(&self->ticktimer, bc->numsamples, bc);
}

void psy_audio_xmsampler_on_timerwork(psy_audio_XMSampler* self,
	psy_audio_BufferContext* bc)
{
	float multip;
		
	psy_audio_xmsampler_workvoices(self, bc);
	// Apply the global volume to the final mix.
	multip = psy_dsp_map_128_1(self->m_GlobalVolume);
	psy_audio_buffer_mulsamples(bc->output, bc->numsamples,
		multip);
}

void psy_audio_xmsampler_workvoices(psy_audio_XMSampler* self,
	psy_audio_BufferContext* bc)
{
	int voice;

	//psy_audio_buffer_clearsamples(bc->output, bc->numsamples);
	for (voice = 0; voice < self->_numVoices; voice++)
	{
		if (psy_audio_xmsamplervoice_isplaying(&self->m_Voices[voice])) {
			psy_audio_xmsamplervoice_work(&self->m_Voices[voice],
				(int)bc->numsamples,
				psy_audio_buffer_at(bc->output, 0),
				psy_audio_buffer_at(bc->output, 1));			
		}
	}
}

void psy_audio_xmsampler_newline(psy_audio_XMSampler* self)
{
	int channel;
	int voice;

	// SampleCounter(0);
	assert(self);

	psy_audio_xmsampler_clearmulticmdmem(self);	
	psy_audio_ticktimer_reset(&self->ticktimer,
		(uintptr_t)psy_audio_machine_samplespertick(
			psy_audio_xmsampler_base(self)));

	//NextSampleTick(Global::player().SamplesPerTick());	
	for (channel = 0; channel < MAX_TRACKS; channel++)
	{
		psy_audio_xmsamplerchannel_seteffectflags(
			psy_audio_xmsampler_rchannel(self, channel), 0);
	}

	for (voice = 0; voice < self->_numVoices; voice++)
	{
		if (psy_audio_xmsamplervoice_isplaying(&self->m_Voices[voice])) {
			psy_audio_xmsamplervoice_newline(&self->m_Voices[voice]);
		}		
	}
	
}

// mfc-psycle: XMSampler::PostNewLine()
//			   Event Order in mfc-psycle is newline; executeline; postnewline
//			   Sequencer can execute notes at every time, so a postnewline
//			   can't be implemented in this way. Instead multicmds are cleared
//			   before notifyline. clearmulticmdmem; notifynewline;
//             Problems may occur if notes are inside a line.
//             Multicmds may be called twice
void psy_audio_xmsampler_clearmulticmdmem(psy_audio_XMSampler* self)
{
	assert(self);
	psy_list_deallocate(&self->multicmdMem, (psy_fp_disposefunc)NULL);
}

void psy_audio_xmsampler_tick(psy_audio_XMSampler* self,
	uintptr_t channelNum, const psy_audio_PatternEvent* pEvData)
{
	bool bInstrumentSet;
	bool bPortaEffect;
	psy_List* ite;
	bool bPorta2Note;
	bool bNoteOn;
	psy_audio_XMSamplerVoice* currentVoice;
	psy_audio_XMSamplerVoice* newVoice;
	psy_audio_XMSamplerChannel* thisChannel;
	psy_audio_PatternEvent transform_event;
	psy_audio_PatternEvent* pData;

	transform_event = *pEvData;
	if (transform_event.inst == psy_audio_NOTECOMMANDS_INST_EMPTY) {
		transform_event.inst = 255;
	}
	pData = &transform_event;
	if (pData->note == psy_audio_NOTECOMMANDS_MIDICC && pData->inst < MAX_TRACKS)
	{
		//TODO: This has one problem, it requires a non-mcm command to trigger the memory.				
		psy_audio_PatternEvent* cmdmem;

		cmdmem = psy_audio_patternevent_clone(pData);
		cmdmem->inst = (uint16_t)channelNum;
		psy_list_append(&self->multicmdMem, cmdmem);
		return;
	}	
	// don't process twk , twf of Mcm Commands
	else if (pData->note > psy_audio_NOTECOMMANDS_RELEASE && pData->note != psy_audio_NOTECOMMANDS_EMPTY) {
		return;
	}

	// define some variables to ease the case checking.
	bInstrumentSet =  (pData->inst < 255);
	bPortaEffect = FALSE;
	for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ite_ev;

		ite_ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ite_ev->inst == (uint16_t)channelNum) {
			bPortaEffect |= (ite_ev->cmd == XM_SAMPLER_CMD_PORTA2NOTE)
				|| (ite_ev->cmd == XM_SAMPLER_CMD_SENDTOVOLUME && (ite_ev->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_TONEPORTAMENTO);
		}
	}
	bPortaEffect |= (pData->cmd == XM_SAMPLER_CMD_PORTA2NOTE) || (pData->cmd == XM_SAMPLER_CMD_SENDTOVOLUME && (pData->parameter & 0xF0) == XM_SAMPLER_CMD_VOL_TONEPORTAMENTO);
	bPorta2Note = (pData->note <= psy_audio_NOTECOMMANDS_B9) && bPortaEffect;
	bNoteOn = (pData->note <= psy_audio_NOTECOMMANDS_B9) && !bPorta2Note;

	currentVoice = NULL;
	newVoice = NULL;
	thisChannel = psy_audio_xmsampler_rchannel(self, (int)channelNum);
	// STEP A: Look for an existing (foreground) playing voice in the current channel.
	currentVoice = psy_audio_xmsampler_getcurrentvoice(self, (int)channelNum);
	if (currentVoice)
	{		
		//This forces Last volume/pan/envelope values to be up-to-date.
		psy_audio_xmsamplerchannel_setforegroundvoice(thisChannel,
			currentVoice);
		if (bPorta2Note)
		{
			//\todo : portamento to note, if the note corresponds to a new sample, the sample gets changed and the position reset to 0.
			psy_audio_xmsamplerchannel_setnote(thisChannel,
				pData->note);
			// If there current voice is stopping, the porta makes the note active again and restarts the envelope, without restarting the sample.
			if (psy_audio_xmsamplervoice_isstopping(currentVoice)) {
				psy_audio_xmsamplervoice_setisstopping(currentVoice, FALSE);		
				xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_amplitudeenvelope(
					currentVoice));
				xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_panenvelope(
					currentVoice));
				xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_filterenvelope(
					currentVoice));
				xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_pitchenvelope(
					currentVoice));				
			}
		} else {
			if (bInstrumentSet)
			{
				//This is present in some "MOD" format files, and is supported by Impulse Tracker/Modplug/Schism, differently/buggy in ST2/ST3 
				// (in st2, note is triggered always, and in st3 it changes the instrument but continues from where it was) and not by FT2/XMPlay.
				//Concretely if an instrument comes without a note, it is different than the one that is playing, and is not stopping (no noteoff received),
				//then, the new instrument is triggered with the last note played.
				//Note that if the instrument is the same, it is only retriggered if it has stopped (See below).
				if (!bNoteOn && pData->note != psy_audio_NOTECOMMANDS_RELEASE && psy_audio_xmsamplerchannel_instrumentno(thisChannel) != pData->inst && psy_audio_xmsamplerchannel_note(thisChannel) != psy_audio_NOTECOMMANDS_RELEASE) {
					bNoteOn = TRUE;
				}
			}
			// Is a new note coming? Then apply the NNA to the playing one.
			if (bNoteOn)
			{				
				switch (psy_audio_instrument_dct(psy_audio_xmsamplervoice_rinstrument(currentVoice)))
				{
				case psy_audio_DUPECHECK_INSTRUMENT:
					//todo: if no instrument specified... should we consider the previous instrument?
					if (pData->inst == psy_audio_xmsamplerchannel_instrumentno(thisChannel))
					{
						if (psy_audio_instrument_dca(psy_audio_xmsamplervoice_rinstrument(currentVoice)) <
							psy_audio_instrument_nna(psy_audio_xmsamplervoice_rinstrument(currentVoice))) {
							psy_audio_xmsamplervoice_setnna(currentVoice,
								psy_audio_instrument_dca(psy_audio_xmsamplervoice_rinstrument(currentVoice)));
						}
					}
					break;
				case psy_audio_DUPECHECK_SAMPLE:
				{
					//todo: if no instrument specified... should we consider the previous instrument?
					// const XMInstrument& _inst = Global::song().xminstruments[thisChannel.InstrumentNo()];
					// int _layer = _inst.NoteToSample(thisChannel.Note()).second;
					// if (_layer == thisChannel.ForegroundVoice()->rWave().Layer())
					//{
					//	if (currentVoice->rInstrument().DCA() < currentVoice->NNA()) currentVoice->NNA(currentVoice->rInstrument().DCA());
					//}
				}
				break;
				case psy_audio_DUPECHECK_NOTE:
					if (pData->note == psy_audio_xmsamplerchannel_note(thisChannel) && pData->inst == psy_audio_xmsamplerchannel_instrumentno(thisChannel))
					{
						if (psy_audio_instrument_dca(psy_audio_xmsamplervoice_rinstrument(currentVoice)) <
							psy_audio_instrument_nna(psy_audio_xmsamplervoice_rinstrument(currentVoice))) {
							psy_audio_xmsamplervoice_setnna(currentVoice,
								psy_audio_instrument_dca(psy_audio_xmsamplervoice_rinstrument(currentVoice)));
						}
					}
					break;
				default:
					break;
				}				
				switch (psy_audio_xmsamplervoice_nna(currentVoice))
				{
				case psy_audio_NNA_STOP:
					psy_audio_xmsamplervoice_noteofffast(currentVoice);
					break;
				case psy_audio_NNA_NOTEOFF:
					psy_audio_xmsamplervoice_noteoff(currentVoice);					
					break;
				case psy_audio_NNA_FADEOUT:
					psy_audio_xmsamplervoice_notefadeout(currentVoice);
					break;
				default:
					break;
				}
				
				//This is necessary for the notedelay command (Because it will enter twice in this method).
				//Else, the call to channel->ForegroundVoice() below would be enough.
				psy_audio_xmsamplervoice_setisbackground(currentVoice, TRUE);				
			} else if (pData->note == psy_audio_NOTECOMMANDS_RELEASE) {
				psy_audio_xmsamplervoice_noteoff(currentVoice);
				psy_audio_xmsamplerchannel_stopbackgroundnotes(thisChannel, psy_audio_NNA_NOTEOFF);				
				psy_audio_xmsamplerchannel_setnote(thisChannel, pData->note);				
			}
		}
	} else if (bPorta2Note)
	{
		// If there is a Porta2Note command, but there is no voice playing, this is converted to a noteOn.
		bPorta2Note = FALSE; bNoteOn = TRUE;
	} else {
		//This is present in some "MOD" format files, and is supported by Impulse Tracker/Modplug/Schism, but not in ST3/FT2/XMPlay
		//Concretely if a sample has been played, has stopped (by reaching the end, not by noteoff/notecut), and an instrument 
		// comes again without a note (be it the same or another), the (new) instrument is played with the previous note of the channel.
		if (bInstrumentSet && !bNoteOn && psy_audio_xmsamplerchannel_note(thisChannel) != psy_audio_NOTECOMMANDS_RELEASE && pData->note == psy_audio_NOTECOMMANDS_EMPTY)
		{
			bNoteOn = TRUE;
		}
	}
	
	if (bInstrumentSet && pData->note != psy_audio_NOTECOMMANDS_RELEASE) {
		// Instrument is always set, even if no new note comes, or no voice playing.
		//\todo: Fix: Set the wave and instrument to the one in the entry. Only if not portatonote.
		psy_audio_xmsamplerchannel_setinstrumentno(thisChannel, pData->inst);		
		if (currentVoice != NULL && !bNoteOn && pData->note != psy_audio_NOTECOMMANDS_RELEASE) {
			//Whenever an instrument appears alone in a channel, the values are reset.
			//todo: It should be reset to the values of the instrument set.
			psy_audio_xmsamplervoice_resetvolandpan(currentVoice, -1, TRUE);

			//Restart also the envelopes
			//This is an FT2 feature (and compatibles, like Modplug in ft2 mode, cubic player,...),
			//but it is not done in Impulse tracker (and compatibles, like schismtracker).
			xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_amplitudeenvelope(
					currentVoice));
			xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_panenvelope(
				currentVoice));
			xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_filterenvelope(
				currentVoice));
			xmenvelopecontroller_noteon(psy_audio_xmsamplervoice_pitchenvelope(
				currentVoice));
		}
	}
	// STEP B: Get a new Voice to work with, and initialize it if needed.
	if (bNoteOn)
	{		
		bool delayed = FALSE;
		for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
			psy_audio_PatternEvent* ite_ev;

			ite_ev = (psy_audio_PatternEvent*)psy_list_entry(ite);		
			if (ite_ev->inst == channelNum) {
				if (ite_ev->cmd == XM_SAMPLER_CMD_EXTENDED && (ite_ev->parameter & 0xf0) == XM_SAMPLER_CMD_E_NOTE_DELAY) {
					delayed = TRUE;
				}
			}
		}
		if (pData->cmd == XM_SAMPLER_CMD_EXTENDED && (pData->parameter & 0xf0) == XM_SAMPLER_CMD_E_NOTE_DELAY)
		{
			delayed = TRUE;
		}
		if (delayed) {
			for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
				psy_audio_PatternEvent* ite_ev;

				ite_ev = (psy_audio_PatternEvent*)psy_list_entry(ite);			
				if (ite_ev->inst == channelNum) {
					//Include also the mcm commands to the delay.					
					psy_audio_xmsamplerchannel_delayednote(thisChannel,
						*ite_ev);
				}
			}
			psy_audio_xmsamplerchannel_delayednote(thisChannel,
				*pData);			
		} else
		{
			psy_audio_Instrument* _inst;
			psy_audio_InstrumentIndex index;
			
			if (pData->note != psy_audio_NOTECOMMANDS_EMPTY) {
				// If instrument set and no note, we don't want to reset the note.
				psy_audio_xmsamplerchannel_setnote(thisChannel, pData->note);
			}
			newVoice = psy_audio_xmsampler_getfreevoice(self,
				psy_audio_xmsamplerchannel_index(thisChannel));
			index = psy_audio_instrumentindex_make(self->instrumentbank,
				psy_audio_xmsamplerchannel_instrumentno(thisChannel));
			_inst = psy_audio_instruments_at(psy_audio_machine_instruments(
				psy_audio_xmsampler_base(self)),
				index);
			if (newVoice && psy_audio_xmsamplerchannel_instrumentno(thisChannel) != 255 && _inst) {
				// && Global::song().xminstruments.IsEnabled(thisChannel.InstrumentNo())) {
				psy_List* _layer;
				psy_audio_Sample* wave;
				
				_layer = psy_audio_instrument_entriesintersect(_inst,
					psy_audio_xmsamplerchannel_note(thisChannel), 127, 0);					
				// int _layer = _inst.NoteToSample(thisChannel.Note()).second;
				// if (Global::song().samples.IsEnabled(_layer))
				wave = NULL;				
				if (_layer)
				{
					psy_audio_InstrumentEntry* entry;
					psy_audio_Samples* samples;

					samples = psy_audio_machine_samples(psy_audio_xmsampler_base(self));
					entry = (psy_audio_InstrumentEntry*)_layer->entry;
					entry->fixedkey = entry->fixedkey;
					wave = psy_audio_samples_at(samples, entry->sampleindex);
				}
				if (wave) {
					int vol = -1;
					int offset = 0;
					int twlength = (int)psy_audio_sample_numframes(wave);
					psy_audio_xmsamplervoice_voiceinit(newVoice, 
						_inst, (int)channelNum, psy_audio_xmsamplerchannel_instrumentno(thisChannel));
					psy_audio_xmsamplerchannel_setforegroundvoice(thisChannel, newVoice);			
					
					for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
						psy_audio_PatternEvent* ite_ev;

						ite_ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
					 	if (ite_ev->inst == channelNum) {
					 		if (ite_ev->cmd == XM_SAMPLER_CMD_SENDTOVOLUME && (ite_ev->parameter & 0xF0) <= XM_SAMPLER_CMD_VOL_VOLUME3) {
					 			vol = ite_ev->parameter << 1;
					 		} else if ((ite_ev->cmd & 0xF0) == XM_SAMPLER_CMD_OFFSET) {
					 			offset = ((ite_ev->cmd & 0x0F) << 16) + (ite_ev->parameter << 8);
								if (offset == 0) {
									offset = psy_audio_xmsamplerchannel_offsetmem(thisChannel);
								}
					 		}
					 	}
					}

					if (pData->cmd == XM_SAMPLER_CMD_VOLUME) {
						vol = pData->parameter;
					} else if (pData->cmd == XM_SAMPLER_CMD_SENDTOVOLUME && (pData->parameter & 0xF0) <= XM_SAMPLER_CMD_VOL_VOLUME3) {
						vol = pData->parameter << 1;
					}					
					psy_audio_xmsamplervoice_noteon(newVoice,
						psy_audio_xmsamplerchannel_note(thisChannel),
						vol, bInstrumentSet);
					//this forces a recalc of the m_Period.
					psy_audio_xmsamplerchannel_setnote(thisChannel,
						psy_audio_xmsamplerchannel_note(thisChannel));
					// Add Here any command that is limited to the scope of the new note.
					// An offset is a good candidate, but a volume is not (volume can apply to an existing note)
					if ((pData->cmd & 0xF0) == XM_SAMPLER_CMD_OFFSET)
					{
						offset = ((pData->cmd & 0x0F) << 16) + (pData->parameter << 8);
						if (offset == 0) {
							offset = psy_audio_xmsamplerchannel_offsetmem(thisChannel);
						}
					}
					if (offset != 0) {
						psy_audio_xmsamplerchannel_setoffsetmem(thisChannel, offset);						
						if (offset < twlength) {
							psy_audio_wavedatacontroller_setposition(&newVoice->m_WaveDataController, offset);
						} else {
							psy_audio_wavedatacontroller_setposition(&newVoice->m_WaveDataController, twlength - 1);
						}
					} else {
						psy_audio_wavedatacontroller_setposition(&newVoice->m_WaveDataController, 0);						
					}
				} else
				{
					bNoteOn = FALSE;
					newVoice = NULL;
					///\TODO: multicommand
					if (pData->cmd == 0) return;
				}
			} else
			{
				// This is a noteon command, but we are out of voices. We will try to process the effect.
				bNoteOn = FALSE;
				newVoice = NULL;
				///\TODO: multicommand
				if (pData->cmd == 0) return;
			}
		}
	}
	if (newVoice == NULL) newVoice = currentVoice;
	// Effect Command
	for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ite_ev;

		ite_ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ite_ev->inst == channelNum) {
			if (ite_ev->cmd == XM_SAMPLER_CMD_SENDTOVOLUME) {
				psy_audio_xmsamplerchannel_seteffect(thisChannel,
					newVoice, ite_ev->parameter, 0, 0);
			} else {
				psy_audio_xmsamplerchannel_seteffect(thisChannel,
					newVoice, 255, ite_ev->cmd, ite_ev->parameter);
			}
		}
	}
	if (pData->cmd == XM_SAMPLER_CMD_SENDTOVOLUME) {
		psy_audio_xmsamplerchannel_seteffect(thisChannel,
			newVoice, pData->parameter, 0, 0);
	} else {
		psy_audio_xmsamplerchannel_seteffect(thisChannel,
			newVoice, 255, pData->cmd, pData->parameter);
	}
}

psy_audio_XMSamplerVoice* psy_audio_xmsampler_getcurrentvoice(
	psy_audio_XMSampler* self, int channelNum)
{
	int current;

	for (current = 0; current < self->_numVoices; current++)
	{		
		if ((psy_audio_xmsamplervoice_channelnum(&self->m_Voices[current]) == channelNum)  // Is this one an active note in this channel?
			&& psy_audio_xmsamplervoice_isplaying(&self->m_Voices[current]) &&
			!psy_audio_xmsamplervoice_isbackground(&self->m_Voices[current]))
		{
			//There can be only one foreground active voice for each channel, so we go out of the loop.
			return &self->m_Voices[current];
		}
	}
	return NULL;
}

psy_audio_XMSamplerVoice* psy_audio_xmsampler_getfreevoice(
	psy_audio_XMSampler* self, int channelNum)
{
	int voice;
	int background;

	//First, see if there's a free voice
	for (voice = 0; voice < self->_numVoices; voice++)
	{
		if (!psy_audio_xmsamplervoice_isplaying(&self->m_Voices[voice])) {
			return  (&self->m_Voices[voice]);
		}
	}
	//If there isn't, See if there are background voices in this channel
	background = -1;
	for (voice = 0; voice < self->_numVoices; voice++)
	{
		if (psy_audio_xmsamplervoice_isbackground(&self->m_Voices[voice])) {
			background = voice;
			if (psy_audio_xmsamplervoice_channelnum(&self->m_Voices[voice]) == channelNum) {
				return  &(self->m_Voices[voice]);
			}
		}
	}
	//If still there isn't, See if there are background voices on other channels.
	//This could be improved in some sort of "older-first".
	if (background != -1) {
		return  &(self->m_Voices[background]);
	}
	return NULL;
}

int psy_audio_xmsampler_getplayingvoices(const psy_audio_XMSampler* self)
{
	int c = 0;
	int i = 0;

	for (i = 0; i < XM_SAMPLER_MAX_POLYPHONY; i++)
	{
		if (psy_audio_xmsamplervoice_isplaying(&self->m_Voices[i])) c++;		
	}
	return c;
}

void psy_audio_xmsampler_initparameters(psy_audio_XMSampler* self)
{
	uintptr_t i;

	psy_audio_custommachineparam_init(&self->param_general,
		"General", "General", MPF_HEADER | MPF_SMALL, 0, 0);
	psy_audio_intmachineparam_init(&self->param_numvoices,
		"limit voices", "limit voices", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->_numVoices, 1, 64);
	psy_audio_choicemachineparam_init(&self->param_resamplingmethod,
		"resample", "resample", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->resamplerquality, 0, 3);
	psy_signal_connect(&self->param_resamplingmethod.machineparam.signal_tweak, self,
		resamplingmethod_tweak);
	for (i = 0; i < psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS; ++i) {
		psy_audio_choicemachineparam_setdescription(&self->param_resamplingmethod, i,
			psy_dsp_multiresampler_name((psy_dsp_ResamplerQuality)i));
	}		
	psy_audio_choicemachineparam_init(&self->param_amigaslides,
		"slide", "slide", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->m_bAmigaSlides,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_amigaslides, 0,
		"linear");
	psy_audio_choicemachineparam_setdescription(&self->param_amigaslides, 1,
		"Amiga");
	psy_audio_choicemachineparam_init(&self->param_usefilters,
		"use filters", "use filters", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->m_UseFilters,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_usefilters, 0,
		"disabled");
	psy_audio_choicemachineparam_setdescription(&self->param_usefilters, 1,
		"enabled");
	psy_audio_choicemachineparam_init(&self->param_panningmode,
		"panning", "panning", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->m_PanningMode,
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
	psy_audio_intmachineparam_init(&self->param_instrumentbank,
		"instr. bank", "instr. bank", MPF_STATE | MPF_SMALL,
		&self->instrumentbank,
		0, 0xFFFFF);	
	psy_audio_custommachineparam_init(&self->param_blank, "", "",
		MPF_NULL | MPF_SMALL, 0, 0);
	psy_audio_infomachineparam_init(&self->param_filter_cutoff,
		"Filter Cutoff", "", MPF_SMALL);
	psy_audio_infomachineparam_init(&self->param_filter_res,
		"Filter Q (Res)", "", MPF_SMALL);
	psy_audio_infomachineparam_init(&self->param_pan, "Panning", "",
		MPF_SMALL);	
	self->channeldisplay = 0;
	psy_audio_intmachineparam_init(&self->param_display_channel,
		"Channel", "Channel", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	psy_signal_connect(&self->param_display_channel.machineparam.signal_tweak, self,
		display_tweak);
	psy_signal_connect(&self->param_display_channel.machineparam.signal_normvalue, self,
		display_normvalue);
	psy_audio_intmachineparam_init(&self->param_display_playback,
		"Playback", "Playback", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	psy_signal_connect(&self->param_display_playback.machineparam.signal_tweak, self,
		display_tweak);
	psy_signal_connect(&self->param_display_playback.machineparam.signal_normvalue, self,
		display_normvalue);
	psy_audio_intmachineparam_init(&self->param_display_playmix,
		"Play mix", "Play mix", MPF_CHECK | MPF_SMALL, NULL, 0, 1);
	psy_signal_connect(&self->param_display_playmix.machineparam.signal_tweak, self,
		display_tweak);
	psy_signal_connect(&self->param_display_playmix.machineparam.signal_normvalue, self,
		display_normvalue);
	psy_audio_custommachineparam_init(&self->ignore_param, "-", "-",
		MPF_IGNORE | MPF_SMALL, 0, 0);
	// tweak zxx macros	
	self->tweak_zxxindex = 0;
	self->tweak_zxxmode = 0;	
	self->tweak_zxxvalue = 0;
	psy_audio_intmachineparam_init(&self->tweakparam_zxxindex,
		"zxxindex", "zxxindex", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->tweak_zxxindex, 1, 64);	
	psy_audio_intmachineparam_init(&self->tweakparam_zxxmode,
		"zxxmode", "zxxmode", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->tweak_zxxmode, 1, 64);	
	psy_audio_intmachineparam_init(&self->tweakparam_zxxvalue,
		"zxxvalue", "zxxvalue", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->tweak_zxxvalue, 1, 64);
	psy_signal_connect(&self->tweakparam_zxxvalue.machineparam.signal_tweak,
		self, ontweakzxxvalue);	
	// Master
	psy_audio_infomachineparam_init(&self->param_masterchannel, "Master", "", MPF_SMALL);
	psy_audio_intmachineparam_init(&self->param_masterslider,
		"Volume", "", MPF_SLIDER | MPF_SMALL, &self->m_GlobalVolume, 0, 128);
	psy_audio_intmachineparam_init(&self->param_masterlevel,
		"", "", MPF_LEVEL | MPF_SMALL, NULL, 0, 100);
	psy_signal_connect(&self->param_masterlevel.machineparam.signal_normvalue, self,
		masterlevel_normvalue);
	psy_audio_intmachineparam_init(&self->param_numplayvoices,
		"Voices in use", "", MPF_INFOLABEL | MPF_SMALL, &self->playingVoices,
		0, XM_SAMPLER_MAX_POLYPHONY);
}

void disposeparameters(psy_audio_XMSampler* self)
{
	psy_audio_custommachineparam_dispose(&self->param_general);
	psy_audio_intmachineparam_dispose(&self->param_numvoices);
	psy_audio_choicemachineparam_dispose(&self->param_resamplingmethod);	
	psy_audio_choicemachineparam_dispose(&self->param_amigaslides);
	psy_audio_choicemachineparam_dispose(&self->param_usefilters);
	psy_audio_choicemachineparam_dispose(&self->param_panningmode);
	psy_audio_choicemachineparam_dispose(&self->param_channelview);	
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
	psy_audio_custommachineparam_dispose(&self->param_channels);
	psy_audio_custommachineparam_dispose(&self->param_blank);
	psy_audio_infomachineparam_dispose(&self->param_filter_cutoff);
	psy_audio_infomachineparam_dispose(&self->param_filter_res);
	psy_audio_infomachineparam_dispose(&self->param_pan);
	psy_audio_intmachineparam_dispose(&self->param_display_channel);
	psy_audio_intmachineparam_dispose(&self->param_display_playback);
	psy_audio_intmachineparam_dispose(&self->param_display_playmix);
	psy_audio_custommachineparam_dispose(&self->ignore_param);
	// tweak
	psy_audio_intmachineparam_dispose(&self->tweakparam_zxxindex);
	psy_audio_intmachineparam_dispose(&self->tweakparam_zxxmode);
	psy_audio_intmachineparam_dispose(&self->tweakparam_zxxvalue);
	// master
	psy_audio_infomachineparam_dispose(&self->param_masterchannel);
	psy_audio_intmachineparam_dispose(&self->param_masterslider);
	psy_audio_intmachineparam_dispose(&self->param_masterlevel);
	psy_audio_intmachineparam_dispose(&self->param_numplayvoices);
}

void psy_audio_xmsampler_stop(psy_audio_XMSampler* self)
{
	//\todo: check that all needed variables/objects are reset.
	int i;
	for (i = 0; i < self->_numVoices; i++)
	{
		psy_audio_xmsamplervoice_noteofffast(&self->m_Voices[i]);
	}
	for (i = 0; i < MAX_TRACKS; i++)
	{
		psy_audio_xmsamplerchannel_restore(&self->m_Channel[i]);		
	}
}

const psy_audio_MachineInfo* xmsampler_info(psy_audio_XMSampler* self)
{	
	return &macinfo;
}

uintptr_t numparameters(psy_audio_XMSampler* self)
{
	return numparametercols(self) * 11;
}

uintptr_t numparametercols(psy_audio_XMSampler* self)
{
	return 10;
}

psy_audio_MachineParam* tweakparameter(psy_audio_XMSampler* self,
	uintptr_t param)
{
	uintptr_t channelidx;
	uintptr_t channelparamidx;
	psy_audio_XMSamplerChannel* channel;

	// global parameter start: 0
	// channel parameter start:
	// XM_SAMPLER_TWK_CHANNEL_START + (channelidx) * XM_SAMPLER_TWK_CHANNEL_START

	if (param < XM_SAMPLER_TWK_CHANNEL_START) {
		// global tweaks
		switch (param) {
			case XM_SAMPLER_TWK_AMIGASLIDES:
				return &self->param_amigaslides.machineparam;
				break;
			case XM_SAMPLER_TWK_GLOBALVOLUME:
				return &self->param_blank.machineparam;
				break;
			case XM_SAMPLER_TWK_PANNINGMODE:
				return &self->param_panningmode.machineparam;
				break;
			case XM_SAMPLER_TWK_SETZXXMACRO_INDEX:
				return &self->tweakparam_zxxindex.machineparam;
				break;
			case XM_SAMPLER_TWK_SETZXXMACRO_MODE:
				return &self->tweakparam_zxxmode.machineparam;
				break;			
			case XM_SAMPLER_TWK_SETZXXMACRO_VALUE:
				return &self->tweakparam_zxxvalue.machineparam;
				break;
			default:				
				break;
		}
	} else {
		// channel tweaks
		channelparamidx = param % XM_SAMPLER_TWK_CHANNEL_START;
		channelidx = param / XM_SAMPLER_TWK_CHANNEL_START - 1;
		if (channelidx < MAX_TRACKS) {
			channel = psy_audio_xmsampler_rchannel(self, (int)channelidx);
			if (channel) {
				switch (channelparamidx) {
				case 0:
					return &channel->slider_param.machineparam;
					break;
				case XM_SAMPLER_TWK_CHANNEL_PANNING:
					return &channel->pan.machineparam;
					break;
				default:
					break;
				}
			}
		}
	}
	return NULL;
}

void ontweakzxxvalue(psy_audio_XMSampler* self,
	psy_audio_IntMachineParam* sender, float value)
{
	psy_audio_xmsampler_setzxxmacro(self,
		self->tweak_zxxindex,		
		self->tweak_zxxmode,
		self->tweak_zxxvalue);
}

uintptr_t numtweakparameters(psy_audio_XMSampler* self)
{
	// global parameter start: 0
	// channel parameter start:
	// XM_SAMPLER_TWK_CHANNEL_START + (channelidx) * XM_SAMPLER_TWK_CHANNEL_START
	return (XM_SAMPLER_TWK_CHANNEL_START + 1) * MAX_TRACKS;
}

psy_audio_MachineParam* parameter(psy_audio_XMSampler* self,
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
		case XM_CHANNELROW + 2: return &self->param_filter_cutoff.machineparam; break;
		case XM_CHANNELROW + 3: return &self->param_filter_res.machineparam; break;
		case XM_CHANNELROW + 4: return &self->param_pan.machineparam;  break;		
		case XM_CHANNELROW + 5: return &self->param_display_channel.machineparam; break;
		case XM_CHANNELROW + 6: return &self->param_display_playback.machineparam; break;
		case XM_CHANNELROW + 7: return &self->param_display_playmix.machineparam; break;
		case XM_CHANNELROW + 8: return &self->ignore_param.machineparam;  break;
		default:
			return &self->param_blank.machineparam; break;
			break;
		}
	} else if (col == 9) {
		// Master Column
		switch (row) {		
		case XM_CHANNELROW + 0: return &self->param_masterchannel.machineparam; break;
		case XM_CHANNELROW + 3: return &self->param_numplayvoices.machineparam; break;
		case XM_CHANNELROW + 5: return &self->param_masterslider.machineparam; break;
		case XM_CHANNELROW + 6: return &self->param_masterlevel.machineparam;  break;
		case XM_CHANNELROW + 7: return &self->ignore_param.machineparam;  break;
		case XM_CHANNELROW + 8: return &self->ignore_param.machineparam;  break;
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
		case 6: return &self->param_instrumentbank.machineparam; break;		
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
		psy_audio_XMSamplerChannel* channel;
		
		channel = psy_audio_xmsampler_rchannel(self, (int)(col - 1 + self->channelbank * 8));
		if (channel) {
			switch (row) {
				case XM_CHANNELROW + 0:
				return &channel->param_channel.machineparam;
					break;
				case XM_CHANNELROW + 1:
					return &channel->param_filtertype.machineparam;
					break;
				case XM_CHANNELROW + 2:
					return &channel->filter_cutoff.machineparam;
					break;
				case XM_CHANNELROW + 3:
					return &channel->filter_res.machineparam;
					break;
				case XM_CHANNELROW + 4:
					return &channel->pan.machineparam;
					break;				
				case XM_CHANNELROW + 5:
					return &channel->slider_param.machineparam;
					break;
				case XM_CHANNELROW + 6:
					return &self->ignore_param.machineparam;					
					break;				
				case XM_CHANNELROW + 7:
					return &channel->surround.machineparam;
					break;
				case XM_CHANNELROW + 8:
					return &channel->mute.machineparam;
					break;
				//case XM_CHANNELROW + 8:
					//return &self->ignore_param.machineparam;
					//break;
				default:
					return &self->param_blank.machineparam; break;
				break;
			}
		}
	}
	return NULL;
}

void resamplingmethod_tweak(psy_audio_XMSampler* self,
	psy_audio_ChoiceMachineParam* sender, float value)
{	
	psy_audio_xmsampler_setresamplerquality(self,
		(int)(value * (sender->maxval - sender->minval) + 0.5f) +
			sender->minval);	
}

void display_tweak(psy_audio_XMSampler* self,
	psy_audio_IntMachineParam* sender, float value)
{
	if (sender == &self->param_display_channel) {
		self->channeldisplay = 0;
	} else if (sender == &self->param_display_playback) {
		self->channeldisplay = 1;
	} else if (sender == &self->param_display_playmix) {
		self->channeldisplay = 2;
	}
	++self->strobe;
}

void display_normvalue(psy_audio_XMSampler* self,
	psy_audio_IntMachineParam* sender, float* rv)
{
	if ((sender == &self->param_display_channel && self->channeldisplay == 0) ||
			(sender == &self->param_display_playback && self->channeldisplay == 1) ||
			(sender == &self->param_display_playmix && self->channeldisplay == 2)) {
		*rv = 1.f;
		return;
	}
	*rv = 0.f;	
}

void masterlevel_normvalue(psy_audio_XMSampler* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	psy_audio_Buffer* memory;

	memory = psy_audio_machine_buffermemory(psy_audio_xmsampler_base(self));
	if (memory) {
		*rv = psy_audio_buffer_rmsdisplay(memory);
	} else {
		*rv = 0.f;
	}
}

uintptr_t paramstrobe(const psy_audio_XMSampler* self)
{
	return self->strobe;
}

void psy_audio_xmsampler_setresamplerquality(psy_audio_XMSampler* self,
	psy_dsp_ResamplerQuality quality)
{	
	int i;

	self->resamplerquality = quality;
	for (i = 0; i < self->_numVoices; i++)
	{
		psy_dsp_multiresampler_setquality(
			&self->m_Voices->m_WaveDataController.resampler,
			quality);					
	}	
}

// mfc-psycle: bool XMSampler::SaveSpecificChunk(RiffFile* riffFile, int version)
int savespecificchunk(psy_audio_XMSampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	int32_t temp;
	int i;
	uint32_t endpos;
	int resamplerquality;
	int status;

	// we cannot calculate the size previous to save, so we write a placeholder
	// and seek back to write the correct value.
	uint32_t size = 0;
	uint32_t filepos = psyfile_getpos(songfile->file);
	if ((status = psyfile_write(songfile->file, &size, sizeof(size)))) {
		return status;
	}
	if ((status = psyfile_write_uint32(songfile->file, XMSAMPLER_VERSION))) {
		return status;
	}
	if ((status = psyfile_write_uint32(songfile->file, self->_numVoices))) {
		return status;
	}
	resamplerquality = psy_dsp_RESAMPLERQUALITY_LINEAR;
	switch (resamplerquality) // (self->resamplerquality)
	{
	case psy_dsp_RESAMPLERQUALITY_ZERO_ORDER: temp = 0; break;
	case psy_dsp_RESAMPLERQUALITY_SPLINE: temp = 2; break;
	case psy_dsp_RESAMPLERQUALITY_SINC: temp = 3; break;
	case psy_dsp_RESAMPLERQUALITY_LINEAR: //fallthrough
	default: temp = 1;
	}
	psyfile_write_int32(songfile->file, temp); // quality
	//TODO: zxxMap cannot be edited right now.
	for (i = 0; i < 128; i++) {
		if ((status = psyfile_write_int32(songfile->file, 0))) { // zxxMap[i].mode);
			return status;
		}
		if ((status = psyfile_write_int32(songfile->file, 0))) { // zxxMap[i].value);
			return status;
		}
	}
	if ((status = psyfile_write_uint8(songfile->file,
			(uint8_t)(self->m_bAmigaSlides != 0)))) {
		return status;
	}
	if ((status = psyfile_write_uint8(songfile->file, (uint8_t)self->m_UseFilters))) {
		return status;
	}
	if ((status = psyfile_write_int32(songfile->file, self->m_GlobalVolume))) {
		return status;
	}
	if ((status = psyfile_write_int32(songfile->file, self->m_PanningMode))) {
		return status;
	}
	for (i = 0; i < MAX_TRACKS; i++) {
		psy_audio_xmsamplerchannel_save(&self->m_Channel[i], songfile);
	}
	if ((status = psyfile_write_uint32(songfile->file, (uint32_t)self->instrumentbank))) {
		return status;
	}
	endpos = psyfile_getpos(songfile->file);
	if (psyfile_seek(songfile->file, filepos) == -1) {
		return PSY_ERRFILE;
	}
	size = (uint32_t)(endpos - filepos - sizeof(size));
	if ((status = psyfile_write_uint32(songfile->file, size))) {
		return status;
	}
	if (psyfile_seek(songfile->file, endpos) == -1) {
		return status;
	}
	return PSY_OK;
}

// mfc-psycle: bool XMSampler::LoadSpecificChunk(RiffFile* riffFile, int version)
int loadspecificchunk(psy_audio_XMSampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	int32_t temp;
	bool wrongState = FALSE;
	uint32_t filevers;
	uint32_t filepos;
	uint32_t size=0;
	int status;

	// psy_audio_xmsampler_defaultC4(self, TRUE);
	// self->instrumentbank = 1;
	if ((status = psyfile_read(songfile->file, &size, sizeof(size)))) {
		return status;
	}
	filepos = psyfile_getpos(songfile->file);
	if ((status = psyfile_read(songfile->file, &filevers, sizeof(filevers)))) {
		return status;
	}
			
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
		psy_dsp_ResamplerQuality resamplertype;

		// numSubtracks
		if ((status = psyfile_read(songfile->file, &temp, sizeof(temp)))) {
			return status;
		}
		self->_numVoices = temp;
		// quality
		if ((status = psyfile_read(songfile->file, &temp, sizeof(temp)))) {
			return status;
		}
		switch (temp) {
			case 2: resamplertype = psy_dsp_RESAMPLERQUALITY_SPLINE; break;
			case 3: resamplertype = psy_dsp_RESAMPLERQUALITY_SINC; break;
			case 0:	resamplertype = psy_dsp_RESAMPLERQUALITY_ZERO_ORDER; break;
			case 1:
			default:
				resamplertype = psy_dsp_RESAMPLERQUALITY_LINEAR;
				break;
		}
		psy_audio_xmsampler_setresamplerquality(self, resamplertype);
		for (i = 0; i < 128; ++i) {
			if ((status = psyfile_read(songfile->file, &zxxMap[i].mode, sizeof(zxxMap[i].mode)))) {
				return status;
			}
			if ((status = psyfile_read(songfile->file, &zxxMap[i].value, sizeof(zxxMap[i].value)))) {
				return status;
			}
		}
		if ((status = psyfile_read(songfile->file, &m_bAmigaSlides, sizeof(m_bAmigaSlides)))) {
			return status;
		}
		self->m_bAmigaSlides = m_bAmigaSlides;
		if ((status = psyfile_read(songfile->file, &m_UseFilters, sizeof(m_UseFilters)))) {
			return status;
		}
		self->m_UseFilters = m_UseFilters;
		if ((status = psyfile_read(songfile->file, &m_GlobalVolume, sizeof(m_GlobalVolume)))) {
			return status;
		}
		if ((status = psyfile_read(songfile->file, &m_PanningMode, sizeof(m_PanningMode)))) {
			return status;
		}
		// self->masterchannel.volume = m_GlobalVolume / 127.f;
		for (i = 0; i < MAX_TRACKS; ++i) {				
			psy_audio_xmsamplerchannel_load(&self->m_Channel[i], songfile);
		}

		if ((filevers & XMSAMPLER_VERSION & 0x0000FFFF) >= 0x02) {
			uint32_t temp32;

			if ((status = psyfile_read(songfile->file, &temp32, sizeof(temp32)))) {
				return status;
			}
			self->instrumentbank = temp32;
		}
	} else {
		wrongState = TRUE;
	}
	if (!wrongState) {
		return PSY_OK;
	} else {
		if (psyfile_seek(songfile->file, filepos + size) == -1) {
			return PSY_ERRFILE;
		}
		return PSY_OK; // FALSE;
	}
	return PSY_OK;
}
