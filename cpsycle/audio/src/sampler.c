// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampler.h"
// audio
#include "constants.h"
#include "instruments.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "samplerdefs.h"
#include "samples.h"
#include "songio.h"
// dsp
#include <operations.h>
#include <valuemapper.h>
// std
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"
// #include "../../detail/trace.h"

static int alteRand(int x) { return (x * rand()) / 32768; }

// psycle-mfc: none
// Machine Info used by the plugincatcher
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
	MACH_SAMPLER,
	NULL,			// NO MODULPATH
	0,				// shellidx	
	PS1SAMPLERHELP	// help text
};

const psy_audio_MachineInfo* psy_audio_sampler_info(void)
{
	return &macinfo;
}

// Voice
static void psy_audio_samplervoice_rampvolume(psy_audio_SamplerVoice* self);
// implementation
// psycle-mfc: Voice::Voice()
void psy_audio_samplervoice_init(psy_audio_SamplerVoice* self, psy_audio_Sampler* sampler)
{
	assert(self);
	// This uses the XMWaveDataController
	// instead a separate Controller like in mfc psycle
	// The missing Controller variables are added to the
	// voice
	self->_pan = 0.5f;
	self->_vol = 0.f;
	self->_lVolDest = 0;
	self->_rVolDest = 0;
	self->_lVolCurr = 0;
	self->_rVolCurr = 0;
	// The XMWaveDataController
	psy_audio_sampleiterator_init(&self->controller, NULL, psy_dsp_RESAMPLERQUALITY_LINEAR);
	// Voice Constructor
	self->sampler = sampler;
	self->inst = NULL;
	self->instrument = 0xFF;
	self->channel = -1;
	self->_sampleCounter = 0;
	self->_cutoff = 0;
	self->effCmd = PS1_SAMPLER_CMD_NONE;
	filter_init(&self->_filter);
	psy_audio_samplervoice_setup(self);
}

// psycle-mfc:: Voice::~Voice()
void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice* self)
{
	assert(self);
	psy_dsp_adsr_dispose(&self->_envelope);
	psy_dsp_adsr_dispose(&self->_filterEnv);
	psy_audio_sampleiterator_dispose(&self->controller);
	filter_dispose(&self->_filter);
}

