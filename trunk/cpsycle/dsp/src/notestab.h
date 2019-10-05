// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(NOTESTAB_H)
#define NOTESTAB_H

typedef unsigned char note_t;

typedef enum {
	NOTESTAB_A440,
	NOTESTAB_A220	
} NotesTabMode;

#define NOTESTAB_DEFAULT NOTESTAB_A220

const char* notetostr(note_t note, NotesTabMode mode);
const char* const * notetab(NotesTabMode mode);

#endif
