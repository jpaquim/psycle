/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "psy2.h"
/* local */
#include "machinefactory.h"
#include "pattern.h"
#include "plugin.h"
#include "plugin_interface.h"
#include "psyconvert.h"
#include "song.h"
#include "songio.h"
#include "vstplugin.h"
#include "wire.h"
/* dsp */
#include <operations.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/os.h"
#include "../../detail/cpu.h"

#define PSY2_EVENT_SIZE 5
#define PSY2_MAX_CONNECTIONS 12

#if defined DIVERSALIS__COMPILER__GNU || defined DIVERSALIS__OS__POSIX
#define _MAX_PATH 4096
#endif

typedef struct cpoint_t {
	int32_t x;
	int32_t y;
} cpoint_t;

static void vstpreload_init(VstPreload*, psy_audio_SongFile*);
static int vstpreload_load(VstPreload*, uintptr_t slot,
	unsigned char* program, int32_t* instance);

/* prototypes */
static int psy2loader_readheader(PSY2Loader*);
static int psy2loader_readsngi(PSY2Loader*);
static int psy2loader_readpatterns(PSY2Loader*);
static int psy2loader_readsequence(PSY2Loader*);
static int psy2loader_readinstruments(PSY2Loader*);
static int psy2loader_readvsts(PSY2Loader*);
static int psy2loader_readmachines(PSY2Loader*);
static int psy2loader_addmachines(PSY2Loader*);
static int psy2loader_machine_load(PSY2Loader*, psy_audio_Machine*, uintptr_t slot);
static int psy2loader_master_load(PSY2Loader*, psy_audio_Machine*,
	uintptr_t slot);
static int psy2loader_sampler_load(PSY2Loader*, psy_audio_Machine*,
	uintptr_t slot);
static int psy2loader_plugin_load(PSY2Loader*, psy_audio_Machine*,
	uintptr_t slot, const char* plugincatchername);
static int psy2loader_plugin_skipload(PSY2Loader*, psy_audio_Machine*,
	uintptr_t slot);
static int psy2loader_postload(PSY2Loader*);
/* implementation */
void psy2loader_init(PSY2Loader* self, psy_audio_SongFile* songfile)
{
	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
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

int psy2loader_load(PSY2Loader* self)
{
	int status;

	self->songfile->legacywires = &self->legacywires;
	self->fp = self->fp;
	self->song = self->song;
	if (status = psy2loader_readheader(self)) {
		return status;
	}
	if (status = psy2loader_readsngi(self)) {
		return status;
	}
	if (status = psy2loader_readpatterns(self)) {
		return status;
	}
	if (status = psy2loader_readsequence(self)) {
		return status;
	}
	if (status = psy2loader_readinstruments(self)) {
		return status;
	}
	if (status = psy2loader_readvsts(self)) {
		return status;
	}
	if (status = psy2loader_readmachines(self)) {
		return status;
	}
	if (status = psy2loader_addmachines(self)) {
		return status;
	}
	if (status = psy2loader_postload(self)) {
		return status;
	}
	return PSY_OK;
}

/*
** Header
** |-|0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
** |0|P|S|Y|2|S|O|N|G|n|a|m|e| | | | |
** |1| | | | | | | | | | | | | | | | |
** |2| | | | | | | | |a|u|t|h|o|r| | |
** |3| | | | | | | | | | | | | | | | |
** |4| | | | | | | | |c|o|m|m|e|n|t| |
** |5| | | | | | | | | | | | | | | | |
** |6| | | | | | | | | | | | | | | | |
** |7| | | | | | | | | | | | | | | | |
** |8| | | | | | | | | | | | | | | | |
** |9| | | | | | | | | | | | | | | | |
** |A| | | | | | | | | | | | | | | | |
** |B| | | | | | | | | | | | | | | | |
** |C| | | | | | | | | | | | | | | | |
** |D| | | | | | | | |b|p|m| |s|p|t| |
** |E|o|
*/
int psy2loader_readheader(PSY2Loader* self)
{
	int status;
	/* Can be NOT NULL terminated. (specsize + 1 for NULL termination) */
	char name[33];
	char author[33];
	char comments[129];
	int32_t beatspermin;
	int32_t sampR;
	int32_t linesperbeat;
	psy_audio_SongProperties songproperties;

	/*
	** PSY2SONG, the Sign of the file, already read by songio,
	** so it isn't done here again
	*/

	/*
	** Label: name
	** Size : 32chars.
	*/
	if (status = psyfile_read(self->fp, name, 32)) {
		return status;
	}
	/* Can be NOT NULL terminated. */
	name[32] = '\0';
	/*
	** Label: author
	** Size : 32chars.
	*/
	if (status = psyfile_read(self->fp, author, 32)) {
		return status;
	}
	/* Can be NOT NULL terminated. */
	author[32] = '\0';
	/*
	** Label: comment
	** Size : 128chars.
	*/
	if (status = psyfile_read(self->fp, comments, 128)) {
		return status;
	}
	/* Can be NOT NULL terminated. */
	comments[128] = '\0';
	/*
	** Label: bpm
	** Size : 1int(32bits)
	** Desc:  Contains The song BPM
	*/
	if (status = psyfile_read(self->fp, &beatspermin, sizeof beatspermin)) {
		return status;
	}
	/*
	** Label: spt
	** Size : 1int(32bits)
	** Desc : Contains The song samples per tick.This is a measure used
	**        internally by the player, but thanks to it, this makes possible
	**        to store the TPB that the format didn’t support.
	**        To get the TPB, use :
    ** 	      ticksPerBeat = 44100 * 15 * 4 / (SamplesPerTick * BeatsPerMin);
	**        Next, Update your SamplesPerTick value as :
	**        SamplesPerTick = SamplesPerTick * (currentSamplesPerSec / 44100);
	** This is done because the original fileformat was hardcoded at 44100Khz.
	** Do the opposite when(if) saving!!!
	**
	** Advise.If, for any case, the SamplesPerTick is under or equal to 0,
	** default to these values : _ticksPerBeat = 4; SamplesPerTick = 4315;
	** (One version did damage this value)
	*/
	if (status = psyfile_read(self->fp, &sampR, sizeof sampR)) {
		return status;
	}
	if (sampR <= 0) {
		/* Shouldn't happen but has happened. */
		linesperbeat = 4;
	} else {
		/* The old format assumes we output at 44100 samples/sec, so... */
		linesperbeat = (int32_t)(44100 * 60 / (sampR * beatspermin));
	}	
	psy_audio_songproperties_init_all(&songproperties,
		name,
		author,
		comments,		
		4,  /* octave */
		linesperbeat,
		24, /* extraticksperbeat */
		0,
		beatspermin);
	psy_audio_song_setproperties(self->song, &songproperties);
	psy_audio_songproperties_dispose(&songproperties);
	return PSY_OK;
}


int psy2loader_readsngi(PSY2Loader* self)
{
	int status;

	if (status = psyfile_read(self->fp, &self->currentoctave,
			sizeof(char))) {
		return status;
	}
	if (status = psyfile_read(self->fp, self->busMachine, 64)) {
		return status;
	}
	if (status = psyfile_read(self->fp, self->playorder, 128)) {
		return status;
	}
	if (status = psyfile_read(self->fp, &self->playlength,
			sizeof(int32_t))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &self->songtracks,
			sizeof(int32_t))) {
		return status;
	}
	return PSY_OK;
}

