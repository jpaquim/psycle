/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "psy3saver.h"
/* local */
#include "constants.h"
#include "plugin_interface.h"
#include "psyconvert.h"
#include "song.h"
#include "songio.h"
#include "machinefactory.h"
#include "wire.h"
/* dsp */
#include <datacompression.h>
#include <operations.h>
#include <valuemapper.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static uint32_t psy_audio_psy3saver_chunkcount(psy_audio_PSY3Saver*);
static uint32_t psy_audio_instruments_smidcount(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_header(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_songinfo(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_sngi(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_seqd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_patd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_epat(psy_audio_PSY3Saver*, int32_t index,
	psy_audio_Pattern*);
static int psy_audio_psy3saver_write_macd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_insd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_smid(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_smsb(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_virg(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_machine(psy_audio_PSY3Saver*,
	psy_audio_Machine*, uint32_t slot);
static void psy_audio_psy3saver_savedllnameandindex(psy_audio_PSY3Saver*,
	const char* name, uintptr_t shellindex);
int psy_audio_psy3saver_saveparammapping(psy_audio_PSY3Saver*,
	psy_audio_Machine*);
int psy_audio_psy3saver_saveismachinebus(psy_audio_PSY3Saver*,
	psy_audio_Machine*);
static int psy_audio_psy3saver_write_connections(psy_audio_PSY3Saver*,
	uintptr_t slot);
static int psy_audio_psy3saver_save_instrument(psy_audio_PSY3Saver*,
	psy_audio_Instrument*);
static int psy_audio_psy3saver_xminstrument_save(psy_audio_PSY3Saver*,
	psy_audio_Instrument*, int32_t version);
static int psy_audio_psy3saver_write_smie(psy_audio_PSY3Saver*,
	psy_dsp_Envelope*, uint32_t version);
static int psy_audio_psy3saver_save_sample(psy_audio_PSY3Saver*,
	psy_audio_Sample*);
static int16_t* psy_audio_psy3saver_clone_array_float_to_int16(float* buffer,
	uintptr_t numframes);

void psy_audio_psy3saver_init(psy_audio_PSY3Saver* self,
	psy_audio_SongFile* songfile)
{
	assert(self);
	assert(songfile);	
	assert(songfile->file);
	assert(songfile->song);

	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
}

void psy_audio_psy3saver_dispose(psy_audio_PSY3Saver* self)
{
	assert(self);
}

int psy_audio_psy3saver_save(psy_audio_PSY3Saver* self)
{
	uint32_t chunkcount;
	int status;

	assert(self);
	assert(self->songfile);
	assert(self->fp);
	assert(self->song);
	
	chunkcount = psy_audio_psy3saver_chunkcount(self);
	if ((status = psy_audio_psy3saver_write_header(self))) {
		return status;
	}
	/* the rest of the modules can be arranged in any order */
	if ((status = psy_audio_psy3saver_write_songinfo(self))) {
		return status;
	}
	if ((status = psy_audio_psy3saver_write_sngi(self))) {
		return status;
	}
	if ((status = psy_audio_psy3saver_write_seqd(self))) {
		return status;
	} 
	if ((status = psy_audio_psy3saver_write_patd(self))) {
		return status;
	}	
	if ((status = psy_audio_psy3saver_write_macd(self))) {
		return status;
	}	
	if ((status = psy_audio_psy3saver_write_insd(self))) {
		return status;
	}
	if ((status = psy_audio_psy3saver_write_smid(self))) {
		return status;
	}	
	if ((status = psy_audio_psy3saver_write_smsb(self))) {
		return status;
	}
	if ((status = psy_audio_psy3saver_write_virg(self))) {
		return status;
	}
	return PSY_OK;
}

uint32_t psy_audio_psy3saver_chunkcount(psy_audio_PSY3Saver* self)
{	
	uint32_t rv;

	assert(self);

	/* 2 chunks(INFO, SNGI.SONG is not counted as a chunk) plus: */
	rv = 2;
	/* PATD */
	rv += (uint32_t)psy_audio_patterns_size(&self->song->patterns);
	/* MACD */
	rv += (uint32_t)psy_audio_machines_size(&self->song->machines);
	/* INSD */
	rv += (uint32_t)psy_audio_instruments_size(&self->song->instruments, 0);
	/* SMID */
	rv += (uint32_t)psy_audio_instruments_smidcount(self);
	/* SMSB */
	rv += (uint32_t)psy_audio_samples_count(&self->song->samples);
	/* SEQD */
	rv += (uint32_t)psy_audio_sequence_width(&self->song->sequence);
	return rv;
}

uint32_t psy_audio_instruments_smidcount(psy_audio_PSY3Saver* self)
{
	uint32_t rv;
	psy_TableIterator it;		
	
	assert(self);

	rv = 0;
	for (it = psy_audio_instruments_begin(&self->song->instruments);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_InstrumentsGroup* group;
		psy_TableIterator it_group;

		if (psy_tableiterator_key(&it) == 0) {
			/* group 0 is counted already as insd */
			continue;
		}		
		group = (psy_audio_InstrumentsGroup*)psy_tableiterator_value(&it);		
		for (it_group = psy_table_begin(&group->container);
				!psy_tableiterator_equal(&it_group, psy_table_end());
				psy_tableiterator_inc(&it_group)) {
			++rv;
		}
	}
	return rv;
}

int psy_audio_psy3saver_write_header(psy_audio_PSY3Saver* self)
{		
	int status;
	uint32_t pos;
	uint32_t chunkcount;	

	assert(self);

	/* id = "PSY3SONG"; // PSY2 was 0.96 to 1.7.2 */
	if ((status = psyfile_write(self->fp, "PSY3",4))) {
		return status;
	}
	if ((status = psyfile_writeheader(self->fp, "SONG",
			CURRENT_FILE_VERSION, 0, &pos))) {
		return status;
	}
	chunkcount = psy_audio_psy3saver_chunkcount(self);
	if ((status = psyfile_write(self->fp, &chunkcount,
			sizeof(chunkcount)))) {
		return status;
	}
	if ((status = psyfile_writestring(self->fp, PSYCLE__TITLE))) {
		return status;
	}
	if ((status = psyfile_writestring(self->fp, PSYCLE__VERSION))) {
		return status;
	}
	if ((status = psyfile_updatesize(self->fp, pos, NULL))) {
		return status;
	}
	return PSY_OK;
}

int psy_audio_psy3saver_write_songinfo(psy_audio_PSY3Saver* self)
{		
	int status;
	uint32_t sizepos;	
	
	assert(self);

	if ((status = psyfile_writeheader(self->fp, "INFO",
			CURRENT_FILE_VERSION_INFO, 0, &sizepos))) {
		return status;
	}
	if ((status = psyfile_writestring(self->fp,
			psy_audio_song_title(self->song)))) {
		return status;
	}
	if ((status = psyfile_writestring(self->fp,
			psy_audio_song_credits(self->song)))) {
		return status;
	}
	if ((status = psyfile_writestring(self->fp,
			psy_audio_song_comments(self->song)))) {
		return status;
	}
	if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
		return status;
	}
	return PSY_OK;
}

/*
** ================== =
** song info
** ===================
*/
/* id = "SNGI"; */
int psy_audio_psy3saver_write_sngi(psy_audio_PSY3Saver* self)
{
	int status;
	uint32_t sizepos;	
	uint32_t i;
	int32_t temp;

	assert(self);

	if ((status = psyfile_writeheader(self->fp, "SNGI",
			CURRENT_FILE_VERSION_SNGI, 0, &sizepos))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, (int32_t)
			psy_audio_song_num_song_tracks(self->song)))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, (int32_t)
			psy_audio_song_bpm(self->song)))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, (int32_t)
			psy_audio_song_lpb(self->song)))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, (int32_t)
			psy_audio_song_octave(self->song)))) {
		return status;
	}
	/* machinesoloed */
	if (psy_audio_machines_soloed(&self->song->machines) !=
			psy_INDEX_INVALID) {
		temp = (int32_t)psy_audio_machines_soloed(&self->song->machines);
	} else {
		temp = -1;
	}
	if ((status = psyfile_write(self->fp, &temp, sizeof(int32_t)))) {
		return status;
	}
	/* tracksoloed */
	if (psy_audio_patterns_tracksoloed(&self->song->patterns) !=
			psy_INDEX_INVALID) {
		temp = (int32_t)psy_audio_patterns_tracksoloed(&self->song->patterns);
	} else {
		temp = -1;
	}
	if ((status = psyfile_write(self->fp, &temp, sizeof(int32_t)))) {
		return status;
	}
	/* seqbus */
	if (psy_audio_machines_selected(&self->song->machines) !=
			psy_INDEX_INVALID) {
		temp = (int32_t)psy_audio_machines_selected(&self->song->machines);
	} else {
		temp = 0;
	}
	if ((status = psyfile_write(self->fp, &temp, sizeof(int32_t)))) {
		return status;
	}
	/* paramselected */
	if ((status = psyfile_write_int32(self->fp, 0))) {
		return status;
	}
	/* auxcolselected */
	if ((status = psyfile_write_int32(self->fp, 0))) {
		return status;
	}
	/* instselected */
	if (psy_audio_instruments_selected(&self->song->instruments).subslot !=
			psy_INDEX_INVALID) {
		temp = (int32_t)
			psy_audio_instruments_selected(&self->song->instruments).subslot;
	} else {
		temp = -1;
	}
	if ((status = psyfile_write(self->fp, &temp, sizeof(int32_t)))) {
		return status;
	}	
	/* sequence width */
	if (psy_audio_sequence_width(&self->song->sequence) > 0) {
		temp = (int32_t)psy_audio_sequence_width(&self->song->sequence);
	} else {
		/* save at least one sequence track */
		temp = 1;
	}
	if ((status = psyfile_write(self->fp, &temp, sizeof(int32_t)))) {
		return status;
	}
	/* pattern track muted/armed */
	for (i = 0; i < self->song->patterns.songtracks; ++i) {
		uint8_t temp8;

		temp8 = psy_audio_patterns_istrackmuted(&self->song->patterns, i);
		if ((status = psyfile_write(self->fp, &temp8, sizeof(uint8_t)))) {
			return status;
		}
		 /* remember to count them */
		temp8 = psy_audio_patterns_istrackarmed(&self->song->patterns, i);
		if ((status = psyfile_write(self->fp, &temp8, sizeof(uint8_t)))) {
			return status;
		}
	}
	/* shareTrackNames */
	if ((status = psyfile_write_uint8(self->fp, 0))) {
		return status;
	}
	if (0) {
		uint32_t t;

		for(t = 0; t < psy_audio_song_num_song_tracks(self->song); ++t) {
			/*_trackNames[0][t]); */
			if ((status = psyfile_writestring(self->fp, ""))) {
				return status;
			}; 
		}
	}	
	/* ticks per beat */
	if ((status = psyfile_write_int32(self->fp, (int32_t)
			psy_audio_song_tpb(self->song)))) {
		return status;
	}
	/* extraticks per beat */
	if ((status = psyfile_write_int32(self->fp, (int32_t)
			psy_audio_song_extra_ticks_per_beat(self->song)))) {
		return status;
	}
	/* sampler index */
	if ((status = psyfile_write_uint32(self->fp, (uint32_t)
			psy_audio_song_sampler_index(self->song)))) {
		return status;
	}
	if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
		return status;
	}
	return PSY_OK;
}

