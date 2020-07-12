// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "ps1.h"
// audio
#include "constants.h"
#include "instruments.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "songio.h"
#include "samples.h"
// dsp
#include <operations.h>
#include <valuemapper.h>
// std
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// Voice
static void ps1samplervoice_rampvolume(PS1SamplerVoice* self);
// implementation
void ps1samplervoice_init(PS1SamplerVoice* self, psy_audio_SamplerPS1* sampler)
{
	self->sampler = sampler;
	self->inst = NULL;
	self->_instrument = 0xFF;
	self->_channel = -1;
	self->_sampleCounter = 0;
	self->_cutoff = 0;
	self->effCmd = PS1_SAMPLER_CMD_NONE;	
	filter_init(&self->_filter);
	psy_audio_sampleiterator_init(&self->controller, NULL, RESAMPLERTYPE_LINEAR);
	ps1samplervoice_setup(self);
	// WaveDataController
	self->_vol = 0.f;
	self->_lVolDest = 0;
	self->_rVolDest = 0;
	self->_lVolCurr = 0;
	self->_rVolCurr = 0;
}

void ps1samplervoice_setup(PS1SamplerVoice* self)
{
	adsr_settings_initdefault(&self->_envelopesettings);
	psy_dsp_adsr_init(&self->_envelope, &self->_envelopesettings, 44100);	
	adsr_settings_initdefault(&self->_filterenvsettings);
	psy_dsp_adsr_init(&self->_filterEnv, &self->_filterenvsettings, 44100);	
	self->_channel = -1;
	self->_triggerNoteOff = 0;
	self->_triggerNoteDelay = 0;
	self->effretTicks = 0;
	self->_effPortaSpeed = 4294967296.0f;
}

void ps1samplervoice_dispose(PS1SamplerVoice* self)
{
	psy_dsp_adsr_dispose(&self->_envelope);
	psy_dsp_adsr_dispose(&self->_filterEnv);
	psy_audio_sampleiterator_dispose(&self->controller);
	filter_dispose(&self->_filter);
}

void ps1samplervoice_newline(PS1SamplerVoice* self)
{
	self->effretTicks = 0;
	self->effCmd = SAMPLER_CMD_NONE;
	if (self->_triggerNoteOff > self->_sampleCounter) self->_triggerNoteOff -= self->_sampleCounter;
	else self->_triggerNoteOff = 0;
	if (self->_triggerNoteDelay > self->_sampleCounter) self->_triggerNoteDelay -= self->_sampleCounter;
	else self->_triggerNoteDelay = 0;
	self->_sampleCounter = 0;
}

void ps1samplervoice_noteoff(PS1SamplerVoice* self)
{
	if (self->_envelope.stage != ENV_OFF)
	{
		psy_dsp_adsr_release(&self->_envelope);
		psy_dsp_adsr_release(&self->_filterEnv);		
		//_envelope._step = (_envelope._value / inst->ENV_RT) * _envelope.sratefactor;
		//_filterEnv._step = (_filterEnv._value / inst->ENV_F_RT) * _envelope.sratefactor;
	}
	self->_triggerNoteDelay = 0;
	self->_triggerNoteOff = 0;
}

void ps1samplervoice_noteofffast(PS1SamplerVoice* self)
{
	if (self->_envelope.stage != ENV_OFF)
	{
		psy_dsp_adsr_fastrelease(&self->_envelope);
		psy_dsp_adsr_fastrelease(&self->_filterEnv);
		//_envelope._step = (_envelope._value / inst->ENV_RT) * _envelope.sratefactor;
		//_filterEnv._step = (_filterEnv._value / inst->ENV_F_RT) * _envelope.sratefactor;
	}
	self->_triggerNoteDelay = 0;
	self->_triggerNoteOff = 0;
}

