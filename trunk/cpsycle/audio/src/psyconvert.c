// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psyconvert.h"
#include "constants.h"

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
		track = entry->track;
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

const psy_audio_LegacyPatternEntry* psy_audio_ptrackline_const(const
	psy_audio_LegacyPattern pattern, int track, int line)
{
	return (const psy_audio_LegacyPatternEntry*)(pattern + (track * EVENT_SIZE) + (line * MULTIPLY));
}

psy_audio_LegacyPatternEntry* psy_audio_ptrackline(
	psy_audio_LegacyPattern pattern, int track, int line)
{
	return (const psy_audio_LegacyPatternEntry*)(pattern + (track * EVENT_SIZE) + (line * MULTIPLY));
}

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
	rv.ENV_F_EA = 0;	
	rv.ENV_F_TP = (int32_t)instrument->filtertype;		
	rv._RPAN = instrument->randompanning != 0.f;
	rv._RCUT = instrument->randomcutoff != 0.f;
	rv._RRES = instrument->_RRES;
	return rv;
}