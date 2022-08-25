/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqedittrack.h"
/* host */
#include "styles.h"
/* audio */
#include <exclusivelock.h>
/* platform */
#include "../../detail/portable.h"

/* SeqEditTrack */

/* prototypes */
static void seqedittrack_on_destroyed(SeqEditTrack*);
static void seqedittrack_on_preferred_size(SeqEditTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqedittrack_on_align(SeqEditTrack*);
static void seqedittrack_on_mouse_down(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_on_mouse_move(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_on_mouse_up(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_on_mouse_double_click(SeqEditTrack*,
	psy_ui_MouseEvent*);
static void seqedittrack_build(SeqEditTrack* self);
static void seqedittrack_on_sequence_insert(SeqEditTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex*);
static void seqedittrack_on_sequence_remove(SeqEditTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex*);
static void seqedittrack_on_sequence_track_reposition(SeqEditTrack*,
	psy_audio_Sequence* sender, uintptr_t trackidx);

/* vtable */
static psy_ui_ComponentVtable seqedittrack_vtable;
static bool seqedittrack_vtable_initialized = FALSE;

static void seqedittrack_vtable_init(SeqEditTrack* self)
{
	if (!seqedittrack_vtable_initialized) {
		seqedittrack_vtable = *(self->component.vtable);
		seqedittrack_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			seqedittrack_on_destroyed;
		seqedittrack_vtable.onalign =
			(psy_ui_fp_component_event)
			seqedittrack_on_align;
		seqedittrack_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			seqedittrack_on_preferred_size;
		seqedittrack_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			seqedittrack_on_mouse_down;
		seqedittrack_vtable.on_mouse_move =
			(psy_ui_fp_component_on_mouse_event)
			seqedittrack_on_mouse_move;
		seqedittrack_vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			seqedittrack_on_mouse_up;
		seqedittrack_vtable.on_mouse_double_click =
			(psy_ui_fp_component_on_mouse_event)
			seqedittrack_on_mouse_double_click;
		seqedittrack_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(seqedittrack_base(self), &seqedittrack_vtable);
}

/* implementation */
void seqedittrack_init(SeqEditTrack* self, psy_ui_Component* parent,
	SeqEditState* state, psy_audio_SequenceTrack* track, uintptr_t track_index)
{
	assert(self);
	assert(track);
	
	psy_ui_component_init(&self->component, parent, NULL);
	seqedittrack_vtable_init(self);	
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);	
	psy_ui_component_set_minimum_size(&self->component,
		psy_ui_size_make_em(0.0, 2.0));	
	self->state = state;	
	self->currtrack = track;
	self->trackindex = track_index;	
	self->entries = NULL;
	if (seqeditstate_sequence(self->state)) {
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_insert,
			self, seqedittrack_on_sequence_insert);
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_remove,
			self, seqedittrack_on_sequence_remove);
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_trackreposition,
			self, seqedittrack_on_sequence_track_reposition);		
	}	
	if (track->height != 0.0) {
		psy_ui_component_set_preferred_height(&self->component,
			psy_ui_value_make_eh(track->height));
	}
	seqedittrack_build(self);
}

void seqedittrack_on_destroyed(SeqEditTrack* self)
{
	if (seqeditstate_sequence(self->state)) {
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_insert,
			self, seqedittrack_on_sequence_insert);
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_remove,
			self, seqedittrack_on_sequence_remove);
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_trackreposition,
			self, seqedittrack_on_sequence_track_reposition);		
	}
	psy_list_free(self->entries);
	self->entries = NULL;
}

SeqEditTrack* seqedittrack_alloc(void)
{
	return (SeqEditTrack*)malloc(sizeof(SeqEditTrack));
}

SeqEditTrack* seqedittrack_allocinit(psy_ui_Component* parent,	
	SeqEditState* state, psy_audio_SequenceTrack* track, uintptr_t track_index)
{
	SeqEditTrack* rv;

	rv = seqedittrack_alloc();
	if (rv) {
		seqedittrack_init(rv, parent, state, track, track_index);
		psy_ui_component_deallocate_after_destroyed(seqedittrack_base(rv));
	}
	return rv;
}