void ps1samplervoice_work(PS1SamplerVoice* self, int numsamples, float* pSamplesL, float* pSamplesR)
{
	float left_output;
	float right_output;
	uintptr_t dstpos;

	// If the sample has been deleted while playing...
	if (!psy_audio_samples_at(
		psy_audio_machine_samples(psy_audio_samplerps1_base(self->sampler)),
			sampleindex_make(self->_instrument, 0))) {
			ps1samplervoice_setup(self);
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
		ps1samplervoice_setup(self);
		return;
	} else if ((self->_triggerNoteOff) && (self->_sampleCounter >= self->_triggerNoteOff))
	{
		ps1samplervoice_noteoff(self);
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
			// Amplitude section
			{
				psy_dsp_adsr_tick(&self->_envelope);
				ps1samplervoice_rampvolume(self);
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
			ps1samplervoice_setup(self);
			break;
		}
	}
}

void ps1samplervoice_rampvolume(PS1SamplerVoice* self)
{
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

int ps1samplervoice_tick(PS1SamplerVoice* self, psy_audio_PatternEvent* pEntry, int channelNum, int baseC,
	psy_List* multicmdMem)
{
	int triggered = 0;
	uint64_t w_offset = 0;
	bool dooffset = FALSE;
	bool dovol = FALSE;
	bool dopan = FALSE;
	bool doporta = FALSE;
	psy_List* ite;
	psy_audio_Instruments* insts;

	//If this sample is not enabled, Voice::Tick is not called. Also, Sampler::Tick takes care of previus instrument used.
	self->_instrument = pEntry->inst;
	insts = psy_audio_machine_instruments(psy_audio_samplerps1_base(self->sampler));
	self->inst = instruments_at(insts,
		instrumentindex_make(self->sampler->instrumentbank, self->_instrument));
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
					psy_audio_machine_samples(psy_audio_samplerps1_base(self->sampler)),
					sampleindex_make(self->_instrument, 0));
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

						sample = psy_audio_samples_at(
							psy_audio_machine_samples(psy_audio_samplerps1_base(self->sampler)),
							sampleindex_make(self->_instrument, 0));
						float const finetune = (float)sample->numframes * 0.01f;
						double speeddouble = pow(2.0f, (pEntry->note + sample->tune - baseC + finetune) / 12.0f) * ((float)sample->numframes / 44100); // Global::player().SampleRate()
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
					self->effVal = (psy_audio_machine_currsamplesperrow(psy_audio_samplerps1_base(self->sampler)) / (self->effretTicks + 1));

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
						psy_audio_samplerps1_base(self->sampler)) / 6.f) * (ev->parameter & 0x0f);
				} else if ((ev->parameter & 0xf0) == PS1_SAMPLER_CMD_EXT_NOTEDELAY && (ev->parameter & 0x0f) != 0) {
					//This means there is always 6 ticks per row whatever number of rows.
					self->_triggerNoteDelay = (psy_audio_machine_currsamplesperrow(
						psy_audio_samplerps1_base(self->sampler)) / 6.f) * (ev->parameter & 0x0f);
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
			psy_audio_machine_samples(psy_audio_samplerps1_base(self->sampler)),
			sampleindex_make(self->_instrument, 0));
		psy_audio_sampleiterator_setsample(&self->controller, sample);				
		if (self->inst->loop)
		{
			double const totalsamples = (double)(
				psy_audio_machine_currsamplesperrow(psy_audio_samplerps1_base(self->sampler))
				 * self->inst->lines);
			speeddouble = (sample->numframes / totalsamples);
		} else
		{
			float const finetune = (float)sample->finetune * 0.01f;
			speeddouble = pow(2.0f, (pEntry->note + sample->tune - baseC + finetune) / 12.0f) *
				((float)sample->samplerate / 44100); // Global::player().SampleRate()
		}
		psy_audio_sampleiterator_setspeed(&self->controller, speeddouble);
		psy_audio_sampleiterator_play(&self->controller);		

		if (!dooffset) { dooffset = TRUE; w_offset = 0; }

		// Init Amplitude Envelope
		//
		psy_dsp_adsr_init(&self->_envelope, &self->_envelopesettings, 44100);		
				
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
		psy_dsp_adsr_init(&self->_envelope, &self->_filterenvsettings, 44100);

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