int psy2loader_readpatterns(PSY2Loader* self)
{
	int status;
	int32_t num;
	int32_t i;

	if (status = psyfile_read(self->fp, &num, sizeof num)) {
		return status;
	}
	for (i = 0; i < num; ++i) {
		int32_t numlines;
		char patternName[32];

		if (status = psyfile_read(self->fp, &numlines,
				sizeof(numlines))) {
			return status;
		}
		if (status = psyfile_read(self->fp, patternName,
				sizeof(patternName))) {
			return status;
		}
		if (numlines > 0)
		{
			/* \todo: tweak_effect should be converted to normal tweaks! */			
			unsigned char* pData;			
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();
			psy_audio_patterns_insert(&self->song->patterns, i, pattern);
			pData = malloc(numlines * OLD_MAX_TRACKS * PSY2_EVENT_SIZE);
			if (status = psyfile_read(self->fp, pData,
					numlines * OLD_MAX_TRACKS * PSY2_EVENT_SIZE)) {
				free(pData);
				pData = NULL;
				return status;
			}
			psy_audio_convert_legacypattern(pattern, pData, OLD_MAX_TRACKS,
				numlines, (uint32_t)psy_audio_song_lpb(self->song));
			free(pData);
			pData = NULL;
		} else {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();
			psy_audio_patterns_insert(&self->song->patterns, i, pattern);
			pattern->length = 64 * 1.0 / psy_audio_song_lpb(self->song);;
		}
	}
	return PSY_OK;
}

int psy2loader_readsequence(PSY2Loader* self)
{
	int32_t i;
	
	psy_audio_sequence_appendtrack(&self->song->sequence,
		psy_audio_sequencetrack_allocinit());
	for (i = 0; i < self->playlength; ++i) {
		psy_audio_sequence_insert(&self->song->sequence,
			psy_audio_orderindex_make(0, i), self->playorder[i]);
	}
	return PSY_OK;
}

