/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "sampler.h"
/* audio */
#include "constants.h"
#include "instruments.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "samplerdefs.h"
#include "samples.h"
#include "songio.h"
/* dsp */
#include <operations.h>
#include <valuemapper.h>
/* std */
#include <assert.h>
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static int alteRand(int x) { return (x * rand()) / 32768; }

/*
** psycle-mfc: none
** Machine Info used by the plugincatcher
*/
static psy_audio_MachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64 | psy_audio_SUPPORTS_INSTRUMENTS,
	psy_audio_MACHMODE_GENERATOR,
	"Sampler"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampler",
	"Psycledelics",
	"help",
	psy_audio_SAMPLER,
	NULL,			/* NO MODULPATH */
	0,				/* shellidx	*/
	PS1SAMPLERHELP, /* help text */
	"Stereo Sampler Unit. Inserts new sampler.",	/* desc */
	"Sampler"		/* category */
};

const psy_audio_MachineInfo* psy_audio_sampler_info(void)
{
	return &macinfo;
}

void samplerwavedatacontroller_init(SamplerWaveDataController* self)
{
	self->wave = NULL;
	self->_speed = 0;
	self->_vol = 0.f;
	self->_lVolDest = 0;
	self->_rVolDest = 0;
	self->_lVolCurr = 0;
	self->_rVolCurr = 0;	
	psy_dsp_multiresampler_init(&self->resampler,
		psy_dsp_RESAMPLERQUALITY_LINEAR);
	self->_pos.QuadPart = 0;
}

void samplerwavedatacontroller_dispose(SamplerWaveDataController* self)
{
	psy_dsp_resampler_dispose(psy_dsp_multiresampler_base(&self->resampler));
}

void samplerwavedatacontroller_work(SamplerWaveDataController* self,
	float* left_output, float* right_output)
{	
	*left_output = psy_dsp_resampler_work_float(
		psy_dsp_multiresampler_base(&self->resampler),
		self->wave->channels.samples[0],
		(float)double_real(&self->_pos),
		self->wave->numframes,
		self->wave->channels.samples[0],
		self->wave->channels.samples[0] + self->wave->numframes - 1);
	if (psy_audio_sample_stero(self->wave))
	{		
		*right_output =
			 psy_dsp_resampler_work_float(
				psy_dsp_multiresampler_base(&self->resampler),
				self->wave->channels.samples[0],
				(float)double_real(&self->_pos),
				self->wave->numframes,
				self->wave->channels.samples[0],
				self->wave->channels.samples[0] + self->wave->numframes - 1);
	}
}