// psycle mfc: Voice::Init
void psy_audio_samplervoice_setup(psy_audio_SamplerVoice* self)
{
	assert(self);
	adsr_settings_initdefault(&self->_envelopesettings);
	psy_dsp_adsr_init(&self->_envelope, &self->_envelopesettings,
		psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
	adsr_settings_initdefault(&self->_filterenvsettings);
	psy_dsp_adsr_init(&self->_filterEnv, &self->_filterenvsettings,
		psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
	self->channel = -1;
	self->_triggerNoteOff = 0;
	self->_triggerNoteDelay = 0;
	self->effretTicks = 0;
	self->_effPortaSpeed = 4294967296.0f;
}

// mfc name: Voice::NewLine()
void psy_audio_samplervoice_newline(psy_audio_SamplerVoice* self)
{
	assert(self);
	self->effretTicks = 0;
	self->effCmd = PS1_SAMPLER_CMD_NONE;
	if (self->_triggerNoteOff > self->_sampleCounter) {
		self->_triggerNoteOff -= self->_sampleCounter;
	} else {
		self->_triggerNoteOff = 0;
	}
	if (self->_triggerNoteDelay > self->_sampleCounter) {
		self->_triggerNoteDelay -= self->_sampleCounter;
	} else {
		self->_triggerNoteDelay = 0;
	}
	self->_sampleCounter = 0;
}

// psycle-mfc: Sampler.hpp class
// Sampler Prototypes
static void psy_audio_sampler_init_voices(psy_audio_Sampler*);
static void psy_audio_sampler_dispose(psy_audio_Sampler*);
static void psy_audio_sampler_dispose_voices(psy_audio_Sampler*);
static void psy_audio_sampler_stop(psy_audio_Sampler*);
static void psy_audio_sampler_stopinstrument(psy_audio_Sampler*,
	int insIdx);
static void psy_audio_sampler_setsamplerate(psy_audio_Sampler*, int sr);
static bool psy_audio_sampler_playstrack(psy_audio_Sampler*, int track);
static int psy_audio_sampler_getcurrentvoice(psy_audio_Sampler*, int track);
static void psy_audio_sampler_newline(psy_audio_Sampler*);
static void psy_audio_sampler_clearmulticmdmem(psy_audio_Sampler*);
static uintptr_t psy_audio_sampler_getfreevoice(psy_audio_Sampler*);
static void psy_audio_sampler_generateaudio(psy_audio_Sampler*, psy_audio_BufferContext*);
static void psy_audio_sampler_ontimertick(psy_audio_Sampler*);
static void psy_audio_sampler_ontimerwork(psy_audio_Sampler*,
	psy_audio_BufferContext*);
static psy_List* psy_audio_sampler_sequencerinsert(psy_audio_Sampler*, psy_List* events);
static void psy_audio_sampler_seqtick(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void psy_audio_sampler_loadspecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void psy_audio_sampler_savespecific(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void psy_audio_sampler_initparameters(psy_audio_Sampler*);
static void psy_audio_sampler_disposeparameters(psy_audio_Sampler*);
static uintptr_t psy_audio_sampler_numparametercols(psy_audio_Sampler*);
static uintptr_t psy_audio_sampler_numparameters(psy_audio_Sampler*);
static psy_audio_MachineParam* psy_audio_sampler_parameter(psy_audio_Sampler*,
	uintptr_t param);
static void psy_audio_sampler_resamplingmethod_tweak(psy_audio_Sampler*,
	psy_audio_ChoiceMachineParam* sender, float value);

static const psy_audio_MachineInfo* psy_audio_sampler_vinfo(psy_audio_Sampler*
	self)
{
	assert(self);
	return &macinfo;
}

static uintptr_t psy_audio_sampler_numinputs(psy_audio_Sampler* self)
{
	assert(self);
	return 0;
}

static uintptr_t psy_audio_sampler_numoutputs(psy_audio_Sampler* self)
{
	assert(self);
	return 2;
}

// vtable
static MachineVtable sampler_vtable;
static bool sampler_vtable_initialized = FALSE;
// implementation
static void sampler_vtable_init(psy_audio_Sampler* self)
{
	assert(self);
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_sampler_base(self)->vtable);
		sampler_vtable.generateaudio = (fp_machine_generateaudio)
			psy_audio_sampler_generateaudio;
		sampler_vtable.seqtick = (fp_machine_seqtick)
			psy_audio_sampler_seqtick;
		sampler_vtable.newline = (fp_machine_newline)
			psy_audio_sampler_newline;
		sampler_vtable.sequencerinsert = (fp_machine_sequencerinsert)
			psy_audio_sampler_sequencerinsert;
		sampler_vtable.stop = (fp_machine_stop)psy_audio_sampler_stop;
		sampler_vtable.info = (fp_machine_info)psy_audio_sampler_vinfo;
		sampler_vtable.dispose = (fp_machine_dispose)
			psy_audio_sampler_dispose;
		sampler_vtable.numinputs = (fp_machine_numinputs)
			psy_audio_sampler_numinputs;
		sampler_vtable.numoutputs = (fp_machine_numoutputs)
			psy_audio_sampler_numoutputs;
		sampler_vtable.loadspecific = (fp_machine_loadspecific)
			psy_audio_sampler_loadspecific;
		sampler_vtable.savespecific = (fp_machine_savespecific)
			psy_audio_sampler_savespecific;
		sampler_vtable.numparametercols = (fp_machine_numparametercols)
			psy_audio_sampler_numparametercols;
		sampler_vtable.numparameters = (fp_machine_numparameters)
			psy_audio_sampler_numparameters;
		sampler_vtable.parameter = (fp_machine_parameter)
			psy_audio_sampler_parameter;
		sampler_vtable_initialized = TRUE;
	}
}

// psycle-mfc: Sampler::Sampler(int index) and Sampler::Init(void)
// called by the MachineFactory
// psy_audio_MachineCallback provides callbacks to the host
// for samplerate, samples and instruments.
// This avoids the static global::player calls of psycle-mfc
void psy_audio_sampler_init(psy_audio_Sampler* self,
	psy_audio_MachineCallback* callback)
{
	int i;

	assert(self);
	// SetSampleRate(Global::player().SampleRate());
	custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_sampler_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_sampler_base(self), "Sampler");
	self->linearslide = TRUE;
	self->resamplerquality = psy_dsp_RESAMPLERQUALITY_SPLINE;
	for (i = 0; i < MAX_TRACKS; i++) {
		self->lastInstrument[i] = 255;
	}
	self->multicmdMem = NULL;	
	psy_audio_sampler_init_voices(self);
	self->resamplerquality = psy_dsp_RESAMPLERQUALITY_LINEAR;
	self->defaultspeed = 1;
	self->instrumentbank = 0;
	self->samplerowcounter = 0;
	self->usefilters = TRUE;
	self->panningmode = psy_audio_PANNING_LINEAR;
	self->samplerowcounter = 0;
	self->instrumentbank = 0;
	psy_audio_sampler_initparameters(self);
	psy_audio_ticktimer_init(&self->ticktimer,
		self, // callback context (sampler)
		(fp_samplerticktimer_ontick)psy_audio_sampler_ontimertick,
		(fp_samplerticktimer_onwork)psy_audio_sampler_ontimerwork);
}

void psy_audio_sampler_init_voices(psy_audio_Sampler* self)
{
	uintptr_t i;

	assert(self);
	self->numvoices = PS1_SAMPLER_DEFAULT_POLYPHONY;
	for (i = 0; i < PS1_SAMPLER_MAX_POLYPHONY; ++i) {
		psy_audio_samplervoice_init(&self->_voices[i], self);
	}
}

psy_audio_Sampler* psy_audio_sampler_alloc(void)
{
	return (psy_audio_Sampler*)malloc(sizeof(psy_audio_Sampler));
}

psy_audio_Sampler* psy_audio_sampler_allocinit(psy_audio_MachineCallback*
	callback)
{
	psy_audio_Sampler* rv;

	rv = psy_audio_sampler_alloc();
	if (rv) {
		psy_audio_sampler_init(rv, callback);
	}
	return rv;
}

// psycle-mfc: Sampler::~Sampler()
void psy_audio_sampler_dispose(psy_audio_Sampler* self)
{	
	assert(self);
	psy_audio_sampler_clearmulticmdmem(self);
	psy_audio_sampler_disposeparameters(self);
	psy_audio_sampler_dispose_voices(self);
	custommachine_dispose(&self->custommachine);
}

void psy_audio_sampler_dispose_voices(psy_audio_Sampler* self)
{
	uintptr_t i;

	assert(self);
	for (i = 0; i < PS1_SAMPLER_MAX_POLYPHONY; ++i) {
		psy_audio_samplervoice_dispose(&self->_voices[i]);
	}
}

// psycle-mfc: Sampler::Stop(void)
void psy_audio_sampler_stop(psy_audio_Sampler* self)
{
	uintptr_t i;

	assert(self);
	for (i = 0; i < self->numvoices; ++i) {
		psy_audio_samplervoice_noteofffast(&self->_voices[i]);
		self->_voices[i]._effPortaSpeed = 4294967296.0f;
	}
}

// psycle-mfc: Sampler::StopInstrument(void)
void psy_audio_sampler_stopinstrument(psy_audio_Sampler* self,
	int insIdx)
{
	uintptr_t i;

	assert(self);
	for (i = 0; i < self->numvoices; ++i) {
		psy_audio_SamplerVoice* pVoice = &self->_voices[i];
		if (pVoice->instrument == insIdx &&
			(pVoice->_envelope.stage != ENV_OFF ||
				pVoice->_triggerNoteDelay > 0)) {
			psy_audio_samplervoice_setup(pVoice);
		}
	}
}

// mfc-psycle: Sampler::NumAuxColumnIndexes()
// not implemented

// mfc-psycle: SetSampleRate(int sr)
void psy_audio_sampler_setsamplerate(psy_audio_Sampler* self, int sr)
{
	uintptr_t i;

	assert(self);
	//Machine::SetSampleRate(sr);
	for (i = 0; i < self->numvoices; ++i) {
		filter_setsamplerate(&self->_voices[i]._filter, sr);
		psy_dsp_adsr_setsamplerate(&self->_voices[i]._envelope, sr);
		psy_dsp_adsr_setsamplerate(&self->_voices[i]._filterEnv, sr);
	}
}

// mfc-psycle: playsTrack(const int track) const
// cpsycle: so far not used 
bool psy_audio_sampler_playstrack(psy_audio_Sampler* self, int track)
{
	assert(self);
	// return (self->TriggerDelayCounter[track] > 0 || GetCurrentVoice(track) != -1);
	return FALSE;
}

// mfc-psycle: Sampler::GetCurrentVoice(int track) const
int psy_audio_sampler_getcurrentvoice(psy_audio_Sampler* self, int track)
{
	uintptr_t voice;

	assert(self);
	for (voice = 0; voice < self->numvoices; ++voice) {
		// ENV_OFF is not checked, because channel will be -1
		if (self->_voices[voice].channel == track &&
			(self->_voices[voice]._triggerNoteDelay > 0 ||
				self->_voices[voice]._envelope.stage != ENV_FASTRELEASE)) {
			return voice;
		}
	}
	return -1;
}

// mfc-psycle: Sampler::NewLine()
void psy_audio_sampler_newline(psy_audio_Sampler* self)
{
	uintptr_t voice;

	assert(self);
	psy_audio_sampler_clearmulticmdmem(self);
	for (voice = 0; voice < self->numvoices; ++voice) {
		psy_audio_samplervoice_newline(&self->_voices[voice]);
	}
	{
		// mfc-psycle: player has a global tick counter
		// cpsycle: sampler has the tick generator itself and needs to
		//          sync/reset it with the newline event of the
		//          player/sequencer and resets the tickcount to 0
		self->samplerowcounter = 0;
		psy_audio_ticktimer_reset(&self->ticktimer,
			(uintptr_t)psy_audio_machine_samplespertick(
				psy_audio_sampler_base(self)));
	}
}

// mfc-psycle: Sampler::PostNewLine()
//			   Event Order in mfc-psycle is newline; executeline; postnewline
//			   Sequencer can execute notes at every time, so a postnewline
//			   can't be implemented in this way. Instead multicmds are cleared
//			   before notifyline. clearmulticmdmem; notifynewline;
//             Problems may occur if notes are inside a line.
//             Multicmds may be called twice
void psy_audio_sampler_clearmulticmdmem(psy_audio_Sampler* self)
{
	assert(self);
	psy_list_deallocate(&self->multicmdMem, (psy_fp_disposefunc)NULL);
}

// mfc-psycle: Sampler::GetFreeVoice() const
uintptr_t psy_audio_sampler_getfreevoice(psy_audio_Sampler* self)
{
	uintptr_t useVoice = UINTPTR_MAX;
	uintptr_t voice;

	assert(self);
	for (voice = 0; voice < self->numvoices; ++voice)	// Find a voice to apply the new note
	{
		switch (self->_voices[voice]._envelope.stage)
		{
			case ENV_OFF:
				if (self->_voices[voice]._triggerNoteDelay == 0)
				{
					useVoice = voice;
					voice = self->numvoices; // Ok, we can go out from the loop already.
				}
				break;
			case ENV_FASTRELEASE:
				useVoice = voice;
				break;
			case ENV_RELEASE:
				if (useVoice == UINTPTR_MAX) {
					useVoice = voice;
				}
				break;
			default:
				break;
		}
	}
	return useVoice;
}

// mfc-psycle: Sampler::Tick(int channel, PatternEntry* pData)
void psy_audio_sampler_seqtick(psy_audio_Sampler* self, uintptr_t channel,
	const psy_audio_PatternEvent* pData)
{
	assert(self);
	// if (_mute) return; // Avoid new note entering when muted.
	psy_audio_PatternEvent* copy;
	psy_audio_PatternEvent data = *pData;
	psy_audio_Samples* samples;
	psy_List* ite;
	uintptr_t useVoice = UINTPTR_MAX;
	uintptr_t voice;
	bool doporta = FALSE;

	if (data.note == NOTECOMMANDS_MIDICC) {
		//TODO: This has one problem, it requires a non-mcm command to trigger the memory.
		data.inst = channel;
		copy = malloc(sizeof(psy_audio_PatternEvent));
		*copy = data;
		psy_list_append(&self->multicmdMem, copy);
		return;
	} else if (data.note > NOTECOMMANDS_RELEASE && data.note != NOTECOMMANDS_EMPTY) {
		// don't process twk , twf of Mcm Commands
		return;
	}
	if (data.inst == 255) {
		data.inst = self->lastInstrument[channel];
		if (data.inst == 255) {
			return;  // no previous sample. Skip
		}
	} else {
		self->lastInstrument[channel] = data.inst;
	}
	samples = psy_audio_machine_samples(psy_audio_sampler_base(self));
	if (!psy_audio_samples_at(samples,
			sampleindex_make(data.inst, self->instrumentbank))) {
		return; // if no wave, return.
	}
	voice = psy_audio_sampler_getcurrentvoice(self, channel);
	if (data.cmd != PS1_SAMPLER_CMD_NONE) {
		// Adding also the current command, to make the loops easier.
		psy_audio_PatternEvent data2 = data;
		data2.inst = channel;
		copy = malloc(sizeof(psy_audio_PatternEvent));
		*copy = data2;
		psy_list_append(&self->multicmdMem, copy);
	}
	doporta = FALSE;
	for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ev->inst == channel) {
			if (ev->cmd == PS1_SAMPLER_CMD_PORTA2NOTE &&
					data.note < NOTECOMMANDS_RELEASE &&
					voice != UINTPTR_MAX) {
				// if (self->linearslide) {   // isLinearSlide()
					// EnablePerformFx();
				// }
				doporta = TRUE;
			} else if (ev->cmd == PS1_SAMPLER_CMD_PORTADOWN ||
					   ev->cmd == PS1_SAMPLER_CMD_PORTAUP) {
				//	if (isLinearSlide()) {
					//	EnablePerformFx();
					//}
			}
		}
	}	
	if (data.note < NOTECOMMANDS_RELEASE && !doporta) {	// Handle Note On.	
		useVoice = psy_audio_sampler_getfreevoice(self); // Find a voice to apply the new note
		if (voice != UINTPTR_MAX) { // NoteOff previous Notes in this channel.
			switch (self->_voices[voice].inst->nna) {
				case 0:
					psy_audio_samplervoice_noteofffast(&self->_voices[voice]);
					break;
				case 1:
					psy_audio_samplervoice_noteoff(&self->_voices[voice]);
					break;
				default:
					break;
			}
			if (useVoice == UINTPTR_MAX) {
				useVoice = voice;
			}
		}
		if (useVoice == UINTPTR_MAX) {	
			// No free voices. Assign first one.
			// This algorithm should be replace by a LRU lookup
			useVoice = 0;
		}
		self->_voices[useVoice].channel = channel;
	} else {
		if (voice != UINTPTR_MAX) {
			if (data.note == NOTECOMMANDS_RELEASE) {
				psy_audio_samplervoice_noteoff(&self->_voices[voice]);  //  Handle Note Off
			}
			useVoice = voice;
		}
		if (useVoice == UINTPTR_MAX) {
			return; // No playing note on this channel. Just go out.
					// Change it if you have channel commands.
		}
	}
	// If you want to make a command that controls more than one voice (the entire channel, for
	// example) you'll need to change this. Otherwise, add it to Voice.Tick().
	// Todo: mfc-psycle: Event Order newline; executeline; postnewline;
	// Sequencer can emit notes inside a line aswell. This may execute multicmds twice.
	psy_audio_samplervoice_tick(&self->_voices[useVoice], &data, channel,
		self->defaultspeed ? NOTECOMMANDS_MIDDLEC : 48, self->multicmdMem);
}

