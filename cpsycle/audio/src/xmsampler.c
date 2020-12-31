// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xmsampler.h"
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

static void dispose(psy_audio_XMSampler*);
static const psy_audio_MachineInfo* info(psy_audio_XMSampler*);
static uintptr_t psy_audio_xmsampler_numinputs(psy_audio_XMSampler* self) { return 0; }
static uintptr_t psy_audio_xmsampler_numoutputs(psy_audio_XMSampler* self) { return 2; }
static void psy_audio_xmsampler_newline(psy_audio_XMSampler*);
static void psy_audio_xmsampler_postnewline(psy_audio_XMSampler*);
static void psy_audio_xmsampler_tick(psy_audio_XMSampler*,
	uintptr_t channelNum, const psy_audio_PatternEvent*);
static void loadspecific(psy_audio_XMSampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_XMSampler*, psy_audio_SongFile*,
	uintptr_t slot);

static void psy_audio_xmsampler_ontimertick(psy_audio_XMSampler*);
static void psy_audio_xmsampler_ontimerwork(psy_audio_XMSampler*,
	psy_audio_BufferContext*);
static void generateaudio(psy_audio_XMSampler*, psy_audio_BufferContext*);

/*
static void seqtick(psy_audio_XMSampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void newline(psy_audio_XMSampler*);
static psy_List* sequencerinsert(psy_audio_XMSampler*, psy_List* events);
static void psy_audio_xmsampler_stop(psy_audio_XMSampler*);

static uintptr_t numparametercols(psy_audio_XMSampler*);
static uintptr_t numparameters(psy_audio_XMSampler*);
static psy_audio_MachineParam* parameter(psy_audio_XMSampler*,
	uintptr_t param);*/

/*static void disposevoices(psy_audio_XMSampler*);
static void disposechannels(psy_audio_XMSampler*);
static void disposeparameters(psy_audio_XMSampler*);
static int alloc_voice(psy_audio_XMSampler*);
static void releaseallvoices(psy_audio_XMSampler*);
static psy_audio_XMSamplerVoice* activevoice(psy_audio_XMSampler*,
	uintptr_t channel);
static void releasevoices(psy_audio_XMSampler*, uintptr_t channel);
static void releaseinst(psy_audio_XMSampler*, uintptr_t channel, uintptr_t inst);
static void nnavoices(psy_audio_XMSampler*, uintptr_t channel);
static void removeunusedvoices(psy_audio_XMSampler* self);

static psy_audio_XMSamplerChannel* sampler_channel(psy_audio_XMSampler*,
	uintptr_t channelnum);
static psy_audio_InstrumentIndex currslot(psy_audio_XMSampler*,
	uintptr_t channel, const psy_audio_PatternEvent*);*/

static psy_audio_MachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64 | MACH_SUPPORTS_INSTRUMENTS,
	MACHMODE_GENERATOR,
	"Sampulse"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampulse",
	"Psycledelics",
	"help",	
	MACH_XMSAMPLER,
	NULL,		// NO MODULPATH
	0,			// shellidx	
	SAMPLERHELP	// help text
};

const psy_audio_MachineInfo* psy_audio_xmsampler_info(void)
{
	return &macinfo;
}

/*static void psy_audio_xmsampler_initparameters(psy_audio_XMSampler*);
static void resamplingmethod_tweak(psy_audio_XMSampler*,
	psy_audio_ChoiceMachineParam* sender, float value);
*/

static MachineVtable sampler_vtable;
static bool sampler_vtable_initialized = FALSE;

