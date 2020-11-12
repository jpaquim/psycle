// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xm.h"
#include "xmdefs.h"
#include "song.h"
#include "songio.h"
#include "xmsampler.h"
#include "xmsongexport.h"

#include <operations.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../detail/portable.h"

static int16_t const BIGMODPERIODTABLE[37 * 8] = //((12note*3oct)+1note)*8fine
{
	//-8fine(also note minus one).......Note........................+7fine
		907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
		808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
		720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
		640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
		570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
		508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
		453,450,447,444,441,437,434,431,428,425,422,419,416,413,410,407,
		404,401,398,395,392,390,387,384,381,379,376,373,370,368,365,363,
		360,357,355,352,350,347,345,342,339,337,335,332,330,328,325,323,
		320,318,316,314,312,309,307,305,302,300,298,296,294,292,290,288,
		285,284,282,280,278,276,274,272,269,268,266,264,262,260,258,256,
		254,253,251,249,247,245,244,242,240,239,237,235,233,232,230,228,
		226,225,224,222,220,219,217,216,214,212,211,209,208,206,205,203,
		202,200,199,198,196,195,193,192,190,189,188,187,185,184,183,181,
		180,179,177,176,175,174,172,171,170,169,167,166,165,164,163,161,
		160,159,158,157,156,155,154,152,151,150,149,148,147,146,145,144,
		143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
		127,126,125,125,123,123,122,121,120,119,118,118,117,116,115,114,
		113,113,112,111,110,109,109,108
};


static void xm_readheader(psy_audio_SongFile*, struct XMFILEHEADER*);
static uint32_t xm_readpatterns(psy_audio_SongFile*, struct XMFILEHEADER*);
static void xm_readinstruments(psy_audio_SongFile*, struct XMFILEHEADER*, uint32_t start);
static void xm_readinstrument(psy_audio_SongFile*, uint32_t slot, uint32_t start, bool xi);
static void xm_makesequence(psy_audio_SongFile*, struct XMFILEHEADER*);
static int xm_patternevent_empty(psy_audio_PatternEvent*);

static uint16_t flip16(uint16_t value) 
{
	uint16_t rv;

	rv = ((value >> 8) & 0x0F) | ((value << 8) & 0xF0);
	return rv;
}

void psy_audio_xm_load(psy_audio_SongFile* self)
{	
	uint32_t nextstart;
	SongProperties songproperties;
	char name[20];
	char comments[4096];
	char trackername[20];
	uint8_t trackername_follows;
	uint16_t xmversion;
	uint32_t beginheader;
	uint32_t beginpatterns;
	struct XMFILEHEADER xmheader;		
	
	psyfile_read(self->file, &name, sizeof(name));
	name[sizeof(name) - 1] = '\0';
	strcpy(comments, "Imported from FastTracker II Module: ");
	strcat(comments, self->file->szName);
	songproperties_init(&songproperties, name, "", comments);
	psy_audio_song_setproperties(self->song, &songproperties);
	trackername_follows = psyfile_read_uint8(self->file);
	if (trackername_follows != XM_TRACKERNAME_FOLLOWS) {
		self->err = 2;
	}
	psyfile_read(self->file, &trackername, sizeof(trackername));
	trackername[sizeof(trackername) - 1] = '\0';
	xmversion = flip16(psyfile_read_uint16(self->file));
	beginheader = psyfile_getpos(self->file);
	xm_readheader(self, &xmheader);
	beginpatterns = beginheader + xmheader.size;
	psyfile_seek(self->file, beginpatterns);	
	nextstart = xm_readpatterns(self, &xmheader);
	psyfile_seek(self->file, nextstart);
	xm_readinstruments(self, &xmheader, nextstart);
	xm_makesequence(self, &xmheader);
}

void psy_audio_xi_load(struct psy_audio_SongFile* self, uintptr_t slot)
{
	char header[22];

	psyfile_read(self->file, header, 21);
	header[21] = '\0';
	if (strcmp(header, "Extended Instrument: ") != 0) {
		return;
	}
	xm_readinstrument(self, slot, 21, TRUE);
}

void xm_readheader(psy_audio_SongFile* self, struct XMFILEHEADER *xmheader)
{	
	xmheader->size = psyfile_read_uint32(self->file);
	xmheader->norder = psyfile_read_uint16(self->file);
	xmheader->restartpos = psyfile_read_uint16(self->file);
	xmheader->channels = psyfile_read_uint16(self->file);
	xmheader->patterns = psyfile_read_uint16(self->file);
	xmheader->instruments = psyfile_read_uint16(self->file);
	xmheader->flags = psyfile_read_uint16(self->file);
	xmheader->tempo = psyfile_read_uint16(self->file);
	xmheader->speed = psyfile_read_uint16(self->file);
	psyfile_read(self->file, xmheader->order, sizeof(xmheader->order));
	if (xmheader->channels >= 2) {
		psy_audio_patterns_setsongtracks(&self->song->patterns, xmheader->channels);
	} else {
		psy_audio_patterns_setsongtracks(&self->song->patterns, 2);
	}
	self->song->properties.bpm = (psy_dsp_beat_t) xmheader->speed;
}

