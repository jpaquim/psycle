// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy2.h"
// local
#include "machinefactory.h"
#include "pattern.h"
#include "plugin.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
#include "vstplugin.h"
#include "wire.h"
// dsp
#include <datacompression.h>
#include <operations.h>
// std
#include <math.h>
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/os.h"

#define PSY2_EVENT_SIZE 5
#define PSY2_MAX_CONNECTIONS 12

typedef struct cpoint_t {
	int32_t x;
	int32_t y;
} cpoint_t;

static void vstpreload_init(VstPreload*, psy_audio_SongFile*);
static bool vstpreload_load(VstPreload*, uintptr_t slot,
	unsigned char* program, int32_t* instance);

// Prototypes
static void psy2loader_readheader(PSY2Loader*);
static void psy2loader_readpatterns(PSY2Loader*);
static void psy2loader_readsequence(PSY2Loader*);
static void psy2loader_readinstruments(PSY2Loader*);
static void psy2loader_readvsts(PSY2Loader*);
static void psy2loader_readmachines(PSY2Loader*);
static void psy2loader_addmachines(PSY2Loader*);
static void psy2loader_machine_load(PSY2Loader*, uintptr_t slot);
static void psy2loader_master_load(PSY2Loader*, psy_audio_Machine*, uintptr_t slot);
static void psy2loader_sampler_load(PSY2Loader*, psy_audio_Machine*, uintptr_t slot);
static void psy2loader_plugin_load(PSY2Loader*, psy_audio_Machine*,
	uintptr_t slot);
static void psy2loader_plugin_skipload(PSY2Loader*, psy_audio_Machine*, uintptr_t slot);
static void psy2loader_postload(PSY2Loader*);
// Implementation
void psy2loader_init(PSY2Loader* self, psy_audio_SongFile* songfile)
{
	self->songfile = songfile;	
	internalmachinesconvert_init(&self->converter);
	psy_audio_legacywires_init(&self->legacywires);
	psy_audio_legacywires_init(&self->legacywiresremapped);
}

void psy2loader_dispose(PSY2Loader* self)
{
	psy_audio_legacywires_dispose(&self->legacywires);
	psy_audio_legacywires_dispose(&self->legacywiresremapped);
	internalmachinesconvert_dispose(&self->converter);
}

void psy2loader_load(PSY2Loader* self)
{
	self->songfile->legacywires = &self->legacywires;	
	psy2loader_readheader(self);
	psyfile_read(self->songfile->file, &self->currentoctave, sizeof(char));
	psyfile_read(self->songfile->file, self->busMachine, 64);
	psyfile_read(self->songfile->file, self->playorder, 128);
	psyfile_read(self->songfile->file, &self->playlength, sizeof(int32_t));
	psyfile_read(self->songfile->file, &self->songtracks, sizeof(int32_t));
	psy2loader_readpatterns(self);
	psy2loader_readsequence(self);
	psy2loader_readinstruments(self);
	psy2loader_readvsts(self);
	psy2loader_readmachines(self);
	psy2loader_addmachines(self);
	psy2loader_postload(self);
}

