/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "blockselection.h"
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"

/* BlockSelection */
/* implementation */
void psy_audio_blockselection_init(psy_audio_BlockSelection* self)
{
	assert(self);

	psy_audio_sequencecursor_init(&self->topleft);
	psy_audio_sequencecursor_init(&self->bottomright);
	self->valid = FALSE;
}

void psy_audio_blockselection_init_all(psy_audio_BlockSelection* self,
	psy_audio_SequenceCursor topleft, psy_audio_SequenceCursor bottomright)
{
	assert(self);

	self->topleft = topleft;
	self->bottomright = bottomright;
	self->valid = TRUE;
}

void psy_audio_blockselection_startdrag(psy_audio_BlockSelection* self,
	psy_audio_SequenceCursor dragselectionbase,
	psy_audio_SequenceCursor cursor, double bpl)
{
	assert(self);

	psy_audio_blockselection_enable(self);
	self->topleft = cursor;
	self->bottomright = cursor;
	if (cursor.cursor.track >= dragselectionbase.cursor.track) {
		self->topleft.cursor.track = dragselectionbase.cursor.track;
		self->bottomright.cursor.track = cursor.cursor.track;
	} else {
		self->topleft.cursor.track = cursor.cursor.track;
		self->bottomright.cursor.track = dragselectionbase.cursor.track;
	}
	if (cursor.cursor.offset >= dragselectionbase.cursor.offset) {
		self->topleft.cursor.offset = dragselectionbase.cursor.offset;
		self->bottomright.cursor.offset = cursor.cursor.offset + bpl;
	} else {
		self->topleft.cursor.offset = cursor.cursor.offset;
		self->bottomright.cursor.offset = dragselectionbase.cursor.offset +
			bpl;
	}
	self->bottomright.cursor.track += 1;
}

void psy_audio_blockselection_drag(psy_audio_BlockSelection* self,
	psy_audio_PatternCursor dragselectionbase,
	psy_audio_PatternCursor cursor, double bpl)
{
	assert(self);

	if (cursor.track >= dragselectionbase.track) {
		self->topleft.cursor.track = dragselectionbase.track;
		self->bottomright.cursor.track = cursor.track + 1;
	} else {
		self->topleft.cursor.track = cursor.track;
		self->bottomright.cursor.track = dragselectionbase.track + 1;
	}
	if (cursor.offset >= dragselectionbase.offset) {
		self->topleft.cursor.offset = dragselectionbase.offset;
		self->bottomright.cursor.offset = cursor.offset + bpl;
	} else {
		self->topleft.cursor.offset = cursor.offset;
		self->bottomright.cursor.offset = dragselectionbase.offset + bpl;
	}
}

psy_audio_BlockSelection psy_audio_blockselection_make(
	psy_audio_SequenceCursor topleft, psy_audio_SequenceCursor bottomright)
{
	psy_audio_BlockSelection rv;

	psy_audio_blockselection_init_all(&rv, topleft, bottomright);
	rv.topleft = topleft;
	rv.bottomright = bottomright;
	return rv;
}