void samplerwavedatacontroller_rampvolume(SamplerWaveDataController* self)
{
	/* calculate volume  (volume ramped) */
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

bool samplerwavedatacontroller_postwork(SamplerWaveDataController* self)
{
	self->_pos.QuadPart += self->_speed;

	// Loop handler
	//
	if ((self->loop.type == psy_audio_SAMPLE_LOOP_NORMAL)
		&& (self->_pos.HighPart >= self->loop.end))
	{
		self->_pos.HighPart -= (self->loop.end - self->loop.start);
	}
	return (self->_pos.HighPart < self->wave->numframes);
}

void samplerwavedatacontroller_setspeed(SamplerWaveDataController* self, double
	speeddouble)
{

}

/* Envelope */
void samplerenvelope_tick(SamplerEnvelope* self, int decaysamples)
{
	switch (self->_stage)
	{
	case SAMPLER_ENV_ATTACK:
		self->_value += self->_step;
		if (self->_value > 1.0f)
		{
			self->_value = 1.0f;
			self->_stage = SAMPLER_ENV_DECAY;
			self->_step = ((1.0f - self->_sustain) / decaysamples) * self->sratefactor;
		}
		break;
	case SAMPLER_ENV_DECAY:
		self->_value -= self->_step;
		if (self->_value < self->_sustain)
		{
			self->_value = self->_sustain;
			self->_stage = SAMPLER_ENV_SUSTAIN;
		}
		break;
	case SAMPLER_ENV_RELEASE:
	case SAMPLER_ENV_FASTRELEASE:
		self->_value -= self->_step;
		if (self->_value <= 0)
		{
			self->_value = 0;
			self->_stage = SAMPLER_ENV_OFF;
		}
		break;
	default:break;
	}
}

/* Voice */
/* prototypes */
static bool psy_audio_samplervoice_sample_deleted(psy_audio_SamplerVoice*);
/* implementation */

/* psycle-mfc: Voice::Voice() */
void psy_audio_samplervoice_init(psy_audio_SamplerVoice* self,
	psy_audio_Sampler* sampler)
{
	assert(self);	
		
	samplerwavedatacontroller_init(&self->controller);
	samplerenvelope_init(&self->envelope);
	samplerenvelope_init(&self->filterenv);
	/* Voice Constructor */
	self->sampler = sampler;
	self->inst = NULL;
	self->instrument = psy_audio_NOTECOMMANDS_INST_EMPTY;
	self->channel = psy_INDEX_INVALID;
	self->samplecounter = 0;
	self->cutoff = 0;
	self->effcmd = PS1_SAMPLER_CMD_NONE;	
	filter_init(&self->filter);	
	psy_audio_samplervoice_setup(self);
}

/* psycle-mfc:: Voice::~Voice() */
void psy_audio_samplervoice_dispose(psy_audio_SamplerVoice* self)
{
	assert(self);
	
	samplerwavedatacontroller_dispose(&self->controller);
	filter_dispose(&self->filter);
}

/* psycle mfc: Voice::Init */
void psy_audio_samplervoice_setup(psy_audio_SamplerVoice* self)
{	
	assert(self);
			
	self->envelope._stage = SAMPLER_ENV_OFF;
	self->envelope._sustain = 0;
	self->filterenv._stage = SAMPLER_ENV_OFF;
	self->filterenv._sustain = 0;
	self->channel = psy_INDEX_INVALID;
	self->triggernoteoff = 0;
	self->triggernotedelay = 0;
	self->effretticks = 0;
	self->effportaspeed = 4294967296.0f;	
}

/* mfc name: Voice::NewLine() */
void psy_audio_samplervoice_newline(psy_audio_SamplerVoice* self)
{
	assert(self);
	self->effretticks = 0;
	self->effcmd = PS1_SAMPLER_CMD_NONE;
	if (self->triggernoteoff > self->samplecounter) {
		self->triggernoteoff -= self->samplecounter;
	} else {
		self->triggernoteoff = 0;
	}
	if (self->triggernotedelay > self->samplecounter) {
		self->triggernotedelay -= self->samplecounter;
	} else {
		self->triggernotedelay = 0;
	}
	self->samplecounter = 0;
}

/* psy_audio_Sampler */
/* psycle-mfc: Sampler.hpp class */
/* prototypes */
static void psy_audio_sampler_init_voices(psy_audio_Sampler*);
static void psy_audio_sampler_dispose(psy_audio_Sampler*);
static void psy_audio_sampler_dispose_voices(psy_audio_Sampler*);
static void psy_audio_sampler_stop(psy_audio_Sampler*);
static void psy_audio_sampler_stopinstrument(psy_audio_Sampler*,
	int insIdx);
static void psy_audio_sampler_setsamplerate(psy_audio_Sampler*, psy_dsp_big_hz_t sr);
static bool psy_audio_sampler_playstrack(psy_audio_Sampler*, int track);
static uintptr_t psy_audio_sampler_getcurrentvoice(psy_audio_Sampler*,
	uintptr_t track);
static void psy_audio_sampler_newline(psy_audio_Sampler*);
static void psy_audio_sampler_clearmulticmdmem(psy_audio_Sampler*);
static uintptr_t psy_audio_sampler_getfreevoice(psy_audio_Sampler*);
static void psy_audio_sampler_generateaudio(psy_audio_Sampler*, psy_audio_BufferContext*);
static void psy_audio_sampler_ontimertick(psy_audio_Sampler*);
static void psy_audio_sampler_ontimerwork(psy_audio_Sampler*,
	psy_audio_BufferContext*);
static psy_List* psy_audio_sampler_sequencerinsert(psy_audio_Sampler*, psy_List* events);
static void psy_audio_sampler_tick(psy_audio_Sampler*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static int psy_audio_sampler_loadspecificchunk(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static int psy_audio_sampler_savespecificchunk(psy_audio_Sampler*, psy_audio_SongFile*,
	uintptr_t slot);
static void psy_audio_sampler_initparameters(psy_audio_Sampler*);
static void psy_audio_sampler_disposeparameters(psy_audio_Sampler*);
static uintptr_t psy_audio_sampler_numparametercols(psy_audio_Sampler*);
static uintptr_t psy_audio_sampler_numparameters(psy_audio_Sampler*);
static psy_audio_MachineParam* psy_audio_sampler_parameter(psy_audio_Sampler*,
	uintptr_t param);
static void psy_audio_sampler_resamplingmethod_tweak(psy_audio_Sampler*,
	psy_audio_ChoiceMachineParam* sender, float value);
static void psy_audio_sampler_defaultC4(psy_audio_Sampler*, bool correct);
static bool psy_audio_sampler_isdefaultC4(const psy_audio_Sampler*);

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

static void psy_audio_sampler_changeresamplerquality(psy_audio_Sampler* self,
	psy_dsp_ResamplerQuality quality);

static psy_dsp_ResamplerQuality psy_audio_sampler_resamplerquality(psy_audio_Sampler* self)
{
	assert(self);
	return self->resamplerquality;
}

static bool psy_audio_sampler_usefilters(const psy_audio_Sampler* self)
{
	assert(self);
	return self->usefilters != FALSE;
}

static void psy_audio_sampler_setnumvoices(psy_audio_Sampler* self, uintptr_t num)
{
	self->numvoices = num;
}
/* vtable */
static MachineVtable sampler_vtable;
static bool sampler_vtable_initialized = FALSE;
static void sampler_vtable_init(psy_audio_Sampler* self)
{
	assert(self);
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_sampler_base(self)->vtable);
		sampler_vtable.generateaudio = (fp_machine_generateaudio)
			psy_audio_sampler_generateaudio;
		sampler_vtable.seqtick = (fp_machine_seqtick)
			psy_audio_sampler_tick;
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
			psy_audio_sampler_loadspecificchunk;
		sampler_vtable.savespecific = (fp_machine_savespecific)
			psy_audio_sampler_savespecificchunk;
		sampler_vtable.numparametercols = (fp_machine_numparametercols)
			psy_audio_sampler_numparametercols;
		sampler_vtable.numparameters = (fp_machine_numparameters)
			psy_audio_sampler_numparameters;
		sampler_vtable.parameter = (fp_machine_parameter)
			psy_audio_sampler_parameter;
		sampler_vtable_initialized = TRUE;
	}
}

/* implementation */

/*
** psycle-mfc: Sampler::Sampler(int index) and Sampler::Init(void)
** called by the MachineFactory
** psy_audio_MachineCallback provides callbacks to the host
** for samplerate, samples and instruments.
** This avoids the static global::player calls of psycle-mfc
*/
void psy_audio_sampler_init(psy_audio_Sampler* self,
	psy_audio_MachineCallback* callback)
{
	int i;

	assert(self);
	/* SetSampleRate(Global::player().SampleRate()); */
	psy_audio_custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_sampler_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_sampler_base(self), "Sampler");
	self->linearslide = TRUE;
	self->resamplerquality = psy_dsp_RESAMPLERQUALITY_SPLINE;
	for (i = 0; i < MAX_TRACKS; i++) {
		self->lastinstrument[i] = 255;
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
		self, /* ticktimer callback context (self of sampler) */
		(fp_samplerticktimer_ontick)psy_audio_sampler_ontimertick,
		(fp_samplerticktimer_onwork)psy_audio_sampler_ontimerwork);
	psy_audio_sampler_setsamplerate(self,
		psy_audio_machine_samplerate(psy_audio_sampler_base(self)));
}

void psy_audio_sampler_init_voices(psy_audio_Sampler* self)
{
	uintptr_t i;

	assert(self);
	self->numvoices = PS1_SAMPLER_DEFAULT_POLYPHONY;
	for (i = 0; i < PS1_SAMPLER_MAX_POLYPHONY; ++i) {
		psy_audio_samplervoice_init(&self->voices[i], self);
	}
}