void psy2loader_postload(PSY2Loader* self)
{
	psy_TableIterator it;
	
	for (it = psy_audio_machines_begin(&self->songfile->song->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_machine_postload(
			(psy_audio_Machine*)psy_tableiterator_value(&it),
			self->songfile,
			psy_tableiterator_key(&it));
	}
}

// Header
// |-|0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
// |0|P|S|Y|2|S|O|N|G|n|a|m|e| | | | |
// |1| | | | | | | | | | | | | | | | |
// |2| | | | | | | | |a|u|t|h|o|r| | |
// |3| | | | | | | | | | | | | | | | |
// |4| | | | | | | | |c|o|m|m|e|n|t| |
// |5| | | | | | | | | | | | | | | | |
// |6| | | | | | | | | | | | | | | | |
// |7| | | | | | | | | | | | | | | | |
// |8| | | | | | | | | | | | | | | | |
// |9| | | | | | | | | | | | | | | | |
// |A| | | | | | | | | | | | | | | | |
// |B| | | | | | | | | | | | | | | | |
// |C| | | | | | | | | | | | | | | | |
// |D| | | | | | | | |b|p|m| |s|p|t| |
// |E|o|
void psy2loader_readheader(PSY2Loader* self)
{
	// Can be NOT NULL terminated. (specsize + 1 for NULL termination)
	char name[33];
	char author[33];
	char comments[129];
	int32_t beatspermin;
	int32_t sampR;
	int32_t linesperbeat;
	psy_audio_SongProperties songproperties;

	// PSY2SONG, the Sign of the file, already read by songio,
	// so it isn't done here again

	// Label: name
	// Size : 32chars.
	psyfile_read(self->songfile->file, name, 32);
	// Can be NOT NULL terminated.
	name[32] = '\0';
	// Label: author
	// Size : 32chars.
	psyfile_read(self->songfile->file, author, 32);
	// Can be NOT NULL terminated.
	author[32] = '\0';
	// Label: comment
	// Size : 128chars.
	psyfile_read(self->songfile->file, comments, 128);
	// Can be NOT NULL terminated.
	comments[128] = '\0';
	// Label: bpm
	// Size : 1int(32bits)
	// Desc:  Contains The song BPM
	psyfile_read(self->songfile->file, &beatspermin, sizeof beatspermin);
	// Label: spt
	// Size : 1int(32bits)
	// Desc : Contains The song samples per tick.This is a measure used internally by the player, but thanks to it, this makes possible to store the TPB that the format didn’t support.To get the TPB, use :
	// 	ticksPerBeat = 44100 * 15 * 4 / (SamplesPerTick * BeatsPerMin);
	// Next, Update your SamplesPerTick value as :
	// SamplesPerTick = SamplesPerTick * (currentSamplesPerSec / 44100);
	// This is done because the original fileformat was hardcoded at 44100Khz.
	// Do the opposite when(if) saving!!!
	// 
	// Advise.If, for any case, the SamplesPerTick is under or equal to 0, default to these values : _ticksPerBeat = 4; SamplesPerTick = 4315;
	// (One version did damage this value)
	psyfile_read(self->songfile->file, &sampR, sizeof sampR);
	if (sampR <= 0) {
		// Shouldn't happen but has happened.
		linesperbeat = 4;
	} else {
		// The old format assumes we output at 44100 samples/sec, so...	
		linesperbeat = (int32_t)(44100 * 60 / (sampR * beatspermin));
	}	
	psy_audio_songproperties_init_all(&songproperties,
		name,
		author,
		comments,
		16, // tracks
		4,  // octave
		linesperbeat,
		24, // extraticksperbeat
		0,
		beatspermin);
	psy_audio_song_setproperties(self->songfile->song, &songproperties);
	psy_audio_songproperties_dispose(&songproperties);
}

void psy2loader_readpatterns(PSY2Loader* self)
{
	int32_t num;
	int32_t i;

	psyfile_read(self->songfile->file, &num, sizeof num);
	for (i = 0; i < num; ++i) {
		int32_t numlines;
		char patternName[32];

		psyfile_read(self->songfile->file, &numlines, sizeof(numlines));
		psyfile_read(self->songfile->file, patternName, sizeof(patternName));
		if (numlines > 0)
		{
			///\todo: tweak_effect should be converted to normal tweaks!				
			psy_audio_PatternNode* node = 0;
			psy_dsp_big_beat_t offset = 0.f;
			unsigned char* pData;
			int32_t c;
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();
			psy_audio_patterns_insert(&self->songfile->song->patterns, i, pattern);
			pData = malloc(OLD_MAX_TRACKS * sizeof(psy_audio_PatternEntry));
			for (c = 0; c < numlines; ++c) {
				int32_t track;
				unsigned char* ptrack;

				psyfile_read(self->songfile->file, pData, OLD_MAX_TRACKS * PSY2_EVENT_SIZE);
				ptrack = pData;
				for (track = 0; track < OLD_MAX_TRACKS; ++track) {
					psy_audio_PatternEvent event;
					// Psy3 PatternEntry format
					// type				offset
					// uint8_t note;		0
					// uint8_t inst;		1
					// uint8_t mach;		2
					// uint8_t cmd;			3
					// uint8_t parameter;	4												
					psy_audio_patternevent_clear(&event);
					event.note = ptrack[0];
					event.inst = (ptrack[1] == 0xFF)
						? event.inst = psy_audio_NOTECOMMANDS_INST_EMPTY
						: ptrack[1];
					event.mach = (ptrack[2] == 0xFF)
						? event.mach = psy_audio_NOTECOMMANDS_MACH_EMPTY
						: ptrack[2];
					event.cmd = ptrack[3];
					event.parameter = ptrack[4];
					if (!psy_audio_patternevent_empty(&event)) {						
						node = psy_audio_pattern_insert(pattern, node, track, offset,
							&event);
					}
					ptrack += PSY2_EVENT_SIZE;
				}
				offset += 1.0 / psy_audio_song_lpb(self->songfile->song);
			}
			pattern->length = numlines *
				(1.0 / psy_audio_song_lpb(self->songfile->song));
			free(pData);
			pData = 0;
		} else {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();
			psy_audio_patterns_insert(&self->songfile->song->patterns, i, pattern);
			pattern->length = 64 * 1.0 / psy_audio_song_lpb(self->songfile->song);;
		}
	}
}

void psy2loader_readsequence(PSY2Loader* self)
{
	int32_t i;
	psy_audio_SequencePosition sequenceposition;

	sequenceposition.tracknode =
		psy_audio_sequence_appendtrack(&self->songfile->song->sequence,
			psy_audio_sequencetrack_allocinit());
	for (i = 0; i < self->playlength; ++i) {
		sequenceposition.trackposition =
			psy_audio_sequence_last(&self->songfile->song->sequence,
				sequenceposition.tracknode);
		psy_audio_sequence_insert(&self->songfile->song->sequence,
			sequenceposition, self->playorder[i]);
	}
}

void psy2loader_readinstruments(PSY2Loader* self)
{
	unsigned char _NNA[OLD_MAX_INSTRUMENTS];
	///\name Amplitude Envelope overview:
	///\{
	/// Attack Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_AT[OLD_MAX_INSTRUMENTS];
	/// Decay Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_DT[OLD_MAX_INSTRUMENTS];
	/// Sustain Level [in %]
	int32_t ENV_SL[OLD_MAX_INSTRUMENTS];
	/// Release Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_RT[OLD_MAX_INSTRUMENTS];
	///\}

		///\name psy_dsp_Filter 
	///\{
	/// Attack Time [in psy_audio_Samples at 44.1Khz]
	int32_t ENV_F_AT[OLD_MAX_INSTRUMENTS];
	/// Decay Time [in psy_audio_Samples at 44.1Khz]
	int32_t ENV_F_DT[OLD_MAX_INSTRUMENTS];
	/// Sustain Level [0..128]
	int32_t ENV_F_SL[OLD_MAX_INSTRUMENTS];
	/// Release Time [in psy_audio_Samples at 44.1Khz]
	int32_t ENV_F_RT[OLD_MAX_INSTRUMENTS];

	/// Cutoff Frequency [0-127]
	int32_t ENV_F_CO[OLD_MAX_INSTRUMENTS];
	/// Resonance [0-127]
	int32_t ENV_F_RQ[OLD_MAX_INSTRUMENTS];
	/// EnvAmount [-128,128]
	int32_t ENV_F_EA[OLD_MAX_INSTRUMENTS];
	/// psy_dsp_Filter Type. See psycle::helpers::dsp::psy_dsp_FilterType. [0..6]
	int32_t ENV_F_TP[OLD_MAX_INSTRUMENTS];
	///\}

	unsigned char _RPAN[OLD_MAX_INSTRUMENTS];
	unsigned char _RCUT[OLD_MAX_INSTRUMENTS];
	unsigned char _RRES[OLD_MAX_INSTRUMENTS];


	int32_t i;
	int32_t tmpwvsl;
	int32_t pans[OLD_MAX_INSTRUMENTS];
	char names[OLD_MAX_INSTRUMENTS][32];
	int32_t instselected;

	psyfile_read(self->songfile->file, &instselected, sizeof instselected);

	for (i = 0; i < OLD_MAX_INSTRUMENTS; ++i)
	{
		psyfile_read(self->songfile->file, names[i], sizeof(names[0]));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &_NNA[i], sizeof(_NNA[0]));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;
		psyfile_read(self->songfile->file, &tmp, sizeof tmp);
		//Truncate to 220 samples boundaries, and ensure it is not zero.
		tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1;
		ENV_AT[i] = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;
		psyfile_read(self->songfile->file, &tmp, sizeof tmp);
		//Truncate to 220 samples boundaries, and ensure it is not zero.
		tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1;
		ENV_DT[i] = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &ENV_SL[i], sizeof ENV_SL[0]);
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;
		psyfile_read(self->songfile->file, &tmp, sizeof tmp);
		//Truncate to 220 samples boundaries, and ensure it is not zero. (also change default value)
		if (tmp == 16) tmp = 220;
		else { tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1; }
		ENV_RT[i] = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;
		psyfile_read(self->songfile->file, &tmp, sizeof tmp);
		//Truncate to 220 samples boundaries, and ensure it is not zero.
		tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1;
		ENV_F_AT[i] = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;
		psyfile_read(self->songfile->file, &tmp, sizeof tmp);
		//Truncate to 220 samples boundaries, and ensure it is not zero.
		tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1;		
		ENV_F_DT[i] = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &ENV_F_SL[i], sizeof ENV_F_SL[0]);
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;
		psyfile_read(self->songfile->file, &tmp, sizeof tmp);
		//Truncate to 220 samples boundaries, and ensure it is not zero.
		tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1;
		ENV_F_RT[i] = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &ENV_F_CO[i], sizeof ENV_F_CO[0]);
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &ENV_F_RQ[i], sizeof ENV_F_RQ[0]);
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &ENV_F_EA[i], sizeof ENV_F_EA[0]);
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &ENV_F_TP[i], sizeof ENV_F_TP[0]);
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &pans[i], sizeof(pans[0]));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &_RPAN[i], sizeof(_RPAN[0]));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &_RCUT[i], sizeof(_RCUT[0]));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psyfile_read(self->songfile->file, &_RRES[i], sizeof(_RRES[0]));
	}

	// progress.m_Progress.SetPos(4096);
	// ::Sleep(1);
	// Waves
	//

	psyfile_read(self->songfile->file, &tmpwvsl, sizeof(int32_t));

	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t w;

		for (w = 0; w < OLD_MAX_WAVES; w++)
		{
			int32_t wltemp;
			psyfile_read(self->songfile->file, &wltemp, sizeof(wltemp));
			if (wltemp > 0)
			{
				if (w == 0)
				{
					psy_audio_Sample* wave;
					short tmpFineTune;
					char dummy[33];
					unsigned short volume = 0;
					unsigned char stereo = 0;
					unsigned char doloop = 0;
					short* pData;
					uint32_t f;

					wave = psy_audio_sample_allocinit(1);
					//Old format assumed 44Khz
					wave->samplerate = 44100;
					wave->panfactor = (float)pans[i] / 256.f; //(value_mapper::map_256_1(pan));
					//Old wavename, not really used anyway.
					psyfile_read(self->songfile->file, dummy, 32);
					wave->name = strdup(names[i]);
					psyfile_read(self->songfile->file, &volume, sizeof volume);
					wave->globalvolume = volume * 0.01f;
					psyfile_read(self->songfile->file, &tmpFineTune, sizeof(short));
					//Current sample uses 100 cents. Older used +-256
					tmpFineTune = (int32_t)((float)tmpFineTune / 2.56f);
					wave->finetune = tmpFineTune;

					psyfile_read(self->songfile->file, &wave->loop.start, sizeof wave->loop.start);
					psyfile_read(self->songfile->file, &wave->loop.end, sizeof wave->loop.end);

					psyfile_read(self->songfile->file, &doloop, sizeof(doloop));
					wave->loop.type = doloop ? psy_audio_SAMPLE_LOOP_NORMAL : psy_audio_SAMPLE_LOOP_DO_NOT;
					psyfile_read(self->songfile->file, &wave->stereo, sizeof(wave->stereo));

					pData = malloc(wltemp * sizeof(short) + 4);// +4 to avoid any attempt at buffer overflow by the code
					psyfile_read(self->songfile->file, pData, wltemp * sizeof(short));
					wave->numframes = wltemp;
					psy_audio_sample_allocwavedata(wave);
					for (f = 0; f < wave->numframes; ++f) {
						short val = (short)pData[f];
						wave->channels.samples[0][f] = (float)val;
					}
					free(pData);
					pData = 0;					
					if (wave->stereo)
					{
						uint32_t f;
						pData = malloc(wltemp * sizeof(short) + 4);// +4 to avoid any attempt at buffer overflow by the code
						psyfile_read(self->songfile->file, pData, wltemp * sizeof(short));
						psy_audio_sample_resize(wave, 2);
						for (f = 0; f < wave->numframes; ++f) {
							short val = (short)pData[f];
							wave->channels.samples[1][f] = (float)val;
						}
						free(pData);
						pData = 0;						
					}
					psy_audio_samples_insert(&self->songfile->song->samples, wave,
						sampleindex_make(i, 0));
					{
						psy_audio_Instrument* instrument;

						instrument = psy_audio_instrument_allocinit();
						psy_audio_instrument_setname(instrument, names[i]);
						psy_audio_instrument_setindex(instrument, i);
						psy_audio_instruments_insert(&self->songfile->song->instruments,
							instrument, psy_audio_instrumentindex_make(0, i));
						// read envelopes
												// ENV_AT
						psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
							1, ENV_AT[i] *1.f / 44100, 1.f);
						// ENV_DT, ENV_SL
						psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
							2, (ENV_AT[i] + ENV_DT[i]) * 1.f / 44100, ENV_SL[i] / 100.f);
						// ENV_RT
						psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
							3, (ENV_AT[i] + ENV_DT[i] + ENV_RT[i]) * 1.f / 44100, 0.f);

						// ENV_F_AT			
						psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
							1, ENV_AT[i] * 1.f / 44100, 1.f);
						// ENV_DT, ENV_SL
						// note: SL map range(128) differs from volume envelope(100)
						psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
							2, (ENV_AT[i] + ENV_DT[i]) * 1.f / 44100, ENV_SL[i] / 128.f);
						// ENV_RT
						psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
							3, (ENV_AT[i] + ENV_DT[i] + ENV_RT[i]) * 1.f / 44100, 0.f);

						instrument->filtercutoff = ENV_F_CO[i] / 127.f;
						instrument->filterres = ENV_F_RQ[i] / 127.f;
						instrument->filtermodamount = ENV_F_EA[i] / 255.f + 0.5f;
						
						instrument->filtertype = (psy_dsp_FilterType)ENV_F_TP[i];

						instrument->randompanning = (_RPAN[i]) ? 1.f : 0.f;
						instrument->randomcutoff = (_RCUT[i]) ? 1.f : 0.f;
						instrument->_RRES = _RRES[i];
					}
				} else
				{
					unsigned char stereo;

					psyfile_skip(self->songfile->file, 42 + sizeof(unsigned char));
					psyfile_read(self->songfile->file, &stereo, sizeof(unsigned char));
					psyfile_skip(self->songfile->file, wltemp);
					if (stereo) {
						psyfile_skip(self->songfile->file, wltemp);
					}
				}
			}
		}
	}
}