int psy2loader_readinstruments(PSY2Loader* self)
{
	int status;
	psy_audio_LegacyInstrument legacy_instrs[OLD_MAX_INSTRUMENTS];	
	int32_t i;
	int32_t tmpwvsl;
	int32_t pans[OLD_MAX_INSTRUMENTS];
	char names[OLD_MAX_INSTRUMENTS][32];
	int32_t instselected;

	if (status = psyfile_read(self->fp, &instselected, sizeof instselected)) {
		return status;
	}

	for (i = 0; i < OLD_MAX_INSTRUMENTS; ++i)
	{
		if (status = psyfile_read(self->fp, names[i], sizeof(names[0]))) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		psy_audio_legacyinstrument_init(&legacy_instrs[i]);
		if (status = psyfile_read(self->fp, &legacy_instrs[i]._NNA,
				sizeof(legacy_instrs[0]._NNA))) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_AT,
				sizeof legacy_instrs[0].ENV_AT)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_DT,
			sizeof legacy_instrs[0].ENV_DT)) {
			return status;
		}
	}

	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_SL,
				sizeof legacy_instrs[0].ENV_SL)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t tmp;

		if (status = psyfile_read(self->fp, &tmp, sizeof tmp)) {
			return status;
		}
		/*
		** Truncate to 220 samples boundaries, and ensure it is not zero.
		** (also change default value)
		*/
		if (tmp == 16) tmp = 220;
		else { tmp = (tmp / 220) * 220; if (tmp <= 0) tmp = 1; }
		legacy_instrs[i].ENV_RT = tmp;
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_AT,
				sizeof legacy_instrs[0].ENV_F_AT)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_DT,
			sizeof legacy_instrs[0].ENV_F_DT)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_SL,
				sizeof legacy_instrs[0].ENV_F_SL)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{		
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_RT,
				sizeof legacy_instrs[0].ENV_F_RT)) {
			return status;
		}		
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_CO,
				sizeof legacy_instrs[0].ENV_F_CO)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_RQ,
				sizeof legacy_instrs[0].ENV_F_RQ)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_EA,
				sizeof legacy_instrs[0].ENV_F_EA)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i].ENV_F_TP,
				sizeof legacy_instrs[0].ENV_F_TP)) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &pans[i], sizeof(pans[0]))) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i]._RPAN,
				sizeof(legacy_instrs[0]._RPAN))) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i]._RCUT,
				sizeof(legacy_instrs[0]._RCUT))) {
			return status;
		}
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		if (status = psyfile_read(self->fp, &legacy_instrs[i]._RRES,
				sizeof(legacy_instrs[0]._RRES))) {
			return status;
		}
	}

	/*
	progress.m_Progress.SetPos(4096);
	::Sleep(1);
	 Waves
	*/

	psyfile_read(self->fp, &tmpwvsl, sizeof(int32_t));

	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t w;

		for (w = 0; w < OLD_MAX_WAVES; w++)
		{
			int32_t wltemp;
			psyfile_read(self->fp, &wltemp, sizeof(wltemp));
			if (wltemp > 0)
			{
				if (w == 0)
				{
					psy_audio_Sample* wave;
					int16_t tmpFineTune;
					char dummy[33];
					uint16_t volume = 0;
					uint8_t stereo = 0;
					uint8_t doloop = 0;
					int16_t* pData;
					uint32_t f;
					uint8_t temp8u;
					uint32_t temp32u;					

					wave = psy_audio_sample_allocinit(1);
					/* Old format assumed 44Khz */
					wave->samplerate = 44100;
					wave->panfactor = (float)pans[i] / 256.f; /* (value_mapper::map_256_1(pan)); */
					/* Old wavename, not really used anyway. */
					psyfile_read(self->fp, dummy, 32);
					wave->name = strdup(names[i]);
					psyfile_read(self->fp, &volume, sizeof volume);
					wave->globalvolume = volume * 0.01f;
					psyfile_read(self->fp, &tmpFineTune, sizeof(short));
					/* Current sample uses 100 cents. Older used +-256 */
					tmpFineTune = (int32_t)((float)tmpFineTune / 2.56f);
					wave->zone.finetune = tmpFineTune;

					psyfile_read(self->fp, &temp32u, sizeof(uint32_t));
					wave->loop.start = temp32u;
					psyfile_read(self->fp, &temp32u, sizeof(uint32_t));
					wave->loop.end = temp32u;
					psyfile_read(self->fp, &doloop, sizeof(doloop));
					wave->loop.type = doloop ? psy_audio_SAMPLE_LOOP_NORMAL : psy_audio_SAMPLE_LOOP_DO_NOT;
					psyfile_read(self->fp, &temp8u, sizeof(uint8_t));
					wave->stereo = temp8u;
					pData = malloc(wltemp * sizeof(int16_t) + 4); /* +4 to avoid any attempt at buffer overflow by the code */
					if (!pData) {
						return PSY_ERRFILE;
					}
					psyfile_read(self->fp, pData, wltemp * sizeof(int16_t));
					wave->numframes = wltemp;
					psy_audio_sample_allocwavedata(wave);
					for (f = 0; f < wave->numframes; ++f) {
						int16_t val;
						
						val = (int16_t)pData[f];
						wave->channels.samples[0][f] = (float)val;
					}
					free(pData);
					pData = 0;					
					if (wave->stereo)
					{
						uint32_t f;
						pData = malloc(wltemp * sizeof(int16_t) + 4); /* +4 to avoid any attempt at buffer overflow by the code */
						if (!pData) {							
							return PSY_ERRFILE;
						}
						psyfile_read(self->fp, pData, wltemp * sizeof(int16_t));
						psy_audio_sample_resize(wave, 2);
						for (f = 0; f < wave->numframes; ++f) {
							int16_t val;

							val = (int16_t)pData[f];
							wave->channels.samples[1][f] = (float)val;
						}
						free(pData);
						pData = 0;						
					}
					psy_audio_samples_insert(&self->song->samples, wave,
						psy_audio_sampleindex_make(i, 0));
					{							
						psy_audio_Instrument* instrument;

						instrument = psy_audio_instrument_allocinit();
						psy_audio_instrument_setname(instrument, names[i]);
						psy_audio_instrument_setindex(instrument, i);
						psy_audio_instruments_insert(&self->song->instruments,
							instrument, psy_audio_instrumentindex_make(0, i));
						/* convert */								
						psy_audio_convert_legacy_to_instrument(instrument,
							legacy_instrs[i]);						
					}
				} else {
					unsigned char stereo;

					psyfile_skip(self->fp, 42 + sizeof(unsigned char));
					psyfile_read(self->fp, &stereo, sizeof(unsigned char));
					psyfile_skip(self->fp, wltemp);
					if (stereo) {
						psyfile_skip(self->fp, wltemp);
					}
				}
			}
		}
	}
	return PSY_OK;
}