void ps1samplervoice_performfxold(PS1SamplerVoice* self)
{
	// 4294967 stands for (2^30/250), meaning that
	//value 250 = (inc)decreases the speed in 1/4th of the original (wave) speed each PerformFx call.
	int64_t shift;
	int64_t speed;
	switch (self->effCmd)
	{
		// 0x01 : Pitch Up
	case PS1_SAMPLER_CMD_PORTAUP:
		shift = (int64_t)(self->effVal) * 4294967ll * (float)(self->controller.sample->numframes) / 44100; // Global::player().SampleRate();
		psy_audio_sampleiterator_setspeed(&self->controller, (self->controller.speed + shift) / 4294967296.0);
		break;
		// 0x02 : Pitch Down
	case PS1_SAMPLER_CMD_PORTADOWN:
		shift = (int64_t)(self->effVal) * 4294967ll * (float)(self->controller.sample->numframes) / 44100; // Global::player().SampleRate();
		speed = self->controller.speed - shift;
		if (speed < 0) speed = 0;
		psy_audio_sampleiterator_setspeed(&self->controller, speed / 4294967296.0);
		break;	
		// 0x03 : Porta to note
	case PS1_SAMPLER_CMD_PORTA2NOTE:
		//effVal is multiplied by -1 in Tick if it needs to slide down.
		shift = (int64_t)(self->effVal) * 4294967ll * (float)(self->controller.sample->numframes) / 44100; // Global::player().SampleRate();
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

void ps1samplervoice_performfxnew(PS1SamplerVoice* self)
{
	//value 1 = (inc)decreases the speed in one seminote each beat.
	double factor = 1.0 / (12.0 * psy_audio_machine_ticksperbeat(
		psy_audio_samplerps1_base(&self->sampler)));
	int64_t speed;
	switch (self->effCmd)
	{
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
		case SAMPLER_CMD_PORTA2NOTE:
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

// Sampler

static char* _psName;

static void generateaudio(psy_audio_SamplerPS1*, psy_audio_BufferContext*);
static void seqtick(psy_audio_SamplerPS1*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void newline(psy_audio_SamplerPS1*);
static psy_List* sequencerinsert(psy_audio_SamplerPS1*, psy_List* events);
static void psy_audio_samplerps1_stop(psy_audio_SamplerPS1*);
static const psy_audio_MachineInfo* info(psy_audio_SamplerPS1*);
static uintptr_t numparametercols(psy_audio_SamplerPS1*);
static uintptr_t numparameters(psy_audio_SamplerPS1*);
static psy_audio_MachineParam* parameter(psy_audio_SamplerPS1*,
	uintptr_t param);
static void dispose(psy_audio_SamplerPS1*);
static void disposeparameters(psy_audio_SamplerPS1*);
static int alloc_voice(psy_audio_SamplerPS1*);
static uintptr_t numinputs(psy_audio_SamplerPS1*);
static uintptr_t numoutputs(psy_audio_SamplerPS1*);
static void loadspecific(psy_audio_SamplerPS1*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_SamplerPS1*, psy_audio_SongFile*,
	uintptr_t slot);
static void psy_audio_samplerps1_enableperformfx(psy_audio_SamplerPS1*);
static void psy_audio_samplerps1_stopinstrument(psy_audio_SamplerPS1*,
	int insIdx);
static void psy_audio_samplerps1_setsamplerate(psy_audio_SamplerPS1*, int sr);
static bool psy_audio_samplerps1_playstrack(psy_audio_SamplerPS1*, int track);
static int psy_audio_samplerps1_getcurrentvoice(psy_audio_SamplerPS1*, int track);
static void postnewline(psy_audio_SamplerPS1*);

static psy_audio_MachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	GENERATOR | 32 | 64 | MACH_SUPPORTS_INSTRUMENTS,
	MACHMODE_GENERATOR,
	"Sampler PS1"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Sampler PS1",
	"Psycledelics",
	"help",	
	MACH_SAMPLER,
	NULL,		// NO MODULPATH
	0,			// shellidx	
	SAMPLERHELP	// help text
};

const psy_audio_MachineInfo* psy_audio_samplerps1_info(void)
{
	return &macinfo;
}

static void psy_audio_samplerps1_initparameters(psy_audio_SamplerPS1*);
static void resamplingmethod_tweak(psy_audio_SamplerPS1*,
	psy_audio_ChoiceMachineParam* sender, float value);
static void psy_audio_samplerps1_ontimertick(psy_audio_SamplerPS1*);
static void psy_audio_samplerps1_ontimerwork(psy_audio_SamplerPS1*,
	psy_audio_BufferContext*);
static int getfreevoice(psy_audio_SamplerPS1*);

static MachineVtable sampler_vtable;
static int sampler_vtable_initialized = 0;

static void sampler_vtable_init(psy_audio_SamplerPS1* self)
{
	if (!sampler_vtable_initialized) {
		sampler_vtable = *(psy_audio_samplerps1_base(self)->vtable);
		sampler_vtable.generateaudio = (fp_machine_generateaudio)generateaudio;
		sampler_vtable.seqtick = (fp_machine_seqtick)seqtick;
		sampler_vtable.newline = (fp_machine_newline)newline;
		sampler_vtable.sequencerinsert = (fp_machine_sequencerinsert)
			sequencerinsert;
		sampler_vtable.stop = (fp_machine_stop)psy_audio_samplerps1_stop;
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

void psy_audio_samplerps1_init(psy_audio_SamplerPS1* self,
	psy_audio_MachineCallback callback)
{
	int i;

	self->linearslide = FALSE; // TRUE;
	self->resamplerquality = RESAMPLERTYPE_SPLINE;
	for (i = 0; i < MAX_TRACKS; i++) {
		self->lastInstrument[i] = 255;
	}
	self->multicmdMem = NULL;
	self->_numVoices = SAMPLER_DEFAULT_POLYPHONY;
	for (int i = 0; i < self->_numVoices; i++) {
		ps1samplervoice_init(&self->_voices[i], self);		
	}
	// SetSampleRate(Global::player().SampleRate());

	custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_samplerps1_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_samplerps1_base(self), "Sampler");
	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;
	self->voices = 0;
	self->resamplerquality = RESAMPLERTYPE_LINEAR;
	self->defaultspeed = 1;
	self->instrumentbank = 0;
	self->samplerowcounter = 0;
	self->usefilters = TRUE;
	self->panningmode = psy_audio_PANNING_LINEAR;
	self->samplerowcounter = 0;
	psy_audio_samplerps1_initparameters(self);	
	self->instrumentbank = 0;	
	psy_audio_samplerticktimer_init(&self->ticktimer,
		self,	// callback context (sampler)
		psy_audio_samplerps1_ontimertick,
		psy_audio_samplerps1_ontimerwork);
}

void psy_audio_samplerps1_stop(psy_audio_SamplerPS1* self)
{
	for (int i = 0; i < self->_numVoices; i++) {
		ps1samplervoice_noteofffast(&self->_voices[i]);
		self->_voices[i]._effPortaSpeed = 4294967296.0f;
	}	
}

void psy_audio_samplerps1_stopinstrument(psy_audio_SamplerPS1* self,
	int insIdx)
{
	for (int i = 0; i < self->_numVoices; i++) {
		PS1SamplerVoice* pVoice = &self->_voices[i];
		if (pVoice->_instrument == insIdx &&
			(pVoice->_envelope.stage != ENV_OFF ||
				pVoice->_triggerNoteDelay > 0)) {
			ps1samplervoice_setup(pVoice);
		}
	}
}

void psy_audio_samplerps1_setsamplerate(psy_audio_SamplerPS1* self, int sr)
{
	//Machine::SetSampleRate(sr);
	for (int i = 0; i < self->_numVoices; i++) {
		filter_setsamplerate(&self->_voices[i]._filter, sr);
		psy_dsp_adsr_setsamplerate(&self->_voices[i]._envelope, sr);
		psy_dsp_adsr_setsamplerate(&self->_voices[i]._filterEnv, sr);
	}
}

bool psy_audio_samplerps1_playstrack(psy_audio_SamplerPS1* self, int track)
{
	// return (self->TriggerDelayCounter[track] > 0 || GetCurrentVoice(track) != -1);
	return FALSE;
}

int psy_audio_samplerps1_getcurrentvoice(psy_audio_SamplerPS1* self, int track)
{
	for (int voice = 0; voice < self->_numVoices; voice++) {
		// ENV_OFF is not checked, because channel will be -1
		if (self->_voices[voice]._channel == track &&
			(self->_voices[voice]._triggerNoteDelay > 0 ||
			 self->_voices[voice]._envelope.stage != ENV_FASTRELEASE)) {
			return voice;
		}
	}
	return -1;
}

void newline(psy_audio_SamplerPS1* self)
{
	for (int voice = 0; voice < self->_numVoices; voice++)
	{
		ps1samplervoice_newline(&self->_voices[voice]);
	}
	{
		self->samplerowcounter = 0;		
		psy_audio_samplerticktimer_reset(&self->ticktimer,
			(uintptr_t)psy_audio_machine_samplespertick(
				psy_audio_samplerps1_base(self)));
	}
}

void postnewline(psy_audio_SamplerPS1* self)
{
	psy_list_deallocate(&self->multicmdMem, (psy_fp_disposefunc)NULL);
}

int getfreevoice(psy_audio_SamplerPS1* self)
{
	int useVoice = -1;
	for (int idx = 0; idx < self->_numVoices; idx++)	// Find a voice to apply the new note
	{
		switch (self->_voices[idx]._envelope.stage)
		{
		case ENV_OFF:
			if (self->_voices[idx]._triggerNoteDelay == 0)
			{
				useVoice = idx;
				idx = self->_numVoices; // Ok, we can go out from the loop already.
			}
			break;
		case ENV_FASTRELEASE:
			useVoice = idx;
			break;
		case ENV_RELEASE:
			if (useVoice == -1) useVoice = idx;
			break;
		default:break;
		}
	}
	return useVoice;
}

void sampler_tick(psy_audio_SamplerPS1* self, int channel, psy_audio_PatternEvent* pData)
{
	// if (self->_mute) return; // Avoid new note entering when muted.

	psy_audio_PatternEvent data = *pData;
	psy_audio_PatternEvent* copy = NULL;
	psy_List* ite;

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

	if (data.inst == 255)
	{
		data.inst = self->lastInstrument[channel];
		if (data.inst == 255) {
			return;  // no previous sample. Skip
		}
	} else { self->lastInstrument[channel] = data.inst; }

	// if (!Global::song().samples.IsEnabled(data._inst)) return; // if no wave, return.

	int idx = psy_audio_samplerps1_getcurrentvoice(self, channel);
	if (data.cmd != PS1_SAMPLER_CMD_NONE) {
		// Adding also the current command, to make the loops easier.
		psy_audio_PatternEvent data2;
		data2 = data;
		data2.inst = channel;
		copy = malloc(sizeof(psy_audio_PatternEvent));
		*copy = data2;
		psy_list_append(&self->multicmdMem, copy);
	}
	bool doporta = FALSE;
	for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ev->inst == channel) {
			if (ev->cmd == PS1_SAMPLER_CMD_PORTA2NOTE && data.note < NOTECOMMANDS_RELEASE && idx != -1) {
				if (self->linearslide) { // isLinearSlide()) {
					// EnablePerformFx();
				}
				doporta = TRUE;
			} else if (ev->cmd == PS1_SAMPLER_CMD_PORTADOWN || ev->cmd == PS1_SAMPLER_CMD_PORTAUP) {
				if (self->linearslide) { //isLinearSlide()) {
					//EnablePerformFx();
				}
			}
		}
	}

	int useVoice = -1;
	if (data.note < NOTECOMMANDS_RELEASE && !doporta)	// Handle Note On.
	{
		useVoice = getfreevoice(self); // Find a voice to apply the new note
		if (idx != -1) // NoteOff previous Notes in this channel.
		{
			switch (self->_voices[idx].inst->nna)
			{
			case 0:
				ps1samplervoice_noteofffast(&self->_voices[idx]);
				// self->_voices[idx].NoteOffFast();
				break;
			case 1:
				ps1samplervoice_noteoff(&self->_voices[idx]);
				//_voices[idx].NoteOff();
				break;
			}
			if (useVoice == -1) { useVoice = idx; }
		}
		if (useVoice == -1)	// No free voices. Assign first one.
		{						// This algorithm should be replace by a LRU lookup
			useVoice = 0;
		}
		self->_voices[useVoice]._channel = channel;
	} else {
		if (idx != -1)
		{
			if (data.note == NOTECOMMANDS_RELEASE) ps1samplervoice_noteoff(&self->_voices[idx]);  //  Handle Note Off
			useVoice = idx;
		}
		if (useVoice == -1) return;   // No playing note on this channel. Just go out.
										// Change it if you have channel commands.
	}
	// If you want to make a command that controls more than one voice (the entire channel, for
	// example) you'll need to change this. Otherwise, add it to Voice.Tick().
	ps1samplervoice_tick(&self->_voices[useVoice], &data, channel,
		self->defaultspeed ? NOTECOMMANDS_MIDDLEC : 48, self->multicmdMem);
}

void enableperformfx(psy_audio_SamplerPS1* self)
{
	for (int i = 0; i < MAX_TRACKS; i++) // Global::song().SONGTRACKS
	{
		//if (self->TriggerDelay[i]._cmd == 0)
		{
			//self->TriggerDelayCounter[i] = 0;
			//self->TriggerDelay[i]._cmd = 0xEF;
			//self->TriggerDelay[i]._parameter = Global::song().TicksPerBeat() / Global::song().LinesPerBeat();
			break;
		}
	}
}

void psy_audio_samplerps1_initparameters(psy_audio_SamplerPS1* self)
{
	uintptr_t i;

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
	psy_audio_intmachineparam_init(&self->param_instrumentbank,
		"instr. bank", "instr. bank", MPF_STATE | MPF_SMALL,
		(int32_t*)&self->instrumentbank,
		0, 1);	
}

void dispose(psy_audio_SamplerPS1* self)
{		
	disposeparameters(self);	
	custommachine_dispose(&self->custommachine);		
}

void disposeparameters(psy_audio_SamplerPS1* self)
{
	psy_audio_intmachineparam_dispose(&self->param_numvoices);
	psy_audio_choicemachineparam_dispose(&self->param_resamplingmethod);
	psy_audio_choicemachineparam_dispose(&self->param_defaultspeed);
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
}

psy_audio_SamplerPS1* psy_audio_samplerps1_alloc(void)
{
	return (psy_audio_SamplerPS1*)malloc(sizeof(psy_audio_SamplerPS1));
}

psy_audio_SamplerPS1* psy_audio_samplerps1_allocinit(psy_audio_MachineCallback callback)
{
	psy_audio_SamplerPS1* rv;

	rv = psy_audio_samplerps1_alloc();
	if (rv) {
		psy_audio_samplerps1_init(rv, callback);
	}
	return rv;
}

void generateaudio(psy_audio_SamplerPS1* self, psy_audio_BufferContext* bc)
{	
	//if (!self->linearslide) {
		for (int voice = 0; voice < self->_numVoices; voice++)
		{
			// A correct implementation needs to take numsamples and player samplerate into account.
			// This will not be fixed to keep sampler compatible with old songs.
			ps1samplervoice_performfxold(&self->_voices[voice]);
		}
	//}
	self->samplerowcounter += bc->numsamples;
	psy_audio_samplerticktimer_update(&self->ticktimer, bc->numsamples, bc);		
}

void psy_audio_samplerps1_ontimertick(psy_audio_SamplerPS1* self)
{
	/*psy_List* p;
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
	}*/
}

void psy_audio_samplerps1_ontimerwork(psy_audio_SamplerPS1* self,
	psy_audio_BufferContext* bc)
{
	for (int voice = 0; voice < self->_numVoices; voice++)
	{
		ps1samplervoice_work(&self->_voices[voice],
			bc->numsamples,
			psy_audio_buffer_at(bc->output, 0),
			psy_audio_buffer_at(bc->output, 1));
	}
}

void seqtick(psy_audio_SamplerPS1* self, uintptr_t channel,
	const psy_audio_PatternEvent* pData)
{		
	// if (_mute) return; // Avoid new note entering when muted.
	psy_audio_PatternEvent* copy;
	psy_audio_PatternEvent data = *pData;
	psy_audio_Samples* samples;
	psy_List* ite;

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

	if (data.inst == 255)
	{
		data.inst = self->lastInstrument[channel];
		if (data.inst == 255) {
			return;  // no previous sample. Skip
		}
	} else { self->lastInstrument[channel] = data.inst; }

	samples = psy_audio_machine_samples(psy_audio_samplerps1_base(self));
	if (!psy_audio_samples_at(samples, sampleindex_make(data.inst, self->instrumentbank))) return; // if no wave, return.

	int idx = psy_audio_samplerps1_getcurrentvoice(self, channel);
	if (data.cmd != SAMPLER_CMD_NONE) {
		// Adding also the current command, to make the loops easier.
		psy_audio_PatternEvent data2 = data;
		data2.inst = channel;
		copy = malloc(sizeof(psy_audio_PatternEvent));
		*copy = data2;
		psy_list_append(&self->multicmdMem, copy);
	}
	bool doporta = FALSE;
	for (ite = self->multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ev->inst == channel) {
			if (ev->cmd == SAMPLER_CMD_PORTA2NOTE && data.note < NOTECOMMANDS_RELEASE && idx != -1) {
				// if (self->linearslide) {   // isLinearSlide()
					// EnablePerformFx();
				// }
				doporta = TRUE;
			} else if (ev->cmd == PS1_SAMPLER_CMD_PORTADOWN || ev->cmd == PS1_SAMPLER_CMD_PORTAUP) {
			//	if (isLinearSlide()) {
				//	EnablePerformFx();
				//}
			}
		}
	}

	int useVoice = -1;
	if (data.note < NOTECOMMANDS_RELEASE && !doporta)	// Handle Note On.
	{
		useVoice = getfreevoice(self); // Find a voice to apply the new note
		if (idx != -1) // NoteOff previous Notes in this channel.
		{
			switch (self->_voices[idx].inst->nna)
			{
			case 0:
				ps1samplervoice_noteofffast(&self->_voices[idx]);
				break;
			case 1:
				ps1samplervoice_noteoff(&self->_voices[idx]);
				break;
			}
			if (useVoice == -1) { useVoice = idx; }
		}
		if (useVoice == -1)	// No free voices. Assign first one.
		{						// This algorithm should be replace by a LRU lookup
			useVoice = 0;
		}
		self->_voices[useVoice]._channel = channel;
	} else {
		if (idx != -1)
		{
			if (data.note == NOTECOMMANDS_RELEASE) ps1samplervoice_noteoff(&self->_voices[idx]);  //  Handle Note Off
			useVoice = idx;
		}
		if (useVoice == -1) return;   // No playing note on this channel. Just go out.
										// Change it if you have channel commands.
	}
	// If you want to make a command that controls more than one voice (the entire channel, for
	// example) you'll need to change this. Otherwise, add it to Voice.Tick().
	ps1samplervoice_tick(&self->_voices[useVoice], &data, channel,
		self->defaultspeed ? NOTECOMMANDS_MIDDLEC : 48, self->multicmdMem);
}

const psy_audio_MachineInfo* info(psy_audio_SamplerPS1* self)
{	
	return &macinfo;
}

uintptr_t numparameters(psy_audio_SamplerPS1* self)
{
	return numparametercols(self) * 8;
}

uintptr_t numparametercols(psy_audio_SamplerPS1* self)
{
	return 10;
}

#define CHANNELROW 2

psy_audio_MachineParam* parameter(psy_audio_SamplerPS1* self, uintptr_t param)
{
	switch (param) {
		case 0: return &self->param_numvoices.machineparam;
			break;
		case 1: return &self->param_resamplingmethod.machineparam;
			break;
		case 2: return &self->param_defaultspeed.machineparam;
			break;
		case 3: return &self->param_instrumentbank.machineparam;
			break;
		default:
			break;
	}
	return NULL;
}

void resamplingmethod_tweak(psy_audio_SamplerPS1* self,
	psy_audio_ChoiceMachineParam* sender, float value)
{	
	psy_audio_samplerps1_setresamplerquality(self,
		(int)(value * (sender->maxval - sender->minval) + 0.5f) +
			sender->minval);	
}

uintptr_t numinputs(psy_audio_SamplerPS1* self)
{
	return 0;
}

uintptr_t numoutputs(psy_audio_SamplerPS1* self)
{
	return 2;
}

static int alteRand(int x) { return (x * rand()) / 32768; }

psy_List* sequencerinsert(psy_audio_SamplerPS1* self, psy_List* events)
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
						psy_audio_samplerps1_base(self));
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
					psy_audio_samplerps1_base(self));				
				psy_list_append(&insert, newentry);
			}
		}
	}
	return insert;
}