/* psycle-mfc: Sampler::~Sampler() */
void psy_audio_sampler_dispose(psy_audio_Sampler* self)
{	
	assert(self);
	psy_audio_sampler_clearmulticmdmem(self);
	psy_audio_sampler_disposeparameters(self);
	psy_audio_sampler_dispose_voices(self);
	psy_audio_custommachine_dispose(&self->custommachine);
}

void psy_audio_sampler_dispose_voices(psy_audio_Sampler* self)
{
	uintptr_t i;

	assert(self);
	for (i = 0; i < PS1_SAMPLER_MAX_POLYPHONY; ++i) {
		psy_audio_samplervoice_dispose(&self->voices[i]);
	}
}

/* psycle-mfc: Sampler::Stop(void) */
void psy_audio_sampler_stop(psy_audio_Sampler* self)
{
	uintptr_t i;

	assert(self);
	for (i = 0; i < self->numvoices; ++i) {
		psy_audio_samplervoice_noteofffast(&self->voices[i]);
		self->voices[i].effportaspeed = 4294967296.0f;
	}
}

/* psycle-mfc: Sampler::StopInstrument(void) */
void psy_audio_sampler_stopinstrument(psy_audio_Sampler* self,
	int insIdx)
{
	uintptr_t i;

	assert(self);
	for (i = 0; i < self->numvoices; ++i) {
		psy_audio_SamplerVoice* voice = &self->voices[i];
		if (voice->instrument == insIdx &&
			(voice->envelope._stage != SAMPLER_ENV_OFF ||
				voice->triggernotedelay > 0)) {
			psy_audio_samplervoice_setup(voice);
		}
	}
}

/* mfc-psycle: Sampler::NumAuxColumnIndexes() */
/* not implemented */

/* mfc-psycle: SetSampleRate(int sr) */
void psy_audio_sampler_setsamplerate(psy_audio_Sampler* self, psy_dsp_big_hz_t sr)
{
	uintptr_t i;

	assert(self);	
	for (i = 0; i < self->numvoices; ++i) {
		filter_setsamplerate(&self->voices[i].filter, (int)sr);
		samplerenvelope_updatesrate(&self->voices[i].envelope, sr);
		samplerenvelope_updatesrate(&self->voices[i].filterenv, sr);		
	}
}

/*
** mfc-psycle: playsTrack(const int track) const
** cpsycle: so far not used
*/
bool psy_audio_sampler_playstrack(psy_audio_Sampler* self, int track)
{
	assert(self);
	/* return (self->TriggerDelayCounter[track] > 0 || GetCurrentVoice(track) != -1); */
	return FALSE;
}

/* mfc-psycle: Sampler::GetCurrentVoice(int track) const */
uintptr_t psy_audio_sampler_getcurrentvoice(psy_audio_Sampler* self, uintptr_t track)
{
	uintptr_t voice;

	assert(self);
	for (voice = 0; voice < self->numvoices; ++voice) {
		// ENV_OFF is not checked, because channel will be -1
		if (self->voices[voice].channel == track &&
			((self->voices[voice].triggernotedelay > 0) ||
				(self->voices[voice].envelope._stage !=
					SAMPLER_ENV_FASTRELEASE))) {
			return voice;
		}
	}
	return psy_INDEX_INVALID;
}

/* mfc-psycle: Sampler::NewLine() */
void psy_audio_sampler_newline(psy_audio_Sampler* self)
{
	uintptr_t voice;

	assert(self);
	psy_audio_sampler_clearmulticmdmem(self);
	for (voice = 0; voice < self->numvoices; ++voice) {
		psy_audio_samplervoice_newline(&self->voices[voice]);
	}
	{
		/*
		** mfc-psycle: player has a global tick counter
		** cpsycle: sampler has the tick generator itself and needs to
		**          sync/reset it with the newline event of the
		**          player/sequencer and resets the tickcount to 0
		*/
		self->samplerowcounter = 0;
		psy_audio_ticktimer_reset(&self->ticktimer,
			(uintptr_t)psy_audio_machine_samplespertick(
				psy_audio_sampler_base(self)));
	}
}

/*
** mfc-psycle: Sampler::PostNewLine()
** 			   Event Order in mfc-psycle is newline; executeline; postnewline
** 			   Sequencer can execute notes at every time, so a postnewline
** 			   can't be implemented in this way. Instead multicmds are cleared
** 			   before notifyline. clearmulticmdmem; notifynewline;
**             Problems may occur if notes are inside a line.
**             Multicmds may be called twice
*/
void psy_audio_sampler_clearmulticmdmem(psy_audio_Sampler* self)
{
	assert(self);
	psy_list_deallocate(&self->multicmdMem, (psy_fp_disposefunc)NULL);
}

/* mfc-psycle: Sampler::GetFreeVoice() const */
uintptr_t psy_audio_sampler_getfreevoice(psy_audio_Sampler* self)
{
	uintptr_t usevoice = psy_INDEX_INVALID;
	uintptr_t voice;

	assert(self);
	for (voice = 0; voice < self->numvoices; ++voice)	/* Find a voice to apply the new note */
	{		
		switch (self->voices[voice].envelope._stage)
		{
			case SAMPLER_ENV_OFF:
				if (self->voices[voice].triggernotedelay == 0)
				{
					usevoice = voice;
					voice = self->numvoices; /* Ok, we can go out from the loop already. */
				}
				break;
			case SAMPLER_ENV_FASTRELEASE:
				usevoice = voice;
				break;
			case SAMPLER_ENV_RELEASE:
				if (usevoice == psy_INDEX_INVALID) {
					usevoice = voice;
				}
				break;
			default:
				break;
		}
	}
	return usevoice;
}