int psy2loader_readvsts(PSY2Loader* self)
{
	int32_t i;

	for (i = 0; i < OLD_MAX_PLUGINS; ++i) {
		psyfile_read(self->fp, &self->vstL[i].valid,
			sizeof(self->vstL[i].valid));
		if (self->vstL[i].valid) {
			int32_t c;

			psyfile_read(self->fp, self->vstL[i].dllName,
				sizeof(self->vstL[i].dllName));
			psy_strlwr(self->vstL[i].dllName);
			psyfile_read(self->fp, &(self->vstL[i].numpars), sizeof(int32_t));
			self->vstL[i].pars = malloc(sizeof(float) * self->vstL[i].numpars);
			for (c=0; c< self->vstL[i].numpars; c++) {
				psyfile_read(self->fp,
					&(self->vstL[i].pars[c]), sizeof(float));
			}
		}
	}
	return PSY_OK;
}

int psy2loader_readmachines(PSY2Loader* self)
{	
	int status;
	int32_t i;	
		
	psyfile_read(self->fp, &self->_machineActive[0], sizeof(self->_machineActive));
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

			factory = self->song->machinefactory;

			psyfile_read(self->fp, &x, sizeof(x));
			psyfile_read(self->fp, &y, sizeof(y));
			psyfile_read(self->fp, &type, sizeof(type));

			if (internalmachinesconvert_pluginname_exists(&self->converter, type, "")) {
				self->pMac[i] = internalmachinesconvert_redirect(&self->converter,
					self->songfile, &i, type, "");
			} else switch (type) {
				case psy_audio_MASTER:
					self->pMac[i] = psy_audio_machinefactory_makemachine(
						factory, psy_audio_MASTER, "", psy_INDEX_INVALID);
					/* psy_audio_machines_insert(&songfile->song->machines, psy_audio_MASTER_INDEX, pMac[i]); */
					if (status = psy2loader_master_load(self, self->pMac[i], i)) {
						return status;
					}
					break;
				case psy_audio_SAMPLER: {
					self->pMac[i] = psy_audio_machinefactory_makemachine(
						factory, psy_audio_SAMPLER, "", psy_INDEX_INVALID);
					if (status = psy2loader_sampler_load(self, self->pMac[i], i)) {
						return status;
					}
					break; }
				case psy_audio_XMSAMPLER:					
					/*
					** psy2 is used for Songs made with Psycle earlier than 1.7
					** It cannot happen that one of those has this new Sampler.
					*/
					assert(0);
					self->pMac[i] = psy_audio_machinefactory_makemachine(
						factory, psy_audio_XMSAMPLER, "", psy_INDEX_INVALID);					
					break;
				case psy_audio_PLUGIN: {
					char sDllName[256];
					char plugincatchername[_MAX_PATH];

					/* Plugin dll name */
					psyfile_read(self->fp, sDllName, sizeof(sDllName));
					psy_strlwr(sDllName);
					if (internalmachinesconvert_pluginname_exists(&self->converter, type, sDllName)) {
						self->pMac[i] = internalmachinesconvert_redirect(&self->converter,
							self->songfile, &i, type, sDllName);
					} else {
						psy_audio_plugincatcher_catchername(sDllName, plugincatchername, 0);
						self->pMac[i] = psy_audio_machinefactory_makemachine(
							factory, psy_audio_PLUGIN, plugincatchername, psy_INDEX_INVALID);
						if (self->pMac[i]) {
							psy2loader_plugin_load(self, self->pMac[i], i, plugincatchername);
						} else {
							self->pMac[i] = psy_audio_machinefactory_makemachine(
								factory, psy_audio_DUMMY, plugincatchername, psy_INDEX_INVALID);
							psy_audio_machine_setslot(self->pMac[i], i);
							psy2loader_plugin_skipload(self, self->pMac[i], i);
							psy_audio_songfile_warn(self->songfile, "replaced missing module ");
							psy_audio_songfile_warn(self->songfile, sDllName);
							psy_audio_songfile_warn(self->songfile, " aka ");
							psy_audio_songfile_warn(self->songfile, psy_audio_machine_editname(self->pMac[i]));
							psy_audio_songfile_warn(self->songfile, " with dummy-plug\n");
							/*
							** Warning: It cannot be known if the missing plugin is a generator
							** or an effect. This will be guessed from the busMachine array.
							*/
						}
					}
					break; }
				case psy_audio_VST:
				case psy_audio_VSTFX: {				
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
					/*
					** The trick: We need to load the information from the file in order to know the "instance" number
					** and be able to create a plugin from the corresponding dll. Later, we will set the loaded settings to
					** the newly created plugin.
					*/
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
						psy_audio_plugincatcher_catchername(
							self->vstL[instance].dllName, plugincatchername, shellIdx);
						pVstPlugin = psy_audio_machinefactory_makemachine(
							factory, psy_audio_PLUGIN, plugincatchername, psy_INDEX_INVALID);
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
						self->pMac[i] = psy_audio_machinefactory_makemachine(factory, psy_audio_DUMMY, plugincatchername, psy_INDEX_INVALID);
						psy_audio_machine_setslot(self->pMac[i], i);
						psy_audio_machine_seteditname(self->pMac[i], plugincatchername);
						/*
						** todo set mode
						** self->pMac[i]->_mode = psy_audio_MACHMODE_FX;						
						** self->pMac[i]->_mode = psy_audio_MACHMODE_GENERATOR;						
						*/
					}
					break; }
				case psy_audio_DUMMY:
					self->pMac[i] = psy_audio_machinefactory_makemachine(factory,
						psy_audio_DUMMY, "", psy_INDEX_INVALID);
					psy_audio_machine_setslot(self->pMac[i], i);
					if (status = psy2loader_machine_load(self, self->pMac[i], i)) {
						return status;
					}
					break;
				default: {
					char sError[128];
					sprintf(sError, "unknown machine type: %i", type);
					self->pMac[i] = psy_audio_machinefactory_makemachine(factory,
						psy_audio_DUMMY, sError, psy_INDEX_INVALID);
					psy_audio_machine_setslot(self->pMac[i], i);
					if (status = psy2loader_plugin_skipload(self, self->pMac[i], i)) {
						return status;
					}
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
	/* Patch 0: Some extra data added around the 1.0 release. */
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		uint8_t _loop;
		psyfile_read(self->fp, &_loop, sizeof(_loop));
	}
	for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
	{
		int32_t _lines;
		psyfile_read(self->fp, &_lines, sizeof(_lines));
	}
	return PSY_OK;
}