void psy2loader_readvsts(PSY2Loader* self)
{
	int32_t i;

	for (i = 0; i < OLD_MAX_PLUGINS; ++i) {
		psyfile_read(self->songfile->file, &self->vstL[i].valid,
			sizeof(self->vstL[i].valid));
		if (self->vstL[i].valid) {
			int32_t c;

			psyfile_read(self->songfile->file, self->vstL[i].dllName,
				sizeof(self->vstL[i].dllName));
			psy_strlwr(self->vstL[i].dllName);
			psyfile_read(self->songfile->file, &(self->vstL[i].numpars), sizeof(int32_t));
			self->vstL[i].pars = malloc(sizeof(float) * self->vstL[i].numpars);
			for (c=0; c< self->vstL[i].numpars; c++) {
				psyfile_read(self->songfile->file,
					&(self->vstL[i].pars[c]), sizeof(float));
			}
		}
	}
}

void psy2loader_readmachines(PSY2Loader* self)
{	
	int32_t i;
		
	psyfile_read(self->songfile->file, &self->_machineActive[0], sizeof(self->_machineActive));
	memset(self->pMac, 0, sizeof(self->pMac));
	for (i = 0; i < 128; ++i) {
		if (i == 20) {
			self = self;
		}
		if (self->_machineActive[i]) {
			int32_t type;
			int32_t x;
			int32_t y;
			psy_audio_MachineFactory* factory;

			factory = self->songfile->song->machinefactory;

			psyfile_read(self->songfile->file, &x, sizeof(x));
			psyfile_read(self->songfile->file, &y, sizeof(y));
			psyfile_read(self->songfile->file, &type, sizeof(type));

			if (internalmachinesconvert_pluginname_exists(&self->converter, type, "")) {
				self->pMac[i] = internalmachinesconvert_redirect(&self->converter,
					self->songfile, &i, type, "");
			} else switch (type) {
				case MACH_MASTER:
					self->pMac[i] = psy_audio_machinefactory_makemachine(factory, MACH_MASTER, "", UINTPTR_MAX);
					// psy_audio_machines_insert(&songfile->song->machines, psy_audio_MASTER_INDEX, pMac[i]);
					psy2loader_master_load(self, self->pMac[i], i);
					break;
				case MACH_SAMPLER: {
					self->pMac[i] = psy_audio_machinefactory_makemachine(factory, MACH_SAMPLER, "", UINTPTR_MAX);
					psy2loader_sampler_load(self, self->pMac[i], i);
					break; }
				case MACH_XMSAMPLER:
					assert(0);
					//pMac[i] = pXMSampler = new XMSampler(i);
					//pMac[i]->Init();
					//pMac[i]->Load(pFile);
					break;
				case MACH_PLUGIN: {
					char sDllName[256];
					char plugincatchername[_MAX_PATH];

					// Plugin dll name
					psyfile_read(self->songfile->file, sDllName, sizeof(sDllName));
					psy_strlwr(sDllName);
					if (internalmachinesconvert_pluginname_exists(&self->converter, type, sDllName)) {
						self->pMac[i] = internalmachinesconvert_redirect(&self->converter,
							self->songfile, &i, type, sDllName);
					} else {
						psy_audio_plugincatcher_catchername(self->songfile->song->machinefactory->catcher,
							sDllName, plugincatchername, 0);
						self->pMac[i] = psy_audio_machinefactory_makemachine(factory, MACH_PLUGIN, plugincatchername, UINTPTR_MAX);
						if (self->pMac[i]) {
							psy2loader_plugin_load(self, self->pMac[i], i);
						} else {
							self->pMac[i] = psy_audio_machinefactory_makemachine(factory, MACH_DUMMY, plugincatchername, UINTPTR_MAX);
							psy_audio_machine_setslot(self->pMac[i], i);
							psy2loader_plugin_skipload(self, self->pMac[i], i);
							psy_audio_songfile_warn(self->songfile, "replaced missing module ");
							psy_audio_songfile_warn(self->songfile, sDllName);
							psy_audio_songfile_warn(self->songfile, " aka ");
							psy_audio_songfile_warn(self->songfile, psy_audio_machine_editname(self->pMac[i]));
							psy_audio_songfile_warn(self->songfile, " with dummy-plug\n");
							// Warning: It cannot be known if the missing plugin is a generator
							// or an effect. This will be guessed from the busMachine array.
						}
					}
					break; }
				case MACH_VST:
				case MACH_VSTFX: {				
					char plugincatchername[_MAX_PATH];
					char sError[128];
					bool berror;
					psy_audio_Machine* pVstPlugin;
					unsigned char program;
					int32_t instance;
					VstPreload vstpreload;					
					int32_t shellIdx;
				
					berror = FALSE;
					shellIdx = 0;
					pVstPlugin = NULL;				
					// The trick: We need to load the information from the file in order to know the "instance" number
					// and be able to create a plugin from the corresponding dll. Later, we will set the loaded settings to
					// the newly created plugin.
					vstpreload_init(&vstpreload, self->songfile);
					vstpreload_load(&vstpreload, i, &program, &instance);			
					assert(instance < OLD_MAX_PLUGINS);	
					if ((!self->vstL[instance].valid)) {
						berror = TRUE;
						sprintf(sError, "VST plug-in missing, or erroneous data in song file \"%s\"",
							self->vstL[instance].dllName);
						plugincatchername[0] = '\0';
						psy_audio_songfile_warn(self->songfile, sError);
						psy_audio_songfile_warn(self->songfile, "\n");
					} else {		
						psy_audio_plugincatcher_catchername(self->songfile->song->machinefactory->catcher,
							self->vstL[instance].dllName, plugincatchername, shellIdx);
						pVstPlugin = psy_audio_machinefactory_makemachine(factory, MACH_PLUGIN, plugincatchername, UINTPTR_MAX);
						if (pVstPlugin) {
							int c;
							int numpars;

							psy_audio_machine_setcurrprogram(pVstPlugin, program);
							psy_audio_machine_setslot(pVstPlugin, i);
							numpars = self->vstL[instance].numpars;
							for (c = 0; c < numpars; ++c) {
								psy_audio_MachineParam* param;

								param = psy_audio_machine_tweakparameter(pVstPlugin, c);
								if (param) {
									psy_audio_machine_parameter_tweak(pVstPlugin,
										param, self->vstL[instance].pars[c]);
								}
							}
						} else {
							berror = TRUE;
							sprintf(sError, "Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.", plugincatchername);
							psy_audio_songfile_warn(self->songfile, sError);
							psy_audio_songfile_warn(self->songfile, "\n");
						}
					}
					if (berror) {
						self->pMac[i] = psy_audio_machinefactory_makemachine(factory, MACH_DUMMY, plugincatchername, UINTPTR_MAX);
						psy_audio_machine_setslot(self->pMac[i], i);
						psy_audio_machine_seteditname(self->pMac[i], plugincatchername);
						// todo set mode
						// self->pMac[i]->_mode = MACHMODE_FX;						
						// self->pMac[i]->_mode = MACHMODE_GENERATOR;						
					}
					break;
				}
				default: {
					char sError[128];
					sprintf(sError, "unknown machine type: %i", type);
					self->pMac[i] = psy_audio_machinefactory_makemachine(factory, MACH_DUMMY, sError, UINTPTR_MAX);
					psy_audio_machine_setslot(self->pMac[i], i);
					psy2loader_plugin_skipload(self, self->pMac[i], i);
					psy_audio_songfile_warn(self->songfile, sError);
					psy_audio_songfile_warn(self->songfile, "\n");
					break; }
			}
			assert(self->pMac[i]);
			if (self->pMac[i]) {
				psy_audio_machine_setposition(self->pMac[i], x, y);
			}
		}
	}	
	// Patch 0: Some extra data added around the 1.0 release.
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		uint8_t _loop;
		psyfile_read(self->songfile->file, &_loop, sizeof(_loop));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t _lines;
		psyfile_read(self->songfile->file, &_lines, sizeof(_lines));
	}
}

