/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PATTERNCURSOR_H
#define psy_audio_PATTERNCURSOR_H

/* dsp */
#include <dsptypes.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_PatternCursor
**
** edit position in the pattern
*/

typedef struct psy_audio_PatternCursor {
	uintptr_t track;
	psy_dsp_big_beat_t offset;
	bool absolute;	
	uintptr_t lpb;
	uintptr_t column;
	uintptr_t digit;
	uintptr_t patternid;
	uint8_t key;
} psy_audio_PatternCursor;

void psy_audio_patterncursor_init(psy_audio_PatternCursor*);

psy_audio_PatternCursor psy_audio_patterncursor_make(
	uintptr_t track, psy_dsp_big_beat_t offset);
psy_audio_PatternCursor psy_audio_patterncursor_make_all(
	uintptr_t track, psy_dsp_big_beat_t offset, uint8_t key);

INLINE psy_dsp_big_beat_t psy_audio_patterncursor_offset(
	const psy_audio_PatternCursor* self)
{
	return self->offset;
}

/* compares two pattern edit positions, if they are equal */
int psy_audio_patterncursor_equal(psy_audio_PatternCursor* lhs,
	psy_audio_PatternCursor* rhs);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERNCURSOR_H */