/*
** ===================
** Sequence data
** ==================
*/
/* id = "SEQD"; */
int psy_audio_psy3saver_write_seqd(psy_audio_PSY3Saver* self)
{		
	int status;
	int32_t index;	

	assert(self);
	assert(self->song);

	status = PSY_OK;
	for (index = 0; index < psy_audio_sequence_width(&self->song->sequence);
			++index) {
		uint32_t sizepos;
		uint32_t nummarkers;
		uint32_t numsamples;
		psy_audio_SequenceTrack* track;		
		psy_List* s;
		
		if ((status = psyfile_writeheader(self->fp, "SEQD",
				CURRENT_FILE_VERSION_SEQD, 0, &sizepos))) {
			return status;
		}
		track = psy_audio_sequence_track_at(&self->song->sequence, index);
		/* sequence track number */
		if ((status = psyfile_write_int32(self->fp, (int32_t)index))) {
			return status;
		}
		/* sequence length */
		if ((status = psyfile_write_int32(self->fp,
				(int32_t)psy_audio_sequence_track_size(
					&self->song->sequence, index)))) {
			return status;
		}
		/* sequence name */
		if ((status = psyfile_writestring(self->fp,
			(track->name)
				? track->name
				: "seq0"))) {
			return status;
		}		
		nummarkers = 0;
		numsamples = 0;
		for (s = track->nodes; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			/* sequence data */
			if (seqentry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				if ((status = psyfile_write_int32(self->fp, (int32_t)
					psy_audio_sequencepatternentry_patternslot(
						(psy_audio_SequencePatternEntry*)seqentry)))) {
					return status;
				}
			} else if(seqentry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
				++numsamples;
				if ((status = psyfile_write_int32(self->fp, (int32_t)INT32_MAX - 2))) {
					return status;
				}
			} else if (seqentry->type == psy_audio_SEQUENCEENTRY_MARKER) {
				++nummarkers;
				if ((status = psyfile_write_int32(self->fp, (int32_t)INT32_MAX - 1))) {
					return status;
				}
			}
		}
		for (s = track->nodes; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			/* sequence data */
			if ((status = psyfile_write_float(self->fp, (float)
					sequenceentry->repositionoffset))) {
				return status;
			}
		}
		if ((status = psyfile_write_uint32(self->fp, nummarkers))) {
			return status;
		}
		for (s = track->nodes; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			if (seqentry->type == psy_audio_SEQUENCEENTRY_MARKER) {
				psy_audio_SequenceMarkerEntry* marker;

				marker = (psy_audio_SequenceMarkerEntry*)seqentry;
				if ((status = psyfile_write_float(self->fp, (float)marker->length))) {
					return status;
				}
				psyfile_writestring(self->fp, marker->text ? marker->text : "");
			}
		}
		if ((status = psyfile_write_uint32(self->fp, numsamples))) {
			return status;
		}
		for (s = track->nodes; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			if (seqentry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
				psy_audio_SequenceSampleEntry* sample;				

				sample = (psy_audio_SequenceSampleEntry*)seqentry;
				if ((status = psyfile_write_int32(self->fp, (int32_t)sample->sampleindex.slot))) {
					return status;
				}
				sample = (psy_audio_SequenceSampleEntry*)seqentry;
				if ((status = psyfile_write_int32(self->fp, (int32_t)sample->sampleindex.subslot))) {
					return status;
				}
				sample = (psy_audio_SequenceSampleEntry*)seqentry;
				if (sample->samplerindex == psy_INDEX_INVALID) {
					if ((status = psyfile_write_uint32(self->fp, (uint32_t)UINT32_MAX))) {
						return status;
					}
				} else {
					if ((status = psyfile_write_uint32(self->fp, (uint32_t)sample->samplerindex))) {
						return status;
					}
				}
			}
		}
		if ((status = psyfile_write_float(self->fp, (float)track->height))) {
			return status;
		}
		if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
			return status;
		}
	}
	return PSY_OK;
}

