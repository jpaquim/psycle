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

// Voice
// implementation
void ps1samplervoice_init(PS1SamplerVoice* self, psy_audio_SamplerPS1* sampler)
{
	self->sampler = NULL;
	self->inst = NULL;
	self->_instrument = 0xFF;
	self->_channel = -1;
	self->_sampleCounter = 0;
	self->_cutoff = 0;
	self->effCmd = PS1_SAMPLER_CMD_NONE;
	filter_init(&self->_filter);
	psy_audio_sampleiterator_init(&self->controller, NULL, RESAMPLERTYPE_LINEAR);
	ps1samplervoice_setup(self);
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

	//If this sample is not enabled, Voice::Tick is not called. Also, Sampler::Tick takes care of previus instrument used.
	self->_instrument = pEntry->inst;
	self->inst = instruments_at(psy_audio_samplerps1_base(self->sampler),
		instrumentindex_make(self->sampler->instrumentbank, self->_instrument));
	// Setup commands that affect the new or already playing voice.
	for (ite = multicmdMem; ite != NULL; psy_list_next(&ite)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(ite);
		if (ev->inst == channelNum) {
			// one shot {
			switch (ev->cmd) {
			case PS1_SAMPLER_CMD_PANNING: {
				//controller._pan = helpers::value_mapper::map_256_1(ite->_parameter);
				//dopan = TRUE;
			} break;
			case PS1_SAMPLER_CMD_OFFSET: {
				//const XMInstrument::WaveData<>& wave = Global::song().samples[_instrument];
				//w_offset = (uint64_t)(ev->parameter * wave.WaveLength()) << 24;
				//dooffset = TRUE;
			} break;
			case PS1_SAMPLER_CMD_VOLUME: {
				//controller._vol = helpers::value_mapper::map_256_1(ite->_parameter);
				//dovol = TRUE;
			} break;
				// }
				// Running {
			case PS1_SAMPLER_CMD_PORTAUP: {
				//self->effVal = ev->parameter;
				//self->effCmd = ev->cmd;
			} break;
			case PS1_SAMPLER_CMD_PORTADOWN: {
				//self->effVal = ev->parameter;
				//self->effCmd = ev->cmd;
			} break;
			case PS1_SAMPLER_CMD_PORTA2NOTE: {
				/*if (_envelope._stage != ENV_OFF) {
					effCmd = ite->_cmd;
					effVal = ite->_parameter;
					if (pEntry->_note < notecommands::release) {
						const XMInstrument::WaveData<>& wave = Global::song().samples[_instrument];
						float const finetune = (float)wave.WaveFineTune() * 0.01f;
						double speeddouble = pow(2.0f, (pEntry->_note + wave.WaveTune() - baseC + finetune) / 12.0f) * ((float)wave.WaveSampleRate() / Global::player().SampleRate());
						_effPortaSpeed = static_cast<int64_t>(speeddouble * 4294967296.0f);
					}
					if (_effPortaSpeed < controller._speed) {
						effVal *= -1;
					}
					doporta = true;
				}*/
			} break;
			case PS1_SAMPLER_CMD_RETRIG: {
				/*if ((ite->_parameter & 0x0f) > 0)
				{
					effretTicks = (ite->_parameter & 0x0f); // number of Ticks.
					effVal = (Global::player().SamplesPerRow() / (effretTicks + 1));

					int volmod = (ite->_parameter & 0xf0) >> 4; // Volume modifier.
					switch (volmod)
					{
					case 0:  //fallthrough
					case 8:	effretVol = 0; effretMode = 0; break;

					case 1:  //fallthrough
					case 2:  //fallthrough
					case 3:  //fallthrough
					case 4:  //fallthrough
					case 5: effretVol = (float)(std::pow(2., volmod - 1) / 64); effretMode = 1; break;

					case 6: effretVol = 0.66666666f;	 effretMode = 2; break;
					case 7: effretVol = 0.5f;			 effretMode = 2; break;

					case 9:  //fallthrough
					case 10:  //fallthrough
					case 11:  //fallthrough
					case 12:  //fallthrough
					case 13: effretVol = (float)(std::pow(2., volmod - 9) * (-1)) / 64; effretMode = 1; break;

					case 14: effretVol = 1.5f;					effretMode = 2; break;
					case 15: effretVol = 2.0f;					effretMode = 2; break;
					}
					_triggerNoteDelay = effVal;
				}*/
			} break;
				// }
			case PS1_SAMPLER_CMD_EXTENDED: {
				/*
				// delayed {
				if ((ite->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEOFF) {
					//This means there is always 6 ticks per row whatever number of rows.
					_triggerNoteOff = (Global::player().SamplesPerRow() / 6.f) * (ite->_parameter & 0x0f);
				} else if ((ite->_parameter & 0xf0) == SAMPLER_CMD_EXT_NOTEDELAY && (ite->_parameter & 0x0f) != 0) {
					//This means there is always 6 ticks per row whatever number of rows.
					_triggerNoteDelay = (Global::player().SamplesPerRow() / 6.f) * (ite->_parameter & 0x0f);
				}
				// }*/
			} break;
			}
		}
	}
	if (pEntry->note < NOTECOMMANDS_RELEASE && !doporta)
	{
		psy_audio_Sample* sample;

		sample = psy_audio_samples_at(
			psy_audio_machine_samples(psy_audio_samplerps1_base(self->sampler)),
			sampleindex_make(self->_instrument, 0));
		psy_audio_sampleiterator_setsample(&self->controller, sample);

		// Init Resampler
		//
		double speeddouble;
		//if (inst->_loop)
		{
			//double const totalsamples = double(Global::player().SamplesPerRow() * inst->_lines);
			//speeddouble = (wave.WaveLength() / totalsamples);
		} //else
		{
			float const finetune = (float)sample->finetune * 0.01f;
			speeddouble = pow(2.0f, (pEntry->note + sample->tune - baseC + finetune) / 12.0f) *
				((float)sample->samplerate / 44100); // Global::player().SampleRate()
		}
		psy_audio_sampleiterator_setspeed(&self->controller, speeddouble);
		//controller._speed = static_cast<int64_t)(speeddouble * 4294967296.0f);

		//if (controller.resampler_data != NULL) resampler.DisposeResamplerData(controller.resampler_data);
		//controller.resampler_data = resampler.GetResamplerData();
		//resampler.UpdateSpeed(controller.resampler_data, speeddouble);

		if (!dooffset) { dooffset = TRUE; w_offset = 0; }

		// Init Amplitude Envelope
		//
		self->_envelopesettings.sustain = self->inst->volumeenvelope.sustain;
		//self->_envelope.sustain = self->inst->volumeenvelope.sustain; // static_cast<float>(inst->ENV_SL) * 0.01f;
		//_envelope._step = (1.0f / inst->ENV_AT) * _envelope.sratefactor;
		//_envelope._value = 0.0f;
		//controller._lVolCurr = controller._lVolDest;
		//controller._rVolCurr = controller._rVolDest;
		//if (!dovol) { dovol = true; controller._vol = 1.f; }
		//if (!dopan) {
			//if (inst->_RPAN) {
				//dopan = true;
				//controller._pan = value_mapper::map_32768_1(rand());
			//} else {
				//dopan = true;
				//controller._pan = Global::song().samples[_instrument].PanFactor();
			//}
		//}

		//Init filter
		//_cutoff = (inst->_RCUT) ? alteRand(inst->ENV_F_CO) : inst->ENV_F_CO;
		//_filter.Ressonance((inst->_RRES) ? alteRand(inst->ENV_F_RQ) : inst->ENV_F_RQ);
		//_filter.Type(inst->ENV_F_TP);
		//_coModify = static_cast<float>(inst->ENV_F_EA);
		//_filterEnv._sustain = value_mapper::map_128_1(inst->ENV_F_SL);
		//_filterEnv._step = (1.0f / inst->ENV_F_AT) * _envelope.sratefactor;
		//_filterEnv._value = 0;

		if (self->_triggerNoteDelay == 0) {
			//_envelope._stage = ENV_ATTACK;
			//_filterEnv._stage = ENV_ATTACK;
		} else {
			//_envelope._stage = ENV_OFF;
			//_filterEnv._stage = ENV_OFF;
		}

		triggered = 1;
	}

	if (dovol || dopan) {
		// Panning calculation -------------------------------------------
		//controller._rVolDest = controller._pan;
		//controller._lVolDest = 1.f - controller._pan;
		//FT2 Style (Two slides) mode, but with max amp = 0.5.
		//if (controller._rVolDest > 0.5f) { controller._rVolDest = 0.5f; }
		//if (controller._lVolDest > 0.5f) { controller._lVolDest = 0.5f; }

		//controller._lVolDest *= Global::song().samples[_instrument].WaveGlobVolume() * controller._vol;
		//controller._rVolDest *= Global::song().samples[_instrument].WaveGlobVolume() * controller._vol;
	}
	if (dooffset) {
		self->controller.pos.QuadPart = w_offset;
	}

	if (triggered)
	{
		//controller._lVolCurr = controller._lVolDest;
		//controller._rVolCurr = controller._rVolDest;
	}

	return triggered;
}