uint32_t xm_readpatterns(psy_audio_SongFile* self, struct XMFILEHEADER *xmheader)
{	
	uintptr_t slot;	
	uint32_t nextstart;

	nextstart = psyfile_getpos(self->file);
	for (slot = 0; slot < xmheader->patterns; ++slot) {
		struct XMPATTERNHEADER patternheader;
		psy_audio_Pattern* pattern;

		psyfile_seek(self->file, nextstart);
		patternheader.size = psyfile_read_uint32(self->file);
		patternheader.packingtype = psyfile_read_uint8(self->file);
		patternheader.rows = psyfile_read_uint16(self->file);
		patternheader.packedsize = psyfile_read_uint16(self->file);
		nextstart += patternheader.size;

		pattern = psy_audio_pattern_allocinit();
		psy_audio_pattern_setlength(pattern, patternheader.rows * (psy_dsp_beat_t) 0.25);
		psy_audio_patterns_insert(&self->song->patterns, slot, pattern);
		if (patternheader.packedsize > 0) {
			unsigned char* packeddata;
			int i = 0;
			int c = 0;
			int track = 0;
			int line = 0;
			int insert = 0;
			psy_dsp_beat_t bpl = 0.25;
			psy_audio_PatternEvent ev;
			psy_audio_PatternNode* node = 0;

			nextstart += patternheader.packedsize;
			psy_audio_patternevent_clear(&ev);
			packeddata = malloc(patternheader.packedsize);
			psyfile_read(self->file, packeddata, patternheader.packedsize);
			while (i < patternheader.packedsize) {
				unsigned char note= psy_audio_NOTECOMMANDS_EMPTY;
				unsigned char instr=255;
				unsigned char vol=0;
				unsigned char type=0;
				unsigned char param=0;
								
				note = packeddata[i++];				
				// is compression bit set?
				if((note & 0x80) == 0x80) {
					unsigned char bReadNote = note & 0x01;
					unsigned char bReadInstr = note & 0x02;
					unsigned char bReadVol = note & 0x04;
					unsigned char bReadType = note & 0x08;
					unsigned char bReadParam  = note & 0x10;

					note = 0;
					if (i >= patternheader.packedsize) break;
					if(bReadNote) note = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					if(bReadInstr) instr = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					if(bReadVol) vol = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					if(bReadType) type = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					if(bReadParam) param = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
				} else
				{
					// read all values
					if (i >= patternheader.packedsize) break;
					instr = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					vol = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					type = packeddata[i++] & 0x7F;
					if (i >= patternheader.packedsize) break;
					param = packeddata[i++] & 0x7F;				
				}

				// xm note = 1..96, 1 = c-0, 97 = key off 
				// psycle note = 0..119, 0 = c-0, 120 = key off
				note = (note & 0x7F);
				if (note >= 97) {
					note = psy_audio_NOTECOMMANDS_RELEASE;
				} else 
				if (note > 0) {
					--note;
				} else {
					note = 255;
				}
				if(vol >= 0x10 && vol <= 0x50)
				{				
				} else {
					vol = 0x80;
				}
				psy_audio_patternevent_init_all(
					&ev,
					note,
					instr,
					0,
					vol,					
					0,
					0);
				if (!xm_patternevent_empty(&ev)) {
					node = psy_audio_pattern_insert(pattern, node, track, line * bpl, &ev);
				}
				++track;
				if (track >= xmheader->channels) {
					track = 0;
					++line;					
				}
			}
			free(packeddata);
		}
	}

	return nextstart;
}

void xm_readinstruments(psy_audio_SongFile* self, struct XMFILEHEADER *xmheader, uint32_t start)
{
	uintptr_t slot;	

	start = psyfile_getpos(self->file);
	for (slot = 0; slot < xmheader->instruments; ++slot) {
		xm_readinstrument(self, slot + 1, start, FALSE);
	}
}

