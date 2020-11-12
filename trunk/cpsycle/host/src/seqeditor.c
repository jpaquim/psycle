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

// SeqEditorHeader
static void seqeditorheader_ondraw(SeqEditorHeader*, psy_ui_Graphics*);
static void seqeditorheader_drawruler(SeqEditorHeader*, psy_ui_Graphics*);
static void seqeditorheader_onpreferredsize(SeqEditorHeader*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable seqeditorheader_vtable;
static int seqeditorheader_vtable_initialized = 0;

static void seqeditorheader_vtable_init(SeqEditorHeader* self)
{
	if (!seqeditorheader_vtable_initialized) {
		seqeditorheader_vtable = *(self->component.vtable);
		seqeditorheader_vtable.ondraw = (psy_ui_fp_ondraw)
			seqeditorheader_ondraw;
		seqeditorheader_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			seqeditorheader_onpreferredsize;
		seqeditorheader_vtable_initialized = 1;
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

static SeqEditorTrackVTable seqeditortrack_vtable;
static bool seqeditortrack_vtable_initialized = FALSE;

static void seqeditortrack_vtable_init(void)
{
	if (!seqeditortrack_vtable_initialized) {
		seqeditortrack_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrack_ondraw_virtual;
		seqeditortrack_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrack_onpreferredsize_virtual;
		seqeditortrack_vtable_initialized = TRUE;
	}
}

void seqeditortrack_init(SeqEditorTrack* self, SeqEditorTracks* parent,
	Workspace* workspace)
{
	seqeditortrack_vtable_init();
	self->vtable = &seqeditortrack_vtable;
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
	Workspace* workspace)
{
	SeqEditorTrack* rv;

	rv = seqeditortrack_alloc();
	if (rv) {
		seqeditortrack_init(rv, parent, workspace);
	}
	return rv;
}

void seqeditortrack_updatetrack(SeqEditorTrack* self, psy_audio_SequenceTrack* track)
{
	self->currtrack = track;
}

void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g, int x, int y)
{
	psy_List* p;
	int width;

	width = 500;
	for (p = self->currtrack->entries; p != NULL; psy_list_next(&p)) {
		psy_audio_SequenceEntry* entry;		
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*)p->entry;
		pattern = psy_audio_patterns_at(&self->workspace->song->patterns,
			entry->patternslot);
		if (pattern) {
			psy_ui_Rectangle r;
			int patternwidth;

			patternwidth = (int)(psy_audio_pattern_length(pattern) / 100 * width);
			r = psy_ui_rectangle_make((int)(entry->offset / 100 * width), y,
				patternwidth, 20);
			psy_ui_drawrectangle(g, r);
		}
	}	
}

void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makeew(12.0);
	rv->height = psy_ui_value_makeeh(2.0);
}

// SeqEditorTracks
// prototypes
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_ondraw(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_onpreferredsize(SeqEditorTracks*, psy_ui_Size*
	limit, psy_ui_Size* rv);
static void seqeditortracks_build(SeqEditorTracks*);
static psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks*);
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
		seqeditortracks_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);	
	seqeditortracks_vtable_init(self);
	self->component.vtable = &seqeditortracks_vtable;
	self->tracks = NULL;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	seqeditortracks_build(self);
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

		track = seqeditortrack_allocinit(self, self->workspace);
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
	self->workspace = workspace;
	seqeditorheader_init(&self->header, &self->component);
	psy_ui_component_setalign(seqeditorheader_base(&self->header),
		psy_ui_ALIGN_TOP);
	seqeditortracks_init(&self->tracks, &self->component,
		workspace);
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);	
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
	psy_ui_component_invalidate(&self->tracks.component);
}
