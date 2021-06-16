/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
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
static void seqedittrack_ondestroy(SeqEditTrack*);
static void seqedittrack_onpreferredsize(SeqEditTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqedittrack_onalign(SeqEditTrack*);
static void seqedittrack_onmousedown(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_onmousemove(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_onmouseup(SeqEditTrack*, psy_ui_MouseEvent*);
static void seqedittrack_onmousedoubleclick(SeqEditTrack*,
	psy_ui_MouseEvent*);
static void seqedittrack_build(SeqEditTrack* self);
static void seqedittrack_onsequenceinsert(SeqEditTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex*);
static void seqedittrack_onsequenceremove(SeqEditTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex*);
static void seqedittrack_onsequencetrackreposition(SeqEditTrack*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
/* vtable */
static psy_ui_ComponentVtable seqedittrack_vtable;
static bool seqedittrack_vtable_initialized = FALSE;

static void seqedittrack_vtable_init(SeqEditTrack* self)
{
	if (!seqedittrack_vtable_initialized) {
		seqedittrack_vtable = *(self->component.vtable);
		seqedittrack_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqedittrack_ondestroy;
		seqedittrack_vtable.onalign =
			(psy_ui_fp_component_onalign)
			seqedittrack_onalign;
		seqedittrack_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqedittrack_onpreferredsize;
		seqedittrack_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmousedown;
		seqedittrack_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmousemove;
		seqedittrack_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmouseup;
		seqedittrack_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqedittrack_onmousedoubleclick;
		seqedittrack_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqedittrack_vtable;
}
/* implementation */
void seqedittrack_init(SeqEditTrack* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	seqedittrack_vtable_init(self);	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_make_em(0.0, 2.0));	
	self->state = state;
	self->workspace = workspace;	
	self->currtrack = NULL;
	self->trackindex = 0;		
	self->view = view;	
	self->entries = NULL;
	if (seqeditstate_sequence(self->state)) {
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_insert,
			self, seqedittrack_onsequenceinsert);
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_remove,
			self, seqedittrack_onsequenceremove);
		psy_signal_connect(
			&seqeditstate_sequence(self->state)->signal_trackreposition,
			self, seqedittrack_onsequencetrackreposition);		
	}
}

void seqedittrack_ondestroy(SeqEditTrack* self)
{
	if (seqeditstate_sequence(self->state)) {
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_insert,
			self, seqedittrack_onsequenceinsert);
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_remove,
			self, seqedittrack_onsequenceremove);
		psy_signal_disconnect(
			&seqeditstate_sequence(self->state)->signal_trackreposition,
			self, seqedittrack_onsequencetrackreposition);		
	}
	psy_list_free(self->entries);
	self->entries = NULL;
}

SeqEditTrack* seqedittrack_alloc(void)
{
	return (SeqEditTrack*)malloc(sizeof(SeqEditTrack));
}

SeqEditTrack* seqedittrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditState* state, Workspace* workspace)
{
	SeqEditTrack* rv;

	rv = seqedittrack_alloc();
	if (rv) {
		seqedittrack_init(rv, parent, view, state, workspace);
		psy_ui_component_deallocateafterdestroyed(seqedittrack_base(rv));
	}
	return rv;
}

void seqedittrack_setsequencetrack(SeqEditTrack* self,
	psy_audio_SequenceTrackNode* tracknode, psy_audio_SequenceTrack* track,
	uintptr_t trackindex)
{
	self->currtrack = track;	
	self->trackindex = trackindex;	
	if (track->height != 0.0) {
		psy_ui_component_setpreferredheight(&self->component,
			psy_ui_value_make_eh(track->height));
	}
	seqedittrack_build(self);		
}

