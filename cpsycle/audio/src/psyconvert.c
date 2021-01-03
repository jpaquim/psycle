// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psyconvert.h"
#include "constants.h"

psy_audio_LegacyPattern psy_audio_allocoldpattern(struct psy_audio_Pattern* pattern, uintptr_t lpb,
	uintptr_t songtracks, int* rv_patternlines)
{
	unsigned char* rv;
	int32_t patternLines;
	int32_t y;
	uint32_t t;
	psy_audio_PatternNode* node;
	size_t patsize;

	patternLines = (int32_t)(pattern->length * lpb + 0.5);
	*rv_patternlines = patternLines;
	patsize = MAX_TRACKS * patternLines * EVENT_SIZE;
	// clear source
	rv = malloc(patsize);
	for (y = 0; y < patternLines; ++y) {
		for (t = 0; t < songtracks; ++t) {
			unsigned char* data;

			data = psy_audio_ptrackline(rv, t, y);
			// Psy3 PatternEntry format
			// type				offset
			// uint8_t note;		0
			// uint8_t inst;		1
			// uint8_t mach;		2
			// uint8_t cmd;			3
			// uint8_t parameter;	4

			// empty entry					
			data[0] = 255;
			data[1] = 255;
			data[2] = 255;
			data[3] = 0;
			data[4] = 0;
		}
	}

	for (node = pattern->events; node != 0; node = node->next) {
		unsigned char* data;
		psy_audio_PatternEntry* entry;
		int32_t y;
		int32_t t;

		entry = (psy_audio_PatternEntry*)node->entry;
		y = (int32_t)(entry->offset * lpb);
		t = entry->track;
		data = psy_audio_ptrackline(rv, t, y);
		// Psy3 PatternEntry format
		// type				offset
		// uint8_t note;		0
		// uint8_t inst;		1
		// uint8_t mach;		2
		// uint8_t cmd;			3
		// uint8_t parameter;	4
		data[0] = psy_audio_patternentry_front(entry)->note;
		data[1] = (uint8_t)(psy_audio_patternentry_front(entry)->inst & 0xFF);
		data[2] = psy_audio_patternentry_front(entry)->mach;
		data[3] = psy_audio_patternentry_front(entry)->cmd;
		data[4] = psy_audio_patternentry_front(entry)->parameter;
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