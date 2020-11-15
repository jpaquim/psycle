// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "seqeditor.h"

#include <patterns.h>
#include <songio.h>

#include <uiapp.h>

#include <math.h>
#include <string.h>

#include "../../detail/trace.h"
#include "../../detail/portable.h"

void seqeditortrackstate_init(SeqEditorTrackState* self)
{
	self->pxperbeat = 5;
}

// SeqEditorHeader
static void seqeditorheader_ondraw(SeqEditorHeader*, psy_ui_Graphics*);
static void seqeditorheader_drawruler(SeqEditorHeader*, psy_ui_Graphics*);
static void seqeditorheader_onpreferredsize(SeqEditorHeader*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable seqeditorheader_vtable;
static bool seqeditorheader_vtable_initialized = FALSE;

static void seqeditorheader_vtable_init(SeqEditorHeader* self)
{
	if (!seqeditorheader_vtable_initialized) {
		seqeditorheader_vtable = *(self->component.vtable);
		seqeditorheader_vtable.ondraw = (psy_ui_fp_ondraw)
			seqeditorheader_ondraw;
		seqeditorheader_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			seqeditorheader_onpreferredsize;
		seqeditorheader_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditorheader_init(SeqEditorHeader* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	seqeditorheader_vtable_init(self);
	self->component.vtable = &seqeditorheader_vtable;
	psy_ui_component_doublebuffer(&self->component);
}

void seqeditorheader_ondraw(SeqEditorHeader* self, psy_ui_Graphics* g)
{
	seqeditorheader_drawruler(self, g);
	psy_ui_textout(g, 0, 0, "Sequencer Tracks", strlen("Sequencer Tracks"));	
}

void seqeditorheader_drawruler(SeqEditorHeader* self, psy_ui_Graphics* g)
{
}

void seqeditorheader_onpreferredsize(SeqEditorHeader* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(500);
	rv->height = psy_ui_value_makepx(20);
}

static void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g, int x, int y);
static void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditortrack_onmousedown_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);

static SeqEditorTrackVTable seqeditortrack_vtable;
static bool seqeditortrack_vtable_initialized = FALSE;

static void seqeditortrack_vtable_init(void)
{
	if (!seqeditortrack_vtable_initialized) {
		seqeditortrack_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrack_ondraw_virtual;
		seqeditortrack_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrack_onpreferredsize_virtual;
		seqeditortrack_vtable.onmousedown = (seqeditortrack_fp_onmousedown)
			seqeditortrack_onmousedown_virtual;
		seqeditortrack_vtable_initialized = TRUE;
	}
}

void seqeditortrack_init(SeqEditorTrack* self, SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_vtable_init();
	self->vtable = &seqeditortrack_vtable;
	self->trackstate = trackstate;
	self->workspace = workspace;
	self->parent = parent;
	self->currtrack = NULL;
}

void seqeditortrack_dispose(SeqEditorTrack* self)
{
}

SeqEditorTrack* seqeditortrack_alloc(void)
{
	return (SeqEditorTrack*)malloc(sizeof(SeqEditorTrack));
}

SeqEditorTrack* seqeditortrack_allocinit(SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	SeqEditorTrack* rv;

	rv = seqeditortrack_alloc();
	if (rv) {
		seqeditortrack_init(rv, parent, trackstate, workspace);
	}
	return rv;
}

void seqeditortrack_updatetrack(SeqEditorTrack* self, psy_audio_SequenceTrack*
	track)
{
	self->currtrack = track;
}

void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g, int x, int y)
{
	psy_List* p;

	for (p = self->currtrack->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_SequenceEntry* entry;		
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*)p->entry;
		pattern = psy_audio_patterns_at(&self->workspace->song->patterns,
			entry->patternslot);
		if (pattern) {
			psy_ui_Rectangle r;
			int patternwidth;
			bool selected;

			patternwidth = (int)(psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);
			r = psy_ui_rectangle_make((int)(entry->offset * self->trackstate->pxperbeat), y,
				patternwidth, 20);
			selected = FALSE;
			if (self->workspace->sequenceselection.editposition.track) {
				psy_audio_SequenceTrack* editpositiontrack;

				editpositiontrack = (psy_audio_SequenceTrack*)psy_list_entry(
					self->workspace->sequenceselection.editposition.track);
				if (editpositiontrack == self->currtrack &&
					(self->workspace->sequenceselection.editposition.trackposition.tracknode == p)) {
					selected = TRUE;					
				}
			}
			if (selected) {
				psy_ui_drawsolidrectangle(g, r, psy_ui_color_make(0x00514536));
			} else {
				psy_ui_drawrectangle(g, r);
			}
		}
	}	
}

