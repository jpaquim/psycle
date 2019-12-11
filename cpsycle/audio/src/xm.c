// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "xm.h"
#include "xmdefs.h"
#include "songio.h"

#include <stdlib.h>
#include <string.h>

static void xm_readheader(SongFile*, struct XMFILEHEADER*);
static void xm_readpatterns(SongFile*, struct XMFILEHEADER*);
static void xm_makesequence(SongFile*, struct XMFILEHEADER*);

static uint16_t flip16(uint16_t value) 
{
	uint16_t rv;

	rv = ((value >> 8) & 0x0F) | ((value << 8) & 0xF0);
	return rv;
}

void xm_load(SongFile* self)
{	
	SongProperties songproperties;
	char name[20];
	char comments[4096];
	char trackername[20];
	uint8_t trackername_follows;
	uint16_t xmversion;
	uint32_t beginheader;
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
	psyfile_seek(self->file, beginheader + xmheader.size);
	xm_readpatterns(self, &xmheader);
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

void xm_readpatterns(SongFile* self, struct XMFILEHEADER *xmheader)
{	
	uintptr_t slot;	

	for (slot = 0; slot < xmheader->patterns; ++slot) {
		struct XMPATTERNHEADER patternheader;
		Pattern* pattern;

		patternheader.size = psyfile_read_uint32(self->file);
		patternheader.packingtype = psyfile_read_uint8(self->file);
		patternheader.rows = psyfile_read_uint16(self->file);
		patternheader.packedsize = psyfile_read_uint16(self->file);

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