void xm_readinstrument(psy_audio_SongFile* self, uint32_t slot, uint32_t start, bool xi)
{
	struct XMINSTRUMENTHEADER instrumentheader;
	psy_audio_Instrument* instrument;

	start = psyfile_seek(self->file, start);
	instrument = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instrument, slot + 1);
	psy_audio_instruments_insert(&self->song->instruments, instrument,
		psy_audio_instrumentindex_make(0, slot));
	if (!xi) {
		instrumentheader.size = psyfile_read_uint32(self->file);
	} else {
		instrumentheader.size = 0;
	}

	psyfile_read(self->file, &instrumentheader.name,
		sizeof(instrumentheader.name));
	instrumentheader.name[sizeof(instrumentheader.name) - 1] = '\0';
	psy_audio_instrument_setname(instrument, instrumentheader.name);

	instrumentheader.type = psyfile_read_uint8(self->file);
	if (!xi) {
		instrumentheader.samples = psyfile_read_uint16(self->file);
	} else {
		uint16_t version;

		psyfile_skip(self->file, 20);
		version = psyfile_read_uint16(self->file);
		if (version != 0x102) {
			return;
		}
		instrumentheader.samples = 1;
	}
	start += instrumentheader.size;
	if (instrumentheader.samples > 0) {
		struct XMSAMPLEHEADER sampleheader;
		XMSAMPLESTRUCT* xmsamples = 0;
		int s;
		int note;
		psy_audio_InstrumentEntry instentry;

		if (!xi) {
			// psy_audio_Sample header size
			psyfile_read(self->file, &sampleheader.shsize, 4);
		}
		// psy_audio_Sample number for all notes
		psyfile_read(self->file, &sampleheader.snum, 96);
		// Points for volume envelope
		psyfile_read(self->file, &sampleheader.venv, 48);
		// Points for panning envelope
		psyfile_read(self->file, &sampleheader.penv, 48);
		// Number of volume points
		psyfile_read(self->file, &sampleheader.vnum, 1);
		// Number of panning points
		psyfile_read(self->file, &sampleheader.pnum, 1);
		// Volume sustain point
		psyfile_read(self->file, &sampleheader.psustain, 1);
		// Volume loop start point
		psyfile_read(self->file, &sampleheader.vloops, 1);
		// Volume loop end point
		psyfile_read(self->file, &sampleheader.vloope, 1);
		// Panning sustain point
		psyfile_read(self->file, &sampleheader.psustain, 1);
		// Panning loop start point
		psyfile_read(self->file, &sampleheader.ploops, 1);
		// Panning loop end point
		psyfile_read(self->file, &sampleheader.ploope, 1);
		// Volume type: bit 0: On; 1: Sustain; 2: Loop
		psyfile_read(self->file, &sampleheader.vtype, 1);
		// Panning type: bit 0: On; 1: Sustain; 2: Loop
		psyfile_read(self->file, &sampleheader.ptype, 1);
		psyfile_read(self->file, &sampleheader.vibtype, 1);
		psyfile_read(self->file, &sampleheader.vibsweep, 1);
		psyfile_read(self->file, &sampleheader.vibdepth, 1);
		psyfile_read(self->file, &sampleheader.vibrate, 1);
		psyfile_read(self->file, &sampleheader.volfade, 2);
		psyfile_read(self->file, &sampleheader.reserved, 2);
		if (xi) {
			psyfile_skip(self->file, 20);
			psyfile_read(self->file, &instrumentheader.samples, 2);
		}
		xmsamples = (XMSAMPLESTRUCT*)malloc(sizeof(
			XMSAMPLESTRUCT) * (int)instrumentheader.samples);
		// create instrument entries
		psy_audio_instrument_clearentries(instrument);
		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex =
			sampleindex_make(slot, sampleheader.snum[0]);
		for (note = 1; note < 96; ++note) {
			if (sampleheader.snum[note] != instentry.sampleindex.subslot) {
				instentry.keyrange.high = note - 1;
				psy_audio_instrument_addentry(instrument, &instentry);
				instentry.keyrange.low = note;
				instentry.sampleindex.subslot = sampleheader.snum[note];
			}
		}
		instentry.keyrange.high = 119;
		psy_audio_instrument_addentry(instrument, &instentry);
		if (!xi) {
			psyfile_seek(self->file, start);
		}
		for (s = 0; s < instrumentheader.samples; ++s) {
			if (!xi) {
				psyfile_seek(self->file, start);
			}
			psyfile_read(self->file, &xmsamples[s].samplen, 4);
			psyfile_read(self->file, &xmsamples[s].loopstart, 4);
			psyfile_read(self->file, &xmsamples[s].looplen, 4);
			psyfile_read(self->file, &xmsamples[s].vol, 1);
			// Finetune (signed byte -16..+15)
			psyfile_read(self->file, &xmsamples[s].finetune, 1);
			// Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
			//		 2 = Ping-pong loop;
			//		 4: 16-bit sampledata				
			psyfile_read(self->file, &xmsamples[s].type, 1);
			// Panning (0-255)
			psyfile_read(self->file, &xmsamples[s].pan, 1);
			// Relative note number (signed byte)
			psyfile_read(self->file, &xmsamples[s].relnote, 1);
			psyfile_read(self->file, &xmsamples[s].res, 1);
			psyfile_read(self->file, &xmsamples[s].name, 22);
			xmsamples[s].name[21] = '\0';
			start += 40;			
		}
		for (s = 0; s < instrumentheader.samples; ++s) {
			psy_audio_Sample* sample;
			int is16bit;

			sample = psy_audio_sample_allocinit(1);
			psy_audio_sample_setname(sample, xmsamples[s].name);

			is16bit = (xmsamples[s].type & 0x10) == 0x10;
			sample->panfactor = xmsamples[s].pan / (psy_dsp_amp_t)255.f;
			// Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
			//		 2 = Ping-pong loop;
			//		 4: 16-bit sampledata
			if (xmsamples[s].looplen > 0) {
				if (xmsamples[s].type & 0x01) {
					sample->loop.type = psy_audio_SAMPLE_LOOP_NORMAL;
				} else
					if (xmsamples[s].type & 0x02) {
						sample->loop.type = psy_audio_SAMPLE_LOOP_BIDI;
					} else {
						sample->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
					}
			} else {
				sample->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
			}
			if (sample->loop.type != psy_audio_SAMPLE_LOOP_DO_NOT) {
				sample->loop.start = xmsamples[s].loopstart / (is16bit ? 2 : 1);
				sample->loop.end = (xmsamples[s].loopstart + xmsamples[s].looplen) /
					(is16bit ? 2 : 1);
			}
			sample->numframes = xmsamples[s].samplen / (is16bit ? 2 : 1);
			sample->samplerate = 8363;
			sample->globalvolume = xmsamples[s].vol / (psy_dsp_amp_t)128.f;
			sample->tune = xmsamples[s].relnote;
			// WaveFineTune has +-100 range in Psycle.
			sample->finetune = (int16_t)(xmsamples[s].finetune / 1.28f);
			// Sounds Stupid, but it isn't. Some modules save sample
			// header when there is no sample.
			if (xmsamples[s].samplen > 0) {
				unsigned char* smpbuf;
				int16_t oldvalue;
				uintptr_t i;
				uintptr_t j;

				smpbuf = malloc(xmsamples[s].samplen);
				psyfile_read(self->file, smpbuf, xmsamples[s].samplen);
				sample->channels.samples[0] = dsp.memory_alloc(sample->numframes,
					sizeof(psy_dsp_amp_t));
				oldvalue = 0;
				for (i = 0, j = 0; i < sample->numframes; ++i) {
					int16_t value;

					if (is16bit) {
						value = (smpbuf[j] & 0xFF) | (smpbuf[j + 1] << 8);
						j += 2;
					} else {
						value = (int16_t)(smpbuf[j] << 8);
						++j;
					}
					value += oldvalue;
					sample->channels.samples[0][i] = (psy_dsp_amp_t)value;
					oldvalue = value;
				}
				free(smpbuf);
			}
			psy_audio_samples_insert(&self->song->samples, sample,
				sampleindex_make(slot, s));
			start += xmsamples[s].samplen;
		}
		free(xmsamples);
	}
}

