/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "psyconvert.h"
/* local */
#include "constants.h"
/* std */
#include <string.h>

/* Pattern */

psy_audio_LegacyPattern psy_audio_allocoldpattern(psy_audio_Pattern* pattern,
	uintptr_t lpb, int* rv_lines)
{
	unsigned char* rv;
	int32_t lines;
	size_t patsize;
	int32_t row;
	uint32_t track;
	psy_audio_PatternNode* node;
	
	/* alloc pattern data */
	lines = (int32_t)(pattern->length * lpb + 0.5);
	*rv_lines = lines;
	patsize = MAX_TRACKS * lines * EVENT_SIZE;
	/* clear source */
	rv = malloc(patsize);
	/* init pattern data */
	for (row = 0; row < lines; ++row) {
		for (track = 0; track < MAX_TRACKS; ++track) {
			psy_audio_LegacyPatternEntry* data;

			data = psy_audio_ptrackline(rv, track, row);			
			/* empty entry */
			data->_note = 255; /* 255 = empty note */
			data->_inst = 255; /* 255 = empty inst */
			data->_mach = 255; /* 255 = empry mach */
			data->_cmd = 0;
			data->_parameter = 0;
		}
	}
	/* write pattern data */
	for (node = pattern->events; node != 0; node = node->next) {
		psy_audio_LegacyPatternEntry* data;
		psy_audio_PatternEntry* entry;
		int32_t row;
		int32_t track;

		entry = (psy_audio_PatternEntry*)node->entry;
		row = (int32_t)(entry->offset * lpb);
		track = (int32_t)entry->track;
		if (track < MAX_TRACKS) {
			data = psy_audio_ptrackline(rv, track, row);
			data->_note = psy_audio_patternentry_front(entry)->note;
			data->_inst = (uint8_t)(psy_audio_patternentry_front(entry)->inst & 0xFF);
			data->_mach = psy_audio_patternentry_front(entry)->mach;
			data->_cmd = psy_audio_patternentry_front(entry)->cmd;
			data->_parameter = psy_audio_patternentry_front(entry)->parameter;
		}
	}
	return rv;
}

void psy_audio_convert_legacypattern(
	psy_audio_Pattern* dst, psy_audio_LegacyPattern src,
	uint32_t numtracks, uint32_t numrows,
	uint32_t lpb)
{
	psy_audio_PatternNode* node;
	uint32_t row;	
	unsigned char* ptrack;
	psy_dsp_big_beat_t bpl;

	assert(dst);
	assert(src);

	node = NULL;
	ptrack = src;
	bpl = 1.0 / lpb;
	for (row = 0; row < numrows; ++row) {
		uint32_t track;
		psy_dsp_big_beat_t offset;		
		
		offset = bpl * row;
		for (track = 0; track < numtracks; ++track) {
			psy_audio_LegacyPatternEntry* psy2ev;
			bool empty;

			psy2ev = (psy_audio_LegacyPatternEntry*)ptrack;
			empty = psy2ev->_note == PSY2_NOTECOMMANDS_EMPTY &&
				psy2ev->_inst == 255 &&
				psy2ev->_mach == 255 &&
				psy2ev->_cmd == 0 &&
				psy2ev->_parameter == 0;
			if (!empty) {
				psy_audio_PatternEvent e;

				psy_audio_patternevent_clear(&e);
				e.note = (psy2ev->_note == 255)
					? psy_audio_NOTECOMMANDS_EMPTY
					: psy2ev->_note;
				e.inst = (psy2ev->_inst == 255)
					? psy_audio_NOTECOMMANDS_INST_EMPTY
					: psy2ev->_inst;
				e.mach = (psy2ev->_mach == 255)
					? psy_audio_NOTECOMMANDS_psy_audio_EMPTY
					: psy2ev->_mach;
				e.cmd = psy2ev->_cmd;
				e.parameter = psy2ev->_parameter;				
				node = psy_audio_pattern_insert(dst, node, track, offset, &e);
			}
			ptrack += EVENT_SIZE;
		}		
	}
	dst->length = numrows * bpl;
}