/*
**	================== =
**	Pattern data
**	===================
*/
/*	id = "PATD"; */
int psy_audio_psy3saver_write_patd(psy_audio_PSY3Saver* self)
{	
	int status = PSY_OK;	
	int32_t temp;
	unsigned char shareTrackNames;	
	psy_TableIterator it;
	
	assert(self);

	it = psy_audio_patterns_begin(&self->song->patterns);
	for (;	!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		int32_t i;

		/* check every pattern for validity */
		i = (int32_t)psy_tableiterator_key(&it);
		if (psy_audio_sequence_patternused(&self->song->sequence, i) ||
				i == psy_audio_GLOBALPATTERN) {
			/* ok save it */
			psy_audio_Pattern* pattern;
			psy_audio_LegacyPattern pat;
			int32_t patternLines;
			int32_t lpb;
			unsigned char* source;
			unsigned char* copy;						
			int32_t index;			
			int32_t size77;			
			uint32_t sizepos;
			int32_t songtracks;
			int32_t y;
			
			pattern = psy_audio_patterns_at(&self->song->patterns, i);									
			lpb = (int32_t)self->song->properties.lpb;
			pat = psy_audio_allocoldpattern(pattern, lpb, &patternLines);
			/* ok save it */
			songtracks = (int32_t)psy_audio_song_num_song_tracks(self->song);
			source = malloc(songtracks * patternLines * EVENT_SIZE);
			copy = source;
			for (y = 0; y < patternLines; y++)
			{
				unsigned char* pData = pat + (y * MULTIPLY);
				memcpy(copy, pData, EVENT_SIZE * songtracks);
				copy += EVENT_SIZE * songtracks;
			}
			size77 = beerz77comp2(source, &copy,
				songtracks * patternLines * EVENT_SIZE);			
			free(source);
			source = NULL;
			
			index = i; /* index */
			if (i > MAX_PATTERNS) {
				/*
				** old psycle versions cannot handle patterns > 255
				** this let them skip the pattern
				*/
				if ((status = psyfile_writeheader(self->fp, "PATD",
						CURRENT_FILE_VERSION_LPATD, 0, &sizepos))) {
					return status;
				}
			} else {
				if ((status = psyfile_writeheader(self->fp, "PATD",
						CURRENT_FILE_VERSION_PATD, 0, &sizepos))) {
					return status;
				}
			}			
			psyfile_write(self->fp, &index, sizeof(index));
			temp = patternLines;
			psyfile_write(self->fp, &temp, sizeof(temp));
			temp = songtracks; /* eventually this may be variable per pattern */
			psyfile_write(self->fp, &temp, sizeof(temp));

			psyfile_writestring(self->fp, psy_audio_pattern_name(pattern));

			psyfile_write(self->fp, &size77, sizeof(size77));
			psyfile_write(self->fp, copy, size77);
			free(copy);
			copy = NULL;
			free(pat);
			pat = NULL;
			
			shareTrackNames = 0;
			if( !shareTrackNames) {
				uint32_t t;
				for(t = 0; t < self->song->patterns.songtracks; ++t) {
					psyfile_writestring(self->fp, ""); /*_trackNames[i][t]); */
				}
			}
			if ((status = psy_audio_psy3saver_write_epat(self, index, pattern))) {
				return status;
			}
			if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
				return status;
			}
		}
	}
	return status;
}