void xm_makesequence(psy_audio_SongFile* self, struct XMFILEHEADER *xmheader)
{
	uintptr_t i;
	psy_audio_SequencePosition sequenceposition;

	sequenceposition.track =
		psy_audio_sequence_appendtrack(&self->song->sequence, psy_audio_sequencetrack_allocinit());
	for (i = 0; i < xmheader->norder; ++i) {
		sequenceposition.trackposition =
			psy_audio_sequence_last(&self->song->sequence, sequenceposition.track);
		psy_audio_sequence_insert(&self->song->sequence, sequenceposition,
			xmheader->order[i]);
	}
}

int xm_patternevent_empty(psy_audio_PatternEvent* self)
{	
	return self->note == psy_audio_NOTECOMMANDS_EMPTY;
}

// Mod song load
static char* AllocReadStr(PsyFile*, int32_t size, signed int start);
static unsigned char ReadUInt1(PsyFile*);
static void psy_audio_mod_loadsampleheader(psy_audio_SongFile*,
	psy_audio_Sample* _wave, int iInstrIdx, struct MODSAMPLEHEADER* m_Samples);
static void psy_audio_mod_loadsampledata(psy_audio_SongFile*,
	psy_audio_Sample* _wave, uintptr_t smplen);
static void psy_audio_mod_loadinstrument(psy_audio_SongFile*, int idx,
	struct MODSAMPLEHEADER* m_Samples);
static void psy_audio_mod_loadpatterns(psy_audio_SongFile*,
	struct MODHEADER*,
	psy_List* modtovirtual);
static void psy_audio_mod_loadsinglepattern(psy_audio_SongFile*, int patIdx,
	int iTracks, psy_List* modtovirtual);
static void psy_audio_mod_makesequence(psy_audio_SongFile*,
	struct MODHEADER* m_Header);
static uint8_t psy_audio_mod_convertperiodtonote(uint16_t period);