void ps1samplervoice_performfxold(PS1SamplerVoice* self)
{
	// 4294967 stands for (2^30/250), meaning that
			//value 250 = (inc)decreases the speed in 1/4th of the original (wave) speed each PerformFx call.
	int64_t shift;
	switch (self->effCmd)
	{
		// 0x01 : Pitch Up
	case PS1_SAMPLER_CMD_PORTAUP:
	//	shift = (int64_t)(effVal) * 4294967ll * (float)(controller.wave->WaveSampleRate()) / Global::player().SampleRate();
		//controller._speed += shift;
		//resampler.UpdateSpeed(controller.resampler_data, controller._speed);
		break;
		// 0x02 : Pitch Down
	case PS1_SAMPLER_CMD_PORTADOWN:
		//shift = (int64_t)(effVal) * 4294967ll * static_cast<float>(controller.wave->WaveSampleRate()) / Global::player().SampleRate();
		//controller._speed -= shift;
		//if (controller._speed < 0) controller._speed = 0;
		//resampler.UpdateSpeed(controller.resampler_data, controller._speed);
		break;
		// 0x03 : Porta to note
	case PS1_SAMPLER_CMD_PORTA2NOTE:
		//effVal is multiplied by -1 in Tick if it needs to slide down.
		//shift = static_cast<int64_t>(effVal) * 4294967ll * static_cast<float>(controller.wave->WaveSampleRate()) / Global::player().SampleRate();
		//controller._speed += shift;
		//if ((effVal < 0 && controller._speed < _effPortaSpeed)
			//|| (effVal > 0 && controller._speed > _effPortaSpeed)) {
			//controller._speed = _effPortaSpeed;
			//self->effCmd = SAMPLER_CMD_NONE;
		//}
		//resampler.UpdateSpeed(controller.resampler_data, controller._speed);
		break;

	default:
		break;
	}
}