static void sampler_vtable_init(psy_audio_XMSampler* self)
{
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_xmsampler_base(self)->vtable);
		sampler_vtable.dispose = (fp_machine_dispose)dispose;
		sampler_vtable.numinputs = (fp_machine_numinputs)psy_audio_xmsampler_numinputs;
		sampler_vtable.numoutputs = (fp_machine_numoutputs)psy_audio_xmsampler_numoutputs;
		sampler_vtable.info = (fp_machine_info)info;
		sampler_vtable.newline = (fp_machine_newline)psy_audio_xmsampler_newline;
		sampler_vtable.seqtick = (fp_machine_seqtick)psy_audio_xmsampler_tick;
		sampler_vtable.loadspecific = (fp_machine_loadspecific)loadspecific;
		sampler_vtable.savespecific = (fp_machine_savespecific)savespecific;
		sampler_vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
	/*	sampler_vtable.seqtick = (fp_machine_seqtick)seqtick;
		
		sampler_vtable.sequencerinsert = (fp_machine_sequencerinsert)
			sequencerinsert;
		sampler_vtable.stop = (fp_machine_stop)psy_audio_xmsampler_stop;
		
		
		/*
		sampler_vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		sampler_vtable.numparameters = (fp_machine_numparameters)numparameters;
		sampler_vtable.parameter = (fp_machine_parameter)parameter;*/
		sampler_vtable_initialized = TRUE;
	}
}

void psy_audio_xmsampler_init(psy_audio_XMSampler* self,
	psy_audio_MachineCallback* callback)
{
	int i;

	psy_audio_custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_xmsampler_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_xmsampler_base(self), "XMSampler");

	self->_numVoices = XM_SAMPLER_MAX_POLYPHONY;
	self->multicmdMem = NULL;	
	self->m_sampleRate = (int)
		psy_audio_machine_samplerate(psy_audio_xmsampler_base(self));	
	for (i = 0; i < self->_numVoices; i++)
	{
		psy_audio_xmsamplervoice_init(&self->m_Voices[i]);
		psy_audio_xmsamplervoice_setpsampler(&self->m_Voices[i], self);		
		psy_audio_xmsamplervoice_reset(&self->m_Voices[i]);
	}

	for (i = 0; i < MAX_TRACKS; i++)
	{
		// mfc-psycle (implicit constructor call)
		psy_audio_xmsamplerchannel_init(&self->m_Channel[i]);
		psy_audio_xmsamplerchannel_setpsampler(&self->m_Voices[i], self);
		psy_audio_xmsamplerchannel_setindex(&self->m_Voices[i], i);		
	}

	psy_audio_ticktimer_init(&self->ticktimer,
		self, // callback context (sampler)
		(fp_samplerticktimer_ontick)psy_audio_xmsampler_ontimertick,
		(fp_samplerticktimer_onwork)psy_audio_xmsampler_ontimerwork);

/*	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;
	self->voices = 0;
	self->resamplerquality = psy_dsp_RESAMPLERQUALITY_LINEAR;
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
	psy_audio_xmsampler_initparameters(self);	
	if (self->xmsamplerload == FALSE) {
		self->instrumentbank = 0;
	} else {
		self->instrumentbank = 1;
	}
	psy_audio_ticktimer_init(&self->ticktimer,
		self, // callback context (sampler)
		(fp_samplerticktimer_ontick)psy_audio_xmsampler_ontimertick,
		(fp_samplerticktimer_onwork)psy_audio_xmsampler_ontimerwork);*/
}

void psy_audio_xmsampler_setsamplerate(psy_audio_XMSampler* self,
	int sr)
{	
	self->m_sampleRate = sr;
	for (int i = 0; i < self->_numVoices; i++)
	{
		if (psy_audio_xmsamplervoice_isplaying(&self->m_Voices[i])) {
			//\todo
			//update envelopes
			//
		}
	}
}

void psy_audio_xmsampler_ontimertick(psy_audio_XMSampler* self)
{

}

void generateaudio(psy_audio_XMSampler* self, psy_audio_BufferContext* bc)
{
	self->samplerowcounter += bc->numsamples;
	psy_audio_ticktimer_update(&self->ticktimer, bc->numsamples, bc);
	// removeunusedvoices(self);
	// psy_audio_xmsamplerchannel_work(&self->masterchannel, bc);
}

void psy_audio_xmsampler_ontimerwork(psy_audio_XMSampler* self,
	psy_audio_BufferContext* bc)
{
	psy_audio_xmsampler_workvoices(self, bc);
}