// fileio
void loadspecific(psy_audio_SamplerPS1* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;
	int resamplertype;
		
	self->instrumentbank = 0;
	psy_audio_samplerps1_defaultC4(self, FALSE);
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
		psy_audio_samplerps1_setresamplerquality(self, resamplertype);
		self->instrumentbank = 0;
		if (size > 3 * sizeof(unsigned int))
		{
			unsigned int internalversion;
			psyfile_read(songfile->file, &internalversion,
				sizeof(internalversion));
			if (internalversion >= 1) {
				uint8_t defaultC4;

				// correct A4 frequency.
				psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4));
				psy_audio_samplerps1_defaultC4(self, defaultC4 != FALSE);
			}
			if (internalversion >= 2) {
				unsigned char slidemode;
				// correct slide.
				psyfile_read(songfile->file, &slidemode, sizeof(slidemode));
				self->linearslide = FALSE;
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

void savespecific(psy_audio_SamplerPS1* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t temp;
	uint32_t size = 3 * sizeof(temp) + 2 * sizeof(bool);
	psyfile_write(songfile->file, &size, sizeof(size));
	temp = self->_numVoices;
	psyfile_write(songfile->file, &temp, sizeof(temp)); // numSubtracks
	switch (self->resamplerquality) {
		case RESAMPLERTYPE_ZERO_ORDER: temp = 0; break;
		case RESAMPLERTYPE_SPLINE: temp = 2; break;
		case RESAMPLERTYPE_SINC: temp = 3; break;
		case RESAMPLERTYPE_LINEAR: //fallthrough
		default: temp = 1;
	}
	psyfile_write(songfile->file, &temp, sizeof(temp)); // quality
	psyfile_write_int32(songfile->file, SAMPLERVERSION);
	psyfile_write_uint8(songfile->file, (uint8_t)psy_audio_samplerps1_isdefaultC4(self)); // correct A4
	psyfile_write_uint8(songfile->file, (uint8_t)self->linearslide); // correct slide
	psyfile_write_uint32(songfile->file, (uint32_t)self->instrumentbank);
}

void psy_audio_samplerps1_setresamplerquality(psy_audio_SamplerPS1* self,
	ResamplerType quality)
{
	for (int voice = 0; voice < self->_numVoices; voice++)
	{
		psy_audio_sampleiterator_setquality(&self->_voices[voice].controller,
			quality);
	}	
}

void psy_audio_samplerps1_enableperformfx(psy_audio_SamplerPS1* self)
{
	/*for (int i = 0; i < Global::song().SONGTRACKS; i++)
	{
		if (TriggerDelay[i]._cmd == 0)
		{
			TriggerDelayCounter[i] = 0;
			TriggerDelay[i]._cmd = 0xEF;
			TriggerDelay[i]._parameter = Global::song().TicksPerBeat() / Global::song().LinesPerBeat();
			break;
		}
	}*/
}