void ps1samplervoice_performfxnew(PS1SamplerVoice* self)
{
	//value 1 = (inc)decreases the speed in one seminote each beat.
	//double factor = 1.0 / (12.0 * Global::song().TicksPerBeat());
	//switch (effCmd)
	//{
		// 0x01 : Pitch Up
	//case SAMPLER_CMD_PORTAUP:
	//	controller._speed *= pow(2.0, effVal * factor);
	//	resampler.UpdateSpeed(controller.resampler_data, controller._speed);
	//	break;
		// 0x02 : Pitch Down
	//case SAMPLER_CMD_PORTADOWN:
		//controller._speed *= pow(2.0, -effVal * factor);
	//	if (controller._speed < 0) controller._speed = 0;
	//	resampler.UpdateSpeed(controller.resampler_data, controller._speed);
	//	break;
		// 0x03 : Porta to note
	//case SAMPLER_CMD_PORTA2NOTE:
		//effVal is multiplied by -1 in Tick() if it needs to slide down.
	//	controller._speed *= pow(2.0, effVal * factor);
	//	if ((effVal < 0 && controller._speed < _effPortaSpeed)
	//		|| (effVal > 0 && controller._speed > _effPortaSpeed)) {
	//		controller._speed = _effPortaSpeed;
	//		effCmd = SAMPLER_CMD_NONE;
	//	}
	//	resampler.UpdateSpeed(controller.resampler_data, controller._speed);
	//	break;

	//default:
	//	break;
	//}
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
static void disposevoices(psy_audio_SamplerPS1*);
static void disposechannels(psy_audio_SamplerPS1*);
static void disposeparameters(psy_audio_SamplerPS1*);
static int alloc_voice(psy_audio_SamplerPS1*);
static void releaseallvoices(psy_audio_SamplerPS1*);
static psy_audio_SamplerVoice* activevoice(psy_audio_SamplerPS1*,
	uintptr_t channel);
static void releasevoices(psy_audio_SamplerPS1*, uintptr_t channel);
static void nnavoices(psy_audio_SamplerPS1*, uintptr_t channel);
static void removeunusedvoices(psy_audio_SamplerPS1* self);
static uintptr_t numinputs(psy_audio_SamplerPS1*);
static uintptr_t numoutputs(psy_audio_SamplerPS1*);
static void loadspecific(psy_audio_SamplerPS1*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_SamplerPS1*, psy_audio_SongFile*,
	uintptr_t slot);
static psy_audio_SamplerChannel* sampler_channel(psy_audio_SamplerPS1*,
	uintptr_t channelnum);
static psy_audio_InstrumentIndex currslot(psy_audio_SamplerPS1*,
	uintptr_t channel, const psy_audio_PatternEvent*);

//
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
	//_macIndex = index;
	//self->_numPars = 0;
	//_type = MACH_SAMPLER;
	//_mode = MACHMODE_GENERATOR;
	//strncpy(_editName, _psName, sizeof(_editName) - 1);
	//_editName[sizeof(_editName) - 1] = '\0';
	//InitializeSamplesVector();

	self->baseC = NOTECOMMANDS_MIDDLEC;
	self->linearslide = TRUE;
	//_resampler.quality(helpers::dsp::resampler::quality::spline);
	for (int i = 0; i < MAX_TRACKS; i++)
	{
		self->lastInstrument[i] = 255;
	}
	// Machine::Init();
	self->multicmdMem = NULL;
	self->_numVoices = SAMPLER_DEFAULT_POLYPHONY;
	for (int i = 0; i < self->_numVoices; i++) {
		ps1samplervoice_init(&self->_voices[i], self);		
	}
	// SetSampleRate(Global::player().SampleRate());


	custommachine_init(&self->custommachine, callback);
	sampler_vtable_init(self);
	psy_audio_samplerps1_base(self)->vtable = &sampler_vtable;
	psy_audio_machine_seteditname(psy_audio_samplerps1_base(self), "XMSampler");
	self->numvoices = SAMPLER_DEFAULT_POLYPHONY;
	self->voices = 0;
	self->resamplerquality = RESAMPLERTYPE_LINEAR;
	self->defaultspeed = 1;
	self->maxvolume = 0xFF;
	self->panpersistent = 0;
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
	psy_audio_samplerps1_initparameters(self);	
	self->instrumentbank = 0;	
	psy_audio_samplerticktimer_init(&self->ticktimer,
		self, // callback context (sampler)
		psy_audio_samplerps1_ontimertick,
		psy_audio_samplerps1_ontimerwork);
}