int psy2loader_addmachines(PSY2Loader* self)
{
	int32_t i;

	/* Validate the machine arrays. */
	for (i = 0; i < 128; ++i) /* First, we add the output volumes to a Matrix for reference later */
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

	/*
	** Patch 1: BusEffects (twf). Try to read it, and if it doesn't exist, generate it.
	** progress.m_Progress.SetPos(8192 + 4096);
	** ::Sleep(1);
	*/
	if (psyfile_read(self->fp, &self->busEffect[0],
		sizeof(self->busEffect)) != PSY_OK)
	{
		int j = 0;
		int i = 0;
		unsigned char invmach[128];
		memset(invmach, 255, sizeof(invmach));
		/*
		** The guessing procedure does not rely on the machmode because if a plugin
		** is missing, then it is always tagged as a generator.
		*/
		for (i = 0; i < 64; i++)
		{
			if (self->busMachine[i] < 128 && self->busMachine[i] != 255) {
				invmach[self->busMachine[i]] = i;
			}
		}
		for (i = 1; i < 128; i++) /* machine 0 is the Master machine. */
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

	/* Validate that there isn't any duplicated machine. */
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

	/*
	** Patch 1.2: Fixes erroneous machine mode when a dummy replaces a bad plugin
	** (missing dll, or when the load process failed).
	** At the same time, we validate the indexes of the busMachine and busEffects arrays.
	*/
	for (i = 0; i < 64; i++)
	{
		if (self->busEffect[i])
		{
			if (self->busEffect[i] > 128 || (self->busEffect[i] < 128 &&
					!self->_machineActive[self->busEffect[i]]))
				self->busEffect[i] = 255;
			/* If there's a dummy, force it to be an effect */
			else
				if (self->busEffect[i] < 128 && self->pMac[self->busEffect[i]] &&
					psy_audio_machine_type(self->pMac[self->busEffect[i]]) == psy_audio_DUMMY)
				{
					/* pMac[busEffect[i]]->_mode = psy_audio_MACHMODE_FX; */
				}
			/*
			** Else if the machine is a generator, move it to gens bus.
			** This can't happen, but it is here for completeness
			*/
				else if (self->busEffect[i] < 128 && self->pMac[self->busEffect[i]] &&
					psy_audio_machine_mode(self->pMac[self->busEffect[i]]) == psy_audio_MACHMODE_GENERATOR)
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
			/* If there's a dummy, force it to be a Generator */
			else if (psy_audio_machine_type(self->pMac[self->busMachine[i]]) == psy_audio_DUMMY)
			{
				/* pMac[busMachine[i]]->_mode = psy_audio_MACHMODE_GENERATOR; */
			}
			/* Else if the machine is an fx, move it to FXs bus. */
			/* This can't happen, but it is here for completeness */
			else if (self->busEffect[i] < 128 && self->pMac[self->busEffect[i]] &&
				psy_audio_machine_mode(self->pMac[self->busMachine[i]]) != psy_audio_MACHMODE_GENERATOR)
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
		/*
		** Psycle no longer uses busMachine and busEffect, since the pMachine Array directly maps
		** to the real machine.
		** Due to this, we have to move machines to where they really are, 
		** and remap the inputs and outputs indexes again... ouch
		** At the same time, we validate each wire.
		*/
		/*
		progress.m_Progress.SetPos(8192 + 4096 + 2048 + 1024);
		::Sleep(1);
		*/
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
				self->_machineActive[i] = FALSE; /* mark as "converted" */
				psy_audio_machines_insert(&self->song->machines, invmach[i], self->pMac[i]);
			}
		}
		/*
		** verify that there isn't any machine that hasn't been copied into _pMachine
		** Shouldn't happen. It would mean a damaged file.			
		*/
		for (i = 0; i < 128; i++)
		{
			if (self->_machineActive[i])
			{
				if (psy_audio_machine_mode(self->pMac[i]) == psy_audio_MACHMODE_GENERATOR)
				{
					while (psy_audio_machines_at(&self->song->machines, j) && j < 64) j++;

					invmach[i] = j;
					psy_audio_machines_insert(&self->song->machines, j,
						self->pMac[i]);
					/* _pMachine[j] = pMac[i]; */
				} else
				{
					while (psy_audio_machines_at(&self->song->machines, k) && k < 128) k++;
					invmach[i] = k;
					/* _pMachine[k] = pMac[i]; */
					psy_audio_machines_insert(&self->song->machines, k,
						self->pMac[i]);
				}
			}
		}
		
		{
			/* Creates Remapped Wires for machine postload and adjust wire volume */
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
							val *= 0.000030517578125f; /* BugFix */
						} else if (val < 0.00004f)
						{
							val *= 32768.0f; /* BugFix */
						}
						wireremapped->_inputConVol = val;
						/* and set the volume. */
						psy_audio_machines_connect(&self->song->machines,
							psy_audio_wire_make(wireremapped->_inputMachine, macremapped_id));
						psy_audio_connections_setwirevolume(&self->song->machines.connections,
							psy_audio_wire_make(wireremapped->_inputMachine, macremapped_id),
							val * wireremapped->_wireMultiplier);
						psy_audio_connections_setpinmapping(&self->song->machines.connections,
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
		/* todo samplerate */
		internalmachineconverter_retweak_song(&self->converter, self->song, 44100.0);
	}
	return PSY_OK;
}

/* old file format machine loaders */
int psy2loader_machine_load(PSY2Loader* self, psy_audio_Machine* machine, uintptr_t slot)
{
	int status;
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	psy_audio_MachineWires* machinewires;

	if (status = psyfile_read(self->fp, _editName, 16)) {
		return status;
	}
	_editName[15] = 0;	
	psy_audio_machine_seteditname(machine, _editName);

	machinewires = psy_audio_read_psy2machinewires(self->fp);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);
	
	if (status = psyfile_read(self->fp, &connectionpoint, sizeof(connectionpoint))) {
		return status;
	}
	/* numInputs and numOutputs */
	psyfile_skip(self->fp, 2 * sizeof(int32_t));

	if (status = psyfile_read(self->fp, &panning, sizeof(panning))) {
		return status;
	}
	psy_audio_machine_setpanning(machine, panning / 128.f);
	psyfile_skip(self->fp, 109);
	return PSY_OK;
}