void psy2loader_addmachines(PSY2Loader* self)
{
	int32_t i;

	// Validate the machine arrays. 
	for (i = 0; i < 128; ++i) // First, we add the output volumes to a Matrix for reference later
	{
		if (!self->_machineActive[i])
		{
			if (self->pMac[i]) {
				psy_audio_machine_dispose(self->pMac[i]);
				free(self->pMac[i]);
				self->pMac[i] = 0;
			}
		} else if (!self->pMac[i])
		{
			self->_machineActive[i] = FALSE;
		}
	}

	// Patch 1: BusEffects (twf). Try to read it, and if it doesn't exist, generate it.
	// progress.m_Progress.SetPos(8192 + 4096);
	// ::Sleep(1);
	if (psyfile_read(self->songfile->file, &self->busEffect[0],
		sizeof(self->busEffect)) == 0)
	{
		int j = 0;
		int i = 0;
		unsigned char invmach[128];
		memset(invmach, 255, sizeof(invmach));
		// The guessing procedure does not rely on the machmode because if a plugin
		// is missing, then it is always tagged as a generator.
		for (i = 0; i < 64; i++)
		{
			if (self->busMachine[i] < 128 && self->busMachine[i] != 255) {
				invmach[self->busMachine[i]] = i;
			}
		}
		for (i = 1; i < 128; i++) // machine 0 is the Master machine.
		{
			if (self->_machineActive[i])
			{
				if (invmach[i] == 255)
				{
					self->busEffect[j] = i;
					j++;
				}
			}
		}
		while (j < 64)
		{
			self->busEffect[j] = 255;
			j++;
		}
	}

	// Validate that there isn't any duplicated machine.
	for (i = 0; i < 64; i++)
	{
		int32_t j;

		for (j = i + 1; j < 64; j++)
		{
			if (self->busMachine[i] == self->busMachine[j]) {
				self->busMachine[j] = 255;
			}
			if (self->busEffect[i] == self->busEffect[j]) {
				self->busEffect[j] = 255;
			}
		}
		for (j = 0; j < 64; j++)
		{
			if (self->busMachine[i] == self->busEffect[j]) {
				self->busEffect[j] = 255;
			}
		}
	}

	// Patch 1.2: Fixes erroneous machine mode when a dummy replaces a bad plugin
	// (missing dll, or when the load process failed).
	// At the same time, we validate the indexes of the busMachine and busEffects arrays.
	for (i = 0; i < 64; i++)
	{
		if (self->busEffect[i])
		{
			if (self->busEffect[i] > 128 || (self->busEffect[i] < 128 &&
					!self->_machineActive[self->busEffect[i]]))
				self->busEffect[i] = 255;
			// If there's a dummy, force it to be an effect
			else
				if (self->busEffect[i] < 128 && self->pMac[self->busEffect[i]] &&
					psy_audio_machine_type(self->pMac[self->busEffect[i]]) == MACH_DUMMY)
				{
					// pMac[busEffect[i]]->_mode = MACHMODE_FX;
				}
			// Else if the machine is a generator, move it to gens bus.
			// This can't happen, but it is here for completeness
				else if (self->busEffect[i] < 128 && self->pMac[self->busEffect[i]] &&
					psy_audio_machine_mode(self->pMac[self->busEffect[i]]) == MACHMODE_GENERATOR)
				{
					int k = 0;
					while (self->busEffect[k] != 255 && k < MAX_BUSES)
					{
						k++;
					}
					self->busMachine[k] = self->busEffect[i];
					self->busEffect[i] = 255;
				}
		}
		if (self->busMachine[i] != 255)
		{
			if (self->busMachine[i] > 128 || (self->busMachine[i] < 128 &&
					!self->_machineActive[self->busMachine[i]]))
				self->busMachine[i] = 255;
			// If there's a dummy, force it to be a Generator
			else if (psy_audio_machine_type(self->pMac[self->busMachine[i]]) == MACH_DUMMY)
			{
				// pMac[busMachine[i]]->_mode = MACHMODE_GENERATOR;
			}
			// Else if the machine is an fx, move it to FXs bus.
			// This can't happen, but it is here for completeness
			else if (self->busEffect[i] < 128 && self->pMac[self->busEffect[i]] &&
				psy_audio_machine_mode(self->pMac[self->busMachine[i]]) != MACHMODE_GENERATOR)
			{
				int j = 0;
				while (self->busEffect[j] != 255 && j < MAX_BUSES)
				{
					j++;
				}
				self->busEffect[j] = self->busMachine[i];
				self->busMachine[i] = 255;
			}
		}
	}


	{
		// Psycle no longer uses busMachine and busEffect, since the pMachine Array directly maps
		// to the real machine.
		// Due to this, we have to move machines to where they really are, 
		// and remap the inputs and outputs indexes again... ouch
		// At the same time, we validate each wire.
		// progress.m_Progress.SetPos(8192 + 4096 + 2048 + 1024);
		//::Sleep(1);
		unsigned char invmach[128];
		int j = 0;
		int k = 64;

		memset(invmach, 255, sizeof(invmach));
		for (i = 0; i < 64; i++)
		{
			if (self->busMachine[i] < 128 && self->busMachine[i] != 255) invmach[self->busMachine[i]] = i;
			if (self->busEffect[i] < 128 && self->busEffect[i] != 255) invmach[self->busEffect[i]] = i + 64;
		}
		invmach[0] = psy_audio_MASTER_INDEX;

		
		for (i = 0; i < 128; i++)
		{
			if (invmach[i] != 255)
			{				
				self->_machineActive[i] = FALSE; // mark as "converted"
				psy_audio_machines_insert(&self->songfile->song->machines, invmach[i], self->pMac[i]);
			}
		}
		// verify that there isn't any machine that hasn't been copied into _pMachine
		// Shouldn't happen. It would mean a damaged file.			
		for (i = 0; i < 128; i++)
		{
			if (self->_machineActive[i])
			{
				if (psy_audio_machine_mode(self->pMac[i]) == MACHMODE_GENERATOR)
				{
					while (psy_audio_machines_at(&self->songfile->song->machines, j) && j < 64) j++;

					invmach[i] = j;
					psy_audio_machines_insert(&self->songfile->song->machines, j,
						self->pMac[i]);
					//_pMachine[j] = pMac[i];
				} else
				{
					while (psy_audio_machines_at(&self->songfile->song->machines, k) && k < 128) k++;
					invmach[i] = k;
					//_pMachine[k] = pMac[i];
					psy_audio_machines_insert(&self->songfile->song->machines, k,
						self->pMac[i]);
				}
			}
		}
		
		{
			// Creates Remapped Wires for machine postload and adjust wire volume			
			psy_TableIterator it;

			for (it = psy_table_begin(&self->legacywires.legacywires);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_MachineWires* wires;
				psy_audio_MachineWires* wiresremapped;
				psy_TableIterator j;
				uintptr_t mac_id;
				uintptr_t macremapped_id;

				wires = psy_tableiterator_value(&it);
				wiresremapped = psy_audio_machinewires_clone(wires);
				mac_id = psy_tableiterator_key(&it);				
				macremapped_id = invmach[mac_id];
				psy_audio_legacywires_insert(&self->legacywiresremapped,
					macremapped_id,
					wiresremapped);				
				for (j = psy_table_begin(wiresremapped);
					!psy_tableiterator_equal(&j, psy_table_end());
					psy_tableiterator_inc(&j)) {
					psy_audio_LegacyWire* wireremapped;					
					
					wireremapped = (psy_audio_LegacyWire*)psy_tableiterator_value(&j);
					if (wireremapped->_inputCon && wireremapped->_inputMachine > -1 && wireremapped->_inputMachine < 128
						&& invmach[wireremapped->_inputMachine] != 255) {
						float val;

						wireremapped->_inputMachine = invmach[wireremapped->_inputMachine];						
						val = wireremapped->_inputConVol;
						if (val > 4.1f)
						{
							val *= 0.000030517578125f; // BugFix
						} else if (val < 0.00004f)
						{
							val *= 32768.0f; // BugFix
						}
						wireremapped->_inputConVol = val;
						// and set the volume.
						psy_audio_machines_connect(&self->songfile->song->machines,
							psy_audio_wire_make(wireremapped->_inputMachine, macremapped_id));
						psy_audio_connections_setwirevolume(&self->songfile->song->machines.connections,
							psy_audio_wire_make(wireremapped->_inputMachine, macremapped_id),
							val * wireremapped->_wireMultiplier);
						psy_audio_connections_setpinmapping(&self->songfile->song->machines.connections,
							psy_audio_wire_make(wireremapped->_inputMachine, macremapped_id),
							&wireremapped->pinmapping);									
					}
					if (wireremapped->_connection) {
						if (wireremapped->_outputMachine > 0 && wireremapped->_outputMachine < 128) {
							wireremapped->_outputMachine = invmach[wireremapped->_outputMachine];
						} else {
							wireremapped->_outputMachine = psy_audio_MASTER_INDEX;
						}
					}
				}
			}
			self->songfile->legacywires = &self->legacywiresremapped;
		}		
		// todo samplerate
		internalmachineconverter_retweak_song(&self->converter, self->songfile->song, 44100.0);
	}
}

