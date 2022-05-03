/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "lysongexport.h"
/* local */
#include "constants.h"
#include "song.h"
#include "songio.h"
/* dsp */
#include <notestab.h>
/* platform */
#include "../../detail/portable.h"

/* save without null char */
static int writestring(PsyFile* file, const char* str)
{
	if (str == NULL) {
		return psyfile_write(file, "", psy_strlen(""));
	}
	return psyfile_write(file, str, psy_strlen(str));
}

/* prototypes */
static int psy_audio_lysongexport_write_header(psy_audio_LySongExport*);
static int psy_audio_lysongexport_write_songinfo(psy_audio_LySongExport*);
static int psy_audio_lysongexport_write_seqd(psy_audio_LySongExport*);
static int psy_audio_lysongexport_write_patd(psy_audio_LySongExport*,
	uintptr_t patindex);
/* implementation */
void psy_audio_lysongexport_init(psy_audio_LySongExport* self,
	psy_audio_SongFile* songfile)
{
	assert(self);

	assert(songfile);
	assert(songfile->file);
	assert(songfile->song);

	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
	self->bemol = TRUE;
}

void psy_audio_lysongexport_dispose(psy_audio_LySongExport* self)
{

}

int psy_audio_lysongexport_save(psy_audio_LySongExport* self)
{	
	int status;
	
	if (status = psy_audio_lysongexport_write_header(self)) {
		return status;
	}	
	if (status = psy_audio_lysongexport_write_songinfo(self)) {
		return status;
	}
	if (status = psy_audio_lysongexport_write_seqd(self)) {
		return status;
	}
	return PSY_OK;
}

int psy_audio_lysongexport_write_header(psy_audio_LySongExport* self)
{
	assert(self);
	
	writestring(self->fp, "\\version \"2.22.1\"\n");
	return PSY_OK;
}

int psy_audio_lysongexport_write_songinfo(psy_audio_LySongExport* self)
{
	assert(self);

	writestring(self->fp, "\\header{\n");
	writestring(self->fp, "title = \"");
	writestring(self->fp, psy_audio_song_title(self->song));
	writestring(self->fp, "\"\n");
	writestring(self->fp, "composer = \"");
	writestring(self->fp, psy_audio_song_credits(self->song));
	writestring(self->fp, "\"\n");
	writestring(self->fp, "subtitle = \"");
	writestring(self->fp, psy_audio_song_comments(self->song));
	writestring(self->fp, "\"\n");
	writestring(self->fp, "}\n");	
	return PSY_OK;
}

int psy_audio_lysongexport_write_seqd(psy_audio_LySongExport* self)
{
	int32_t index;	

	assert(self);
	assert(self->song);
	
	for (index = 0; index < psy_audio_sequence_width(&self->song->sequence);
			++index) {
		psy_audio_SequenceTrack* track;	
		psy_List* s;

		track = psy_audio_sequence_track_at(&self->song->sequence, index);
		if (!track) {
			continue;
		}
		for (s = track->nodes; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			/* sequence data */
			if (seqentry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_SequencePatternEntry* patentry;

				patentry = (psy_audio_SequencePatternEntry*)seqentry;
				psy_audio_lysongexport_write_patd(self, patentry->patternslot);
			}
		}
	}	
	return PSY_OK;
}

int psy_audio_lysongexport_write_patd(psy_audio_LySongExport* self,
	uintptr_t patindex)
{
	psy_audio_Pattern* pattern;

	pattern = psy_audio_patterns_at(&self->song->patterns, patindex);
	if (pattern) {
		psy_audio_PatternNode* node;
		psy_dsp_big_beat_t length;
		bool lengthchanged;
		bool lineschanged;
		psy_dsp_big_beat_t lines;
		bool addpoint;
		bool repeatline;
		uintptr_t track;
		
		writestring(self->fp, "<<\n");
		for (track = 0; track < MAX_TRACKS; ++track) {
			if (!psy_audio_pattern_track_used(pattern, track)) {
				continue;
			}
			writestring(self->fp, "{\n");			
			lengthchanged = FALSE;
			length = 1.0;
			lines = 4.0;
			lineschanged = FALSE;
			addpoint = FALSE;
			repeatline = FALSE;
			for (node = pattern->events; node != 0; node = node->next) {
				psy_audio_PatternEntry* entry;
				psy_audio_PatternNode* nextnode;
				psy_List* p;
				const char* notestr;

				entry = (psy_audio_PatternEntry*)node->entry;
				if (entry->track != track) {
					continue;
				}
				nextnode = psy_audio_patternnode_next_track(node, entry->track);
				if (nextnode) {
					psy_audio_PatternEntry* nextentry;
					psy_dsp_big_beat_t newlength;
					psy_dsp_big_beat_t newlines;
					psy_dsp_big_beat_t d;

					nextentry = (psy_audio_PatternEntry*)nextnode->entry;
					newlength = nextentry->offset - entry->offset;
					newlines = 4 / newlength;
					d = newlines - floor(newlines);
					if (d > 1 / 64.0) {
						addpoint = TRUE;
						newlines = floor(newlines) * 2;
					}
					if (newlines != lines) {
						lines = newlines;
						lineschanged = TRUE;
					}
					if (newlength != length) {
						lengthchanged = TRUE;
						length = newlength;
					}
				}
				for (p = entry->events; p != NULL; psy_list_next(&p)) {
					psy_audio_PatternEvent* ev;

					ev = (psy_audio_PatternEvent*)psy_list_entry(p);
					notestr = psy_dsp_notetostr(ev->note,
						(self->bemol)
						? psy_dsp_NOTESTAB_LY_BEMOL_A440
						: psy_dsp_NOTESTAB_LY_SHARP_A440);
					writestring(self->fp, notestr);
					if (lengthchanged) {
						char lenstr[64];

						if (lineschanged || repeatline || addpoint) {
							psy_snprintf(lenstr, 64, "%d", (int)((floor)(lines)));
							writestring(self->fp, lenstr);
							lineschanged = FALSE;
							repeatline = FALSE;
						}
						if (addpoint) {
							writestring(self->fp, ".");
							addpoint = FALSE;
							repeatline = TRUE;
						}
						lengthchanged = FALSE;
					}
					if (p->next) {
						writestring(self->fp, " ");
					}
				} /* patternentry loop end */
				if (node->next) {
					writestring(self->fp, " ");
				}
			} /* patternnode loop end */
			writestring(self->fp, "\n}\n");
		} /* track loop end */
		writestring(self->fp, ">>\n");		
	}
	return PSY_OK;
}