int psy2loader_master_load(PSY2Loader* self, psy_audio_Machine* master, uintptr_t slot)
{
	char editname[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t _outDry;
	psy_audio_MachineWires* machinewires;

	psyfile_read(self->fp, editname, 16);
	editname[15] = 0;
	
	machinewires = psy_audio_read_psy2machinewires(self->fp);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);

	psyfile_read(self->fp, &connectionpoint, sizeof(connectionpoint));
	/* numInputs and numOutputs */
	psyfile_skip(self->fp, 2 * sizeof(int32_t));

	psyfile_read(self->fp, &panning, sizeof(panning));
	psy_audio_machine_setpanning(master, panning / 128.f);
	psyfile_skip(self->fp, 40);

	/* outdry */
	psyfile_read(self->fp, &_outDry, sizeof(int));
	{
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(master, 0);
		if (param) {
			psy_audio_machine_parameter_tweak(master,
				param, (float)sqrt(_outDry / 256.f / 4.f));
		}
	}
	psyfile_skip(self->fp, 65);
	return PSY_OK;
}

int psy2loader_sampler_load(PSY2Loader* self, psy_audio_Machine* machine,
	uintptr_t slot)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t _numVoices;
	int32_t interpol;	
	psy_audio_MachineParam* param;
	psy_audio_MachineWires* machinewires;

	psyfile_read(self->fp, _editName, 16);
	_editName[15] = 0;
	
	psy_audio_machine_seteditname(machine, _editName);
	machinewires = psy_audio_read_psy2machinewires(self->fp);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);

	psyfile_read(self->fp, &connectionpoint, sizeof(connectionpoint));
	/* numInputs and numOutputs */
	psyfile_skip(self->fp, 2 * sizeof(int32_t));

	psyfile_read(self->fp, &panning, sizeof(panning));
	psyfile_skip(self->fp, 8 * sizeof(int)); /* SubTrack[] */
	psyfile_read(self->fp, &_numVoices, sizeof(_numVoices)); /* numSubtracks */

	if (_numVoices < 4)
	{
		/* Psycle versions < 1.1b2 had polyphony per channel,not per machine. */
		_numVoices = 8;
	}
	psyfile_read(self->fp, &interpol, sizeof(int32_t)); /* interpol */
	psyfile_skip(self->fp, 69);
	/* Num Voices */
	param = psy_audio_machine_tweakparameter(machine, 0);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(machine, param, _numVoices);
	}
	/* Quality */
	param = psy_audio_machine_tweakparameter(machine, 1);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(machine, param, interpol);
	}
	/* Default Speed (C3) */
	param = psy_audio_machine_tweakparameter(machine, 2);
	if (param) {
		psy_audio_machine_parameter_tweak_scaled(machine, param, 0);
	}
	return PSY_OK;
}