void seqedittrack_build(SeqEditTrack* self)
{
	psy_List* p;
	uintptr_t c;	

	psy_ui_component_clear(&self->component);
	psy_list_free(self->entries);
	self->entries = NULL;
	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	for (p = self->currtrack->entries, c = 0; p != NULL;
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
					&workspace_song(self->workspace)->patterns,
					psy_audio_sequencepatternentry_patternslot(
						seqpatternentry));
				if (pattern) {
					SeqEditPatternEntry* seqeditpatternentry;

					seqeditpatternentry = seqeditpatternentry_allocinit(
						&self->component, self->view, seqpatternentry,
						psy_audio_orderindex_make(self->trackindex, c),
						self->state);
					if (seqeditpatternentry) {
						psy_ui_component_setalign(
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
					&self->component, self->view, seqsampleentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqsampleentry) {
					psy_ui_component_setalign(
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
					&self->component, self->view, seqmarkerentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqeditmarkerentry) {
					psy_ui_component_setalign(seqeditmarkerentry_base(seqeditmarkerentry),
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

void seqedittrack_onalign(SeqEditTrack* self)
{		
	psy_List* p;
	psy_List* q;
	psy_ui_Size size;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	size = psy_ui_component_scrollsize(&self->component);	
	p = self->entries;
	for (p = self->entries, q = self->currtrack->entries;
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

void seqedittrack_onpreferredsize(SeqEditTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&workspace_song(self->workspace)->patterns);
	}
	trackduration += 400.0;
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
		trackduration);
	rv->height = self->state->lineheight;
}

void seqedittrack_onmousedown(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		if (self->state->dragstatus == SEQEDIT_DRAG_NONE) {
			psy_audio_OrderIndex seqpos;

			seqpos = workspace_sequenceeditposition(self->state->workspace);
			if (self->trackindex != seqpos.track) {
				seqpos.track = self->trackindex;
				seqpos.order = psy_INDEX_INVALID;
				workspace_setseqeditposition(self->state->workspace, seqpos);
			}
		} else {
			psy_ui_component_capture(&self->component);
		}
	}
}

void seqedittrack_onmousemove(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{	
	if (self->state->dragstatus == SEQEDIT_DRAG_NONE ||
		self->state->dragstatus == SEQEDIT_DRAG_REMOVE ||
		self->state->seqentry == NULL) {
		return;
	}
	if (self->state->dragstatus == SEQEDIT_DRAG_START) {
		psy_dsp_big_beat_t dragposition;
		psy_dsp_big_beat_t bpl;

		dragposition = seqeditstate_pxtobeat(self->state, ev->pt.x) -
			psy_audio_sequenceentry_offset(self->state->seqentry);
		bpl = (1.0 / (psy_dsp_big_beat_t)psy_audio_player_lpb(
			workspace_player(self->workspace)));
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
			seqeditstate_pxtobeat(self->state, ev->pt.x));
		psy_audio_sequenceentry_setlength(self->state->seqentry,
			psy_max(1.0, dragposition - psy_audio_sequenceentry_offset(
				self->state->seqentry)));
		psy_audio_sequence_reposition_track(
			&workspace_song(self->workspace)->sequence, self->currtrack);
			
	} else if ((self->state->dragtype & SEQEDIT_DRAGTYPE_MOVE) ==
			SEQEDIT_DRAGTYPE_MOVE) {
		psy_dsp_big_beat_t dragposition;

		dragposition = seqeditstate_pxtobeat(self->state, ev->pt.x);
		if (dragposition - (self->state->seqentry->offset -
			self->state->seqentry->repositionoffset) >= 0) {
			dragposition = seqeditstate_quantize(self->state,
				dragposition);
		} else {
			dragposition = self->state->seqentry->offset -
				self->state->seqentry->repositionoffset;
		}
		self->state->seqentry->repositionoffset = dragposition -
			(self->state->seqentry->offset -
				self->state->seqentry->repositionoffset);
		self->state->seqentry->offset = dragposition;
		psy_audio_sequence_reposition_track(
			&workspace_song(self->workspace)->sequence, self->currtrack);
	}	
}

void seqedittrack_onmouseup(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);	
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

void seqedittrack_onmousedoubleclick(SeqEditTrack* self, psy_ui_MouseEvent* ev)
{
	if (psy_audio_orderindex_invalid(&self->state->dragseqpos)) {
		sequencecmds_newentry(self->state->cmds, self->state->inserttype);
		psy_ui_mouseevent_stop_propagation(ev);
	}	
}

void seqedittrack_onsequenceinsert(SeqEditTrack* self,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqedittrack_build(self);
	}
}

void seqedittrack_onsequenceremove(SeqEditTrack* self,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqedittrack_build(self);
	}
}

void seqedittrack_onsequencetrackreposition(SeqEditTrack* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	seqedittrack_onalign(self);
	psy_ui_component_invalidate(&self->component);
}