// mfc-psycle: Voice::Tick(PatternEntry* pEntry,int channelNum, dsp::resampler& resampler, int baseC, std::vector<PatternEntry>&multicmdMem)
int psy_audio_samplervoice_tick(psy_audio_SamplerVoice* self, psy_audio_PatternEvent* pEntry,
	int channelNum, int basec, psy_List* multicmdMem)
{
	int triggered = 0;
	uint64_t w_offset = 0;
	bool dooffset = FALSE;
	bool dovol = FALSE;
	bool dopan = FALSE;
	bool doporta = FALSE;
	psy_List* ite;
	psy_audio_Instruments* insts;

	assert(self);
	//If this sample is not enabled, Voice::Tick is not called. Also, Sampler::Tick takes care of previus instrument used.
	self->instrument = pEntry->inst;
	insts = psy_audio_machine_instruments(psy_audio_sampler_base(self->sampler));
	self->inst = psy_audio_instruments_at(insts,
		psy_audio_instrumentindex_make(self->sampler->instrumentbank, self->instrument));
	// Setup commands that affect the new or already playing voice.
	for (ite = multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ev->inst == channelNum) {
			// one shot {
			switch (ev->cmd) {
			case PS1_SAMPLER_CMD_PANNING: {
				self->_pan = psy_dsp_map_256_1(ev->parameter);
				//dopan = TRUE;
			} break;
			case PS1_SAMPLER_CMD_OFFSET: {
				psy_audio_Sample* sample;

				sample = psy_audio_samples_at(
					psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
					sampleindex_make(self->instrument, 0));
				w_offset = (uint64_t)(ev->parameter * sample->numframes) << 24;
				dooffset = TRUE;
			} break;
			case PS1_SAMPLER_CMD_VOLUME: {
				self->_vol = psy_dsp_map_256_1(ev->parameter);
				dovol = TRUE;
			} break;
			// }
			// Running {
			case PS1_SAMPLER_CMD_PORTAUP: {
				self->effVal = ev->parameter;
				self->effCmd = ev->cmd;
			} break;
			case PS1_SAMPLER_CMD_PORTADOWN: {
				self->effVal = ev->parameter;
				self->effCmd = ev->cmd;
			} break;
			case PS1_SAMPLER_CMD_PORTA2NOTE: {
				if (self->_envelope.stage != ENV_OFF) {
					self->effCmd = ev->cmd;
					self->effVal = ev->parameter;
					if (pEntry->note < NOTECOMMANDS_RELEASE) {
						psy_audio_Sample* sample;
						double speeddouble;
						double finetune;

						sample = psy_audio_samples_at(
							psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
							sampleindex_make(self->instrument, 0));
						finetune = sample->numframes * 0.01;
						speeddouble = pow(2.0, (((double)pEntry->note +
							(double)sample->tune - (double)basec) + finetune) / 12.0) *
							((double)sample->numframes /
								psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
						self->_effPortaSpeed = (int64_t)(speeddouble * 4294967296.0f);
					}
					if (self->_effPortaSpeed < self->controller.speed) {
						self->effVal *= -1;
					}
					doporta = TRUE;
				}
			} break;
			case PS1_SAMPLER_CMD_RETRIG: {
				if ((ev->parameter & 0x0f) > 0)
				{
					self->effretTicks = (ev->parameter & 0x0f); // number of Ticks.
					self->effVal = (psy_audio_machine_currsamplesperrow(psy_audio_sampler_base(self->sampler)) / (self->effretTicks + 1));

					int volmod = (ev->parameter & 0xf0) >> 4; // Volume modifier.
					switch (volmod)
					{
					case 0:  //fallthrough
					case 8:	self->effretVol = 0; self->effretMode = 0; break;

					case 1:  //fallthrough
					case 2:  //fallthrough
					case 3:  //fallthrough
					case 4:  //fallthrough
					case 5: self->effretVol = (float)(pow(2., volmod - 1) / 64); self->effretMode = 1; break;

					case 6: self->effretVol = 0.66666666f;	 self->effretMode = 2; break;
					case 7: self->effretVol = 0.5f;			 self->effretMode = 2; break;

					case 9:  //fallthrough
					case 10:  //fallthrough
					case 11:  //fallthrough
					case 12:  //fallthrough
					case 13: self->effretVol = (float)(pow(2., volmod - 9) * (-1)) / 64; self->effretMode = 1; break;

					case 14: self->effretVol = 1.5f;					self->effretMode = 2; break;
					case 15: self->effretVol = 2.0f;					self->effretMode = 2; break;
					}
					self->_triggerNoteDelay = self->effVal;
				}
			} break;
				// }
			case PS1_SAMPLER_CMD_EXTENDED: {
				// delayed {
				if ((ev->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEOFF) {
					//This means there is always 6 ticks per row whatever number of rows.
					self->_triggerNoteOff = (psy_audio_machine_currsamplesperrow(
						psy_audio_sampler_base(self->sampler)) / 6.f) * (ev->parameter & 0x0f);
				} else if ((ev->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEDELAY && (ev->parameter & 0x0f) != 0) {
					//This means there is always 6 ticks per row whatever number of rows.
					self->_triggerNoteDelay = (psy_audio_machine_currsamplesperrow(
						psy_audio_sampler_base(self->sampler)) / 6.f) * (ev->parameter & 0x0f);
				}
				//}
			} break;
			}
		}
	}
	if (pEntry->note < NOTECOMMANDS_RELEASE && !doporta)
	{
		psy_audio_Sample* sample;
		double speeddouble;

		sample = psy_audio_samples_at(
			psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
			sampleindex_make(self->instrument, 0));
		psy_audio_sampleiterator_setsample(&self->controller, sample);
		if (self->inst->loop)
		{
			double const totalsamples = (double)(
				psy_audio_machine_currsamplesperrow(psy_audio_sampler_base(self->sampler))
				* self->inst->lines);
			speeddouble = (sample->numframes / totalsamples);
		} else
		{
			float const finetune = (float)sample->finetune * 0.01f;
			speeddouble = pow(2.0f, (pEntry->note + sample->tune - basec + finetune) / 12.0f) *
				((float)sample->samplerate /
					psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
		}
		psy_audio_sampleiterator_setspeed(&self->controller, speeddouble);
		psy_audio_sampleiterator_play(&self->controller);

		if (!dooffset) { dooffset = TRUE; w_offset = 0; }

		// Init Amplitude Envelope
		//
		psy_dsp_adsr_init(&self->_envelope, &self->_envelopesettings,
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
		self->_lVolCurr = self->_lVolDest;
		self->_rVolCurr = self->_rVolDest;
		if (!dovol) { dovol = TRUE; self->_vol = 1.f; }
		if (!dopan) {
			if (self->inst->randompan) {
				dopan = TRUE;
				self->_pan = psy_dsp_map_32768_1(rand());
			} else {
				dopan = TRUE;
				self->_pan = self->controller.sample->panfactor;
			}
		}

		//Init filter
		psy_dsp_adsr_init(&self->_envelope, &self->_filterenvsettings,
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
		if (self->_triggerNoteDelay == 0) {
			psy_dsp_adsr_start(&self->_envelope);
			psy_dsp_adsr_start(&self->_filterEnv);
		}
		triggered = 1;
	}

	if (dovol || dopan) {
		// Panning calculation -------------------------------------------
		self->_rVolDest = self->_pan;
		self->_lVolDest = 1.f - self->_pan;
		//FT2 Style (Two slides) mode, but with max amp = 0.5.
		if (self->_rVolDest > 0.5f) { self->_rVolDest = 0.5f; }
		if (self->_lVolDest > 0.5f) { self->_lVolDest = 0.5f; }

		self->_lVolDest *= self->controller.sample->globalvolume * self->_vol;
		self->_rVolDest *= self->controller.sample->globalvolume * self->_vol;
	}
	if (dooffset) {
		self->controller.pos.QuadPart = w_offset;
	}

	if (triggered)
	{
		self->_lVolCurr = self->_lVolDest;
		self->_rVolCurr = self->_rVolDest;
	}

	return triggered;
}

// mfc-psycle: void Sampler::EnablePerformFx()
// cpsycle: none
//			Ticks are generated from the sampler
//			(see psy_audio_sampler_newline)

// mfc-psycle: GenerateAudioInTicks(int /*startSample*/,  int numSamples)
void psy_audio_sampler_generateaudio(psy_audio_Sampler* self, psy_audio_BufferContext* bc)
{
	assert(self);
	if (!self->linearslide) {
		uintptr_t voice;
		
		for (voice = 0; voice < self->numvoices; ++voice) {
			// A correct implementation needs to take numsamples and player samplerate into account.
			// This will not be fixed to keep sampler compatible with old songs.
			psy_audio_samplervoice_performfxold(&self->_voices[voice]);
		}
	}
	// updates the ticktimer
	// mfc psycle: done in player, generateaudio and enableperformfx
	//             (see psy_audio_sampler_newline)
	//			   mfc adds here delays and retrigger cmds to the TriggerDelay
	//             cpsycle adds them in sequenceinsert
	self->samplerowcounter += bc->numsamples;
	psy_audio_ticktimer_update(&self->ticktimer, bc->numsamples, bc);
}

void psy_audio_sampler_ontimertick(psy_audio_Sampler* self)
{
	assert(self);
	if (self->linearslide) {
		uintptr_t voice;

		for (voice = 0; voice < self->numvoices; ++voice) {
			if (psy_audio_ticktimer_tickcount(&self->ticktimer) != 0) {
				psy_audio_samplervoice_performfxnew(&self->_voices[voice]);
			}
		}
	}
}

void psy_audio_sampler_ontimerwork(psy_audio_Sampler* self,
	psy_audio_BufferContext* bc)
{
	uintptr_t voice;

	assert(self);
	for (voice = 0; voice < self->numvoices; ++voice) {
		psy_audio_samplervoice_work(&self->_voices[voice],
			bc->numsamples,
			psy_audio_buffer_at(bc->output, 0),
			psy_audio_buffer_at(bc->output, 1));
	}
}

// mfc-psycle: none
//			   times delay and retrigger cmds and passes them back to the
//			   sequencer for retrigger (see ps1samplervoice_generateaudio)
psy_List* psy_audio_sampler_sequencerinsert(psy_audio_Sampler* self, psy_List* events)
{
	psy_List* p;
	psy_List* insert = 0;

	assert(self);
	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* event;

		entry = p->entry;
		event = patternentry_front(entry);
		if (event->cmd == PS1_SAMPLER_CMD_EXTENDED) {
			if ((event->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEOFF) {
				psy_audio_PatternEntry* noteoff;

				// This means there is always 6 ticks per row whatever number of rows.
				//_triggerNoteOff = (Global::player().SamplesPerRow()/6.f)*(ite->_parameter & 0x0f);
				noteoff = patternentry_allocinit();
				patternentry_front(noteoff)->note = NOTECOMMANDS_RELEASE;
				patternentry_front(noteoff)->mach = patternentry_front(entry)->mach;
				noteoff->delta += /*entry->offset*/ +(event->parameter & 0x0f) / 6.f *
					psy_audio_machine_currbeatsperline(
						psy_audio_sampler_base(self));
				psy_list_append(&insert, noteoff);
			} else
				if ((event->parameter & 0xF0) == PS1_SAMPLER_CMD_EXT_NOTEDELAY) {
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

// mfc-psycle: void Voice::Work(int numsamples, helpers::dsp::resampler::work_func_type pResamplerWork, float* pSamplesL, float* pSamplesR)
// cpsycle:	Resampler is part of the sampleiterator (WaveDataController) and
//          doesn't need to be passed to work
void psy_audio_samplervoice_work(psy_audio_SamplerVoice* self, int numsamples, float* pSamplesL, float* pSamplesR)
{
	float left_output;
	float right_output;
	uintptr_t dstpos;

	assert(self);
	// If the sample has been deleted while playing...
	if (!psy_audio_samples_at(
		psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
		sampleindex_make(self->instrument, 0))) {
		psy_audio_samplervoice_setup(self);
		return;
	}

	self->_sampleCounter += numsamples;
	if (self->_triggerNoteDelay > 0)
	{
		if (self->_sampleCounter >= self->_triggerNoteDelay)
		{
			if (self->effretTicks > 0)
			{
				self->effretTicks--;
				self->_triggerNoteDelay = self->_sampleCounter + self->effVal;
				self->_envelope.step = (1.0f / (self->inst->volumeenvelope.attack * self->_envelope.samplerate));
				self->_filterEnv.step = (1.0f / (self->inst->filterenvelope.attack * self->_envelope.samplerate));
				self->controller.pos.QuadPart = 0;
				if (self->effretMode == 1)
				{
					self->_lVolDest += self->effretVol;
					self->_rVolDest += self->effretVol;
				} else if (self->effretMode == 2)
				{
					self->_lVolDest *= self->effretVol;
					self->_rVolDest *= self->effretVol;
				}
			} else
			{
				self->_triggerNoteDelay = 0;
			}
			self->_envelope.stage = ENV_ATTACK;
		} else if (self->_envelope.stage == ENV_OFF)
		{
			return;
		}
	} else
		if (self->_envelope.stage == ENV_OFF)
		{
			psy_audio_samplervoice_setup(self);
			return;
		} else if ((self->_triggerNoteOff) && (self->_sampleCounter >= self->_triggerNoteOff))
		{
			psy_audio_samplervoice_noteoff(self);
		}

		dstpos = 0;
		while (numsamples)
		{
			left_output = 0;
			right_output = 0;

			intptr_t nextsamples = min(psy_audio_sampleiterator_prework(&self->controller,
				numsamples, FALSE), numsamples);
			numsamples -= nextsamples;
			while (nextsamples)
			{
				intptr_t diff;

				left_output = psy_audio_sampleiterator_work(&self->controller, 0);
				if (self->controller.sample->stereo) {
					right_output = psy_audio_sampleiterator_work(&self->controller, 1);
				}
				// Amplitude section
				{
					psy_dsp_adsr_tick(&self->_envelope);
					psy_audio_samplervoice_rampvolume(self);
				}
				// Filter section
				//
				if (filter_type(&self->_filter) != F_NONE)
				{
					psy_dsp_adsr_tick(&self->_filterEnv);
					int newcutoff = (int)(self->_cutoff + self->_filterEnv.value * self->_coModify + 0.5f);
					if (newcutoff < 0) {
						newcutoff = 0;
					} else if (newcutoff > 127) {
						newcutoff = 127;
					}
					filter_setcutoff(&self->_filter, newcutoff);
					if (self->controller.sample->stereo) {
						self->_filter.vtable->workstereo(&self->_filter, &left_output, &right_output);
					} else {
						left_output = self->_filter.vtable->work(&self->_filter, left_output);
					}
				}

				if (!self->controller.sample->stereo) {
					right_output = left_output;
				}
				right_output *= self->_rVolCurr * self->_envelope.value;
				left_output *= self->_lVolCurr * self->_envelope.value;
				pSamplesL[dstpos] += left_output;
				pSamplesR[dstpos] += right_output;
				++dstpos;
				nextsamples--;
				diff = psy_audio_sampleiterator_inc(&self->controller);
				self->controller.m_pL += diff;
				self->controller.m_pR += diff;
			}
			psy_audio_sampleiterator_postwork(&self->controller);
			if (!psy_audio_sampleiterator_playing(&self->controller)) {
				psy_audio_samplervoice_setup(self);
				break;
			}
		}
}

// mfc-psycle: WaveDataController::RampVolume
void psy_audio_samplervoice_rampvolume(psy_audio_SamplerVoice* self)
{
	assert(self);
	// calculate volume  (volume ramped)
	if (self->_lVolCurr > self->_lVolDest) {
		self->_lVolCurr -= 0.005f;
		if (self->_lVolCurr < self->_lVolDest)	self->_lVolCurr = self->_lVolDest;
	} else if (self->_lVolCurr < self->_lVolDest) {
		self->_lVolCurr += 0.005f;
		if (self->_lVolCurr > self->_lVolDest)	self->_lVolCurr = self->_lVolDest;
	}
	if (self->_rVolCurr > self->_rVolDest) {
		self->_rVolCurr -= 0.005f;
		if (self->_rVolCurr < self->_rVolDest) self->_rVolCurr = self->_rVolDest;
	} else if (self->_rVolCurr < self->_rVolDest) {
		self->_rVolCurr += 0.005f;
		if (self->_rVolCurr > self->_rVolDest)	self->_rVolCurr = self->_rVolDest;
	}
}

// mfc-psycle: Voice::NoteOff()
void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice* self)
{
	assert(self);
	psy_dsp_adsr_release(&self->_envelope);
	psy_dsp_adsr_release(&self->_filterEnv);
	self->_triggerNoteDelay = 0;
	self->_triggerNoteOff = 0;
}

// mfc-psycle: Voice::NoteOffFast()
void psy_audio_samplervoice_noteofffast(psy_audio_SamplerVoice* self)
{
	assert(self);
	psy_dsp_adsr_fastrelease(&self->_envelope);
	psy_dsp_adsr_fastrelease(&self->_filterEnv);
	self->_triggerNoteDelay = 0;
	self->_triggerNoteOff = 0;
}

// mfc-psycle: Voice::PerformFxOld(dsp::resampler& resampler)
void psy_audio_samplervoice_performfxold(psy_audio_SamplerVoice* self)
{
	assert(self);
	// 4294967 stands for (2^30/250), meaning that
	//value 250 = (inc)decreases the speed in 1/4th of the original (wave) speed each PerformFx call.
	int64_t shift;
	int64_t speed;
	switch (self->effCmd)
	{
		// 0x01 : Pitch Up
	case PS1_SAMPLER_CMD_PORTAUP:
		shift = (int64_t)(self->effVal) * 4294967ll * (float)(self->controller.sample->numframes) /
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler));
		psy_audio_sampleiterator_setspeed(&self->controller, (self->controller.speed + shift) / 4294967296.0);
		break;
		// 0x02 : Pitch Down
	case PS1_SAMPLER_CMD_PORTADOWN:
		shift = (int64_t)(self->effVal) * 4294967ll * (float)(self->controller.sample->numframes) /
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler));
		speed = self->controller.speed - shift;
		if (speed < 0) speed = 0;
		psy_audio_sampleiterator_setspeed(&self->controller, speed / 4294967296.0);
		break;
		// 0x03 : Porta to note
	case PS1_SAMPLER_CMD_PORTA2NOTE:
		//effVal is multiplied by -1 in Tick if it needs to slide down.
		shift = (int64_t)(self->effVal) * 4294967ll * (float)(self->controller.sample->numframes) /
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler));
		speed = self->controller.speed + shift;
		if ((self->effVal < 0 && self->controller.speed < self->_effPortaSpeed)
			|| (self->effVal > 0 && self->controller.speed > self->_effPortaSpeed)) {
			self->controller.speed = self->_effPortaSpeed;
			self->effCmd = PS1_SAMPLER_CMD_NONE;
		}
		psy_audio_sampleiterator_setspeed(&self->controller, speed / 4294967296.0);
		break;
	default:
		break;
	}
}

// mfc-psycle: Voice::PerformFxNew(dsp::resampler& resampler)
void psy_audio_samplervoice_performfxnew(psy_audio_SamplerVoice* self)
{	
	double factor;
	int64_t speed;

	assert(self);
	// value 1 = (inc)decreases the speed in one seminote each beat.
	factor = 1.0 / (12.0 * psy_audio_machine_ticksperbeat(
		psy_audio_sampler_base(self->sampler)));
	switch (self->effCmd) {
		// 0x01 : Pitch Up
		case PS1_SAMPLER_CMD_PORTAUP:
			speed = self->controller.speed * pow(2.0, self->effVal * factor);
			psy_audio_sampleiterator_setspeed(&self->controller, speed / 4294967296.0);
			break;
			// 0x02 : Pitch Down
		case PS1_SAMPLER_CMD_PORTADOWN:
			speed = self->controller.speed * pow(2.0, -self->effVal * factor);
			if (speed < 0) speed = 0;
			psy_audio_sampleiterator_setspeed(&self->controller, speed / 4294967296.0);
			break;
			// 0x03 : Porta to note
		case PS1_SAMPLER_CMD_PORTA2NOTE:
			//effVal is multiplied by -1 in Tick() if it needs to slide down.
			speed = self->controller.speed * pow(2.0, self->effVal * factor);
			if ((self->effVal < 0 && speed < self->_effPortaSpeed)
				|| (self->effVal > 0 && speed > self->_effPortaSpeed)) {
				speed = self->_effPortaSpeed;
				self->effCmd = PS1_SAMPLER_CMD_NONE;
			}
			psy_audio_sampleiterator_setspeed(&self->controller, speed / 4294967296.0);
			break;
		default:
			break;
	}
}

// mfc-psycle: Sampler::ChangeResamplerQuality(helpers::dsp::resampler::quality::type quality)
void psy_audio_sampler_setresamplerquality(psy_audio_Sampler* self,
	psy_dsp_ResamplerQuality quality)
{
	uintptr_t voice;

	assert(self);
	for (voice = 0; voice < self->numvoices; ++voice) {
		psy_audio_sampleiterator_setquality(&self->_voices[voice].controller,
			quality);
	}
}

// mfc-psycle: Sampler::LoadSpecificChunk(RiffFile* pFile, int version)
void psy_audio_sampler_loadspecific(psy_audio_Sampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	psy_dsp_ResamplerQuality resamplerquality;

	assert(self);
	self->instrumentbank = 0;
	psy_audio_sampler_defaultC4(self, FALSE);
	self->linearslide = FALSE;
	size = 0;
	psyfile_read(songfile->file, &size, sizeof(size));
	if (size) {
		/// Version 0
		int32_t temp;
		psyfile_read(songfile->file, &temp, sizeof(temp)); // numSubtracks
		self->numvoices = temp;
		psyfile_read(songfile->file, &temp, sizeof(temp)); // quality		
		switch (temp) {
			case 2:
				resamplerquality = psy_dsp_RESAMPLERQUALITY_SPLINE;
				break;
			case 3:
				resamplerquality = psy_dsp_RESAMPLERQUALITY_SINC;
				break;
			case 0:
				resamplerquality = psy_dsp_RESAMPLERQUALITY_ZERO_ORDER;
				break;
			case 1:
			default:
				resamplerquality = psy_dsp_RESAMPLERQUALITY_LINEAR;
				break;
		}
		psy_audio_sampler_setresamplerquality(self, resamplerquality);
		self->instrumentbank = 0;
		if (size > 3 * sizeof(uint32_t)) {
			uint32_t internalversion;

			psyfile_read(songfile->file, &internalversion,
				sizeof(internalversion));
			if (internalversion >= 1) {
				uint8_t defaultC4;

				// correct A4 frequency.
				psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4));
				psy_audio_sampler_defaultC4(self, defaultC4 != FALSE);
			}
			if (internalversion >= 2) {
				uint8_t slidemode;
				// correct slide.
				psyfile_read(songfile->file, &slidemode, sizeof(slidemode));
				self->linearslide = slidemode;
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

// mfc-psycle: Sampler::SaveSpecificChunk(RiffFile* pFile) 
void psy_audio_sampler_savespecific(psy_audio_Sampler* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t temp;
	uint32_t size;
	
	assert(self);
	size = 3 * sizeof(temp) + 2 * sizeof(bool);
	psyfile_write(songfile->file, &size, sizeof(size));
	temp = self->numvoices;
	psyfile_write(songfile->file, &temp, sizeof(temp)); // numSubtracks
	switch (self->resamplerquality) {
		case psy_dsp_RESAMPLERQUALITY_ZERO_ORDER:
			temp = 0;
			break;
		case psy_dsp_RESAMPLERQUALITY_SPLINE:
			temp = 2;
			break;
		case psy_dsp_RESAMPLERQUALITY_SINC:
			temp = 3;
			break;
		case psy_dsp_RESAMPLERQUALITY_LINEAR: // fallthrough
		default:
			temp = 1;
			break;
	}
	psyfile_write(songfile->file, &temp, sizeof(temp)); // quality
	psyfile_write_int32(songfile->file, SAMPLERVERSION);
	psyfile_write_uint8(songfile->file, (uint8_t)psy_audio_sampler_isdefaultC4(self)); // correct A4
	psyfile_write_uint8(songfile->file, (uint8_t)self->linearslide); // correct slide
	psyfile_write_uint32(songfile->file, (uint32_t)self->instrumentbank);
}

// mfc-psycle: bool Sampler::Load(RiffFile* pFile)
//			   part of psy2.c

void psy_audio_sampler_initparameters(psy_audio_Sampler* self)
{
	psy_dsp_ResamplerQuality quality;

	assert(self);
	psy_audio_intmachineparam_init(&self->param_numvoices,
		"Limit Voices", "Limit Voices", MPF_STATE,
		(int32_t*)&self->numvoices, 1, PS1_SAMPLER_MAX_POLYPHONY);
	psy_audio_choicemachineparam_init(&self->param_resamplingmethod,
		"Quality", "Quality", MPF_STATE,
		(int32_t*)&self->resamplerquality, 0, 3);
	psy_signal_connect(&self->param_resamplingmethod.machineparam.signal_tweak, self,
		psy_audio_sampler_resamplingmethod_tweak);
	for (quality = 0; quality < psy_dsp_RESAMPLERQUALITY_NUMRESAMPLERS;
			++quality) {
		psy_audio_choicemachineparam_setdescription(&self->param_resamplingmethod,
			quality, psy_dsp_multiresampler_name((psy_dsp_ResamplerQuality)
				quality));
	}
	psy_audio_choicemachineparam_init(&self->param_defaultspeed,
		"Default Speed", "Default Speed", MPF_STATE,
		(int32_t*)&self->defaultspeed,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 0,
		"played by C3");
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 1,
		"played by C4");
	psy_audio_intmachineparam_init(&self->param_instrumentbank,
		"Instrumentbank", "Instrumentbank", MPF_STATE,
		(int32_t*)&self->instrumentbank,
		0, 1);
}

void psy_audio_sampler_disposeparameters(psy_audio_Sampler* self)
{
	assert(self);
	psy_audio_intmachineparam_dispose(&self->param_numvoices);
	psy_audio_choicemachineparam_dispose(&self->param_resamplingmethod);
	psy_audio_choicemachineparam_dispose(&self->param_defaultspeed);
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
}

uintptr_t psy_audio_sampler_numparameters(psy_audio_Sampler* self)
{
	assert(self);
	return 4;
}

uintptr_t psy_audio_sampler_numparametercols(psy_audio_Sampler* self)
{
	assert(self);
	return 4;
}

psy_audio_MachineParam* psy_audio_sampler_parameter(psy_audio_Sampler* self,
	uintptr_t param)
{
	assert(self);
	switch (param) {
		case 0:
			return psy_audio_intmachineparam_base(&self->param_numvoices);
			break;
		case 1:
			return psy_audio_choicemachineparam_base(&self->param_resamplingmethod);
			break;
		case 2:
			return psy_audio_choicemachineparam_base(&self->param_defaultspeed);
			break;
		case 3:
			return psy_audio_intmachineparam_base(&self->param_instrumentbank);
			break;
		default:
			break;
	}
	return NULL;
}

void psy_audio_sampler_resamplingmethod_tweak(psy_audio_Sampler* self,
	psy_audio_ChoiceMachineParam* sender, float value)
{
	assert(self);
	psy_audio_sampler_setresamplerquality(self,
		psy_audio_choicemachineparam_choice(sender));		
}
