// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_NOTESTAB_H
#define psy_dsp_NOTESTAB_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char psy_dsp_note_t;

static bool psy_dsp_isblack(uint8_t key)
{
	intptr_t offset = key % 12;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	return (offset == 1 || offset == 3 || offset == 6 || offset == 8
		|| offset == 10);
}

static bool psy_dsp_iskey_c(intptr_t key)
{
	return (key % 12) == 0;
}

static bool psy_dsp_iskey_e(intptr_t key)
{
	return (key % 12) == 5;
}

typedef enum {
	psy_dsp_NOTESTAB_A440,
	psy_dsp_NOTESTAB_A220,	
	psy_dsp_NOTESTAB_GMPERCUSSION, // General MIDI Level 1
} psy_dsp_NotesTabMode;

#define psy_dsp_NOTESTAB_DEFAULT psy_dsp_NOTESTAB_A220

extern char* hex_tab[16];

const char* psy_dsp_notetostr(psy_dsp_note_t note, psy_dsp_NotesTabMode mode);
const char* const * psy_dsp_notetab(psy_dsp_NotesTabMode mode);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_NOTESTAB_H */