/* mfc-psycle: Sampler::Tick(int channel, PatternEntry* pData) */
void psy_audio_sampler_tick(psy_audio_Sampler* self, uintptr_t channel,
	const psy_audio_PatternEvent* pData)
{		
	psy_audio_PatternEvent data = *pData;
	psy_audio_Samples* samples;
	psy_List* ite;
	uintptr_t usevoice = UINTPTR_MAX;
	uintptr_t voice;
	bool doporta = FALSE;

	assert(self);	
	
	/*
	** machine work already does this
	** if (_mute) return; // Avoid new note entering when muted.
	*/

	if (data.note == psy_audio_NOTECOMMANDS_MIDICC) {
		/*
		** TODO: This has one problem, it requires a non-mcm command to trigger
		** the memory.
		*/
		psy_audio_PatternEvent* cmdmem;
		
		cmdmem = psy_audio_patternevent_clone(&data);
		cmdmem->inst = (uint16_t)channel;		
		psy_list_append(&self->multicmdMem, cmdmem);
		return;
	} else if (data.note > psy_audio_NOTECOMMANDS_RELEASE &&
			data.note != psy_audio_NOTECOMMANDS_EMPTY) {
		/* don't process twk , twf of Mcm Commands */
		return;
	}
	if (data.inst == psy_audio_NOTECOMMANDS_INST_EMPTY) {
		data.inst = self->lastinstrument[channel];
		if (data.inst == psy_audio_NOTECOMMANDS_INST_EMPTY) {
			return;  /* no previous sample. Skip */
		}
	} else {
		self->lastinstrument[channel] = data.inst;
	}
	samples = psy_audio_machine_samples(psy_audio_sampler_base(self));
	if (!psy_audio_samples_at(samples,
			psy_audio_sampleindex_make(data.inst, self->instrumentbank))) {
		return; /* if no wave, return. */
	}
	voice = psy_audio_sampler_getcurrentvoice(self, channel);
	if (data.cmd != PS1_SAMPLER_CMD_NONE) {
		/* Adding also the current command, to make the loops easier. */
		psy_audio_PatternEvent* cmdmem;

		cmdmem = psy_audio_patternevent_clone(&data);
		cmdmem->inst = (uint16_t)channel;
		psy_list_append(&self->multicmdMem, cmdmem);			
	}
	doporta = FALSE;
	for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ev->inst == channel) {
			if (ev->cmd == PS1_SAMPLER_CMD_PORTA2NOTE &&
					data.note < psy_audio_NOTECOMMANDS_RELEASE &&
					voice != psy_INDEX_INVALID) {
				if (self->linearslide) {
					// EnablePerformFx();
				}
				doporta = TRUE;
			} else if (ev->cmd == PS1_SAMPLER_CMD_PORTADOWN ||
					   ev->cmd == PS1_SAMPLER_CMD_PORTAUP) {
				if (self->linearslide) {
					/*	EnablePerformFx(); */
				}
			}
		}
	}
	usevoice = psy_INDEX_INVALID;
	if (data.note < psy_audio_NOTECOMMANDS_RELEASE && !doporta) {	/* Handle Note On. */
		usevoice = psy_audio_sampler_getfreevoice(self); /* Find a voice to apply the new note */
		if (voice != psy_INDEX_INVALID) { /* NoteOff previous Notes in this channel. */
			if (self->voices[voice].inst) {
				switch (self->voices[voice].inst->nna) {
				case 0:
					psy_audio_samplervoice_noteofffast(&self->voices[voice]);
					break;
				case 1:
					psy_audio_samplervoice_noteoff(&self->voices[voice]);
					break;
				default:
					break;
				}
			}
			if (usevoice == psy_INDEX_INVALID) {				
				usevoice = voice;
			}
		}
		if (usevoice == psy_INDEX_INVALID) {
			/*
			** No free voices. Assign first one.
			** This algorithm should be replace by a LRU lookup
			*/
			usevoice = 0;
		}
		self->voices[usevoice].channel = channel;
	} else {
		if (voice != psy_INDEX_INVALID) {
			if (data.note == psy_audio_NOTECOMMANDS_RELEASE) {
				psy_audio_samplervoice_noteoff(&self->voices[voice]);  //  Handle Note Off
			}
			usevoice = voice;
		}
		if (usevoice == psy_INDEX_INVALID) {
			return; /*
					** No playing note on this channel. Just go out.
					** Change it if you have channel commands.
					*/
		}
	}
	/*
	** If you want to make a command that controls more than one voice (the entire channel, for
	** example) you'll need to change this. Otherwise, add it to Voice.Tick().
	** Todo: mfc-psycle: Event Order newline; executeline; postnewline;
	** Sequencer can emit notes inside a line aswell. This may execute multicmds twice.
	*/
	psy_audio_samplervoice_tick(&self->voices[usevoice], &data, channel,
		self->defaultspeed ? psy_audio_NOTECOMMANDS_MIDDLEC : 48, self->multicmdMem);
}