/*
** =====================
** extended pattern data
** =====================
*/
int psy_audio_psy3saver_write_epat(psy_audio_PSY3Saver* self, int32_t index,
	psy_audio_Pattern* pattern)
{	
	int status = PSY_OK;			
	psy_audio_PatternNode* node;						

	assert(self);

	/* timesig */	
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)pattern->timesig.numerator))) {
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp,
		(uint32_t)pattern->timesig.denominator))) {
		return status;
	}
	/* length */
	if ((status = psyfile_write_float(self->fp, (float)pattern->length))) {
		return status;
	}	
	/* num pattern entries */
	if ((status = psyfile_write_int32(self->fp,
			(int32_t)psy_list_size(pattern->events)))) {
		return status;
	}			
	/* write events */
	for (node = pattern->events; node != 0; node = node->next) {
		psy_audio_PatternEntry* entry;
		psy_List* p;
				
		entry = (psy_audio_PatternEntry*) node->entry;
		if ((status = psyfile_write_int32(self->fp, (int32_t)entry->track))) {
			return status;
		}
		if ((status = psyfile_write_float(self->fp, (float)entry->offset))) {
			return status;
		}				
		/* num events */
		if ((status = psyfile_write_int32(self->fp,
				(int32_t)psy_list_size(entry->events)))) {
			return status;
		}				
		entry = (psy_audio_PatternEntry*) node->entry;				
		for (p = entry->events; p != NULL; psy_list_next(&p)) {
			psy_audio_PatternEvent* ev;

			ev = (psy_audio_PatternEvent*)psy_list_entry(p);
			if ((status = psyfile_write_int32(self->fp, ev->note))) {
				return status;
			}
			if ((status = psyfile_write_int32(self->fp, ev->inst))) {
				return status;
			}
			if ((status = psyfile_write_int32(self->fp, ev->mach))) {
				return status;
			}
			if ((status = psyfile_write_int32(self->fp, ev->vol))) {
				return status;
			}
			if ((status = psyfile_write_int32(self->fp, ev->cmd))) {
				return status;
			}
			if ((status = psyfile_write_int32(self->fp, ev->parameter))) {
				return status;
			}
		}
	}	
	return status;
}

/*
**	===================
**	machine data
**	===================
*/
/*	id = "MACD"; */

int psy_audio_psy3saver_write_macd(psy_audio_PSY3Saver* self)
{
	int32_t index;
	int status = PSY_OK;

	assert(self);

	for (index = 0; index < MAX_MACHINES; ++index) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(&self->song->machines, index);
		if (machine) {			
			uint32_t sizepos;

			if ((status = psyfile_writeheader(self->fp, "MACD",
					CURRENT_FILE_VERSION_MACD, 0, &sizepos))) {
				return status;
			}			
			if ((status = psyfile_write_int32(self->fp, index))) {
				return status;
			}
			psy_audio_psy3saver_write_machine(self, machine, index);
			if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
				return status;
			}
		}
	}
	return status;
}

int psy_audio_psy3saver_write_machine(psy_audio_PSY3Saver* self, psy_audio_Machine* machine,
	uint32_t slot)
{
	const psy_audio_MachineInfo* info;
	int status = PSY_OK;
	double x;
	double y;

	assert(self);

	info = psy_audio_machine_info(machine);
	if (info) {			
		if ((status =psyfile_write_int32(self->fp, (int32_t)info->type))) {
			return status;
		}
		psy_audio_psy3saver_savedllnameandindex(self, info->modulepath, info->shellidx);
		if ((status = psyfile_write_uint8(self->fp, (uint8_t)
				psy_audio_machine_bypassed(machine)))) {
			return status;
		}
		if ((status = psyfile_write_uint8(self->fp, (uint8_t)
				psy_audio_machine_muted(machine)))) {
			return status;
		}
		if ((status = psyfile_write_int32(self->fp, (int32_t)
			(psy_audio_machine_panning(machine) * 128.f)))) {
			return status;
		}
		psy_audio_machine_position(machine, &x, &y);
		if ((status = psyfile_write_int32(self->fp, (int32_t)x))) {
			return status;
		}
		if ((status = psyfile_write_int32(self->fp, (int32_t)y))) {
			return status;
		}
		if ((status = psy_audio_psy3saver_write_connections(self, slot))) {
			return status;
		}
		psyfile_writestring(self->fp, psy_audio_machine_editname(machine)
			? psy_audio_machine_editname(machine) : "");
		psy_audio_machine_savespecific(machine, self->songfile, slot);
		if ((status = psy_audio_machine_savewiremapping(machine, self->songfile, slot))) {
			return status;
		}
		if ((status = psy_audio_psy3saver_saveparammapping(self, machine))) {
			return status;
		}
		if ((status = psy_audio_psy3saver_saveismachinebus(self, machine))) {
			return status;
		}
	}
	return status;
}

int psy_audio_psy3saver_write_connections(psy_audio_PSY3Saver* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;		
	int status = PSY_OK;
	uintptr_t maxkey;
	uintptr_t c;
	bool incon;
	bool outcon;

	assert(self);
	
	sockets = psy_audio_connections_at(&self->song->machines.connections, slot);
	if ((status = psyfile_write_int32(self->fp,
			(int32_t)((sockets) ? wiresockets_size(&sockets->inputs) : 0)))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp,
		(int32_t)((sockets) ? wiresockets_size(&sockets->outputs) : 0)))) {	
		return status;
	}	
	maxkey = MAX_CONNECTIONS - 1;
	for (c = 0; c <= maxkey; ++c) {
		psy_audio_WireSocket* in;
		psy_audio_WireSocket* out;
		float invol;

		if (sockets) {
			in = psy_audio_wiresockets_at(&sockets->inputs, c);
			out = psy_audio_wiresockets_at(&sockets->outputs, c);
		} else {
			in = NULL;
			out = NULL;
		}
		invol = 1.f;
		
		if (in) {								
			invol = in->volume;
			if ((status = psyfile_write_int32(self->fp,
					(int32_t)(in->slot)))) {
				return status;
			}
			incon = TRUE;
		} else {				
			if ((status = psyfile_write_int32(self->fp, -1))) {
				return status;
			}
			incon = FALSE;
		}
		if (out) {					
			if ((status = psyfile_write_int32(self->fp,
					(int32_t)(out->slot)))) {
				return status;
			}
			outcon = TRUE;
		} else {				
			if ((status = psyfile_write_int32(self->fp, -1))) {
				return status;
			}
			outcon = FALSE;
		}
		/* Incoming connections Machine vol */
		if ((status = psyfile_write_float(self->fp, invol))) {
			return status;
		}
		/* Value to multiply input_convol[] to have a 0.0...1.0 range */
		if ((status = psyfile_write_float(self->fp, 1.f))) {
			return status;
		}
		if ((status = psyfile_write_uint8(self->fp,
				(uint8_t)outcon))) {
			return status;
		}
		if ((status = psyfile_write_uint8(self->fp,
				(uint8_t)incon))) {
			return status;
		}
	}	
	return status;
}