// old file format machine loaders
void psy2loader_machine_load(PSY2Loader* self, uintptr_t slot)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	psy_audio_MachineWires* machinewires;

	psyfile_read(self->songfile->file, _editName, 16);
	_editName[15] = 0;	
	
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);
	
	psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	psyfile_read(self->songfile->file, &panning, sizeof(panning));
	// Machine::SetPan(_panning);
	psyfile_skip(self->songfile->file, 109);
}

void psy2loader_master_load(PSY2Loader* self, psy_audio_Machine* master, uintptr_t slot)
{
	char editname[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t _outDry;
	psy_audio_MachineWires* machinewires;

	psyfile_read(self->songfile->file, editname, 16);
	editname[15] = 0;
	
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);

	psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	psyfile_read(self->songfile->file, &panning, sizeof(panning));
	// Machine::SetPan(_panning);
	psyfile_skip(self->songfile->file, 40);

	// outdry
	psyfile_read(self->songfile->file, &_outDry, sizeof(int));
	{
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(master, 0);
		if (param) {
			psy_audio_machine_parameter_tweak(master,
				param, (float)sqrt(_outDry / 256.f / 4.f));
		}
	}
	psyfile_skip(self->songfile->file, 65);
}

void psy2loader_sampler_load(PSY2Loader* self, psy_audio_Machine* machine,
	uintptr_t slot)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t _numVoices;
	int32_t interpol;	
	psy_audio_MachineParam* param;
	psy_audio_MachineWires* machinewires;

	psyfile_read(self->songfile->file, _editName, 16);
	_editName[15] = 0;
	
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);

	psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	psyfile_read(self->songfile->file, &panning, sizeof(panning));
	psyfile_skip(self->songfile->file, 8 * sizeof(int)); // SubTrack[]
	psyfile_read(self->songfile->file, &_numVoices, sizeof(_numVoices)); // numSubtracks

	if (_numVoices < 4)
	{
		// Psycle versions < 1.1b2 had polyphony per channel,not per machine.
		_numVoices = 8;
	}
	psyfile_read(self->songfile->file, &interpol, sizeof(int32_t)); // interpol		
	psyfile_skip(self->songfile->file, 69);
	// Num Voices
	param = psy_audio_machine_tweakparameter(machine, 0);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(machine, param, _numVoices);
	}
	// Quality
	param = psy_audio_machine_tweakparameter(machine, 1);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(machine, param, interpol);
	}
	// Default Speed (C3)
	param = psy_audio_machine_tweakparameter(machine, 2);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
	}
}