const psy_audio_LegacyPatternEntry* psy_audio_ptrackline_const(const
	psy_audio_LegacyPattern pattern, int track, int line)
{
	return (const psy_audio_LegacyPatternEntry*)(pattern + (track * EVENT_SIZE) + (line * MULTIPLY));
}

psy_audio_LegacyPatternEntry* psy_audio_ptrackline(
	psy_audio_LegacyPattern pattern, int track, int line)
{
	return (psy_audio_LegacyPatternEntry*)(pattern + (track * EVENT_SIZE) + (line * MULTIPLY));
}

/* Instrument */
void psy_audio_legacyinstrument_init(psy_audio_LegacyInstrument* self)
{
	self->sampler_to_use = -1;
	self->_LOCKINST = FALSE;

	/* Reset envelope */
	self->ENV_AT = 1;
	self->ENV_DT = 1;
	self->ENV_SL = 100;
	self->ENV_RT = 220;

	self->ENV_F_AT = 1;
	self->ENV_F_DT = 16500;
	self->ENV_F_SL = 64;
	self->ENV_F_RT = 16500;

	self->ENV_F_CO = 91;
	self->ENV_F_RQ = 11;
	self->ENV_F_EA = 0;
	self->ENV_F_TP = F_NONE;

	self->_loop = FALSE;
	self->_lines = 16;

	self->_NNA = 0; /* NNA set to Note Cut [Fast Release] */

	self->_RPAN = FALSE;
	self->_RCUT = FALSE;
	self->_RRES = FALSE;	
}

psy_audio_LegacyInstrument psy_audio_legacyinstrument(const psy_audio_Instrument* instrument)
{
	psy_audio_LegacyInstrument rv;	

	assert(instrument);
	
	rv._loop = instrument->loop != FALSE;			
	rv._lines = (int32_t)instrument->lines;			
	rv._NNA = (uint8_t)instrument->nna;
	/* volume envelope */
	rv.ENV_AT = (int32_t)(psy_dsp_envelope_attacktime(
		&instrument->volumeenvelope) * 44100 + 0.5f);	
	rv.ENV_DT = (int32_t)((psy_dsp_envelope_decaytime(
		&instrument->volumeenvelope)) * 44100 + 0.5f);	
	rv.ENV_SL = (int32_t)(psy_dsp_envelope_sustainvalue(
		&instrument->volumeenvelope) * 100);
	rv.ENV_RT = (int32_t)((psy_dsp_envelope_releasetime(
		&instrument->volumeenvelope)) * 44100 + 0.5f);
	/* filter envelope */
	rv.ENV_F_AT = (int32_t)(psy_dsp_envelope_attacktime(
		&instrument->filterenvelope) * 44100 + 0.5f);
	rv.ENV_F_DT = (int32_t)(psy_dsp_envelope_decaytime(
		&instrument->filterenvelope) * 44100 + 0.5f);
	rv.ENV_F_SL = (int32_t)(psy_dsp_envelope_sustainvalue(
		&instrument->filterenvelope) * 128);
	rv.ENV_F_RT = (int32_t)(psy_dsp_envelope_releasetime(
		&instrument->filterenvelope) * 44100 + 0.5f);
	rv.ENV_F_CO = (int32_t)(instrument->filtercutoff * 127.f);
	rv.ENV_F_RQ = (int32_t)(instrument->filterres * 127);
	rv.ENV_F_EA = (int32_t)(instrument->filtermodamount * 128);
	rv.ENV_F_TP = (int32_t)instrument->filtertype;		
	rv._RPAN = instrument->randompanning != 0.f;
	rv._RCUT = instrument->randomcutoff != 0.f;	;
	rv._RRES = instrument->randomresonance != 0.f;
	return rv;
}

