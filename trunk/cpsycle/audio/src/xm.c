// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xm.h"
#include "xmdefs.h"
#include "songio.h"

#include <stdlib.h>
#include <string.h>

static void xm_readheader(SongFile*, struct XMFILEHEADER*);
static uint32_t xm_readpatterns(SongFile*, struct XMFILEHEADER*);
static void xm_readinstruments(SongFile*, struct XMFILEHEADER*, uint32_t start);
static void xm_makesequence(SongFile*, struct XMFILEHEADER*);

static uint16_t flip16(uint16_t value) 
{
	uint16_t rv;

	rv = ((value >> 8) & 0x0F) | ((value << 8) & 0xF0);
	return rv;
}

void xm_load(SongFile* self)
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
	song_setproperties(self->song, &songproperties);
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

void xm_readheader(SongFile* self, struct XMFILEHEADER *xmheader)
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
		patterns_setsongtracks(&self->song->patterns, xmheader->channels);
	} else {
		patterns_setsongtracks(&self->song->patterns, 2);
	}
	self->song->properties.bpm = (psy_dsp_beat_t) xmheader->speed;
}

uint32_t xm_readpatterns(SongFile* self, struct XMFILEHEADER *xmheader)
{	
	uintptr_t slot;	
	uint32_t nextstart;

	nextstart = psyfile_getpos(self->file);
	for (slot = 0; slot < xmheader->patterns; ++slot) {
		struct XMPATTERNHEADER patternheader;
		Pattern* pattern;

		psyfile_seek(self->file, nextstart);
		patternheader.size = psyfile_read_uint32(self->file);
		patternheader.packingtype = psyfile_read_uint8(self->file);
		patternheader.rows = psyfile_read_uint16(self->file);
		patternheader.packedsize = psyfile_read_uint16(self->file);
		nextstart += patternheader.size;

		pattern = pattern_allocinit();
		pattern_setlength(pattern, patternheader.rows * (psy_dsp_beat_t) 0.25);
		patterns_insert(&self->song->patterns, slot, pattern);
		if (patternheader.packedsize > 0) {
			unsigned char* packeddata;
			int i = 0;
			int c = 0;
			int track = 0;
			int line = 0;
			int insert = 0;
			psy_dsp_beat_t bpl = 0.25;
			PatternEvent ev;
			PatternNode* node = 0;

			nextstart += patternheader.packedsize;
			patternevent_clear(&ev);
			packeddata = malloc(patternheader.packedsize);
			psyfile_read(self->file, packeddata, patternheader.packedsize);
			while (i < patternheader.packedsize) {
				unsigned char note= NOTECOMMANDS_EMPTY;
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

				note = (note & 0x7F);
				if (note >= 97) {
					note = NOTECOMMANDS_RELEASE;
				} else 
				if (note > 0) {
					--note;
				} else {
					note = 255;
				}
				patternevent_init(&ev, note, instr, 0, 0, 0);
				if (!patternevent_empty(&ev)) {
					node = pattern_insert(pattern, node, track, line * bpl, &ev);
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

void xm_readinstruments(SongFile* self, struct XMFILEHEADER *xmheader, uint32_t start)
{
	uintptr_t slot;	

	for (slot = 0; slot < xmheader->instruments; ++slot) {
		struct XMINSTRUMENTHEADER instrumentheader;		
		Instrument* instrument;

		instrument = instrument_allocinit();
			instruments_insert(&self->song->instruments, instrument, slot);
		start = psyfile_getpos(self->file);
		instrumentheader.size = psyfile_read_uint32(self->file);
		psyfile_read(self->file, &instrumentheader.name,
			sizeof(instrumentheader.name));
		instrumentheader.name[sizeof(instrumentheader.name) - 1] = '\0';
		instrumentheader.type = psyfile_read_uint8(self->file);
		instrumentheader.samples = psyfile_read_uint16(self->file);

		start += instrumentheader.size;
		if (instrumentheader.samples >  0) {
			struct XMSAMPLEHEADER sampleheader;
			XMSAMPLESTRUCT* xmsamples = 0;
			int s;		
			
			// Sample header size
			psyfile_read(self->file, &sampleheader.shsize, 4);
			// Sample number for all notes
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
			xmsamples = (XMSAMPLESTRUCT*) malloc(sizeof(
				XMSAMPLESTRUCT) * (int)instrumentheader.samples);
			psyfile_seek(self->file, start);
			for (s = 0; s < instrumentheader.samples; ++s) {				
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
			}		
			for (s = 0; s < instrumentheader.samples; ++s) {
				Sample* sample;				

				sample = sample_allocinit();
				sample_setname(sample, xmsamples[s].name);
				sample->numframes = xmsamples[s].samplen;
				buffer_resize(&sample->channels, 1);				
				if (xmsamples[s].samplen) {
					unsigned char* pData;
					int16_t oldvalue;
					uintptr_t i;

					sample->channels.samples[0] = malloc(sizeof(float)
						* sample->numframes);
					pData = malloc(xmsamples[s].samplen);
					psyfile_read(self->file, pData, xmsamples[s].samplen);
					oldvalue = 0;
					for (i = 0; i < sample->numframes; ++i) {
						int8_t value;
						
						value = pData[i] + oldvalue;
						sample->channels.samples[0][i] = (float) value * 256;
							;
					}
					free(pData);
				}
				samples_insert(&self->song->samples, sample,
					sampleindex_make(slot, s));
			}
			free(xmsamples);			
		}
	}
}

void xm_makesequence(SongFile* self, struct XMFILEHEADER *xmheader)
{
	uintptr_t i;
	SequencePosition sequenceposition;

	sequenceposition.track =
		sequence_appendtrack(&self->song->sequence, sequencetrack_allocinit());
	for (i = 0; i < xmheader->norder; ++i) {
		sequenceposition.trackposition =
			sequence_last(&self->song->sequence, sequenceposition.track);
		sequence_insert(&self->song->sequence, sequenceposition,
			xmheader->order[i]);
	}
}