int psy2loader_plugin_load(PSY2Loader* self, psy_audio_Machine* machine, uintptr_t slot,
	const char* plugincatchername)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t numParameters;
	int32_t* Vals;
	uintptr_t i;
	psy_audio_MachineWires* machinewires;

	assert(machine);
	psyfile_read(self->fp, _editName, 16);
	_editName[15] = 0;
	psy_audio_machine_seteditname(machine, _editName);
	psyfile_read(self->fp, &numParameters, sizeof(numParameters));
	if (numParameters > 0) {
		Vals = malloc(sizeof(int32_t) * numParameters);		
		psyfile_read(self->fp, Vals, numParameters * sizeof(int32_t));
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
		
		/* 
		** Fix for 64 bit platform. This assume that psy2 files are created with a
		** 32 bit os but pooplog-fm reports 64 bit size. File load will be corrupted.
		** todo: need to be fixed in mfc psycle aswell
		*/
#if (DIVERSALIS__CPU__SIZEOF_POINTER == 8)
		if (plugincatchername && strcmp(plugincatchername,
				"pooplog-fm-ultralight:0") == 0) {
			size = 312;
		} else if (plugincatchername && strcmp(plugincatchername,
				"pooplog-fm-laboratory:0") == 0) {
			size = 1368;
		} else {
			size = (uint32_t)psy_audio_machine_datasize(machine);
		}
#else
	size = (uint32_t)psy_audio_machine_datasize(machine);
#endif
		/* pFile->Read(&size,sizeof(int));	// This SHOULD be the right thing to do */
		if (size)
		{
			uint8_t* pData;
			
			pData = (uint8_t*)malloc(size);
			psyfile_read(self->fp, pData, size); /* Number of parameters */
			psy_audio_machine_putdata(machine, pData); /* Internal load */
			free(pData);
			pData = 0;
		}
	}	
	free(Vals);
	Vals = 0;
	
	machinewires = psy_audio_read_psy2machinewires(self->fp);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);	

	psyfile_read(self->fp, &connectionpoint, sizeof(connectionpoint));
	/* numInputs and numOutputs */
	psyfile_skip(self->fp, 2 * sizeof(int32_t));

	psyfile_read(self->fp, &panning, sizeof(panning));
	psy_audio_machine_setpanning(machine, panning / 128.f);
	psyfile_skip(self->fp, 109);
	return PSY_OK;
}