void psy_audio_psy3saver_savedllnameandindex(psy_audio_PSY3Saver* self, const char* filename,
	uintptr_t shellindex)
{
	char str[256];	
	char idxtext[8];
	int32_t index;

	assert(self);

	str[0] = '\0';
	if (filename) {
		psy_Path path;
		idxtext[0] = '\0';

		psy_path_init(&path, filename);		
		if (strcmp(psy_path_ext(&path), "so") == 0) {
			psy_path_setext(&path, "dll");			
		}
		index = (int32_t)shellindex;
		if (index != 0) {
			int32_t divisor = 16777216;
			int32_t i;
			idxtext[4] = 0;
			for (i = 0; i < 4; ++i) {
				int32_t residue = index % divisor;
				idxtext[3 - i] = index / divisor;
				index = residue;
				divisor = divisor / 256;
			}
		}
		psy_snprintf(str, 256, "%s.%s%s", psy_path_name(&path),
			psy_path_ext(&path), idxtext);
		psy_path_dispose(&path);
	}
	psyfile_write(self->fp, str, ((uint32_t)(psy_strlen(str) + 1)));
}

int psy_audio_psy3saver_saveparammapping(psy_audio_PSY3Saver* self,
	psy_audio_Machine* machine)
{
	uint8_t numMaps = 0;
	int i;
	int status;
	psy_audio_ParamTranslator* translator;

	assert(self);

	translator = psy_audio_machine_instparamtranslator(machine);
	for (i = 0; i < 256; ++i) {
		uint32_t param;
		
		if (translator) {
			param = (uint32_t)psy_audio_paramtranslator_translate(translator, i);
		} else {
			param = i;
		}
		if (param < psy_audio_machine_numparameters(machine)) {
			++numMaps;
		}
	}
	if ((status = psyfile_write(self->fp, "PMAP", 4))) {
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, numMaps))) {
		return status;
	}	
	for (i = 0; i < 256; ++i) {
		uint32_t param;

		if (translator) {
			param = (uint32_t)psy_audio_paramtranslator_translate(translator, i);
		} else {
			param = (uint32_t)i;
		}		
		if (param < psy_audio_machine_numparameters(machine)) {
			uint8_t idx;
			uint16_t value;			

			idx = i;
			value = param;
			if ((status = psyfile_write_uint8(self->fp, idx))) {
				return status;
			}
			if ((status = psyfile_write_uint16(self->fp, value))) {
				return status;
			}
		}
	}
	return PSY_OK;
}

int psy_audio_psy3saver_saveismachinebus(psy_audio_PSY3Saver* self,
	psy_audio_Machine* machine)
{
	int status;
	uint8_t isbus;

	assert(self);
	assert(machine);

	if ((status = psyfile_write(self->fp, "PBUS", 4))) {
		return status;
	}
	isbus = psy_audio_machine_isbus(machine) != FALSE;
	if ((status = psyfile_write_uint8(self->fp, isbus))) {
		return status;
	}
	return PSY_OK;
}

/*
**	================== =
**	instrument data
**	===================
*/
/*	id = "INSD"; */
int psy_audio_psy3saver_write_insd(psy_audio_PSY3Saver* self)
{
	psy_TableIterator it;
	uint32_t sizepos;
	int status = PSY_OK;	

	assert(self);

	for (it = psy_audio_instruments_groupbegin(&self->song->instruments, 0);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
				
		if ((status = psyfile_writeheader(self->fp, "INSD",
				CURRENT_FILE_VERSION_INSD, 0, &sizepos))) {
			return status;
		}
		if ((status = psyfile_write_int32(self->fp, (int32_t)
				psy_tableiterator_key(&it)))) {
			return status;
		}
		instrument = (psy_audio_Instrument*) psy_tableiterator_value(&it);
		psy_audio_psy3saver_save_instrument(self, instrument);
		if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
			return status;
		}
	}
	return status;
}

/* insd */
int psy_audio_psy3saver_save_instrument(psy_audio_PSY3Saver* self,
	psy_audio_Instrument* instrument)
{	
	int status;
	psy_audio_LegacyInstrument legacy_instr;

	assert(self);
	assert(instrument);

	legacy_instr = psy_audio_legacyinstrument(instrument);	
	if ((status = psyfile_write_uint8(self->fp, legacy_instr._loop != FALSE))) {
		return status;
	}	
	if ((status = psyfile_write_int32(self->fp,
			legacy_instr._lines))) {
		return status;
	}	
	if ((status = psyfile_write_uint8(self->fp, legacy_instr._NNA))) {
		return status;
	}
	/* Amp envelope */
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_AT))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_DT))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_SL))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_RT))) {
		return status;
	}
	/* Filter envelope */
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_AT))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_DT))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_SL))) {
		return status;
	}	
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_RT))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_CO))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_RQ))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp,  legacy_instr.ENV_F_EA))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.ENV_F_TP))) {
		return status;
	}
	/*
	** No longer saving pan in version 2
	** legacypan	
	*/
	if ((status = psyfile_write_int32(self->fp, 128))) {
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, legacy_instr._RPAN != FALSE))) {
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, legacy_instr._RCUT != FALSE))) {
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, legacy_instr._RRES != FALSE))) {
		return status;
	}
	/* No longer saving name in version 2 */
	if ((status = psyfile_write_uint8(self->fp, 0))) {
		return status;
	}
	/* No longer saving wave subchunk in version 2 */
	/* legacynumwaves; */
	if ((status = psyfile_write_int32(self->fp, 0))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr.sampler_to_use))) {
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, legacy_instr._LOCKINST))) {
		return status;
	}
	return PSY_OK;
}