// mfc-psycle: Voice::Tick(PatternEntry* pEntry,int channelNum, dsp::resampler& resampler, int baseC, std::vector<PatternEntry>&multicmdMem)
int psy_audio_samplervoice_tick(psy_audio_SamplerVoice* self, psy_audio_PatternEvent* pEntry,
	uintptr_t channelNum, int basec, psy_List* multicmdMem)
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
				self->controller._pan = psy_dsp_map_256_1(ev->parameter);
				//dopan = TRUE;
			} break;
			case PS1_SAMPLER_CMD_OFFSET: {
				psy_audio_Sample* sample;

				sample = psy_audio_samples_at(
					psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
					psy_audio_sampleindex_make(self->instrument, 0));
				w_offset = (uint64_t)(ev->parameter * sample->numframes) << 24;
				dooffset = TRUE;
			} break;
			case PS1_SAMPLER_CMD_VOLUME: {
				self->controller._vol = psy_dsp_map_256_1(ev->parameter);
				dovol = TRUE;
			} break;
			// }
			// Running {
			case PS1_SAMPLER_CMD_PORTAUP: {
				self->effval = ev->parameter;
				self->effcmd = ev->cmd;
			} break;
			case PS1_SAMPLER_CMD_PORTADOWN: {
				self->effval = ev->parameter;
				self->effcmd = ev->cmd;
			} break;
			case PS1_SAMPLER_CMD_PORTA2NOTE: {
				if (self->envelope._stage != SAMPLER_ENV_OFF) {
					self->effcmd = ev->cmd;
					self->effval = ev->parameter;
					if (pEntry->note < psy_audio_NOTECOMMANDS_RELEASE) {
						psy_audio_Sample* sample;
						double speeddouble;
						double finetune;

						sample = psy_audio_samples_at(
							psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
							psy_audio_sampleindex_make(self->instrument, 0));
						finetune = sample->numframes * 0.01;
						speeddouble = pow(2.0, (((double)pEntry->note +
							(double)psy_audio_sample_tune(sample) - (double)basec) + finetune) / 12.0) *
							((double)sample->numframes /
								psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler)));
						self->effportaspeed = (int64_t)(speeddouble * 4294967296.0f);
					}
					if (self->effportaspeed < self->controller._speed) {
						self->effval *= -1;
					}
					doporta = TRUE;
				}
			} break;
			case PS1_SAMPLER_CMD_RETRIG: {
				if ((ev->parameter & 0x0f) > 0)
				{
					int volmod;

					self->effretticks = (ev->parameter & 0x0f); // number of Ticks.
					self->effval = (int)((psy_audio_machine_currsamplesperrow(psy_audio_sampler_base(self->sampler))
						/ ((int)self->effretticks + 1)));

					volmod = (ev->parameter & 0xf0) >> 4; // Volume modifier.
					switch (volmod)
					{
					case 0:  //fallthrough
					case 8:	self->effretVol = 0; self->effretmode = 0; break;

					case 1:  //fallthrough
					case 2:  //fallthrough
					case 3:  //fallthrough
					case 4:  //fallthrough
					case 5: self->effretVol = (float)(pow(2., volmod - 1) / 64); self->effretmode = 1; break;

					case 6: self->effretVol = 0.66666666f;	 self->effretmode = 2; break;
					case 7: self->effretVol = 0.5f;			 self->effretmode = 2; break;

					case 9:  //fallthrough
					case 10:  //fallthrough
					case 11:  //fallthrough
					case 12:  //fallthrough
					case 13: self->effretVol = (float)(pow(2., volmod - 9) * (-1)) / 64; self->effretmode = 1; break;

					case 14: self->effretVol = 1.5f;					self->effretmode = 2; break;
					case 15: self->effretVol = 2.0f;					self->effretmode = 2; break;
					}
					self->triggernotedelay = self->effval;
				}
			} break;
				// }
			case PS1_SAMPLER_CMD_EXTENDED: {
				// delayed {
				if ((ev->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEOFF) {
					//This means there is always 6 ticks per row whatever number of rows.
					self->triggernoteoff = (int)((psy_audio_machine_currsamplesperrow(
						psy_audio_sampler_base(self->sampler)) / 6.f) * (ev->parameter & 0x0f));
				} else if ((ev->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEDELAY && (ev->parameter & 0x0f) != 0) {
					//This means there is always 6 ticks per row whatever number of rows.
					self->triggernotedelay = (int)((psy_audio_machine_currsamplesperrow(
						psy_audio_sampler_base(self->sampler)) / 6.f) * (ev->parameter & 0x0f));
				}
				//}
			} break;
			}
		}
	}
	if (pEntry->note < psy_audio_NOTECOMMANDS_RELEASE && !doporta)
	{
		psy_audio_Sample* sample;		
		double speeddouble;
		int16_t tune;

		if (!dooffset) { dooffset = TRUE; w_offset = 0; }

		if (self->inst) {
			psy_List* layer;

			layer = psy_audio_instrument_entriesintersect(self->inst, pEntry->note, 127, 0.0);
			if (layer) {
				psy_audio_InstrumentEntry* entry;
				psy_audio_Samples* samples;

				entry = (psy_audio_InstrumentEntry*)layer->entry;
				samples = psy_audio_machine_samples(psy_audio_sampler_base(self->sampler));
				sample = psy_audio_instrumententry_sample(entry, samples);
				self->controller.wave = sample;
				if (entry->use_loop) {
					self->controller.loop = entry->loop;
					self->controller._pos.HighPart = entry->loop.start;
					self->controller._pos.LowPart = 0;
					dooffset = FALSE; // already set										
				} else {
					self->controller.loop = sample->loop;					
				}
				tune = psy_audio_instrumententry_tune(entry, samples);
			} else {
				sample = psy_audio_samples_at(
					psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
					psy_audio_sampleindex_make(self->instrument, 0));
				self->controller.wave = sample;
				self->controller.loop = sample->loop;
				tune = psy_audio_sample_tune(sample);
			}
		} else {
			self->controller.wave = NULL;
			return FALSE;
		}
		
		psy_dsp_multiresampler_setquality(&self->controller.resampler,
			self->sampler->resamplerquality);
		if (self->inst->loop)
		{
			double const totalsamples = (double)(
				psy_audio_machine_currsamplesperrow(psy_audio_sampler_base(self->sampler))
				* self->inst->lines);
			speeddouble = (sample->numframes / totalsamples);
		} else
		{
			double finetune = psy_audio_sample_finetune(sample) * 0.01;
			speeddouble = pow(2.0f, (pEntry->note + tune - basec + finetune) / 12.0f) *
				(sample->samplerate / psy_audio_machine_samplerate(
					psy_audio_sampler_base(self->sampler)));
		}
		self->controller._speed = (int64_t)(speeddouble * 4294967296.0f);
		psy_dsp_resampler_setspeed(&self->controller.resampler.resampler, speeddouble);

		

		// Init Amplitude Envelope
		//		
		{
			int ENV_AT;
			self->envelope._sustain =
				psy_dsp_envelope_sustainvalue(&self->inst->volumeenvelope);
			ENV_AT = (int32_t)(psy_dsp_envelope_attacktime(
				&self->inst->volumeenvelope) * 44100 + 0.5f);
			self->envelope._step = (1.0f /
				ENV_AT) * self->envelope.sratefactor;
			self->envelope._value = 0.0f;
		}
		self->controller._lVolCurr = self->controller._lVolDest;
		self->controller._rVolCurr = self->controller._rVolDest;
		if (!dovol) { dovol = TRUE; self->controller._vol = 1.f; }
		if (!dopan) {
			if (self->inst->randompanning != 0.f) {
				dopan = TRUE;
				self->controller._pan = psy_dsp_map_32768_1(rand());
			} else {
				dopan = TRUE;
				self->controller._pan = self->controller.wave->panfactor;
			}
		}						

		//Init filter
		if (self->inst) {
			bool rcut;
			int32_t ENV_F_AT;

			rcut = self->inst->randomcutoff != 0.f;
			self->cutoff = (int)((rcut)
				? alteRand((int)(self->inst->filtercutoff * 127))
				: self->inst->filtercutoff * 127);
			// filter_setcutoff(&self->filter, self->cutoff);
			filter_settype(&self->filter, self->inst->filtertype);			
			filter_setressonance(&self->filter,
				(self->inst->randomresonance != 0.f)
					? alteRand((int32_t)(self->inst->filterres * 127))
					: (int32_t)(self->inst->filterres * 127));
			self->comodify = floor(self->inst->filtermodamount * 128.f);
			ENV_F_AT = (int32_t)(psy_dsp_envelope_attacktime(
				&self->inst->filterenvelope) * 44100 + 0.5f);
			if (ENV_F_AT <= 0) ENV_F_AT = 1;
			self->filterenv._sustain =
				psy_dsp_envelope_sustainvalue(&self->inst->filterenvelope);
			self->filterenv._step = (1.0f / ENV_F_AT) * self->envelope.sratefactor;
			self->filterenv._value = 0;
		}

		if (self->triggernotedelay == 0) {
			self->envelope._stage = SAMPLER_ENV_ATTACK;
			self->filterenv._stage = SAMPLER_ENV_ATTACK;
		} else {
			self->envelope._stage = SAMPLER_ENV_OFF;
			self->filterenv._stage = SAMPLER_ENV_OFF;
		}

		triggered = 1;
	}

	if (dovol || dopan) {
		// Panning calculation -------------------------------------------
		self->controller._rVolDest = self->controller._pan;
		self->controller._lVolDest = 1.f - self->controller._pan;
		//FT2 Style (Two slides) mode, but with max amp = 0.5.
		if (self->controller._rVolDest > 0.5f) { self->controller._rVolDest = 0.5f; }
		if (self->controller._lVolDest > 0.5f) { self->controller._lVolDest = 0.5f; }

		self->controller._lVolDest *= self->controller.wave->globalvolume * self->controller._vol;
		self->controller._rVolDest *= self->controller.wave->globalvolume * self->controller._vol;
	}
	if (dooffset) {
		self->controller._pos.QuadPart = w_offset;
	}

	if (triggered)
	{
		self->controller._lVolCurr = self->controller._lVolDest;
		self->controller._rVolCurr = self->controller._rVolDest;
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
			psy_audio_samplervoice_performfxold(&self->voices[voice]);
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
				psy_audio_samplervoice_performfxnew(&self->voices[voice]);
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
		psy_audio_samplervoice_work(&self->voices[voice],
			psy_audio_buffercontext_numsamples(bc),			
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
		event = psy_audio_patternentry_front(entry);
		if (event->cmd == PS1_SAMPLER_CMD_EXTENDED) {
			if ((event->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEOFF) {
				psy_audio_PatternEntry* noteoff;

				// This means there is always 6 ticks per row whatever number of rows.
				//triggernoteoff = (Global::player().SamplesPerRow()/6.f)*(ite->_parameter & 0x0f);
				noteoff = psy_audio_patternentry_allocinit();
				psy_audio_patternentry_front(noteoff)->note = psy_audio_NOTECOMMANDS_RELEASE;
				psy_audio_patternentry_front(noteoff)->mach = psy_audio_patternentry_front(entry)->mach;
				noteoff->delta += /*entry->offset*/ +(event->parameter & 0x0f) / 6.f *
					psy_audio_machine_currbeatsperline(
						psy_audio_sampler_base(self));
				psy_list_append(&insert, noteoff);
			} else
				if ((event->parameter & 0xF0) == PS1_SAMPLER_CMD_EXT_NOTEDELAY) {
					psy_audio_PatternEntry* newentry;
					psy_audio_PatternEvent* ev;
					int numticks;

					newentry = psy_audio_patternentry_clone(entry);
					ev = psy_audio_patternentry_front(newentry);
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
void psy_audio_samplervoice_work(psy_audio_SamplerVoice* self, uintptr_t numsamples, float* pSamplesL, float* pSamplesR)
{
	float left_output;
	float right_output;
	uintptr_t dstpos;

	assert(self);
	// If the sample has been deleted while playing...
	if (psy_audio_samplervoice_sample_deleted(self)) {
		psy_audio_samplervoice_setup(self);
		return;
	}

	self->samplecounter += numsamples;
	if (self->triggernotedelay > 0)
	{
		if (self->samplecounter >= self->triggernotedelay)
		{
			if (self->effretticks > 0)
			{
				int32_t ENV_AT;
				int32_t ENV_F_AT;
								
				self->effretticks--;
				self->triggernotedelay = (int)self->samplecounter + self->effval;
				ENV_AT = (int32_t)(psy_dsp_envelope_attacktime(
					&self->inst->volumeenvelope) * 44100 + 0.5f);
				if (ENV_AT <= 0) ENV_AT = 1;
				self->envelope._step = (1.0f / ENV_AT) * (float)self->envelope.sratefactor;
				ENV_F_AT = (int32_t)(psy_dsp_envelope_attacktime(
					&self->inst->filterenvelope) * 44100 + 0.5f);	
				if (ENV_F_AT <= 0) ENV_F_AT = 1;
				self->filterenv._step = (1.0f / ENV_F_AT) * (float)self->envelope.sratefactor;
				self->controller._pos.QuadPart = 0;
				if (self->effretmode == 1)
				{
					self->controller._lVolDest += self->effretVol;
					self->controller._rVolDest += self->effretVol;
				} else if (self->effretmode == 2)
				{
					self->controller._lVolDest *= self->effretVol;
					self->controller._rVolDest *= self->effretVol;
				}
			} else
			{
				self->triggernotedelay = 0;
			}			
			self->envelope._stage = SAMPLER_ENV_ATTACK;
		} else if (self->envelope._stage == SAMPLER_ENV_OFF)
		{
			return;
		}
	} else if (self->envelope._stage == SAMPLER_ENV_OFF)
	{
		psy_audio_samplervoice_setup(self);
		return;
	} else if ((self->triggernoteoff) && (self->samplecounter >= self->triggernoteoff))
	{
		psy_audio_samplervoice_noteoff(self);
	}

	dstpos = 0;
	while (numsamples)
	{
		left_output = 0;
		right_output = 0;

		samplerwavedatacontroller_work(&self->controller, &left_output, &right_output);
		
		// Amplitude section			
		{
			int ENV_DT;
			
			ENV_DT = (int32_t)(psy_dsp_envelope_decaytime(
				&self->inst->volumeenvelope) * 44100 + 0.5f);
			if (ENV_DT <= 0) ENV_DT = 1;
			samplerenvelope_tick(&self->envelope, ENV_DT);		
			samplerwavedatacontroller_rampvolume(&self->controller);
		}
		// Filter section
		if (filter_type(&self->filter) != F_NONE) {
			int newcutoff;
			int ENV_F_DT;

			ENV_F_DT = (int32_t)(psy_dsp_envelope_decaytime(
				&self->inst->filterenvelope) * 44100 + 0.5f);
			samplerenvelope_tick(&self->filterenv, ENV_F_DT);	
			if (ENV_F_DT <= 0) ENV_F_DT = 1;
			newcutoff = self->cutoff +
				psy_dsp_roundf(self->filterenv._value * self->comodify * 128.f);
			if (newcutoff < 0) {
				newcutoff = 0;
			} else if (newcutoff > 127) {
				newcutoff = 127;
			}
			filter_setcutoff(&self->filter, newcutoff);
			if (self->controller.wave->stereo) {
				self->filter.vtable->workstereo(&self->filter, &left_output, &right_output);
			} else {
				left_output = self->filter.vtable->work(&self->filter, left_output);
			}
		}

		if (!self->controller.wave->stereo) {
			right_output = left_output;
		}		
		right_output *= self->controller._rVolCurr * self->envelope._value;
		left_output *= self->controller._lVolCurr * self->envelope._value;

		// Move sample position
		if (!samplerwavedatacontroller_postwork(&self->controller)) {
			psy_audio_samplervoice_setup(self);
			break;
		}

		*pSamplesL++ = *pSamplesL + left_output;
		*pSamplesR++ = *pSamplesR + right_output;
		numsamples--;
	}	
}

// mfc-psycle: if check in work method
// If the sample has been deleted while playing...
bool psy_audio_samplervoice_sample_deleted(psy_audio_SamplerVoice* self)
{		
	return !psy_audio_samples_at(
		psy_audio_machine_samples(psy_audio_sampler_base(self->sampler)),
		psy_audio_sampleindex_make(self->instrument, 0));
}

// mfc-psycle: Voice::NoteOff()
void psy_audio_samplervoice_noteoff(psy_audio_SamplerVoice* self)
{
	assert(self);
	if (self->envelope._stage != SAMPLER_ENV_OFF)
	{
		int ENV_RT;
		int ENV_F_RT;

		ENV_RT = (int32_t)((psy_dsp_envelope_releasetime(
			&self->inst->volumeenvelope)) * 44100 + 0.5f);
		ENV_F_RT = (int32_t)((psy_dsp_envelope_releasetime(
			&self->inst->filterenvelope)) * 44100 + 0.5f);
		self->envelope._stage = SAMPLER_ENV_RELEASE;
		self->filterenv._stage = SAMPLER_ENV_RELEASE;
		self->envelope._step = (self->envelope._value / ENV_RT) * self->envelope.sratefactor;
		self->filterenv._step = (self->filterenv._value / ENV_F_RT) * self->envelope.sratefactor;
	}
	self->triggernotedelay = 0;
	self->triggernoteoff = 0;
}

// mfc-psycle: Voice::NoteOffFast()
void psy_audio_samplervoice_noteofffast(psy_audio_SamplerVoice* self)
{
	assert(self);
	if (self->envelope._stage != SAMPLER_ENV_OFF)
	{
		self->envelope._stage = SAMPLER_ENV_FASTRELEASE;
		self->envelope._step = self->envelope._value / OVERLAPTIME;

		self->filterenv._stage = SAMPLER_ENV_RELEASE;
		self->filterenv._step = self->filterenv._value / OVERLAPTIME;
	}
	self->triggernotedelay = 0;
	self->triggernoteoff = 0;
}

// mfc-psycle: Voice::PerformFxOld(dsp::resampler& resampler)
void psy_audio_samplervoice_performfxold(psy_audio_SamplerVoice* self)
{
	// 4294967 stands for (2^30/250), meaning that
	//value 250 = (inc)decreases the speed in 1/4th of the original (wave) speed each PerformFx call.
	int64_t shift;	

	assert(self);
	switch (self->effcmd)
	{
		// 0x01 : Pitch Up
	case PS1_SAMPLER_CMD_PORTAUP:
		shift = (int64_t)(self->effval) * 4294967ll * (double)(self->controller.wave->numframes) /
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler));
		self->controller._speed += shift;
		psy_dsp_resampler_setspeed(&self->controller.resampler.resampler,
			self->controller._speed);
		break;
		// 0x02 : Pitch Down
	case PS1_SAMPLER_CMD_PORTADOWN:
		shift = (int64_t)(self->effval) * 4294967ll * (double)(self->controller.wave->numframes) /
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler));
		self->controller._speed -= shift;
		if (self->controller._speed < 0) self->controller._speed = 0;
		psy_dsp_resampler_setspeed(&self->controller.resampler.resampler,
			self->controller._speed);
		break;
		// 0x03 : Porta to note
	case PS1_SAMPLER_CMD_PORTA2NOTE:
		//effval is multiplied by -1 in Tick if it needs to slide down.
		shift = (int64_t)(self->effval) * 4294967ll * (double)(self->controller.wave->numframes) /
			psy_audio_machine_samplerate(psy_audio_sampler_base(self->sampler));
		self->controller._speed += shift;
		if ((self->effval < 0 && self->controller._speed < self->effportaspeed)
			|| (self->effval > 0 && self->controller._speed > self->effportaspeed)) {
			self->controller._speed = self->effportaspeed;
			self->effcmd = PS1_SAMPLER_CMD_NONE;
		}
		psy_dsp_resampler_setspeed(&self->controller.resampler.resampler,
			self->controller._speed);
		//psy_audio_wavedatacontroller_setspeed(&self->controller, speed / 4294967296.0);
		break;
	default:
		break;
	}
}

// mfc-psycle: Voice::PerformFxNew(dsp::resampler& resampler)
void psy_audio_samplervoice_performfxnew(psy_audio_SamplerVoice* self)
{	
	double factor;	

	assert(self);
	// value 1 = (inc)decreases the speed in one seminote each beat.
	factor = 1.0 / (12.0 * psy_audio_machine_ticksperbeat(
		psy_audio_sampler_base(self->sampler)));
	switch (self->effcmd) {
		// 0x01 : Pitch Up
		case PS1_SAMPLER_CMD_PORTAUP:
			self->controller._speed *= pow(2.0, self->effval * factor);
			psy_dsp_resampler_setspeed(&self->controller.resampler.resampler,
				self->controller._speed);
			break;
			// 0x02 : Pitch Down
		case PS1_SAMPLER_CMD_PORTADOWN:
			self->controller._speed *= pow(2.0, -self->effval * factor);
			if (self->controller._speed < 0) self->controller._speed = 0;
			psy_dsp_resampler_setspeed(&self->controller.resampler.resampler,
				self->controller._speed);
			break;
			// 0x03 : Porta to note
		case PS1_SAMPLER_CMD_PORTA2NOTE:
			//effval is multiplied by -1 in Tick() if it needs to slide down.
			self->controller._speed *= pow(2.0, self->effval * factor);
			if ((self->effval < 0 && self->controller._speed < self->effportaspeed)
				|| (self->effval > 0 && self->controller._speed > self->effportaspeed)) {
				self->controller._speed = self->effportaspeed;
				self->effcmd = PS1_SAMPLER_CMD_NONE;
			}			
			psy_dsp_resampler_setspeed(&self->controller.resampler.resampler,
				self->controller._speed);
			break;
		default:
			break;
	}
}

// mfc-psycle: Sampler::ChangeResamplerQuality(helpers::dsp::resampler::quality::type quality)
void psy_audio_sampler_changeresamplerquality(psy_audio_Sampler* self,
	psy_dsp_ResamplerQuality quality)
{
	assert(self);

	self->resamplerquality = quality;	
}

// mfc-psycle: Sampler::LoadSpecificChunk(RiffFile* pFile, int version)
int psy_audio_sampler_loadspecificchunk(psy_audio_Sampler* self,
	psy_audio_SongFile* songfile, uintptr_t machslot)
{
	uint32_t size;
	psy_dsp_ResamplerQuality resamplerquality;

	assert(self);

	// mfc-psycle has no instrument banks. If a song comes from mfc-psycle the
	// sampler uses bank 0 (sampulse bank 1) else the saved one is used.
	// (see internalversion 3 loading)
	self->instrumentbank = 0;
	psy_audio_sampler_defaultC4(self, FALSE);
	self->linearslide = FALSE;
	size = 0;
	psyfile_read(songfile->file, &size, sizeof(size));
	if (size) {
		/// Version 0
		int32_t temp;

		// numvoices
		psyfile_read(songfile->file, &temp, sizeof(temp));
		self->numvoices = temp;
		// quality
		psyfile_read(songfile->file, &temp, sizeof(temp));
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
		psy_audio_sampler_changeresamplerquality(self, resamplerquality);		
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
				// use instrument bank (not available in mfc-psycle)
				psyfile_read(songfile->file, &instrbank, sizeof(instrbank));
				self->instrumentbank = instrbank;
			}
		}
	}
	return PSY_OK;
}

