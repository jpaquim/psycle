// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_NOTESTAB_H
#define psy_dsp_NOTESTAB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char psy_dsp_note_t;

typedef enum {
	psy_dsp_NOTESTAB_A440,
	psy_dsp_NOTESTAB_A220	
} psy_dsp_NotesTabMode;

#define psy_dsp_NOTESTAB_DEFAULT psy_dsp_NOTESTAB_A220

const char* psy_dsp_notetostr(psy_dsp_note_t note, psy_dsp_NotesTabMode mode);
const char* const * psy_dsp_notetab(psy_dsp_NotesTabMode mode);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_NOTESTAB_H */