void psy2loader_plugin_load(PSY2Loader* self, psy_audio_Machine* machine, uintptr_t slot)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t numParameters;
	int* Vals;
	uintptr_t i;
	psy_audio_MachineWires* machinewires;

	assert(machine);
	psyfile_read(self->songfile->file, _editName, 16);
	_editName[15] = 0;
	psy_audio_machine_seteditname(machine, _editName);
	psyfile_read(self->songfile->file, &numParameters, sizeof(numParameters));
	if (numParameters > 0) {
		Vals = malloc(sizeof(int32_t) * numParameters);		
		psyfile_read(self->songfile->file, Vals, numParameters * sizeof(int));
		for (i = 0; i < (uintptr_t)numParameters; ++i) {
			psy_audio_MachineParam* param;

			param = psy_audio_machine_parameter(machine, i);
			if (param) {
				psy_audio_machine_parameter_tweak_scaled(machine, param, Vals[i]);
			}
		}
	} else {
		Vals = NULL;
	}
	
	{
		uint32_t size;
		
		size = (uint32_t)psy_audio_machine_datasize(machine);
		//pFile->Read(&size,sizeof(int));	// This SHOULD be the right thing to do
		if (size)
		{
			uint8_t* pData;
			
			pData = (uint8_t*)malloc(size);
			psyfile_read(self->songfile->file, pData, size); // Number of parameters
			psy_audio_machine_putdata(machine, pData); // Internal load
			free(pData);
			pData = 0;
		}
	}	
	free(Vals);
	Vals = 0;
	
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);	

	psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	psyfile_read(self->songfile->file, &panning, sizeof(panning));
	psy_audio_machine_setpanning(machine, panning / 128.f);
	psyfile_skip(self->songfile->file, 109);
}