bool psy_audio_mod_load(psy_audio_SongFile* self)
{
	SongProperties songproperties;
	char* text;
	const char* comment = "Imported from MOD Module : ";
	struct MODSAMPLEHEADER m_Samples[32];
	struct MODHEADER m_Header;
	char pID[5];
	char* pSongName;
	int i;

	// check validity
	if (!psy_audio_mod_isvalid(self)) {
		return FALSE;
	}
	// song.CreateMachine(MACH_XMSAMPLER, rand() / 64, rand() / 80, "sampulse", 0);
	//		song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,0.5f); // This is done later, when determining the number of channels.
	// song.seqBus = 0;
	// m_pSampler = static_cast<XMSampler*>(song._pMachine[0]);
	// m_pSampler->XMSampler::PanningMode(XMSampler::PanningMode::TwoWay);
	// get song name

	pSongName = AllocReadStr(self->file, 20, 0);
	if (pSongName == NULL)
		return FALSE;

	text = (char*)malloc(sizeof(comment) + strlen(self->path) + 1);
	psy_snprintf(text, sizeof(comment) + strlen(self->path), "%s%s", comment, self->path);
	songproperties_init(&songproperties, pSongName, "", text);
	free(text);
	psy_audio_song_setproperties(self->song, &songproperties);	

	// get data
	psyfile_seek(self->file, 20);
	for (i = 0; i < 31; i++) {
		psy_audio_Sample* wave;

		wave = psy_audio_sample_allocinit(1);		
		psy_audio_mod_loadsampleheader(self, wave, i, m_Samples);
		psy_audio_samples_insert(&self->song->samples, wave,
			sampleindex_make(i, 0));
	}
	psyfile_seek(self->file, 950);	
	psyfile_read(self->file, &m_Header, sizeof(m_Header));	
	pID[0] = m_Header.pID[0]; pID[1] = m_Header.pID[1]; pID[2] = m_Header.pID[2]; pID[3] = m_Header.pID[3]; pID[4] = 0;
	// m_pSampler->IsAmigaSlides(true);
	
	if (!strncmp(pID, "M.K.", 4)) {
		self->song->properties.tracks = 4;
		// -9dB
		// song.InsertConnectionNonBlocking(0, MASTER_INDEX, 0, 0, 0.355f);
	} else if (!strncmp(pID, "M!K!", 4)) {
		// -9dB
		self->song->properties.tracks = 4;
		// song.InsertConnectionNonBlocking(0, MASTER_INDEX, 0, 0, 0.355f);
	} else if (!strncmp(pID + 1, "CHN", 4)) {
		char tmp[2];
		tmp[0] = pID[0]; tmp[1] = 0;
		self->song->properties.tracks = atoi(tmp);
		// -9dB
		// song.InsertConnectionNonBlocking(0, MASTER_INDEX, 0, 0, 0.355f);
	} else if (!strncmp(pID + 2, "CH", 4)) {
		char tmp[3];
		tmp[0] = pID[0]; tmp[1] = pID[1]; tmp[2] = 0;
		self->song->properties.tracks = atoi(tmp);
		//-9dB
		// song.InsertConnectionNonBlocking(0, MASTER_INDEX, 0, 0, 0.355f); }
	}
	self->song->properties.bpm = 125;
	self->song->properties.lpb = 4;
	self->song->properties.tpb = 24;
	self->song->properties.extraticksperbeat = 0;
	
	if (self->song->properties.tracks <= 8) {
		for (i = 0; i < self->song->properties.tracks; i++)
		{
			if (i % 4 == 0 || i % 4 == 3) {
				// m_pSampler->rChannel(i).DefaultPanFactorFloat(0.25f, true);
			} else {
				// m_pSampler->rChannel(i).DefaultPanFactorFloat(0.75, true);
			}
		}
	} else {
		for (i = 0; i < self->song->properties.tracks; i++)
		{
			//m_pSampler->rChannel(i).DefaultPanFactorFloat(0.5f, true);
		}
	}
	/*std::map<int, int> modtovirtual;
	int virtidx = MAX_MACHINES;
	for (int i = 0; i < 31; i++) {
		if (m_Samples[i].sampleLength > 0) {
			modtovirtual[i] = virtidx;
			virtidx++;
		}
	}*/
	psy_audio_mod_loadpatterns(self, &m_Header, NULL); // , modtovirtual);
	for (i = 0; i < 31; i++) {
		psy_audio_mod_loadinstrument(self, i, m_Samples);
		//if (song.xminstruments.IsEnabled(i)) {
			//song.SetVirtualInstrument(modtovirtual[i], 0, i);
		//}
	}
	return TRUE;
}

bool psy_audio_mod_isvalid(psy_audio_SongFile* self)
{
	bool bIsValid = FALSE;
	char* pID = AllocReadStr(self->file, 4, 1080);

	if (pID) {
		bIsValid = !strncmp(pID, "M.K.", 4);
		if (!bIsValid) bIsValid = !strncmp(pID, "M!K!", 4);
		if (!bIsValid) bIsValid = !strncmp(pID + 1, "CHN", 4);
		if (!bIsValid) bIsValid = !strncmp(pID + 2, "CH", 4);

		free(pID);
	}
	return bIsValid;
}

char* AllocReadStr(PsyFile* file, int32_t size, signed int start)
{
	// allocate space
	char* pData = (char*)malloc(size + 1);
	if (pData == NULL)
		return NULL;

	// null terminate
	pData[size] = 0;

	// go to offset
	if (start >= 0)
		psyfile_seek(file, start);

	// read data
	if (psyfile_read(file, pData, size))
		return pData;

	free(pData);
	return NULL;
}

// inlines
unsigned char ReadUInt1(PsyFile* file)
{
	uint8_t i;
	return psyfile_read(file, &i, 1) ? i : 0;
}

void psy_audio_mod_loadinstrument(psy_audio_SongFile* self, int idx,
	struct MODSAMPLEHEADER* m_Samples)
{
	psy_audio_Instrument* instr;

	instr = psy_audio_instrument_allocinit();
	psy_audio_instrument_setname(instr, m_Samples[idx].sampleName);	

	if (m_Samples[idx].sampleLength > 0)
	{
		psy_audio_Sample* sample;
		psy_audio_InstrumentEntry instentry;

		sample = psy_audio_samples_at(&self->song->samples, sampleindex_make(idx, 0));
		psy_audio_mod_loadsampledata(self, sample, m_Samples[idx].sampleLength);
		
		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex =
			sampleindex_make(idx, 0);		
		psy_audio_instrument_addentry(instr, &instentry);
	}

	//instr.ValidateEnabled();
	psy_audio_instruments_insert(&self->song->instruments, instr,
		psy_audio_instrumentindex_make(0, idx));
}

void psy_audio_mod_loadsampleheader(psy_audio_SongFile* self, psy_audio_Sample* _wave, int iInstrIdx,
	struct MODSAMPLEHEADER *m_Samples)
{
	unsigned short smpLen[32];
	bool bLoop;
	char tmpfine;

	psyfile_read(self->file, m_Samples[iInstrIdx].sampleName, 22);
	m_Samples[iInstrIdx].sampleName[21] = '\0';