/*
** ========================
** Sampulse Instrument data
** ========================
*/
/* id = "SMID"; */
int psy_audio_psy3saver_write_smid(psy_audio_PSY3Saver* self)
{
	psy_TableIterator it;		
	int status = PSY_OK;
	psy_Table instsaved;

	assert(self);

	psy_table_init(&instsaved);
	for (it = psy_audio_instruments_begin(&self->song->instruments);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_InstrumentsGroup* group;
		psy_TableIterator it_group;
		uintptr_t groupindex;

		groupindex = (uint32_t)psy_tableiterator_key(&it);
		if (groupindex == 0) {
			/* saved as insd(reserved sampler ps1 group) */
			continue;
		}
		group = (psy_audio_InstrumentsGroup*)psy_tableiterator_value(&it);
		for (it_group = psy_table_begin(&group->container);
				!psy_tableiterator_equal(&it_group, psy_table_end());
				psy_tableiterator_inc(&it_group)) {
			psy_audio_Instrument* instrument;
			uintptr_t instindex;
			uint32_t sizepos;
			uint8_t revertindex;

			instrument = (psy_audio_Instrument*)psy_tableiterator_value(&it_group);
			instindex = psy_tableiterator_key(&it_group);
			if ((status = psyfile_writeheader(self->fp, "SMID",
					CURRENT_FILE_VERSION_SMID, 0, &sizepos))) {
				psy_table_dispose(&instsaved);
				return status;
			}
			if (psy_table_exists(&instsaved, instindex)) {
				/*
				** if already a sample at this index exists
				** save backward from max range that mfc-psycle skips it
				*/
				instindex = UINT32_MAX - instindex;
				revertindex = TRUE;
			} else {
				revertindex = FALSE;
				psy_table_insert(&instsaved, instindex, NULL);
			}
			/*
			** write index
			** is instrument already used index is stored reversed from UINT32_MAX
			*/
			if ((status = psyfile_write_uint32(self->fp, (uint32_t)instindex))) {
				psy_table_dispose(&instsaved);
				return status;
			}			
			if ((status = psy_audio_psy3saver_xminstrument_save(self, instrument,
					CURRENT_FILE_VERSION_SMID))) {
				psy_table_dispose(&instsaved);
				return status;
			}
			/* version extends mfc-psycle format with instrument groupindex */
			if ((status = psyfile_write_uint8(self->fp, revertindex))) {
				psy_table_dispose(&instsaved);
				return status;
			}
			if ((status = psyfile_write_uint32(self->fp, (uint32_t)groupindex))) {
				psy_table_dispose(&instsaved);
				return status;
			}
			if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
				psy_table_dispose(&instsaved);
				return status;
			}
		}
	}
	psy_table_dispose(&instsaved);
	return PSY_OK;
}

int psy_audio_psy3saver_xminstrument_save(psy_audio_PSY3Saver* self,
	psy_audio_Instrument* instrument, int32_t version)
{
	int status;

	assert(self);
	assert(instrument);

	status = PSY_OK;
	if (!instrument->enabled) {
		return PSY_OK;
	}
	/* instrument name */
	if ((status = psyfile_writestring(self->fp, instrument->name))) {
		return status;
	}
	/* lines */
	if ((status = psyfile_write_uint16(self->fp,
			(uint16_t)instrument->lines))) {
		return status;
	}
	/* global volume[0..1.0f] */
	if ((status = psyfile_write_float(self->fp,
			(float)instrument->globalvolume))) {
		return status;
	}
	/* volumefade speed [0..1.0f] */
	if ((status = psyfile_write_float(self->fp,
			(float)instrument->volumefadespeed))) {
		return status;
	}		
	/* initial panning speed [-1..1] */
	if ((status = psyfile_write_float(self->fp,
			(float)instrument->volumefadespeed))) {
		return status;
	}
	/* panning enabled */
	if ((status = psyfile_write_uint8(self->fp,
			(uint8_t)instrument->panenabled))) {
		return status;
	}
	/* surround enabled */
	if ((status = psyfile_write_uint8(self->fp,
			instrument->surround))) {
		return status;
	}
	/* Note number for center pan position */
	if ((status = psyfile_write_uint8(self->fp,
			instrument->notemodpancenter))) {
		return status;
	}
	/* -32..32. 1/256th of panFactor change per seminote */
	if ((status = psyfile_write_int8(self->fp,
			instrument->notemodpansep))) {
		return status;
	}
	/* filter cutoff */
	if ((status = psyfile_write_uint8(self->fp,
			(uint8_t)(instrument->filtercutoff * 127.f)))) {
		return status;
	}
	/* filter resonance */
	if ((status = psyfile_write_uint8(self->fp,
			(uint8_t)(instrument->filterres * 127.f)))) {
		return status;
	}
	/* unused */
	if ((status = psyfile_write_uint16(self->fp, 0))) {
		return status;
	}
	/* filtertype */
	if ((status = psyfile_write_uint32(self->fp,
		(uint32_t)instrument->filtertype))) {
		return status;
	}
	/* random volume */
	if ((status = psyfile_write_float(self->fp,
		(float)instrument->randomvolume))) {
		return status;
	}
	/* random panning */
	if ((status = psyfile_write_float(self->fp,
		(float)instrument->randompanning))) {
		return status;
	}
	/* random cutoff */
	if ((status = psyfile_write_float(self->fp,
			(float)instrument->randomcutoff))) {
		return status;
	}
	/* random resonance */
	if ((status = psyfile_write_float(self->fp,
			(float)instrument->randomresonance))) {
		return status;
	}
	/* nna */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)instrument->nna))) {
		return status;
	}
	/* dct */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)instrument->dct))) {
		return status;
	}
	/* dca */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)instrument->dca))) {
		return status;
	}		
	{
		LegacyNoteMap notemap;		
		uint8_t i;
				
		notemap = psy_audio_legacynotemap(instrument->entries);		
		for (i = 0; i < LEGACY_NOTE_MAP_SIZE; i++) {
			if ((status = psyfile_write_uint8(self->fp, notemap.map[i].first))) {
					return status;
			}
			if ((status = psyfile_write_uint8(self->fp,
					notemap.map[i].second))) {
				return status;
			}							
		}		
	}	
	psy_audio_psy3saver_write_smie(self, &instrument->volumeenvelope, version);	
	psy_audio_psy3saver_write_smie(self, &instrument->panenvelope, version);
	psy_audio_psy3saver_write_smie(self, &instrument->filterenvelope, version);
	psy_audio_psy3saver_write_smie(self, &instrument->pitchenvelope, version);
	
	return status;
}