void seqedittrack_build(SeqEditTrack* self)
{
	psy_List* p;
	uintptr_t c;	

	psy_ui_component_clear(&self->component);
	psy_list_free(self->entries);
	self->entries = NULL;
	if (!seqeditstate_sequence(self->state)) {
		return;
	}	
	if (!self->currtrack) {
		return;
	}
	for (p = self->currtrack->nodes, c = 0; p != NULL;
			psy_list_next(&p), ++c) {
		psy_audio_SequenceEntry* seqentry;
		psy_audio_Pattern* pattern;

		seqentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		if (seqentry) {
			switch (seqentry->type) {
			case psy_audio_SEQUENCEENTRY_PATTERN: {
				psy_audio_SequencePatternEntry* seqpatternentry;

				seqpatternentry = (psy_audio_SequencePatternEntry*)seqentry;
				pattern = psy_audio_patterns_at(
					seqeditstate_patterns(self->state),
					psy_audio_sequencepatternentry_patternslot(
						seqpatternentry));
				if (pattern) {
					SeqEditPatternEntry* seqeditpatternentry;

					seqeditpatternentry = seqeditpatternentry_allocinit(
						&self->component, seqpatternentry,
						psy_audio_orderindex_make(self->trackindex, c),
						self->state);
					if (seqeditpatternentry) {
						psy_ui_component_set_align(
							seqeditpatternentry_base(seqeditpatternentry),
								psy_ui_ALIGN_LEFT);
						psy_list_append(&self->entries, seqeditpatternentry);
					}
				}
				break; }
			case psy_audio_SEQUENCEENTRY_SAMPLE: {
				psy_audio_SequenceSampleEntry* seqsampleentry;
				SeqEditSampleEntry* seqeditsampleentry;

				seqsampleentry = (psy_audio_SequenceSampleEntry*)seqentry;
				seqeditsampleentry = seqeditsampleentry_allocinit(
					&self->component, seqsampleentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqsampleentry) {
					psy_ui_component_set_align(
						seqeditsampleentry_base(seqeditsampleentry),
						psy_ui_ALIGN_LEFT);
					psy_list_append(&self->entries, seqeditsampleentry);							
				}
				seqeditsampleentry->wavebox.preventselection = TRUE;
				break; }
			case psy_audio_SEQUENCEENTRY_MARKER: {
				psy_audio_SequenceMarkerEntry* seqmarkerentry;				
				SeqEditMarkerEntry* seqeditmarkerentry;

				seqmarkerentry = (psy_audio_SequenceMarkerEntry*)seqentry;
				seqeditmarkerentry = seqeditmarkerentry_allocinit(
					&self->component,seqmarkerentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqeditmarkerentry) {
					psy_ui_component_set_align(
						seqeditmarkerentry_base(seqeditmarkerentry),
						psy_ui_ALIGN_LEFT);
					psy_list_append(&self->entries, seqeditmarkerentry);					
				}
				break; }
			default:
				break;
			}
		}
	}
	psy_ui_component_align(&self->component);
}

void seqedittrack_on_align(SeqEditTrack* self)
{		
	psy_List* p;
	psy_List* q;
	psy_ui_Size size;

	if (!seqeditstate_sequence(self->state)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	size = psy_ui_component_scroll_size(&self->component);	
	p = self->entries;
	for (p = self->entries, q = self->currtrack->nodes;
			p != NULL && q != NULL; psy_list_next(&p), q = q->next) {
		psy_ui_Component* component;
		psy_audio_SequenceEntry* seqentry;		

		component = (psy_ui_Component*)psy_list_entry(p);
		seqentry = (psy_audio_SequenceEntry*)psy_list_entry(q);
		if (seqentry) {			
			psy_ui_component_setposition(component,
				psy_ui_rectangle_make(
					psy_ui_point_make_px(
						seqeditstate_beattopx(self->state,
							psy_audio_sequenceentry_offset(seqentry)),
						0.0),
					psy_ui_size_make(
						psy_ui_value_make_px(
							seqeditstate_beattopx(self->state,
								psy_audio_sequenceentry_length(seqentry))),
						size.height)));
		}
	}	
}

void seqedittrack_on_preferred_size(SeqEditTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			seqeditstate_patterns(self->state));
	}
	trackduration += 400.0;
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
		trackduration);
	rv->height = self->state->line_height;
}

void seqedittrack_on_mouse_down(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	if (psy_ui_mouseevent_button(ev) == 1) {
		if (self->state->dragstatus == SEQEDIT_DRAG_NONE) {
			psy_audio_OrderIndex seqpos;
			
			seqpos = seqeditstate_editposition(self->state);
			if (self->trackindex != seqpos.track) {
				psy_audio_SequenceCursor cursor;
				
				psy_audio_sequencecursor_init(&cursor);
				psy_audio_sequencecursor_set_order_index(&cursor,
					psy_audio_orderindex_make(self->trackindex,
					psy_INDEX_INVALID));
				psy_audio_sequence_set_cursor(seqeditstate_sequence(
					self->state), cursor);				
			}
		} else {
			psy_ui_component_capture(&self->component);
		}
	}
}

