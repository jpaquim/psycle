/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "blockselection.h"
/* local */
#include "sequence.h"
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
	psy_audio_sequencecursor_init(&self->drag_base);
	self->valid = FALSE;
}

void psy_audio_blockselection_init_all(psy_audio_BlockSelection* self,
	psy_audio_SequenceCursor topleft, psy_audio_SequenceCursor bottomright)
{
	assert(self);

	self->topleft = self->drag_base = topleft;
	self->bottomright = bottomright;
	self->valid = TRUE;
}

psy_audio_BlockSelection psy_audio_blockselection_make(
	psy_audio_SequenceCursor topleft,
	psy_audio_SequenceCursor bottomright)
{
	psy_audio_BlockSelection rv;

	psy_audio_blockselection_init_all(&rv, topleft, bottomright);
	rv.topleft = rv.drag_base = topleft;
	rv.bottomright = bottomright;
	return rv;
}

void psy_audio_blockselection_startdrag(psy_audio_BlockSelection* self,	
	psy_audio_SequenceCursor cursor)
{
	double bpl;	

	assert(self);
	
	psy_audio_blockselection_enable(self);
	bpl = psy_audio_sequencecursor_bpl(&cursor);
	self->drag_base = self->topleft;
	self->topleft = cursor;
	self->bottomright = cursor;	
	if (cursor.track >= self->drag_base.track) {
		self->topleft.track = self->drag_base.track;
		self->bottomright.track = cursor.track;
	} else {
		self->topleft.track = cursor.track;
		self->bottomright.track = self->drag_base.track;
	}
	/* todo abs */
	if (cursor.offset >= self->drag_base.offset) {
		psy_audio_sequencecursor_set_offset(&self->topleft,
			self->drag_base.offset);
		psy_audio_sequencecursor_set_offset(&self->bottomright,
			cursor.offset + bpl);
	} else {
		psy_audio_sequencecursor_set_offset(&self->topleft, cursor.offset);
		psy_audio_sequencecursor_set_offset(&self->bottomright,
			self->drag_base.offset + bpl);
	}
	self->bottomright.track += 1;
	self->topleft.key = 100;
	self->bottomright.key = 0;	
}

void psy_audio_blockselection_drag(psy_audio_BlockSelection* self,
	psy_audio_SequenceCursor cursor, psy_audio_Sequence* sequence)
{
	double bpl;

	assert(self);

	bpl = psy_audio_sequencecursor_bpl(&cursor);
	if (cursor.track >= self->drag_base.track) {
		self->topleft.track = self->drag_base.track;
		self->bottomright.track = cursor.track + 1;
	} else {
		self->topleft.track = cursor.track;
		self->bottomright.track = self->drag_base.track + 1;
	}
	if (psy_audio_sequencecursor_offset_abs(&cursor, sequence) >=
			psy_audio_sequencecursor_offset_abs(&self->drag_base, sequence)) {
		psy_audio_sequencecursor_set_order_index(&self->topleft,
			self->drag_base.order_index);
		psy_audio_sequencecursor_set_offset(&self->topleft,
			self->drag_base.offset);
		psy_audio_sequencecursor_set_order_index(&self->bottomright,
			cursor.order_index);
		psy_audio_sequencecursor_set_offset(&self->bottomright,
			cursor.offset + bpl);		
	} else {
		psy_audio_sequencecursor_set_order_index(&self->topleft,
			cursor.order_index);
		psy_audio_sequencecursor_set_offset(&self->topleft,
			cursor.offset);			
		psy_audio_sequencecursor_set_order_index(&self->bottomright,
			self->drag_base.order_index);			
		psy_audio_sequencecursor_set_offset(&self->bottomright,
			self->drag_base.offset + bpl);		
	}	
}