void psy2loader_plugin_skipload(PSY2Loader* self, psy_audio_Machine* machine, uintptr_t slot)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t numParameters;
	psy_audio_MachineWires* machinewires;

	assert(self && machine);

	psyfile_read(self->songfile->file, _editName, 16);
	_editName[15] = 0;
	psy_audio_machine_seteditname(machine, _editName);
	psy_audio_machine_setslot(machine, slot);
	
	psyfile_read(self->songfile->file, &numParameters, sizeof(numParameters));
	psyfile_skip(self->songfile->file, numParameters * sizeof(int32_t));
	/* This SHOULD be done, but it breaks the fileformat.
		int size;
		pFile->Read(&size,sizeof(int));
		if (size)
		{
			pFile->Skip(size);
	}*/
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);
	psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	psyfile_read(self->songfile->file, &panning, sizeof(panning));
	psyfile_skip(self->songfile->file, 109);
}

void vstpreload_init(VstPreload* self, psy_audio_SongFile* songfile)
{
	self->songfile = songfile;
}

bool vstpreload_load(VstPreload* self, uintptr_t slot, unsigned char* _program, int32_t* _instance)
{
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	uint8_t old;
	psy_audio_MachineWires* machinewires;

	psyfile_read(self->songfile->file, self->_editName, 16);
	self->_editName[15] = '\0';
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);

	psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint));
	// numInputs and numOutputs
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	psyfile_read(self->songfile->file, &panning, sizeof(panning));
	psyfile_skip(self->songfile->file, 109);

	
	psyfile_read(self->songfile->file, &old, sizeof old); // old format
	psyfile_read(self->songfile->file, _instance, sizeof *_instance); // ovst.instance
	if (old)
	{
		char mch;
		psyfile_read(self->songfile->file, &mch, sizeof mch);
		_program = 0;
	} else
	{
		psyfile_read(self->songfile->file, &_program, sizeof _program);
	}
	return TRUE;
}

psy_audio_MachineWires* psy_audio_read_psy2machinewires(PsyFile* file)
{
	psy_audio_MachineWires* rv;
	uintptr_t c; // current legacywire (connection) id
	
	assert(file);

	// Create machinewires with PSY2_MAX_CONNECTIONS
	rv = psy_audio_machinewires_allocinit();
	assert(rv);
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		psy_audio_machinewires_insert(rv, c,
			psy_audio_legacywire_allocinit());
	}
	// Read connections
	// Incoming connections Machine number
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		int32_t input;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &input, sizeof(input));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_inputMachine = input;
		}
	}
	// Outgoing connections Machine number
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		int32_t output;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &output, sizeof(output));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_outputMachine = output;
		}
	}
	// Incoming connections Machine vol
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		float inputconvol;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &inputconvol, sizeof(inputconvol));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_inputConVol = inputconvol;
		}
	}
	// Outgoing connections activated
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		uint8_t connection;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &connection, sizeof(connection));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_connection = connection;
		}
	}
	// Incoming connections activated
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		uint8_t inputcon;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &inputcon, sizeof(inputcon));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_inputCon = inputcon;
		}
	}
	return rv;
}