void seqedittrack_on_mouse_move(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{	
	if (self->state->dragstatus == SEQEDIT_DRAG_NONE ||
		self->state->dragstatus == SEQEDIT_DRAG_REMOVE ||
		self->state->seqentry == NULL) {
		return;
	}
	if (self->state->dragstatus == SEQEDIT_DRAG_START) {
		psy_dsp_big_beat_t dragposition;
		psy_dsp_big_beat_t bpl;

		dragposition = seqeditstate_pxtobeat(self->state,
			psy_ui_mouseevent_pt(ev).x) -
			psy_audio_sequenceentry_offset(self->state->seqentry);
		bpl = 1.0 / ((psy_dsp_big_beat_t)
			seqeditstate_sequence(self->state)->cursor.lpb);		
		/* start drag only if a difference of one line exists */
		if (fabs(dragposition - self->state->dragposition) >= bpl) {			
			self->state->dragstatus =
				(self->state->dragtype == SEQEDIT_DRAGTYPE_REORDER)
				? SEQEDIT_DRAG_REORDER
				: SEQEDIT_DRAG_MOVE;			
		}		
	} else if (self->state->dragstatus == SEQEDIT_DRAG_LENGTH) {
		psy_dsp_big_beat_t dragposition;		
					
		dragposition = seqeditstate_quantize(self->state,
			seqeditstate_pxtobeat(self->state, psy_ui_mouseevent_pt(ev).x));
		psy_audio_sequenceentry_setlength(self->state->seqentry,
			psy_max(1.0, dragposition - psy_audio_sequenceentry_offset(
				self->state->seqentry)));			
	} else if ((self->state->dragtype & SEQEDIT_DRAGTYPE_MOVE) ==
			SEQEDIT_DRAGTYPE_MOVE) {
		psy_dsp_big_beat_t dragposition;
		psy_dsp_big_beat_t repositionoffset;

		dragposition = seqeditstate_pxtobeat(self->state,
			psy_ui_mouseevent_pt(ev).x);
		if (dragposition - (self->state->seqentry->offset -
			self->state->seqentry->repositionoffset) >= 0) {
			dragposition = seqeditstate_quantize(self->state,
				dragposition);
		} else {
			dragposition = self->state->seqentry->offset -
				self->state->seqentry->repositionoffset;
		}
		repositionoffset = dragposition -
			(self->state->seqentry->offset -
				self->state->seqentry->repositionoffset);
		if (self->state->seqentry->repositionoffset != repositionoffset) {
			self->state->seqentry->repositionoffset = repositionoffset;
			self->state->seqentry->offset = dragposition;			
			psy_audio_sequence_reposition_track(seqeditstate_sequence(
				self->state), self->currtrack);
		}
	}	
}

void seqedittrack_on_mouse_up(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_release_capture(&self->component);	
	if (self->state->dragstatus == SEQEDIT_DRAG_REORDER) {
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_reorder(seqeditstate_sequence(self->state),
			self->state->dragseqpos, self->state->cursorposition);
		psy_audio_exclusivelock_leave();
		psy_ui_mouseevent_stop_propagation(ev);
	}
	self->state->seqentry = NULL;
	self->state->dragstatus = SEQEDIT_DRAG_NONE;
}

void seqedittrack_on_mouse_double_click(SeqEditTrack* self,
	psy_ui_MouseEvent* ev)
{
	if (psy_audio_orderindex_invalid(&self->state->dragseqpos)) {
		sequencecmds_newentry(self->state->cmds, self->state->inserttype);
		psy_ui_mouseevent_stop_propagation(ev);
	}	
}

void seqedittrack_on_sequence_insert(SeqEditTrack* self,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqedittrack_build(self);
	}
}

void seqedittrack_on_sequence_remove(SeqEditTrack* self,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqedittrack_build(self);
	}
}

void seqedittrack_on_sequence_track_reposition(SeqEditTrack* self,
	psy_audio_Sequence* sender, uintptr_t track_idx)
{
	assert(self);
	
	if (self->trackindex == track_idx) {
		seqedittrack_on_align(self);
		psy_ui_component_invalidate(&self->component);
	}
}