void psy_audio_samplerps1_stop(psy_audio_SamplerPS1* self)
{
	for (int i = 0; i < self->_numVoices; i++) {
		ps1samplervoice_noteofffast(&self->_voices[i]);
		//_voices[i].NoteOffFast();
		self->_voices[i]._effPortaSpeed = 4294967296.0f;
	}
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
}

void psy_audio_samplerps1_stopinstrument(psy_audio_SamplerPS1* self,
	int insIdx)
{
	for (int i = 0; i < self->_numVoices; i++)
	{
		PS1SamplerVoice* pVoice = &self->_voices[i];
		if (pVoice->_instrument == insIdx &&
			(pVoice->_envelope.stage != ENV_OFF ||
				pVoice->_triggerNoteDelay > 0))
		{
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
	for (int voice = 0; voice < self->_numVoices; voice++)
	{
		// ENV_OFF is not checked, because channel will be -1
		if (self->_voices[voice]._channel == track && (self->_voices[voice]._triggerNoteDelay > 0 || self->_voices[voice]._envelope.stage != ENV_FASTRELEASE))
		{
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

			voice = (psy_audio_SamplerVoice*)p->entry;
			if (psy_audio_samplervoice_isplaying(voice)) {
				psy_audio_samplervoice_newline(voice);
			}
		}
		psy_audio_samplerticktimer_reset(&self->ticktimer,
			(uintptr_t)
			psy_audio_machine_samplespertick(psy_audio_samplerps1_base(self)));
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
	ps1samplervoice_tick(&self->_voices[useVoice], &data, channel, self-> baseC, self->multicmdMem);
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
	disposevoices(self);
	disposechannels(self);
	custommachine_dispose(&self->custommachine);		
}

void disposeparameters(psy_audio_SamplerPS1* self)
{
	psy_audio_intmachineparam_dispose(&self->param_numvoices);
	psy_audio_choicemachineparam_dispose(&self->param_resamplingmethod);
	psy_audio_choicemachineparam_dispose(&self->param_defaultspeed);
	psy_audio_intmachineparam_dispose(&self->param_instrumentbank);
}

void disposevoices(psy_audio_SamplerPS1* self)
{
	psy_list_deallocate(&self->voices, (psy_fp_disposefunc)
		psy_audio_samplervoice_dispose);
}

void disposechannels(psy_audio_SamplerPS1* self)
{
	psy_table_disposeall(&self->channels, (psy_fp_disposefunc)
		psy_audio_samplerchannel_dispose);
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

psy_audio_SamplerChannel* sampler_channel(psy_audio_SamplerPS1* self, uintptr_t track)
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

void generateaudio(psy_audio_SamplerPS1* self, psy_audio_BufferContext* bc)
{	
	self->samplerowcounter += bc->numsamples;
	psy_audio_samplerticktimer_update(&self->ticktimer, bc->numsamples, bc);		
	removeunusedvoices(self);
}

void psy_audio_samplerps1_ontimertick(psy_audio_SamplerPS1* self)
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

void psy_audio_samplerps1_ontimerwork(psy_audio_SamplerPS1* self,
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

void seqtick(psy_audio_SamplerPS1* self, uintptr_t channelnum,
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
			psy_audio_samplerps1_base(self)),
			currslot(self, channelnum, &event));
		if (instrument) {
			voice = psy_audio_samplervoice_allocinit(self, instrument,
				channel,
				channelnum,
				psy_audio_machine_samplerate(psy_audio_samplerps1_base(self)));
			psy_list_append(&self->voices, voice);
		}
	}	
	if (voice) {
		psy_audio_samplervoice_seqtick(voice, &event);
	}
}

psy_audio_InstrumentIndex currslot(psy_audio_SamplerPS1* self, uintptr_t channel,
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

void releaseallvoices(psy_audio_SamplerPS1* self)
{
	psy_List* p;
	
	for (p = self->voices; p != NULL; psy_list_next(&p)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*) p->entry;		
		psy_audio_samplervoice_release(voice);		
	}
}

void releasevoices(psy_audio_SamplerPS1* self, uintptr_t channel)
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

void nnavoices(psy_audio_SamplerPS1* self, uintptr_t channel)
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

psy_audio_SamplerVoice* activevoice(psy_audio_SamplerPS1* self, uintptr_t channel)
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

void removeunusedvoices(psy_audio_SamplerPS1* self)
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
		if (size > 3 * sizeof(unsigned int))
		{
			unsigned int internalversion;
			psyfile_read(songfile->file, &internalversion,
				sizeof(internalversion));
			if (internalversion >= 1) {
				uint8_t defaultC4;

				// correct A4 frequency.
				psyfile_read(songfile->file, &defaultC4, sizeof(defaultC4));
				psy_audio_samplerps1_defaultC4(self, defaultC4 != 0);
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

void savespecific(psy_audio_SamplerPS1* self, psy_audio_SongFile* songfile,
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

void psy_audio_samplerps1_setresamplerquality(psy_audio_SamplerPS1* self,
	ResamplerType quality)
{
	psy_List* it;
	
	for (it = self->voices; it != NULL; psy_list_next(&it)) {
		psy_audio_SamplerVoice* voice;

		voice = (psy_audio_SamplerVoice*)(it->entry);
		psy_audio_samplervoice_setresamplerquality(voice, quality);
	}
}