	smpLen[iInstrIdx] = (ReadUInt1(self->file) * 0x100 + ReadUInt1(self->file)) * 2;
	m_Samples[iInstrIdx].sampleLength = smpLen[iInstrIdx];
	m_Samples[iInstrIdx].finetune = ReadUInt1(self->file);
	m_Samples[iInstrIdx].volume = ReadUInt1(self->file);
	m_Samples[iInstrIdx].loopStart = ((ReadUInt1(self->file) * 256 + ReadUInt1(self->file)) * 2);
	m_Samples[iInstrIdx].loopLength = (ReadUInt1(self->file) * 256 + ReadUInt1(self->file)) * 2;

	// parse
	bLoop = (m_Samples[iInstrIdx].loopLength > 3);

	if (smpLen[iInstrIdx] > 0)
	{		
		//_wave->channels.samples[0] = dsp.memory_alloc(smpLen[iInstrIdx],
			//sizeof(psy_dsp_amp_t));
	}

	if (bLoop)
	{
		_wave->loop.type = (psy_audio_SampleLoopType)psy_audio_SAMPLE_LOOP_NORMAL;
		_wave->loop.start = m_Samples[iInstrIdx].loopStart;
		if (m_Samples[iInstrIdx].loopStart + m_Samples[iInstrIdx].loopLength > smpLen[iInstrIdx])
		{
			_wave->loop.end = smpLen[iInstrIdx];
		} else _wave->loop.end = m_Samples[iInstrIdx].loopStart + m_Samples[iInstrIdx].loopLength;
	} else {
		_wave->loop.type = (psy_audio_SampleLoopType)psy_audio_SAMPLE_LOOP_DO_NOT;
	}

	_wave->defaultvolume = 1.0f; // m_Samples[iInstrIdx].volume * 2;
	_wave->samplerate = 8363;
	tmpfine = (char)m_Samples[iInstrIdx].finetune;
	if (tmpfine > 7) tmpfine -= 16;
	_wave->finetune = (uint16_t)(tmpfine * 12.5);// finetune has +-100 range in Psycle
	psy_audio_sample_setname(_wave, m_Samples[iInstrIdx].sampleName);
}

void psy_audio_mod_loadsampledata(psy_audio_SongFile* self, psy_audio_Sample* _wave, uintptr_t smplen)
{
	// parse
	short wNew = 0;	
	int sampleCnt;
	int j;

	// cache sample data
	unsigned char* smpbuf = malloc(smplen); // new unsigned char[smpLen[iInstrIdx]];
	psyfile_read(self->file, smpbuf, smplen);

	sampleCnt = smplen;
	_wave->channels.samples[0] = dsp.memory_alloc(smplen, sizeof(psy_dsp_amp_t));
	_wave->numframes = smplen;
	// 8 bit mono sample
	for (j = 0; j < sampleCnt; j++)
	{
		//In mods, samples are signed integer, so we can simply left shift
		wNew = (smpbuf[j] << 8);
		_wave->channels.samples[0][j] = wNew;
	}			
	// cleanup
	free(smpbuf);
}

void psy_audio_mod_loadpatterns(psy_audio_SongFile* self,
	struct MODHEADER *m_Header, psy_List* modtovirtual)
{	
	uintptr_t j;

	// get pattern data
	psyfile_seek(self->file, 1084);
	for (j = 0; j < m_Header->songlength; ++j) {
		psy_audio_mod_loadsinglepattern(self, j, self->song->properties.tracks,
			modtovirtual);
	}
	psy_audio_mod_makesequence(self, m_Header);
//	if (speedpatch) {
	//	song.SONGTRACKS++;
	//}
}

void psy_audio_mod_makesequence(psy_audio_SongFile* self, struct MODHEADER* m_Header)
{
	uintptr_t i;
	psy_audio_SequencePosition sequenceposition;

	sequenceposition.track =
		psy_audio_sequence_appendtrack(&self->song->sequence, psy_audio_sequencetrack_allocinit());
	for (i = 0; i < m_Header->songlength; ++i) {
		sequenceposition.trackposition =
			psy_audio_sequence_last(&self->song->sequence, sequenceposition.track);
		psy_audio_sequence_insert(&self->song->sequence, sequenceposition,
			m_Header->order[i]);
	}
}