/*
**	==================================== =
**	XMInstrument (sampulse) envelope data
**	=====================================
*/
/*	id = "SMIE"; */
int psy_audio_psy3saver_write_smie(psy_audio_PSY3Saver* self,
	psy_dsp_Envelope* envelope, uint32_t version)
{
	int status;
	uint32_t sizepos;
	uint32_t i;

	assert(self);

	status = PSY_OK;
	if ((status = psyfile_writeheader(self->fp, "SMIE",
			version, 0, &sizepos))) {
		return status;
	}	
	/* envelope enabled */
	if ((status = psyfile_write_uint8(self->fp,
			(uint32_t)envelope->enabled))) {
		return status;
	}
	/* envelope carry */
	if ((status = psyfile_write_uint8(self->fp,
			(uint32_t)envelope->carry))) {
		return status;
	}
	/* envelope loop start */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)envelope->loopstart))) {
		return status;
	}
	/* envelope loop end */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)envelope->loopend))) {
		return status;
	}
	/* envelope sustain begin */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)envelope->sustainbegin))) {
		return status;
	}
	/* envelope sustain end */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)envelope->sustainend))) {
		return status;
	}
	/* envelope num points */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)psy_dsp_envelope_numofpoints(envelope)))) {
		return status;
	}
	for (i = 0; i < psy_dsp_envelope_numofpoints(envelope); i++)
	{
		/* time in ms (int) */
		if ((status = psyfile_write_int32(self->fp,
				(int32_t)psy_dsp_envelope_time(envelope, i) * 1000))) {
			return status;
		}
		/* value */
		if ((status = psyfile_write_float(self->fp,
				(float)psy_dsp_envelope_value(envelope, i)))) {
			return status;
		}		
	}
	/* envelope num points */
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)psy_dsp_envelope_mode(envelope)))) {
		return status;
	}
	/* adsr */
	if ((status = psyfile_write_uint8(self->fp, FALSE))) {
		return status;
	}
	if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
		return status;
	}
	return status;
}

/*
**	================== =
**	Sampulse sample data
**	===================
*/
/*	id = "SMSB"; */

int psy_audio_psy3saver_write_smsb(psy_audio_PSY3Saver* self)
{
	psy_TableIterator it;	
	uint32_t sizepos;	
	int status = PSY_OK;
	psy_Table groupsaved;

	assert(self);

	psy_table_init(&groupsaved);
	for (it = psy_audio_samples_begin(&self->song->samples);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_SamplesGroup* group;
		psy_TableIterator it_group;
		uintptr_t index;

		index = (int32_t)psy_tableiterator_key(&it);
		group = (psy_audio_SamplesGroup*)psy_tableiterator_value(&it);
		for (it_group = psy_audio_samplesgroup_begin(group);
				!psy_tableiterator_equal(&it_group, psy_table_end());
				psy_tableiterator_inc(&it_group)) {
			psy_audio_Sample* sample;		
			uintptr_t subindex;			
			uint8_t revertindex;

			sample = (psy_audio_Sample*)psy_tableiterator_value(&it_group);
			subindex = psy_tableiterator_key(&it_group);			
			assert(sample);		
			if ((status = psyfile_writeheader(self->fp, "SMSB",
					CURRENT_FILE_VERSION_SMSB, 0, &sizepos))) {
				psy_table_dispose(&groupsaved);
				return status;
			}
			if (psy_table_exists(&groupsaved, index)) {
				/*
				** if already a sample at this index exists
				** save backward from max range that mfc-psycle skips it
				*/
				index = UINT32_MAX - index;
				revertindex = TRUE;
			} else {
				revertindex = FALSE;
				psy_table_insert(&groupsaved, index, NULL);
			}
			if ((status = psyfile_write_uint32(self->fp, (uint32_t)index))) {
				psy_table_dispose(&groupsaved);
				return status;
			}						
			if ((status = psy_audio_psy3saver_save_sample(self, sample))) {
				psy_table_dispose(&groupsaved);
				return status;				
			}			
			/* 
			** version extends mfc-psycle format with sample subslot
			** is sample already used index is stored reversed from UINT32_MAX
			*/
			if ((status = psyfile_write_uint8(self->fp, revertindex))) {
				psy_table_dispose(&groupsaved);
				return status;
			}			
			if ((status = psyfile_write_uint32(self->fp, (uint32_t)subindex))) {
				psy_table_dispose(&groupsaved);
				return status;
			}
			if ((status = psyfile_updatesize(self->fp, sizepos, NULL))) {
				psy_table_dispose(&groupsaved);
				return status;
			}
		}
	}
	psy_table_dispose(&groupsaved);
	return PSY_OK;
}

int psy_audio_psy3saver_save_sample(psy_audio_PSY3Saver* self, psy_audio_Sample* sample)
{			
	unsigned char* data1;
	unsigned char* data2;
	uint32_t size1;
	uint32_t size2;
	short* wavedata_left = 0;
	short* wavedata_right = 0;
	int status = PSY_OK;

	assert(self);
	assert(sample);
	
	if (psy_audio_buffer_numchannels(&sample->channels) > 0) {
		wavedata_left = psy_audio_psy3saver_clone_array_float_to_int16(
			psy_audio_buffer_at(&sample->channels, 0), sample->numframes);
		if (!wavedata_left) {
			return PSY_ERRFILE;
		}
	}
	if (psy_audio_buffer_numchannels(&sample->channels) > 1) {
		wavedata_right = psy_audio_psy3saver_clone_array_float_to_int16(
			psy_audio_buffer_at(&sample->channels, 1), sample->numframes);
		if (!wavedata_right) {
			free(wavedata_left);
			return PSY_ERRFILE;
		}
	}		
	size1 = (uint32_t)soundsquash(wavedata_left, &data1, (uint32_t)sample->numframes);
	if (sample->stereo) {		
		size2 = (uint32_t)soundsquash(wavedata_right, &data2,
			(uint32_t)sample->numframes);
	}
	if ((status = psyfile_writestring(self->fp, sample->name))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp, (uint32_t)
			(uint32_t)sample->numframes))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_float(self->fp, (float)
			sample->globalvolume))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint16(self->fp, (uint16_t)
			(sample->defaultvolume)))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp, (uint32_t)
			sample->loop.start))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp, (uint32_t)
			sample->loop.end))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_int32(self->fp,
			sample->loop.type))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp, (uint32_t)
			sample->sustainloop.start))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp, (uint32_t)
			sample->sustainloop.end))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_int32(self->fp, sample->sustainloop.type))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp,
			(uint32_t)sample->samplerate))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_int16(self->fp, sample->zone.tune))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_int16(self->fp, sample->zone.finetune))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, sample->stereo))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, (uint8_t)
			sample->panenabled))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_float(self->fp, sample->panfactor))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, sample->surround))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, sample->vibrato.attack))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, sample->vibrato.speed))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, sample->vibrato.depth))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint8(self->fp, (uint8_t)
			sample->vibrato.type))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write_uint32(self->fp, size1))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if ((status = psyfile_write(self->fp, (void*)data1, size1))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	free(data1);
	free(wavedata_left);	
	if (sample->stereo) {
		if ((status = psyfile_write_uint32(self->fp, size2))) {
			free(data2);
			free(wavedata_right);
			return status;
		}
		if ((status = psyfile_write(self->fp, (void*)data2, size2))) {
			free(data2);
			free(wavedata_right);
			return status;
		}
		free(data2);
		free(wavedata_right);
	}
	return status;
}