void psy_audio_convert_legacy_to_instrument(
	struct psy_audio_Instrument* instrument,
	psy_audio_LegacyInstrument src)
{		
	psy_audio_NewNoteAction nna;
	int32_t ENV_AT;
	int32_t ENV_DT;
	int32_t ENV_RT;
	int32_t ENV_SL;
	int32_t ENV_F_AT;
	int32_t ENV_F_DT;
	int32_t ENV_F_RT;
	int32_t ENV_F_SL;	
	
	instrument->loop = src._loop;		
	instrument->lines = src._lines;	
	/*
	** \verbatim
	** NNA values overview:
	**
	** 0 = Note Cut      [Fast Release 'Default']
	** 1 = Note Release  [Release Stage]
	** 2 = Note Continue [No NNA]
	** \endverbatim
	*/
	switch (src._NNA) {
	case 0:
		nna = psy_audio_NNA_STOP;
		break;
	case 1:
		nna = psy_audio_NNA_NOTEOFF;
		break;
	case 2:
		nna = psy_audio_NNA_CONTINUE;
		break;
	default:
		nna = psy_audio_NNA_STOP;
		break;
	}
	psy_audio_instrument_setnna(instrument, nna);
	/*	
	** ENV_VOL
	*/
	/* Truncate to 220 samples boundaries, and ensure it is not zero. */	
	ENV_AT = (src.ENV_AT / 220) * 220; if (ENV_AT <= 0) ENV_AT = 1;
	ENV_DT = (src.ENV_DT / 220) * 220; if (ENV_DT <= 0) ENV_DT = 1;
	ENV_RT = (src.ENV_RT / 220) * 220; if (ENV_RT <= 0) ENV_RT = 1;
	ENV_SL = src.ENV_SL;
	if (ENV_RT == 16) ENV_RT = 220;
	psy_dsp_envelope_setadsr(&instrument->volumeenvelope,				
		ENV_AT * 1.f / 44100,
		ENV_DT * 1.f / 44100,
		ENV_SL / 100.f,
		ENV_RT * 1.f / 44100);
	/*
	** ENV_FILTER
	*/	
	ENV_F_AT = (src.ENV_F_AT / 220) * 220; if (ENV_F_AT <= 0) ENV_F_AT = 1;
	ENV_F_DT = (src.ENV_F_DT / 220) * 220; if (ENV_F_DT <= 0) ENV_F_DT = 1;
	ENV_F_RT = (src.ENV_F_RT / 220) * 220; if (ENV_F_RT <= 0) ENV_F_RT = 1;
	ENV_F_SL = src.ENV_F_SL;
	psy_dsp_envelope_setadsr(&instrument->filterenvelope,
		ENV_F_AT * 1.f / 44100,
		ENV_F_DT * 1.f / 44100,
		/* note: SL map range(128) differs from volume envelope(100) */
		ENV_F_SL / 128.f,
		ENV_F_RT * 1.f / 44100);	
	instrument->filtercutoff = src.ENV_F_CO / 127.f;
	instrument->filterres = src.ENV_F_RQ / 127.f;
	instrument->filtermodamount = src.ENV_F_EA / 128.f; /* -128 .. 128 to [-1 .. 1] */
	instrument->filtertype = (psy_dsp_FilterType)src.ENV_F_TP;
	instrument->randompanning = (src._RPAN) ? 1.f : 0.f;
	instrument->randomcutoff = (src._RCUT) ? 1.f : 0.f;
	instrument->randomresonance = (src._RRES) ? 1.f : 0.f;
}

/* Instrument NoteMap */
LegacyNoteMap psy_audio_legacynotemap(psy_List* entries)
{
	LegacyNoteMap notemap;
	psy_List* p;		

	memset(&notemap, 0, sizeof(LegacyNoteMap));
	for (p = entries; p != NULL; psy_list_next(&p)) {
		psy_audio_InstrumentEntry* entry;
		uint8_t key;

		entry = (psy_audio_InstrumentEntry*)psy_list_entry(p);
		for (key = (uint8_t)entry->keyrange.low; key <= (uint8_t)entry->keyrange.high; ++key) {
			if (key < LEGACY_NOTE_MAP_SIZE) {
				notemap.map[key] = legacynotepair_make(
					(entry->fixedkey == psy_audio_NOTECOMMANDS_EMPTY)
					? key
					: entry->fixedkey,
					(uint8_t)entry->sampleindex.slot);
			}
		}
	}	
	return notemap;
}
