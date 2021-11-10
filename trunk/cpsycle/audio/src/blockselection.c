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

psy_audio_BlockSelection psy_audio_blockselection_make(
	psy_audio_SequenceCursor topleft, psy_audio_SequenceCursor bottomright)
{
	psy_audio_BlockSelection rv;

	psy_audio_blockselection_init_all(&rv, topleft, bottomright);
	rv.topleft = topleft;
	rv.bottomright = bottomright;
	return rv;
}

void psy_audio_blockselection_startdrag(psy_audio_BlockSelection* self,
	psy_audio_SequenceCursor dragselectionbase,
	psy_audio_SequenceCursor cursor, double bpl)
{
	assert(self);

	psy_audio_blockselection_enable(self);
	self->topleft = cursor;
	self->bottomright = cursor;
	if (cursor.track >= dragselectionbase.track) {
		self->topleft.track = dragselectionbase.track;
		self->bottomright.track = cursor.track;
	} else {
		self->topleft.track = cursor.track;
		self->bottomright.track = dragselectionbase.track;
	}
	if (cursor.offset >= dragselectionbase.offset) {
		self->topleft.offset = dragselectionbase.offset;
		self->bottomright.offset = cursor.offset + bpl;
	} else {
		self->topleft.offset = cursor.offset;
		self->bottomright.offset = dragselectionbase.offset +
			bpl;
	}
	self->bottomright.track += 1;
}

void psy_audio_blockselection_drag(psy_audio_BlockSelection* self,
	psy_audio_SequenceCursor dragselectionbase,
	psy_audio_SequenceCursor cursor, double bpl)
{
	assert(self);

	if (cursor.track >= dragselectionbase.track) {
		self->topleft.track = dragselectionbase.track;
		self->bottomright.track = cursor.track + 1;
	} else {
		self->topleft.track = cursor.track;
		self->bottomright.track = dragselectionbase.track + 1;
	}
	if (cursor.offset >= dragselectionbase.offset) {
		self->topleft.offset = dragselectionbase.offset;
		self->bottomright.offset = cursor.offset + bpl;
	} else {
		self->topleft.offset = cursor.offset;
		self->bottomright.offset = dragselectionbase.offset + bpl;
	}
}

void psy_audio_blockselection_select(psy_audio_BlockSelection* self,
	uintptr_t track, uintptr_t numtracks,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length)
{	
	self->topleft.offset = offset;
	self->topleft.track = track;
	self->bottomright.offset = offset + length;
	self->bottomright.track = track + numtracks;
	psy_audio_blockselection_enable(self);	
}