void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&self->workspace->song->patterns);
	}
	rv->width = psy_ui_value_makepx((intptr_t)(self->trackstate->pxperbeat *
		trackduration));
	rv->height = psy_ui_value_makeeh(2.0);
}

void seqeditortrack_onmousedown_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	psy_List* p;
	uintptr_t selected;


	for (p = self->currtrack->entries, selected = 0; p != NULL;
			psy_list_next(&p), ++selected) {
		psy_audio_SequenceEntry* entry;
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*)p->entry;
		pattern = psy_audio_patterns_at(&self->workspace->song->patterns,
			entry->patternslot);
		if (pattern) {
			psy_ui_Rectangle r;
			int patternwidth;

			patternwidth = (int)(psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);
			r = psy_ui_rectangle_make((int)(entry->offset * self->trackstate->pxperbeat), 0,
				patternwidth, 20);
			if (psy_ui_rectangle_intersect(&r, ev->x, ev->y)) {
				if (self->currtrack) {
					uintptr_t trackindex;

					trackindex = psy_list_entry_index(self->workspace->song->sequence.tracks,
						self->currtrack);
					if (trackindex != UINTPTR_MAX) {
						psy_audio_sequenceselection_seteditposition(
							&self->workspace->sequenceselection,
							psy_audio_sequence_at(&self->workspace->song->sequence,
								trackindex, selected));
						workspace_setsequenceselection(self->workspace,
							*&self->workspace->sequenceselection);
					}					
				}
				break;
			}			
		}
	}
}

// SeqEditorTracks
// prototypes
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_ondraw(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_onpreferredsize(SeqEditorTracks*, psy_ui_Size*
	limit, psy_ui_Size* rv);
static void seqeditortracks_onmousedown(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_build(SeqEditorTracks*);
static psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks*);
static void seqeditortracks_onsequenceselectionchanged(SeqEditorTracks*, Workspace*);
// vtable
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);
		seqeditortracks_vtable.ondraw = (psy_ui_fp_ondraw)seqeditortracks_ondraw;
		seqeditortracks_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			seqeditortracks_onpreferredsize;
		seqeditortracks_vtable.onmousedown = (psy_ui_fp_onmousedown)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	self->workspace = workspace;
	self->trackstate = trackstate;
	psy_ui_component_init(&self->component, parent);	
	seqeditortracks_vtable_init(self);
	self->component.vtable = &seqeditortracks_vtable;
	self->tracks = NULL;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	seqeditortracks_build(self);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged, self,
		seqeditortracks_onsequenceselectionchanged);
}

void seqeditortracks_ondestroy(SeqEditorTracks* self, psy_ui_Component* sender)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	uintptr_t t;

	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
	for (t = 0; t < 3; ++t) {
		SeqEditorTrack* track;

		track = seqeditortrack_allocinit(self, self->trackstate,
			self->workspace);
		if (track) {
			psy_list_append(&self->tracks, track);
		}
	}
}

void seqeditortracks_ondraw(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence = seqeditortracks_sequence(self);

	seqeditortracks_build(self);
	if (sequence) {
		psy_audio_SequenceTracks* seqnode;
		psy_List* seqeditnode;
		int cpx, cpy;

		psy_ui_settextcolor(g, psy_ui_color_make(0x00FFFFFF));
		cpx = 0;
		cpy = 0;
		for (seqeditnode = self->tracks, seqnode = sequence->tracks;
				seqnode != NULL && seqeditnode != NULL;
				psy_list_next(&seqnode)) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;

			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			seqeditortrack_updatetrack(seqedittrack, seqtrack);
			seqeditortrack_ondraw(seqedittrack, g, cpx, cpy);
			cpy += 25;
		}
	}
}

psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks* self)
{
	if (self->workspace->song) {
		return &self->workspace->song->sequence;
	}
	return NULL;
}

void seqeditortracks_onpreferredsize(SeqEditorTracks* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	seqeditortracks_build(self);
	if (self->workspace->song) {
		psy_audio_SequenceTracks* seqnode;
		psy_List* seqeditnode;
		int cpxmax, cpymax;
		psy_ui_TextMetric tm;

		cpxmax = 0;
		cpymax = 0;
		tm = psy_ui_component_textmetric(&self->component);
		for (seqeditnode = self->tracks,
				seqnode = self->workspace->song->sequence.tracks;
			seqnode != NULL && seqeditnode != NULL;
			psy_list_next(&seqnode)) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;
			psy_ui_Size limit;
			psy_ui_Size preferredtracksize;

			limit = psy_ui_component_size(&self->component);

			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			seqeditortrack_updatetrack(seqedittrack, seqtrack);
			seqeditortrack_onpreferredsize(seqedittrack, &limit, &preferredtracksize);
			cpxmax = max(cpxmax, psy_ui_value_px(&preferredtracksize.width, &tm));
			cpymax += psy_ui_value_px(&preferredtracksize.height, &tm);
		}
		rv->width = psy_ui_value_makepx(cpxmax);
		rv->height = psy_ui_value_makepx(cpymax);
	} else {
		rv->width = psy_ui_value_makepx(0);
		rv->height = psy_ui_value_makepx(0);
	}
}

void seqeditortracks_onmousedown(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	psy_audio_Sequence* sequence = seqeditortracks_sequence(self);

	seqeditortracks_build(self);
	if (sequence) {
		psy_audio_SequenceTracks* seqnode;
		psy_List* seqeditnode;
		int cpx, cpy;

		cpx = 0;
		cpy = 0;
		for (seqeditnode = self->tracks, seqnode = sequence->tracks;
			seqnode != NULL && seqeditnode != NULL;
			psy_list_next(&seqnode)) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;

			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			seqeditortrack_updatetrack(seqedittrack, seqtrack);
			cpy += 25;
			if (cpy > ev->y) {
				psy_ui_MouseEvent trackev;
				
				trackev = *ev;
				trackev.y = ev->y - (cpy - 25);
				seqeditortrack_onmousedown(seqedittrack, &trackev);
			}
		}
	}
}

void seqeditortracks_onsequenceselectionchanged(SeqEditorTracks* self,
	Workspace* sender)
{
	psy_ui_component_invalidate(&self->component);
}

// SeqEditor
// prototypes
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag, psy_audio_SongFile*);
static void seqeditor_updatesong(SeqEditor*, psy_audio_Song*);
static void seqeditor_onsequencechanged(SeqEditor*, psy_audio_Sequence* sender);
// vtable
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);		
		seqeditor_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	seqeditor_vtable_init(self);
	self->component.vtable = &seqeditor_vtable;
	psy_ui_component_doublebuffer(&self->component);
	seqeditortrackstate_init(&self->trackstate);
	self->workspace = workspace;
	seqeditorheader_init(&self->header, &self->component);
	psy_ui_component_setalign(seqeditorheader_base(&self->header),
		psy_ui_ALIGN_TOP);
	seqeditortracks_init(&self->tracks, &self->component,
		&self->trackstate, workspace);
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_resize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20.0),
			psy_ui_value_makeeh(12.0)));
	psy_ui_component_preventpreferredsize(&self->component);
	seqeditor_updatesong(self, workspace->song);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	seqeditor_updatesong(self, workspace->song);
}

void seqeditor_updatesong(SeqEditor* self, psy_audio_Song* song)
{
	if (song) {
		psy_signal_connect(&song->sequence.sequencechanged, self,
			seqeditor_onsequencechanged);
	}
}

void seqeditor_onsequencechanged(SeqEditor* self, psy_audio_Sequence* sender)
{
	psy_ui_component_updateoverflow(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
}