// mfc-psycle: Sampler::SaveSpecificChunk(RiffFile* pFile) 
int psy_audio_sampler_savespecificchunk(psy_audio_Sampler* self,
	psy_audio_SongFile* songfile, uintptr_t macslot)
{
	uint32_t temp;
	uint32_t size;
	
	assert(self);
	size = 3 * sizeof(temp) + 2 * sizeof(bool);
	psyfile_write(songfile->file, &size, sizeof(size));
	temp = (uint32_t)self->numvoices;
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
	return PSY_OK;
}

/*
** mfc-psycle: bool Sampler::Load(RiffFile* pFile)
**			   part of psy2.c
*/

void psy_audio_sampler_initparameters(psy_audio_Sampler* self)
{
	psy_dsp_ResamplerQuality quality;

	assert(self);
	psy_audio_intmachineparam_init(&self->param_numvoices,
		"Polyphony", "Polyphony", MPF_STATE,
		(int32_t*)&self->numvoices, 1, PS1_SAMPLER_MAX_POLYPHONY);
	psy_audio_choicemachineparam_init(&self->param_resamplingmethod,
		"Resampling Method", "Resampling Method", MPF_STATE,
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
		"Default speed played by", "Default Speed", MPF_STATE,
		(int32_t*)&self->defaultspeed,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 0,
		"C3");
	psy_audio_choicemachineparam_setdescription(&self->param_defaultspeed, 1,
		"C4");
	psy_audio_choicemachineparam_init(&self->param_slidemode,
		"Slide mode", "Slide mode", MPF_STATE,
		(int32_t*)&self->linearslide,
		0, 1);
	psy_audio_choicemachineparam_setdescription(&self->param_slidemode, 0,
		"old buggy slide");
	psy_audio_choicemachineparam_setdescription(&self->param_slidemode, 1,
		"Linear slide mode");
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
	psy_audio_choicemachineparam_dispose(&self->param_slidemode);
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
}

uintptr_t psy_audio_sampler_numparameters(psy_audio_Sampler* self)
{
	assert(self);

	return 5;
}

uintptr_t psy_audio_sampler_numparametercols(psy_audio_Sampler* self)
{
	assert(self);

	return 1;
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
			return psy_audio_choicemachineparam_base(&self->param_slidemode);
		case 4:
			return psy_audio_intmachineparam_base(&self->param_instrumentbank);
			break;
		
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

	psy_audio_sampler_changeresamplerquality(self,
		psy_audio_choicemachineparam_choice(sender));		
}

void psy_audio_sampler_defaultC4(psy_audio_Sampler* self, bool correct)
{
	assert(self);
	self->defaultspeed = correct;
}

bool psy_audio_sampler_isdefaultC4(const psy_audio_Sampler* self)
{
	assert(self);
	return self->defaultspeed;
}