void psy_audio_xmsampler_workvoices(psy_audio_XMSampler* self,
	psy_audio_BufferContext* bc)
{
	int voice;

	// psy_audio_buffer_clearsamples(bc->output, bc->numsamples);
	for (voice = 0; voice < self->_numVoices; voice++)
	{
		if (psy_audio_xmsamplervoice_isplaying(&self->m_Voices[voice])) {
			psy_audio_xmsamplervoice_work(&self->m_Voices[voice],
				bc->numsamples,
				bc->output->samples[0],
				bc->output->samples[1]);			
		}
	}
}

void psy_audio_xmsampler_newline(psy_audio_XMSampler* self)
{
	int channel;
	int voice;

	// SampleCounter(0);
	self->m_TickCount = 0;

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

void psy_audio_xmsampler_postnewline(psy_audio_XMSampler* self)
{
	psy_list_deallocate(&self->multicmdMem, NULL);	
}

void psy_audio_xmsampler_tick(psy_audio_XMSampler* self,
	uintptr_t channelNum, const psy_audio_PatternEvent* pData)
{
	bool bInstrumentSet;
	bool bPortaEffect;
	psy_List* ite;
	bool bPorta2Note;
	bool bNoteOn;
	psy_audio_XMSamplerVoice* currentVoice;
	psy_audio_XMSamplerVoice* newVoice;
	psy_audio_XMSamplerChannel* thisChannel;	

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
	bInstrumentSet =  (pData->inst < psy_audio_NOTECOMMANDS_INST_EMPTY);
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
	thisChannel = psy_audio_xmsampler_rchannel(self, channelNum);
	// STEP A: Look for an existing (foreground) playing voice in the current channel.
	currentVoice = psy_audio_xmsampler_getcurrentvoice(self, channelNum);
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
				//currentVoice->AmplitudeEnvelope().NoteOn();
				//currentVoice->PanEnvelope().NoteOn();
				//currentVoice->FilterEnvelope().NoteOn();
				//currentVoice->PitchEnvelope().NoteOn();
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
				/*switch (currentVoice->rInstrument().DCT())
				{
				case XMInstrument::DupeCheck::INSTRUMENT:
					//todo: if no instrument specified... should we consider the previous instrument?
					if (pData->_inst == thisChannel.InstrumentNo())
					{
						if (currentVoice->rInstrument().DCA() < currentVoice->NNA()) currentVoice->NNA(currentVoice->rInstrument().DCA());
					}
					break;
				case XMInstrument::DupeCheck::SAMPLE:
				{
					//todo: if no instrument specified... should we consider the previous instrument?
					const XMInstrument& _inst = Global::song().xminstruments[thisChannel.InstrumentNo()];
					int _layer = _inst.NoteToSample(thisChannel.Note()).second;
					if (_layer == thisChannel.ForegroundVoice()->rWave().Layer())
					{
						if (currentVoice->rInstrument().DCA() < currentVoice->NNA()) currentVoice->NNA(currentVoice->rInstrument().DCA());
					}
				}
				break;
				case XMInstrument::DupeCheck::NOTE:
					if (pData->_note == thisChannel.Note() && pData->_inst == thisChannel.InstrumentNo())
					{
						if (currentVoice->rInstrument().DCA() < currentVoice->NNA()) currentVoice->NNA(currentVoice->rInstrument().DCA());
					}
					break;
				default:
					break;
				}
				*/
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
				// currentVoice->IsBackground(true);
			} else if (pData->note == psy_audio_NOTECOMMANDS_RELEASE) {
				//currentVoice->NoteOff();
				//thisChannel.StopBackgroundNotes(XMInstrument::NewNoteAction::NOTEOFF);
				//thisChannel.Note(pData->_note);
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
			/*currentVoice->ResetVolAndPan(-1, true);

			//Restart also the envelopes
			//This is an FT2 feature (and compatibles, like Modplug in ft2 mode, cubic player,...),
			//but it is not done in Impulse tracker (and compatibles, like schismtracker).
			currentVoice->AmplitudeEnvelope().NoteOn();
			currentVoice->PanEnvelope().NoteOn();
			currentVoice->PitchEnvelope().NoteOn();
			currentVoice->FilterEnvelope().NoteOn();*/
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
			index = psy_audio_instrumentindex_make(0,
				psy_audio_xmsamplerchannel_instrumentno(thisChannel));
			_inst = psy_audio_instruments_at(psy_audio_machine_instruments(
				psy_audio_xmsampler_base(self)),
				index);
			if (newVoice && psy_audio_xmsamplerchannel_instrumentno(thisChannel) != psy_audio_NOTECOMMANDS_INST_EMPTY
					&& _inst) {
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
					wave = psy_audio_samples_at(samples, entry->sampleindex);
				}
				if (wave) {
					int vol = -1;
					int offset = 0;
					int twlength = psy_audio_sample_numframes(wave);
					psy_audio_xmsamplervoice_voiceinit(newVoice, 
						_inst, channelNum, psy_audio_xmsamplerchannel_instrumentno(thisChannel));
					psy_audio_xmsamplerchannel_setforegroundvoice(thisChannel, newVoice);			
					
					// for (std::vector<PatternEntry>::const_iterator ite = multicmdMem.begin(); ite != multicmdMem.end(); ++ite) {
					// 	if (ite->_inst == channelNum) {
					// 		if (ite->_cmd == CMD::SENDTOVOLUME && (ite->_parameter & 0xF0) <= CMD_VOL::VOL_VOLUME3) {
					// 			vol = ite->_parameter << 1;
					// 		} else if ((ite->_cmd & 0xF0) == CMD::OFFSET) {
					// 			offset = ((ite->_cmd & 0x0F) << 16) + ite->_parameter << 8;
					// 			if (offset == 0) offset = thisChannel.OffsetMem();
					// 		}
					// 	}
					// }

					// if (pData->_cmd == CMD::VOLUME) vol = pData->_parameter;
					// else if (pData->_cmd == CMD::SENDTOVOLUME && (pData->_parameter & 0xF0) <= CMD_VOL::VOL_VOLUME3) vol = pData->_parameter << 1;
					psy_audio_xmsamplervoice_noteon(newVoice,
						psy_audio_xmsamplerchannel_note(thisChannel),
						vol, bInstrumentSet);
					//this forces a recalc of the m_Period.
					psy_audio_xmsamplerchannel_setnote(thisChannel,
						psy_audio_xmsamplerchannel_note(thisChannel));
					// Add Here any command that is limited to the scope of the new note.
					// An offset is a good candidate, but a volume is not (volume can apply to an existing note)
					/* if ((pData->_cmd & 0xF0) == CMD::OFFSET)
					{
						offset = ((pData->_cmd & 0x0F) << 16) + pData->_parameter << 8;
						if (offset == 0) offset = thisChannel.OffsetMem();
					}
					if (offset != 0) {
						thisChannel.OffsetMem(offset);
						if (offset < twlength) { newVoice->rWave().Position(offset); } else { newVoice->rWave().Position(twlength - 1); }
					} else { newVoice->rWave().Position(0); }*/
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

	//First, see if there's a free voice
	for (voice = 0; voice < self->_numVoices; voice++)
	{
		if (!psy_audio_xmsamplervoice_isplaying(&self->m_Voices[voice])) {
			return  (&self->m_Voices[voice]);
		}
	}
	//If there isn't, See if there are background voices in this channel
	int background = -1;
	for (int voice = 0; voice < self->_numVoices; voice++)
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




/*
void psy_audio_xmsampler_initparameters(psy_audio_XMSampler* self)
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
	for (i = 0; i < psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS; ++i) {
		psy_audio_choicemachineparam_setdescription(&self->param_resamplingmethod, i,
			psy_dsp_multiresampler_name((psy_dsp_ResamplerQuality)i));
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
	psy_audio_xmsamplerchannel_init(&self->masterchannel, UINTPTR_MAX);
	psy_audio_custommachineparam_init(&self->ignore_param, "-", "-",
		MPF_IGNORE | MPF_SMALL, 0, 0);
}
*/
void dispose(psy_audio_XMSampler* self)
{		
	//disposeparameters(self);
	//disposevoices(self);
	//disposechannels(self);
	psy_audio_custommachine_dispose(&self->custommachine);		
}
/*
void disposeparameters(psy_audio_XMSampler* self)
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
	psy_audio_choicemachineparam_dispose(&self->param_panpersistent);
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
	psy_audio_custommachineparam_dispose(&self->param_channels);
	psy_audio_custommachineparam_dispose(&self->param_blank);
	psy_audio_infomachineparam_dispose(&self->param_filter_cutoff);
	psy_audio_infomachineparam_dispose(&self->param_filter_res);
	psy_audio_infomachineparam_dispose(&self->param_pan);
	psy_audio_custommachineparam_dispose(&self->ignore_param);
}

void disposevoices(psy_audio_XMSampler* self)
{
	psy_list_deallocate(&self->voices, (psy_fp_disposefunc)
		psy_audio_xmsamplervoice_dispose);
}

void disposechannels(psy_audio_XMSampler* self)
{
	psy_table_disposeall(&self->channels, (psy_fp_disposefunc)
		psy_audio_xmsamplerchannel_dispose);
	psy_audio_xmsamplerchannel_dispose(&self->masterchannel);
}


psy_audio_XMSamplerChannel* sampler_channel(psy_audio_XMSampler* self, uintptr_t track)
{
	psy_audio_XMSamplerChannel* rv;

	rv = psy_table_at(&self->channels, track);
	if (rv == NULL) {
		rv = malloc(sizeof(psy_audio_XMSamplerChannel));
		psy_audio_xmsamplerchannel_init(rv, track);
		psy_table_insert(&self->channels, track, (void*) rv);
	}
	return rv;
}



void psy_audio_xmsampler_ontimertick(psy_audio_XMSampler* self)
{
	psy_List* p;
	uintptr_t c = 0;
	psy_TableIterator it;

	// first notify channels
	for (it = psy_table_begin(&self->channels);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_XMSamplerChannel* channel;

		channel = (psy_audio_XMSamplerChannel*)psy_tableiterator_value(&it);
		// SetEffect is called by seqtick
		if (psy_audio_ticktimer_tickcount(&self->ticktimer) != 0) {
			psy_audio_xmsamplerchannel_performfx(channel);			
		}
	}
	// secondly notify voices
	for (p = self->voices; p != NULL && c < self->numvoices; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*)p->entry;
		if (psy_audio_ticktimer_tickcount(&self->ticktimer) != 0) {
			psy_audio_xmsamplervoice_performfx(voice);
		}
	}
}

void psy_audio_xmsampler_ontimerwork(psy_audio_XMSampler* self,
	psy_audio_BufferContext* bc)
{
	psy_List* p;
	uintptr_t c = 0;
	
	// psy_audio_buffer_clearsamples(bc->output, bc->numsamples);
	for (p = self->voices; p != NULL && c < self->numvoices;
			psy_list_next(&p), ++c) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*)p->entry;
		if (psy_audio_xmsamplervoice_isplaying(voice)) {
			psy_audio_xmsamplervoice_work(voice, bc->output,
				bc->numsamples);
		}
	}
}

void seqtick(psy_audio_XMSampler* self, uintptr_t channelnum,
	const psy_audio_PatternEvent* ev)
{		
	psy_audio_XMSamplerVoice* voice = 0;
	psy_audio_XMSamplerChannel* channel = 0;	
	psy_audio_PatternEvent event;

	assert(ev);
	if (ev == NULL) {
		return;
	}
	event = *ev;
	if (event.cmd == XM_SAMPLER_CMD_EXTENDED) {
		if ((event.parameter & 0xF0) == XM_SAMPLER_CMD_E_NOTE_DELAY) {
			// skip for now and reinsert in sequencerinsert
			// with delayed offset
			return;
		}
	}	
	channel = sampler_channel(self, channelnum);
	if (channel) {
		psy_audio_xmsamplerchannel_seteffect(channel, &event);
	}
	if (event.note == psy_audio_NOTECOMMANDS_RELEASE) {
		//if (event.inst == psy_audio_NOTECOMMANDS_INST_EMPTY) {
			releasevoices(self, channelnum);
		//} else {
			//releaseinst(self, channelnum, event.inst);
		//}
		return;
	}
	if (event.note < psy_audio_NOTECOMMANDS_RELEASE) {
		nnavoices(self, channelnum);
	} else {
		voice = activevoice(self, channelnum);
	}
	if (!voice) {
		psy_audio_Instrument* instrument;
		psy_audio_InstrumentIndex index;

		index = currslot(self, channelnum, &event);
		instrument = psy_audio_instruments_at(psy_audio_machine_instruments(
			psy_audio_xmsampler_base(self)),
			index);
		if (instrument) {
			voice = psy_audio_xmsamplervoice_allocinit(self, instrument,
				index.subslot,
				channel,
				channelnum,				
				psy_audio_machine_samplerate(psy_audio_xmsampler_base(self)));
			psy_list_append(&self->voices, voice);
		}
	}	
	if (voice) {
		psy_audio_xmsamplervoice_seqtick(voice, &event);
	}
}

void newline(psy_audio_XMSampler* self)
{
	psy_List* p;
	psy_TableIterator it;
	
	self->samplerowcounter = 0;
	// first notify channels
	for (it = psy_table_begin(&self->channels);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_XMSamplerChannel* channel;

		channel = (psy_audio_XMSamplerChannel*)psy_tableiterator_value(&it);		
		psy_audio_xmsamplerchannel_newline(channel);		
	}
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*) p->entry;	
		if (psy_audio_xmsamplervoice_isplaying(voice)) {
			psy_audio_xmsamplervoice_newline(voice);
		}
	}
	psy_audio_ticktimer_reset(&self->ticktimer,	
		(uintptr_t)
		psy_audio_machine_samplespertick(psy_audio_xmsampler_base(self)));
}

void psy_audio_xmsampler_stop(psy_audio_XMSampler* self)
{
	psy_TableIterator it;

	releaseallvoices(self);
	// first notify channels
	for (it = psy_table_begin(&self->channels);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_XMSamplerChannel* channel;

		channel = (psy_audio_XMSamplerChannel*)psy_tableiterator_value(&it);
		psy_audio_xmsamplerchannel_restore(channel);
	}
}

psy_audio_InstrumentIndex currslot(psy_audio_XMSampler* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	int rv;

	if (event->inst != psy_audio_NOTECOMMANDS_EMPTY) {
		psy_table_insert(&self->lastinst, channel, (void*)(uintptr_t)event->inst);
		rv = event->inst;
	} else
	if (psy_table_exists(&self->lastinst, channel)) {
		rv = (int)(uintptr_t) psy_table_at(&self->lastinst, channel);
	} else { 
		rv = psy_audio_NOTECOMMANDS_EMPTY;
	}
	return psy_audio_instrumentindex_make(self->instrumentbank, rv);
}

void releaseallvoices(psy_audio_XMSampler* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*) p->entry;		
		psy_audio_xmsamplervoice_release(voice);		
	}
}

void releasevoices(psy_audio_XMSampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_xmsamplervoice_release(voice);
		}
	}
}

void releaseinst(psy_audio_XMSampler* self, uintptr_t channel, uintptr_t inst)
{
	psy_List* p;

	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*)p->entry;
		if (voice->channelnum == channel && voice->instidx) {
			psy_audio_xmsamplervoice_release(voice);
		}
	}
}

void nnavoices(psy_audio_XMSampler* self, uintptr_t channel)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*) p->entry;
		if (voice->channelnum == channel) {			
			psy_audio_xmsamplervoice_nna(voice);
		}
	}
}

psy_audio_XMSamplerVoice* activevoice(psy_audio_XMSampler* self, uintptr_t channel)
{
	psy_audio_XMSamplerVoice* rv = NULL;	
	psy_List* p = NULL;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*) p->entry;
		if (voice->channelnum == channel && !psy_dsp_envelopecontroller_releasing(&voice->amplitudeenvelope)
				&& psy_dsp_envelopecontroller_playing(&voice->amplitudeenvelope)) {
			rv = voice;
			break;
		}
	}
	return rv;
}

void removeunusedvoices(psy_audio_XMSampler* self)
{
	psy_List* p;
	psy_List* q;
		
	for (p = self->voices; p != NULL; p = q) {
		psy_audio_XMSamplerVoice* voice;

		q = p->next;
		voice = (psy_audio_XMSamplerVoice*)p->entry;		
		if (!psy_dsp_envelopecontroller_playing(&voice->amplitudeenvelope) ||
				!psy_audio_xmsamplervoice_isplaying(voice)) {
			psy_audio_xmsamplervoice_dispose(voice);
			free(voice);
			psy_list_remove(&self->voices, p);
		}			
	}
}
*/
const psy_audio_MachineInfo* info(psy_audio_XMSampler* self)
{	
	return &macinfo;
}
/*
uintptr_t numparameters(psy_audio_XMSampler* self)
{
	return numparametercols(self) * 8;
}

uintptr_t numparametercols(psy_audio_XMSampler* self)
{
	return 10;
}

#define CHANNELROW 2

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
		psy_audio_XMSamplerChannel* channel;
		
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

void resamplingmethod_tweak(psy_audio_XMSampler* self,
	psy_audio_ChoiceMachineParam* sender, float value)
{	
	psy_audio_xmsampler_setresamplerquality(self,
		(int)(value * (sender->maxval - sender->minval) + 0.5f) +
			sender->minval);	
}

static int alteRand(int x) { return (x * rand()) / 32768; }

psy_List* sequencerinsert(psy_audio_XMSampler* self, psy_List* events)
{
	psy_List* p;
	psy_List* insert = 0;

	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* event;

		entry = p->entry;
		event = psy_audio_patternentry_front(entry);
		if (event->cmd == XM_SAMPLER_CMD_EXTENDED) {
			if ((event->parameter & 0xf0) == XM_SAMPLER_CMD_E_DELAYED_NOTECUT) {
				psy_audio_PatternEntry* noteoff;

				// This means there is always 6 ticks per row whatever number of rows.
				//triggernoteoff = (Global::player().SamplesPerRow()/6.f)*(ite->_parameter & 0x0f);
				noteoff = psy_audio_patternentry_allocinit();
				psy_audio_patternentry_front(noteoff)->note = psy_audio_NOTECOMMANDS_RELEASE;
				psy_audio_patternentry_front(noteoff)->mach = psy_audio_patternentry_front(entry)->mach;
				//  entry->offset
				noteoff->delta += + (event->parameter & 0x0f) / 6.f *
					psy_audio_machine_currbeatsperline(
						psy_audio_xmsampler_base(self));
				psy_list_append(&insert, noteoff);
			} else 
			if ((event->parameter & 0xF0) == XM_SAMPLER_CMD_E_NOTE_DELAY) {
				psy_audio_PatternEntry* newentry;
				psy_audio_PatternEvent* ev;
				int numticks;

				newentry = psy_audio_patternentry_clone(entry);
				ev = psy_audio_patternentry_front(newentry);
				numticks = event->parameter & 0x0f;
				ev->cmd = 0;
				ev->parameter = 0;
				newentry->delta += numticks * psy_audio_machine_beatspertick(
					psy_audio_xmsampler_base(self));				
				psy_list_append(&insert, newentry);
			}
		}
	}
	return insert;
}
*/
// fileio
void loadspecific(psy_audio_XMSampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	int32_t temp;
	bool wrongState = FALSE;
	uint32_t filevers;
	size_t filepos;
	uint32_t size=0;

	// psy_audio_xmsampler_defaultC4(self, TRUE);
	// self->instrumentbank = 1;
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
		psy_dsp_ResamplerQuality resamplertype;

		// numSubtracks
		psyfile_read(songfile->file, &temp, sizeof(temp));
		self->_numVoices = temp;
		// quality
		psyfile_read(songfile->file, &temp, sizeof(temp));			
		switch (temp) {
			case 2: resamplertype = psy_dsp_RESAMPLERQUALITY_SPLINE; break;
			case 3: resamplertype = psy_dsp_RESAMPLERQUALITY_SINC; break;
			case 0:	resamplertype = psy_dsp_RESAMPLERQUALITY_ZERO_ORDER; break;
			case 1:
			default:
				resamplertype = psy_dsp_RESAMPLERQUALITY_LINEAR;
				break;
		}
		// psy_audio_xmsampler_setresamplerquality(self, resamplertype);
		for (i = 0; i < 128; ++i) {
			psyfile_read(songfile->file, &zxxMap[i].mode, sizeof(zxxMap[i].mode));
			psyfile_read(songfile->file, &zxxMap[i].value, sizeof(zxxMap[i].value));
		}

		psyfile_read(songfile->file, &m_bAmigaSlides, sizeof(m_bAmigaSlides));
		self->m_bAmigaSlides = m_bAmigaSlides;
		psyfile_read(songfile->file, &m_UseFilters, sizeof(m_UseFilters));
		self->m_UseFilters = m_UseFilters;
		psyfile_read(songfile->file, &m_GlobalVolume, sizeof(m_GlobalVolume));
		psyfile_read(songfile->file, &m_PanningMode, sizeof(m_PanningMode));
		// self->masterchannel.volume = m_GlobalVolume / 127.f;
		for (i = 0; i < MAX_TRACKS; ++i) {				
			psy_audio_xmsamplerchannel_load(&self->m_Channel[i], songfile);
		}

		if ((filevers & XMSAMPLER_VERSION & 0x0000FFFF) >= 0x02) {
			uint32_t temp32;

			psyfile_read(songfile->file, &temp32, sizeof(temp32));
			// self->instrumentbank = temp32;
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
}

void savespecific(psy_audio_XMSampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	int32_t temp;
	int i;
	size_t endpos;
	int resamplerquality;

	// we cannot calculate the size previous to save, so we write a placeholder
	// and seek back to write the correct value.
	uint32_t size = 0;
	size_t filepos = psyfile_getpos(songfile->file);
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write_uint32(songfile->file, XMSAMPLER_VERSION);
	psyfile_write_uint32(songfile->file, self->_numVoices);
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
		psyfile_write_int32(songfile->file, 0); // zxxMap[i].mode);
		psyfile_write_int32(songfile->file, 0); // zxxMap[i].value);
	}
	psyfile_write_uint8(songfile->file, (uint8_t)(self->m_bAmigaSlides != 0));
	psyfile_write_uint8(songfile->file, (uint8_t) self->m_UseFilters);
	psyfile_write_int32(songfile->file, self->m_GlobalVolume);
	psyfile_write_int32(songfile->file, self->m_PanningMode);
	for (i = 0; i < MAX_TRACKS; i++) {		
		psy_audio_xmsamplerchannel_save(&self->m_Channel[i], songfile);
	}
	psyfile_write_uint32(songfile->file, (uint32_t)1); // self->instrumentbank);
	endpos = psyfile_getpos(songfile->file);
	psyfile_seek(songfile->file, filepos);
	size = (uint32_t)(endpos - filepos - sizeof(size));
	psyfile_write_uint32(songfile->file, size);
	psyfile_seek(songfile->file, endpos);
}
/*
void psy_audio_xmsampler_setresamplerquality(psy_audio_XMSampler* self,
	psy_dsp_ResamplerQuality quality)
{
	psy_List* it;
	
	for (it = self->voices; it != NULL; psy_list_next(&it)) {
		psy_audio_XMSamplerVoice* voice;

		voice = (psy_audio_XMSamplerVoice*)(it->entry);
		psy_audio_xmsamplervoice_setresamplerquality(voice, quality);
	}
}
*/