// return address of next pattern, 0 for invalid
void psy_audio_mod_loadsinglepattern(psy_audio_SongFile* self, int patIdx, int iTracks, psy_List* modtovirtual)
{

	short iNumRows = 64;
	psy_audio_Pattern* pattern;
	unsigned char lastmach[64];
	psy_audio_PatternEvent e;
	unsigned char mentry[4];
	psy_audio_PatternNode* node;
	int row;
	int col;

	memset(lastmach, 255, sizeof(char) * 64);
	pattern = psy_audio_pattern_allocinit();	
	//psy_audio_pattern_setname(pattern, "unnamed");
	//psy_audio_pattern_setlength(pattern, 0.25 * iNumRows);
	psy_audio_patterns_insert(&self->song->patterns, patIdx, pattern);
	node = 0;
	// get next values
	for (row = 0; row < iNumRows; row++)
	{
		for (col = 0; col < iTracks; col++)
		{
			// reset
			unsigned char note = psy_audio_NOTECOMMANDS_EMPTY;
			unsigned char instr = 255;
			unsigned char type = 0;
			unsigned char param = 0;
			unsigned short period = 428;
			int exchwave[3] = { psy_audio_WAVEFORMS_SINUS,
				psy_audio_WAVEFORMS_SAWDOWN,
				psy_audio_WAVEFORMS_SQUARE
			};

			// read note
			mentry[0] = ReadUInt1(self->file); mentry[1] = ReadUInt1(self->file);
			mentry[2] = ReadUInt1(self->file); mentry[3] = ReadUInt1(self->file);
			instr = ((mentry[0] & 0xF0) + (mentry[2] >> 4));
			period = ((mentry[0] & 0x0F) << 8) + mentry[1];
			type = (mentry[2] & 0x0F);
			param = mentry[3];
			note = psy_audio_mod_convertperiodtonote(period);

			psy_audio_patternevent_clear(&e);
			// translate
			e.parameter = param;
			
			switch (type) {
			case XMCMD_ARPEGGIO:
				if (param != 0) {
					e.cmd = XM_SAMPLER_CMD_ARPEGGIO;
				} else {
					e.cmd = XM_SAMPLER_CMD_NONE;
				}
				break;
			case XMCMD_PORTAUP:
				e.cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
				break;
			case XMCMD_PORTADOWN:
				e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
				break;
			case XMCMD_PORTA2NOTE:
				e.cmd = XM_SAMPLER_CMD_PORTA2NOTE;
				break;
			case XMCMD_VIBRATO:
				e.cmd = XM_SAMPLER_CMD_VIBRATO;
				break;
			case XMCMD_TONEPORTAVOL:
				e.cmd = XM_SAMPLER_CMD_TONEPORTAVOL;
				break;
			case XMCMD_VIBRATOVOL:
				e.cmd = XM_SAMPLER_CMD_VIBRATOVOL;
				break;
			case XMCMD_TREMOLO:
				e.cmd = XM_SAMPLER_CMD_TREMOLO;
				break;
			case XMCMD_PANNING:
				e.cmd = XM_SAMPLER_CMD_PANNING;
				break;
			case XMCMD_OFFSET:
				e.cmd = XM_SAMPLER_CMD_OFFSET;
				break;
			case XMCMD_VOLUMESLIDE:
				e.cmd = XM_SAMPLER_CMD_VOLUMESLIDE;
				e.parameter = param;
				break;
			case XMCMD_POSITION_JUMP:
				//e.cmd = XM_SAMPLER_CMD_JUMP_TO_ORDER;
				break;
			case XMCMD_VOLUME:
				e.cmd = XM_SAMPLER_CMD_VOLUME;
				e.parameter = param; // <= 0x40 ? param * 2 : 0x80;
				break;
			case XMCMD_PATTERN_BREAK:
				//e._cmd = psy_audio_PatternCmd::psy_audio_PATTERNCMD_BREAK_TO_LINE;
				// e._parameter = ((param & 0xF0) >> 4) * 10 + (param & 0x0F);
				break;
			case XMCMD_EXTENDED:
				switch (param & 0xf0) {
				case XMCMD_E_FINE_PORTA_UP:
					e.cmd = XM_SAMPLER_CMD_PORTAMENTO_UP;
					e.parameter = 0xF0 + (param & 0x0F);
					break;
				case XMCMD_E_FINE_PORTA_DOWN:
					e.cmd = XM_SAMPLER_CMD_PORTAMENTO_DOWN;
					e.parameter = 0xF0 + (param & 0x0F);
					break;
				case XMCMD_E_GLISSANDO_STATUS:
					e.cmd = XM_SAMPLER_CMD_EXTENDED;
					e.parameter = XM_SAMPLER_CMD_E_GLISSANDO_TYPE | ((param == 0) ? 0 : 1);
					break;
				case XMCMD_E_VIBRATO_WAVE:
					e.cmd = XM_SAMPLER_CMD_EXTENDED;
					e.parameter = XM_SAMPLER_CMD_E_VIBRATO_WAVE | exchwave[param & 0x3];
					break;
				case XMCMD_E_FINETUNE:
					e.cmd = XM_SAMPLER_CMD_NONE;
					e.parameter = 0;
					break;
				case XMCMD_E_PATTERN_LOOP:
					//e.cmd = psy_audio_PatternCmd::psy_audio_PATTERNCMD_EXTENDED;
					//e.parameter = psy_audio_PatternCmd::psy_audio_PATTERNCMD_PATTERN_LOOP | (param & 0xf);
					break;
				case XMCMD_E_TREMOLO_WAVE:
					// e._cmd = XMSampler::CMD::psy_audio_PATTERNCMD_EXTENDED;
					// e._parameter = XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[param & 0x3];
					break;
				case XMCMD_E_MOD_RETRIG:
					// e._cmd = XMSampler::CMD::RETRIG;
					// e._parameter = param & 0xf;
					break;
				case XMCMD_E_FINE_VOLUME_UP:
					// e._cmd = XMSampler::CMD::VOLUMESLIDE;
					// e._parameter = 0x0f + ((param & 0xf) << 4);
					break;
				case XMCMD_E_FINE_VOLUME_DOWN:
					// e._cmd = XMSampler::CMD::VOLUMESLIDE;
					// e._parameter = 0xf0 + (param & 0xf);
					break;
				case XMCMD_E_DELAYED_NOTECUT:
					e.cmd = XM_SAMPLER_CMD_EXTENDED;
					e.parameter = XM_SAMPLER_CMD_E_DELAYED_NOTECUT | (param & 0xf);
					break;
				case XMCMD_E_NOTE_DELAY:
					e.cmd = XM_SAMPLER_CMD_EXTENDED;
					e.parameter = XM_SAMPLER_CMD_E_NOTE_DELAY | (param & 0xf);
					break;
				case XMCMD_E_PATTERN_DELAY:
					//e.cmd = psy_audio_PatternCmd::psy_audio_PATTERNCMD_EXTENDED;
					//e.parameter = psy_audio_PatternCmd::psy_audio_PATTERNCMD_PATTERN_DELAY | (param & 0xf);
					break;
				default:
					e.cmd = XM_SAMPLER_CMD_NONE;
					e.parameter = 0;
					break;
				}
				break;
			case XMCMD_SETSPEED:
				/*if (param < 32)
				{
					e._cmd = psy_audio_PatternCmd::psy_audio_PATTERNCMD_EXTENDED;
					int extraticks = 0;
					e._parameter = XMSampler::CalcLPBFromSpeed(param, extraticks);
					if (extraticks != 0) {
						speedpatch = true;
						PatternEntry entry(notecommands::empty, 0xFF, 0xFF, 0, 0);
						entry._cmd = psy_audio_PatternCmd::psy_audio_PATTERNCMD_EXTENDED;
						entry._parameter = psy_audio_PatternCmd::psy_audio_PATTERNCMD_ROW_EXTRATICKS | extraticks;
						WritePatternEntry(song, patIdx, row, song.SONGTRACKS, entry);
					}
				} else
				{
					e._cmd = psy_audio_PatternCmd::psy_audio_PATTERNCMD_SET_TEMPO;
				}*/
				break;
			default:
				e.cmd = XM_SAMPLER_CMD_NONE;
				break;
			}
			// instrument/note
			if (note != psy_audio_NOTECOMMANDS_EMPTY) e.note = note + 12;
			else e.note = note;
			if (instr != 0) e.inst = instr; // -1;
			else e.inst = psy_audio_NOTECOMMANDS_INST_EMPTY;

			// If empty, do not inform machine
			if (e.note == psy_audio_NOTECOMMANDS_EMPTY && e.inst == psy_audio_NOTECOMMANDS_INST_EMPTY && e.cmd == 00 && e.parameter == 00) {
				e.mach = psy_audio_NOTECOMMANDS_MACH_EMPTY;
			}
			// if instrument without note, or note without instrument, cannot use virtual instrument, so use sampulse directly
			else if ((e.note == psy_audio_NOTECOMMANDS_EMPTY && e.inst != psy_audio_NOTECOMMANDS_INST_EMPTY) || (e.note < psy_audio_NOTECOMMANDS_RELEASE && e.inst == psy_audio_NOTECOMMANDS_INST_EMPTY)) {
				e.mach = 0;
				if (e.inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
					//We cannot use the virtual instrument, but we should remember which it is.
					//std::map<int, int>::const_iterator it = modtovirtual.find(e._inst);
					//if (it != modtovirtual.end()) {
						//lastmach[col] = it->second;
					//}
				}
			}
			//default behaviour, let's find the virtual instrument.
			else {
				/*std::map<int, int>::const_iterator it = modtovirtual.find(e._inst);
				if (it == modtovirtual.end()) {
					if (e._inst != 255) {
						e._mach = 0;
						lastmach[col] = e._mach;
					} else if (lastmach[col] != 255) {
						e._mach = lastmach[col];
					} else {
						e._mach = 255;
					}
				} else {
					e._mach = it->second;
					e._inst = 255;
				}*/
			}						
			if (!psy_audio_patternevent_empty(&e)) {
				e.mach = 0;
				node = psy_audio_pattern_insert(pattern, node, col, (psy_dsp_beat_t)(row * 0.25), &e);
			}
		}		
	}
}

uint8_t psy_audio_mod_convertperiodtonote(uint16_t period)
{
	if (period == 0) {
		return 255;
	} else if (BIGMODPERIODTABLE[295] <= period && period <= BIGMODPERIODTABLE[0]) {
		int count2 = 0;
		for (; count2 < 37; count2++)
		{
			if (period == BIGMODPERIODTABLE[count2 * 8]) {
				break;
			} else if (period > BIGMODPERIODTABLE[count2 * 8]) {
				if ((count2 * 8) - 4 < 0 || period < BIGMODPERIODTABLE[(count2 * 8) - 4]) {
					break;
				}
				count2--;
				break;
				// TODO: Supposedly, we should add the command XMCMD_E::E_FINETUNE
			}
		}
		return count2 + 36 - 1;
	} else {
		double note = (152.89760383681376337437517761588 // 48 + 12*log2(1.0/428.0)
			- log10(period) * 39.863137138648348174443833153873); //12/log10(2);
		return (unsigned char)(note);
	}
}

void psy_audio_xm_save(struct psy_audio_SongFile* self)
{
	XMSongExport xmexport;

	xmsongexport_init(&xmexport);
	xmsongexport_exportsong(&xmexport, self);
	xmsongexport_dispose(&xmexport);
}