int psy2loader_plugin_skipload(PSY2Loader* self, psy_audio_Machine* machine, uintptr_t slot)
{
	char _editName[32];
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	int32_t numParameters;
	psy_audio_MachineWires* machinewires;

	assert(self && machine);

	psyfile_read(self->fp, _editName, 16);
	_editName[15] = 0;
	psy_audio_machine_seteditname(machine, _editName);
	psy_audio_machine_setslot(machine, slot);
	
	psyfile_read(self->fp, &numParameters, sizeof(numParameters));
	psyfile_skip(self->fp, numParameters * sizeof(int32_t));
	/* This SHOULD be done, but it breaks the fileformat.
		int size;
		pFile->Read(&size,sizeof(int));
		if (size)
		{
			pFile->Skip(size);
		}
	*/
	machinewires = psy_audio_read_psy2machinewires(self->fp);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);
	psyfile_read(self->fp, &connectionpoint, sizeof(connectionpoint));
	/* numInputs and numOutputs */
	psyfile_skip(self->fp, 2 * sizeof(int32_t));

	psyfile_read(self->fp, &panning, sizeof(panning));
	psyfile_skip(self->fp, 109);
	return PSY_OK;
}

void vstpreload_init(VstPreload* self, psy_audio_SongFile* songfile)
{
	self->songfile = songfile;
}

int vstpreload_load(VstPreload* self, uintptr_t slot, unsigned char* program, int32_t* instance)
{
	int status;
	cpoint_t connectionpoint[PSY2_MAX_CONNECTIONS];
	int32_t panning;
	uint8_t old;
	psy_audio_MachineWires* machinewires;	

	if (status = psyfile_read(self->songfile->file, self->_editName, 16)) {
		return status;
	}
	self->_editName[15] = '\0';
	machinewires = psy_audio_read_psy2machinewires(self->songfile->file);
	psy_audio_legacywires_insert(self->songfile->legacywires, slot, machinewires);

	if (status = psyfile_read(self->songfile->file, &connectionpoint, sizeof(connectionpoint))) {
		return status;
	}
	/* numInputs and numOutputs */
	psyfile_skip(self->songfile->file, 2 * sizeof(int32_t));

	if (status = psyfile_read(self->songfile->file, &panning,
			sizeof(panning))) {
		return status;
	}
	psyfile_skip(self->songfile->file, 109);
	
	/* old format */
	if (status = psyfile_read(self->songfile->file, &old, sizeof old)) {
		return status;
	}
	/* ovst.instance */
	if (status = psyfile_read(self->songfile->file, instance, sizeof *instance)) { 
		return status;
	}
	if (old)
	{
		char mch;
		if (status = psyfile_read(self->songfile->file, &mch, sizeof mch)) {
			return status;
		}
		program = 0;
	} else
	{
		if (status = psyfile_read(self->songfile->file, program, sizeof *program)) {
			return status;
		}
	}
	return PSY_OK;
}

psy_audio_MachineWires* psy_audio_read_psy2machinewires(PsyFile* file)
{
	psy_audio_MachineWires* rv;
	uintptr_t c; /* current legacywire (connection) id */
	
	assert(file);

	/* Create machinewires with PSY2_MAX_CONNECTIONS */
	rv = psy_audio_machinewires_allocinit();
	assert(rv);
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		psy_audio_machinewires_insert(rv, c,
			psy_audio_legacywire_allocinit());
	}
	/*
	** Read connections
	** Incoming connections Machine number
	*/
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		int32_t input;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &input, sizeof(input));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_inputMachine = input;
		}
	}
	/* Outgoing connections Machine number */
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		int32_t output;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &output, sizeof(output));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_outputMachine = output;
		}
	}
	/* Incoming connections Machine vol */
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		float inputconvol;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &inputconvol, sizeof(inputconvol));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_inputConVol = inputconvol;
		}
	}
	/* Outgoing connections activated */
	for (c = 0; c < PSY2_MAX_CONNECTIONS; ++c) {
		uint8_t connection;
		psy_audio_LegacyWire* currwire;

		psyfile_read(file, &connection, sizeof(connection));
		currwire = psy_audio_machinewires_at(rv, c);
		if (currwire) {
			currwire->_connection = connection;
		}
	}
	/* Incoming connections activated */
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

int psy2loader_postload(PSY2Loader* self)
{
	psy_TableIterator it;

	for (it = psy_audio_machines_begin(&self->song->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_machine_postload(
			(psy_audio_Machine*)psy_tableiterator_value(&it),
			self->songfile,
			psy_tableiterator_key(&it));
	}
	return PSY_OK;
}
