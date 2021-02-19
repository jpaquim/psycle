// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psyconvert.h"
#include "constants.h"
// std
#include <string.h>

// Pattern
psy_audio_LegacyPattern psy_audio_allocoldpattern(struct psy_audio_Pattern* pattern, uintptr_t lpb,
	int* rv_lines)
{
	unsigned char* rv;
	int32_t lines;
	size_t patsize;
	int32_t row;
	uint32_t track;
	psy_audio_PatternNode* node;
	
	// alloc pattern data
	lines = (int32_t)(pattern->length * lpb + 0.5);
	*rv_lines = lines;
	patsize = MAX_TRACKS * lines * EVENT_SIZE;
	// clear source
	rv = malloc(patsize);
	// init pattern data
	for (row = 0; row < lines; ++row) {
		for (track = 0; track < MAX_TRACKS; ++track) {
			psy_audio_LegacyPatternEntry* data;

			data = psy_audio_ptrackline(rv, track, row);			
			// empty entry					
			data->_note = 255; // 255 = empty note
			data->_inst = 255; // 255 = empty inst
			data->_mach = 255; // 255 = empry mach
			data->_cmd = 0;
			data->_parameter = 0;
		}
	}
	// write pattern data
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

	node = NULL;
	for (row = 0; row < numrows; ++row) {
		uint32_t track;
		psy_dsp_big_beat_t offset;
		psy_dsp_big_beat_t bpl;

		bpl = 1.0 / lpb;
		offset = bpl * row;
		for (track = 0; track < MAX_TRACKS; ++track) {
			psy_audio_LegacyPatternEntry* psy2ev;
			bool empty;

			psy2ev = psy_audio_ptrackline(src, track, row);
			empty = psy2ev->_note == PSY2_NOTECOMMANDS_EMPTY &&
				psy2ev->_inst == 255 &&
				psy2ev->_mach == 255 &&
				psy2ev->_cmd == 0 &&
				psy2ev->_parameter == 0;
			if (!empty) {
				psy_audio_PatternEvent event;

				psy_audio_patternevent_clear(&event);
				event.note = (psy2ev->_note == 255) ? psy_audio_NOTECOMMANDS_EMPTY : psy2ev->_note;
				event.inst = (psy2ev->_inst == 255) ? psy_audio_NOTECOMMANDS_INST_EMPTY : psy2ev->_inst;
				event.mach = (psy2ev->_mach == 255) ? psy_audio_NOTECOMMANDS_psy_audio_EMPTY : psy2ev->_mach;
				event.cmd = psy2ev->_cmd;
				event.parameter = psy2ev->_parameter;
				node = psy_audio_pattern_insert(dst, node, track, offset,
					&event);
			}
		}
	}
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

// Instrument
psy_audio_LegacyInstrument psy_audio_legacyinstrument(const psy_audio_Instrument* instrument)
{
	psy_audio_LegacyInstrument rv;	

	assert(instrument);
	
	rv._loop = instrument->loop != FALSE;			
	rv._lines = (int32_t)instrument->lines;			
	rv._NNA = (uint8_t)instrument->nna;
	// volume envelope
	rv.ENV_AT = (int32_t)(psy_dsp_envelope_attacktime(
		&instrument->volumeenvelope) * 44100 + 0.5f);	
	rv.ENV_DT = (int32_t)((psy_dsp_envelope_decaytime(
		&instrument->volumeenvelope)) * 44100 + 0.5f);	
	rv.ENV_SL = (int32_t)(psy_dsp_envelope_sustainvalue(
		&instrument->volumeenvelope) * 100);
	rv.ENV_RT = (int32_t)((psy_dsp_envelope_releasetime(
		&instrument->volumeenvelope)) * 44100 + 0.5f);
	// filter envelope
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

// Instrument NoteMap
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