int16_t* psy_audio_psy3saver_clone_array_float_to_int16(float* buffer,
	uintptr_t numframes)
{
	if (buffer && numframes > 0) {
		int16_t* rv;

		rv = (int16_t*) malloc(numframes * sizeof(int16_t));
		if (rv) {
			uintptr_t frame;

			for (frame = 0; frame < numframes; ++frame) {
				rv[frame] = (int16_t)buffer[frame];
			}
		}
		return rv;
	}
	return NULL;
}

/*
** ==============================
** Virtual Instrument (Generator)
** ==============================
*/
/* id = "VIRG"; */

int psy_audio_psy3saver_write_virg(psy_audio_PSY3Saver* self)
{
	int status;
	int32_t i;
	uint32_t sizepos;

	assert(self);

	status = PSY_OK;	
	for (i = MAX_MACHINES; i < MAX_VIRTUALINSTS; i++) {
		psy_audio_Machine* machine;
		int32_t inst_idx;
		int32_t mac_idx;
		inst_idx = -1;
		mac_idx = -1;

		if ((machine = psy_audio_machines_at(&self->song->machines, i))) {
			psy_audio_MachineParam* param;			
			
			param = psy_audio_machine_parameter(machine, 0);
			if (param) {				
				inst_idx = (int32_t)psy_audio_machine_parameter_scaledvalue(machine, param);
			}
			param = psy_audio_machine_parameter(machine, 1);
			if (param) {
				mac_idx = (int32_t)psy_audio_machine_parameter_scaledvalue(machine, param);
			}						
			if (inst_idx != -1 && mac_idx != -1) {
				if ((status = psyfile_writeheader(self->fp, "VIRG",
					CURRENT_FILE_VERSION_VIRG, 0, &sizepos))) {
					return status;
				}
				if ((status = psyfile_write_int32(self->fp, (int32_t)i))) {
					return status;
				}
				if ((status = psyfile_write_int32(self->fp, (int32_t)mac_idx))) {
					return status;
				}
				if ((status = psyfile_write_int32(self->fp, (int32_t)inst_idx))) {
					return status;
				}
			}
		}
	}
	return PSY_OK;
}

/* Instrument */
int psy_audio_psy3saver_saveinstrument(psy_audio_PSY3Saver* self,
	psy_audio_Instrument* instr)
{
	uint32_t riffpos;
	uint32_t pos;
	uint32_t size;
	uint32_t index;
	int status;
	psy_List* sampidxs;
	psy_List* ite;

	assert(self);
	assert(self->songfile);
	assert(self->fp);
	assert(self->song);

	if ((status = psyfile_write(self->fp, "RIFF", 4))) {
		return status;
	}	
	riffpos = psyfile_getpos(self->fp);
	size = 0;
	if ((status = psyfile_write(self->fp, &size, 4))) {
		return status;
	}
	if ((status = psyfile_write(self->fp, "PSYI", 4))) {
		return status;
	}
	if ((status = psyfile_writeheader(self->fp, "SMID",
			CURRENT_FILE_VERSION_SMID, 0, &pos))) {
		return status;
	}
	index = 0;
	if ((status = psyfile_write(self->fp, &index, sizeof(uint32_t)))) {
		return status;
	}
	psy_audio_psy3saver_xminstrument_save(self, instr,
		CURRENT_FILE_VERSION_SMID);
	if ((status = psyfile_updatesize(self->fp, pos, NULL))) {
		return status;
	}
	
	sampidxs = NULL;
	for (ite = instr->entries; ite != NULL; psy_list_next(&ite)) {
		psy_audio_InstrumentEntry* entry;		

		entry = (psy_audio_InstrumentEntry*)psy_list_entry(ite);
		if (!psy_list_findentry(sampidxs, &entry->sampleindex)) {
			psy_list_append(&sampidxs, &entry->sampleindex);
		}
	}
	/*
	** this writes just the first sample of a sample group because
	** todo extent file format to store subsamples aswell
	*/
	for (ite = sampidxs; ite != NULL; psy_list_next(&ite)) {
		uint32_t pos;
		uint32_t index;
		psy_audio_SampleIndex* sampidx;
		psy_audio_Sample* sample;
		
		sampidx = (psy_audio_SampleIndex*)psy_list_entry(ite);
		assert(sampidx);
		sample = psy_audio_samples_at(&self->song->samples, *sampidx);
		if (sample) {
			if ((status = psyfile_writeheader(self->fp, "SMSB",
				CURRENT_FILE_VERSION_SMSB, 0, &pos))) {
				return status;
			}
			index = (uint32_t)sampidx->slot;
			/* todo subsamples */
			if ((status = psyfile_write(self->fp, &index, sizeof(uint32_t)))) {
				return status;
			}

			if (sample) {
				if ((status = psy_audio_psy3saver_save_sample(self, sample))) {
					return status;
				}
			} else {
				return PSY_ERRFILE;
			}
			if ((status = psyfile_updatesize(self->fp, pos, NULL))) {
				return status;
			}
		}
	}
	if ((status = psyfile_updatesize(self->fp, riffpos, NULL))) {
		return status;
	}
	return PSY_OK;
}
