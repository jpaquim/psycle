// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "seqeditor.h"
// host
#include "sequencetrackbox.h"
// audio
#include <exclusivelock.h>
#include <patterns.h>
#include <songio.h>
// ui
#include <uiapp.h>
// std
#include <math.h>
#include <string.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 5

void seqeditortrackstate_init(SeqEditorTrackState* self)
{
	self->pxperbeat = DEFAULT_PXPERBEAT;
	self->defaultlineheight = psy_ui_value_makeeh(1.2);
	self->lineheight = self->defaultlineheight;
	self->linemargin = psy_ui_value_makeeh(0.5);
}

// SeqEditorRuler
static void seqeditorruler_ondraw(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_drawruler(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler*, Workspace*);
static void seqeditorruler_onpreferredsize(SeqEditorRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable seqeditorruler_vtable;
static bool seqeditorruler_vtable_initialized = FALSE;

static void seqeditorruler_vtable_init(SeqEditorRuler* self)
{
	if (!seqeditorruler_vtable_initialized) {
		seqeditorruler_vtable = *(self->component.vtable);
		seqeditorruler_vtable.ondraw = (psy_ui_fp_component_ondraw)
			seqeditorruler_ondraw;
		seqeditorruler_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			seqeditorruler_onpreferredsize;
		seqeditorruler_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditorruler_init(SeqEditorRuler* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	seqeditorruler_vtable_init(self);
	self->component.vtable = &seqeditorruler_vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->trackstate = trackstate;
	self->workspace = workspace;
	self->rulerbaselinecolour = psy_ui_colour_make(0x00555555);
	self->rulermarkcolour = psy_ui_colour_make(0x00666666);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged, self,
		seqeditorruler_onsequenceselectionchanged);
}

void seqeditorruler_ondraw(SeqEditorRuler* self, psy_ui_Graphics* g)
{	
	seqeditorruler_drawruler(self, g);
}

void seqeditorruler_drawruler(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	intptr_t baseline;
	intptr_t linewidth;
	psy_dsp_big_beat_t duration;
	psy_dsp_big_beat_t clipstart;
	psy_dsp_big_beat_t clipend;
	psy_dsp_big_beat_t currbeat;
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	baseline = size.height - 1;	
	duration = (size.width + psy_ui_component_scrollleft(&self->component)) /
		(psy_dsp_big_beat_t)self->trackstate->pxperbeat;
	//psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence);
	linewidth = (intptr_t)(duration * self->trackstate->pxperbeat);
	psy_ui_setcolour(g, self->rulerbaselinecolour);
	psy_ui_drawline(g, 0, baseline, linewidth, baseline);
	clipstart = 0;
	clipend = duration;
	for (currbeat = clipstart; currbeat <= clipend; currbeat += 16.0) {
		intptr_t cpx;
		char txt[40];

		cpx = (intptr_t)(currbeat * self->trackstate->pxperbeat);
		psy_ui_drawline(g, cpx, baseline, cpx, baseline - tm.tmHeight / 3);
		psy_snprintf(txt, 40, "%d", (int)(currbeat));
		psy_ui_textout(g, (int)cpx + 3, baseline - tm.tmHeight, txt, strlen(txt));
	}
}

void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler* self, Workspace* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditorruler_onpreferredsize(SeqEditorRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (workspace_song(self->workspace)) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		intptr_t linewidth;
		psy_dsp_big_beat_t duration;

		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		duration = psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence);
		linewidth = (intptr_t)(duration * self->trackstate->pxperbeat);
		rv->width = psy_ui_value_makepx(linewidth);
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
	rv->height = psy_ui_value_makeeh(1.0);
}

// SeqEditorTrack
// prototypes
static void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g, intptr_t x, intptr_t y);
static void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static bool seqeditortrack_onmousedown_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static bool seqeditortrack_onmousemove_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static bool seqeditortrack_onmouseup_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
// vtable
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
		seqeditortrack_vtable.onmousemove = (seqeditortrack_fp_onmousemove)
			seqeditortrack_onmousemove_virtual;
		seqeditortrack_vtable.onmouseup = (seqeditortrack_fp_onmouseup)
			seqeditortrack_onmouseup_virtual;
		seqeditortrack_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortrack_init(SeqEditorTrack* self, SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_vtable_init();
	self->vtable = &seqeditortrack_vtable;
	self->trackstate = trackstate;
	self->workspace = workspace;
	self->parent = parent;
	self->currtrack = NULL;
	self->trackindex = 0;
	self->drag_sequenceitem_node = NULL;
	self->dragstarting = FALSE;
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
	track, uintptr_t trackindex)
{
	self->currtrack = track;
	self->trackindex = trackindex;
}

void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g, intptr_t x, intptr_t y)
{
	psy_List* p;
	intptr_t c;
	psy_dsp_big_beat_t clipstart;
	psy_dsp_big_beat_t clipend;
	psy_ui_TextMetric tm;
	intptr_t lineheight;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {		
		return;
	}
	clipstart = g->clip.left / (psy_dsp_big_beat_t)self->trackstate->pxperbeat;
	clipend = g->clip.right / (psy_dsp_big_beat_t)self->trackstate->pxperbeat;
	tm = psy_ui_component_textmetric(&self->parent->component);
	lineheight = psy_ui_value_px(&self->trackstate->lineheight, &tm);
	for (p = self->currtrack->entries, c = 0; p != NULL;
			psy_list_next(&p), ++c) {
		psy_audio_SequenceEntry* sequenceentry;
		psy_audio_Pattern* pattern;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
			psy_audio_sequenceentry_patternslot(sequenceentry));
		if (pattern) {
			psy_ui_Rectangle r;
			intptr_t patternwidth;
			bool selected;
			char text[256];

			if (sequenceentry->offset + psy_audio_pattern_length(pattern) < clipstart ||
				sequenceentry->offset > clipend) {
				continue;
			}
			
			patternwidth = (intptr_t)(psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);
			r = psy_ui_rectangle_make((intptr_t)(sequenceentry->offset * self->trackstate->pxperbeat), y,
				patternwidth, lineheight);
			selected = FALSE;
			if (self->workspace->sequenceselection.editposition.tracknode) {
				psy_audio_SequenceTrack* editpositiontrack;

				editpositiontrack = (psy_audio_SequenceTrack*)psy_list_entry(
					self->workspace->sequenceselection.editposition.tracknode);
				if (editpositiontrack == self->currtrack &&
					(self->workspace->sequenceselection.editposition.trackposition.sequencentrynode == p)) {
					selected = TRUE;					
				}
			}
			if (selected) {
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00514536));
				psy_ui_setcolour(g, psy_ui_colour_make(0x00555555));
				psy_ui_drawrectangle(g, r);				
			} else {
				psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00333333));
				psy_ui_setcolour(g, psy_ui_colour_make(0x00444444));
				psy_ui_drawrectangle(g, r);				
			}
			if (generalconfig_showingpatternnames(psycleconfig_general(
					workspace_conf(self->workspace)))) {
				psy_audio_Pattern* pattern;

				pattern = psy_audio_sequenceentry_pattern(sequenceentry,
					&workspace_song(self->workspace)->patterns);
				if (pattern) {
					psy_snprintf(text, 20, "%02X: %s", c,
						psy_audio_pattern_name(pattern));
				} else {
					psy_snprintf(text, 20, "%02X:%02X(ERR)", c,
						(int)psy_audio_sequenceentry_patternslot(sequenceentry));
				}
				psy_ui_textoutrectangle(g, r.left + 2, r.top, psy_ui_ETO_CLIPPED, r,
					text, strlen(text));
			} else {
				psy_snprintf(text, 256, "%02X",
					(int)psy_audio_sequenceentry_patternslot(sequenceentry));
				psy_ui_textoutrectangle(g, r.left + 2, r.top, psy_ui_ETO_CLIPPED, r,
					text, strlen(text));
			}
		}
	}
	if (self->drag_sequenceitem_node && !self->dragstarting) {
		psy_ui_Rectangle r;
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;
		psy_audio_SequenceEntry* sequenceentry;
		intptr_t cpx;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(
			self->drag_sequenceitem_node);
		tm = psy_ui_component_textmetric(&self->parent->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->parent->component), &tm);
		cpx = (intptr_t)(self->itemdragposition * self->trackstate->pxperbeat);
		r = psy_ui_rectangle_make(cpx, y, 2, lineheight);
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00CACACA));		
	}
}

void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&workspace_song(self->workspace)->patterns);
	}
	rv->width = psy_ui_value_makepx((intptr_t)(self->trackstate->pxperbeat *
		trackduration));
	rv->height = psy_ui_value_makeeh(2.0);
}

bool seqeditortrack_onmousedown_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	psy_List* sequenceitem_node;
	uintptr_t selected;
	
	psy_ui_component_capture(&self->parent->component);
	self->drag_sequenceitem_node = NULL;
	for (sequenceitem_node = self->currtrack->entries, selected = 0;
			sequenceitem_node != NULL;
			psy_list_next(&sequenceitem_node), ++selected) {
		psy_audio_SequenceEntry* entry;
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*)psy_list_entry(sequenceitem_node);		
		pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
			entry->patternslot);
		if (pattern) {
			psy_ui_Rectangle r;
			intptr_t patternwidth;
			psy_ui_TextMetric tm;
			intptr_t lineheight;
			
			tm = psy_ui_component_textmetric(&self->parent->component);
			lineheight = psy_ui_value_px(&self->trackstate->lineheight, &tm);
			patternwidth = (intptr_t)(psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);
			r = psy_ui_rectangle_make((intptr_t)(entry->offset * self->trackstate->pxperbeat), 0,
				patternwidth, lineheight);
			if (psy_ui_rectangle_intersect(&r, ev->x, ev->y)) {
				if (self->currtrack) {
					uintptr_t trackindex;

					trackindex = psy_list_entry_index(workspace_song(self->workspace)->sequence.tracks,
						self->currtrack);
					if (trackindex != UINTPTR_MAX) {
						psy_audio_sequenceselection_seteditposition(
							&self->workspace->sequenceselection,
							psy_audio_sequence_at(&workspace_song(self->workspace)->sequence,
								trackindex, selected));
						workspace_setsequenceselection(self->workspace,
							*&self->workspace->sequenceselection);
					}					
				}
				self->drag_sequenceitem_node = sequenceitem_node;
				self->parent->capture = self;
				self->itemdragposition = entry->offset;
				self->dragstarting = TRUE;				
			}			
		}
	}
	return TRUE;
}

bool seqeditortrack_onmousemove_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	if (self->drag_sequenceitem_node) {
		psy_audio_SequenceEntry* sequenceentry;
		psy_dsp_big_beat_t dragposition;

		sequenceentry = (psy_audio_SequenceEntry*)self->drag_sequenceitem_node->entry;
		dragposition = (psy_dsp_big_beat_t)(ev->x / self->trackstate->pxperbeat);
		if (dragposition - (sequenceentry->offset - sequenceentry->repositionoffset) >= 0) {
			self->itemdragposition = dragposition;
		}
		self->dragstarting = FALSE;
		psy_ui_component_invalidate(&self->parent->component);
	}
	return TRUE;
}

bool seqeditortrack_onmouseup_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->parent->component);
	self->parent->capture = NULL;
	if (self->drag_sequenceitem_node && !self->dragstarting) {
		psy_audio_SequenceEntry* sequenceentry;

		sequenceentry = (psy_audio_SequenceEntry*)self->drag_sequenceitem_node->entry;
		sequenceentry->repositionoffset = self->itemdragposition - (sequenceentry->offset - sequenceentry->repositionoffset);
		sequenceentry->offset = self->itemdragposition;
		self->drag_sequenceitem_node = NULL;
		psy_audio_sequence_reposition_track(&workspace_song(self->workspace)->sequence, self->currtrack);
		psy_signal_emit(&workspace_song(self->workspace)->sequence.sequencechanged, self, 0);
	}
	self->drag_sequenceitem_node = NULL;
	self->dragstarting = FALSE;
	return TRUE;
}

// SeqEditorTrackHeader
// prototypes
static void seqeditortrackheader_ondraw(SeqEditorTrackHeader* self, psy_ui_Graphics* g, intptr_t x, intptr_t y);
static void seqeditortrackheader_onpreferredsize(SeqEditorTrackHeader*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static bool seqeditortrackheader_onmousedown(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
static bool seqeditortrackheader_onmousemove(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
static bool seqeditortrackheader_onmouseup(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
// vtable
static SeqEditorTrackVTable seqeditortrackheader_vtable;
static bool seqeditortrackheader_vtable_initialized = FALSE;

static void seqeditortrackheader_vtable_init(SeqEditorTrackHeader* self)
{
	if (!seqeditortrackheader_vtable_initialized) {
		seqeditortrackheader_vtable = *(self->base.vtable);
		seqeditortrackheader_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrackheader_ondraw;
		seqeditortrackheader_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrackheader_onpreferredsize;
		seqeditortrackheader_vtable.onmousedown = (seqeditortrack_fp_onmousedown)
			seqeditortrackheader_onmousedown;
		seqeditortrackheader_vtable.onmousemove = (seqeditortrack_fp_onmousemove)
			seqeditortrackheader_onmousemove;
		seqeditortrackheader_vtable.onmouseup = (seqeditortrack_fp_onmouseup)
			seqeditortrackheader_onmouseup;
		seqeditortrackheader_vtable_initialized = TRUE;
	}
}
void seqeditortrackheader_init(SeqEditorTrackHeader* self,
	struct SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_init(&self->base, parent, trackstate, workspace);
	seqeditortrackheader_vtable_init(self);
	self->base.vtable = &seqeditortrackheader_vtable;
}

SeqEditorTrackHeader* seqeditortrackheader_alloc(void)
{
	return (SeqEditorTrackHeader*)malloc(sizeof(SeqEditorTrackHeader));
}

SeqEditorTrackHeader* seqeditortrackheader_allocinit(SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	SeqEditorTrackHeader* rv;

	rv = seqeditortrackheader_alloc();
	if (rv) {
		seqeditortrackheader_init(rv, parent, trackstate, workspace);
	}
	return rv;
}

void seqeditortrackheader_ondraw(SeqEditorTrackHeader* self,
	psy_ui_Graphics* g, intptr_t x, intptr_t y)
{	
	psy_audio_SequenceTrack* edittrack;
	SequenceTrackBox trackbox;
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;

	if (self->base.workspace->sequenceselection.editposition.tracknode) {
		edittrack = (psy_audio_SequenceTrack*)
			self->base.workspace->sequenceselection.editposition.tracknode->entry;
	} else {
		edittrack = NULL;
	}
	tm = psy_ui_component_textmetric(&self->base.parent->component);
	size = psy_ui_component_intsize(&self->base.parent->component);
	sequencetrackbox_init(&trackbox,
		psy_ui_rectangle_make(
			x, y, size.width,
			psy_ui_value_px(&self->base.trackstate->lineheight, &tm)),
		tm, self->base.currtrack, self->base.trackindex,
		self->base.currtrack == edittrack);
	sequencetrackbox_draw(&trackbox, g);
}

void seqeditortrackheader_onpreferredsize(SeqEditorTrackHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(rv);

	*rv = psy_ui_size_makeem(15.0, 2.0);	
}

bool seqeditortrackheader_onmousedown(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	if (self->base.currtrack) {
		psy_audio_SequenceTrack* edittrack;
		SequenceTrackBox trackbox;
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(&self->base.parent->component);
		size = psy_ui_component_intsize(&self->base.parent->component);
		if (self->base.workspace->sequenceselection.editposition.tracknode) {
			edittrack = (psy_audio_SequenceTrack*)
				self->base.workspace->sequenceselection.editposition.tracknode->entry;
		} else {
			edittrack = NULL;
		}
		sequencetrackbox_init(&trackbox,
			psy_ui_rectangle_make(
				0, 0, size.width,
				psy_ui_value_px(&self->base.trackstate->lineheight, &tm)),
			tm, self->base.currtrack, self->base.trackindex,
			self->base.currtrack == edittrack);
		switch (sequencetrackbox_hittest(&trackbox, ev->x, 0)) {
		case SEQUENCETRACKBOXEVENT_MUTE:
			if (self->base.currtrack) {
				self->base.currtrack->mute = !self->base.currtrack->mute;
			}
			break;
		case SEQUENCETRACKBOXEVENT_SOLO:

			break;
		case SEQUENCETRACKBOXEVENT_DEL: {
			psy_audio_SequencePosition position;
			position = psy_audio_sequence_at(&self->base.workspace->song->sequence,
				self->base.trackindex, 0);
			psy_audio_exclusivelock_enter();
			self->base.currtrack = NULL;
			psy_audio_sequence_removetrack(&self->base.workspace->song->sequence,
				position.tracknode);
			psy_audio_exclusivelock_leave();
			return FALSE;
			break; }
									  // fallthrough
		case SEQUENCETRACKBOXEVENT_SELECT:
		default: {
			uintptr_t track;

			track = self->base.trackindex;
			if (track >= psy_audio_sequence_sizetracks(
					&self->base.workspace->song->sequence)) {
				if (psy_audio_sequence_sizetracks(&self->base.workspace->song->sequence) > 0) {
					track = psy_audio_sequence_sizetracks(&self->base.workspace->song->sequence) - 1;
				} else {
					track = 0;
				}
			}
			psy_audio_sequenceselection_seteditposition(&self->base.workspace->sequenceselection,
				psy_audio_sequence_at(&self->base.workspace->song->sequence, track, 0));
			workspace_setsequenceselection(self->base.workspace,
				self->base.workspace->sequenceselection);									
			break; }
		}
		psy_ui_component_invalidate(&self->base.parent->component);
	} else {
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_appendtrack(&self->base.workspace->song->sequence,
			psy_audio_sequencetrack_allocinit());
		psy_audio_exclusivelock_leave();		
		return FALSE;
	}
	return TRUE;
}

bool seqeditortrackheader_onmousemove(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	return TRUE;
}

bool seqeditortrackheader_onmouseup(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	return TRUE;
}

// SeqEditorTracks
// prototypes
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_ondraw(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_drawplayline(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_onpreferredsize(SeqEditorTracks*, psy_ui_Size*
	limit, psy_ui_Size* rv);
static void seqeditortracks_onmousedown(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmousemove(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmouseup(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static bool seqeditortracks_notifymouse(SeqEditorTracks*, psy_ui_MouseEvent*,
	bool (*fp_notify)(SeqEditorTrack*, psy_ui_MouseEvent*));
static void seqeditortracks_build(SeqEditorTracks*);
static psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks*);
static void seqeditortracks_onsequenceselectionchanged(SeqEditorTracks*, Workspace*);
static void seqeditortracks_ontimer(SeqEditorTracks*, uintptr_t timerid);
// vtable
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);
		seqeditortracks_vtable.ondraw = (psy_ui_fp_component_ondraw)seqeditortracks_ondraw;
		seqeditortracks_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			seqeditortracks_onpreferredsize;
		seqeditortracks_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			seqeditortracks_onmousemove;
		seqeditortracks_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			seqeditortracks_onmouseup;
		seqeditortracks_vtable.ontimer = (psy_ui_fp_component_ontimer)
			seqeditortracks_ontimer;
		seqeditortracks_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, int mode, Workspace* workspace)
{
	self->workspace = workspace;
	self->trackstate = trackstate;
	self->mode = mode;
	psy_ui_component_init(&self->component, parent);	
	seqeditortracks_vtable_init(self);
	self->component.vtable = &seqeditortracks_vtable;
	self->tracks = NULL;
	self->lastplaylinepx = -1;
	self->capture = NULL;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	seqeditortracks_build(self);
	psy_signal_connect(&workspace->signal_sequenceselectionchanged, self,
		seqeditortracks_onsequenceselectionchanged);
	if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
		psy_ui_component_starttimer(&self->component, 0, 50);
	}
}

void seqeditortracks_ondestroy(SeqEditorTracks* self, psy_ui_Component* sender)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;

	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
	sequence = seqeditortracks_sequence(self);
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		
		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			SeqEditorTrack* seqedittrack;

			if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
				seqedittrack = seqeditortrack_allocinit(self, self->trackstate,
					self->workspace);
			} else if (self->mode == SEQEDITOR_TRACKMODE_HEADER) {
				seqedittrack = seqeditortrackheader_base(
					seqeditortrackheader_allocinit(self, self->trackstate,
						self->workspace));
			} else {
				seqedittrack = NULL;
			}
			if (seqedittrack) {
				psy_list_append(&self->tracks, seqedittrack);
				seqeditortrack_updatetrack(seqedittrack,
					(psy_audio_SequenceTrack*)t->entry, c);
			}
		}
		// add an empty track for the add track button
		if (self->mode == SEQEDITOR_TRACKMODE_HEADER) {		
			SeqEditorTrack* seqedittrack;

			seqedittrack = seqeditortrackheader_base(
				seqeditortrackheader_allocinit(self, self->trackstate,
					self->workspace));
			if (seqedittrack) {
				psy_list_append(&self->tracks, seqedittrack);
				seqeditortrack_updatetrack(seqedittrack,
					NULL, c + 1);
			}
		}
	}
	self->capture = NULL;
}

void seqeditortracks_ondraw(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence = seqeditortracks_sequence(self);	
	if (sequence) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		intptr_t cpx, cpy;
		uintptr_t c;
		psy_ui_TextMetric tm;
		intptr_t linemargin;
		intptr_t lineheight;

		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
		cpx = 0;		
		cpy = 0;
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, &tm);		
		lineheight = psy_ui_value_px(&self->trackstate->lineheight, &tm);
		for (seqeditnode = self->tracks, seqnode = sequence->tracks, c = 0;
				seqeditnode != NULL;
				seqnode = (seqnode) ? seqnode->next : seqnode,
				psy_list_next(&seqeditnode),
				++c) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;

			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			if (seqnode) {
				seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			} else {
				seqtrack = NULL;
			}
			seqeditortrack_updatetrack(seqedittrack, seqtrack, c);
			seqeditortrack_ondraw(seqedittrack, g, cpx, cpy);
			cpy += lineheight + linemargin;
		}		
		if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
			seqeditortracks_drawplayline(self, g);
		}
	}
}

void seqeditortracks_drawplayline(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;
		intptr_t cpx;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);
		cpx = (intptr_t)(psy_audio_player_position(workspace_player(self->workspace)) *
			self->trackstate->pxperbeat);
		psy_ui_drawline(g, cpx, 0, cpx, size.height);
		self->lastplaylinepx = cpx;
	}
}

bool seqeditortracks_playlinechanged(SeqEditorTracks* self)
{
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	intptr_t cpx;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	cpx = (intptr_t)(psy_audio_player_position(workspace_player(self->workspace)) *
		self->trackstate->pxperbeat);
	return cpx != self->lastplaylinepx;
}

psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->sequence;
	}
	return NULL;
}

void seqeditortracks_onpreferredsize(SeqEditorTracks* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (workspace_song(self->workspace)) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		intptr_t cpxmax, cpymax;
		psy_ui_TextMetric tm;
		uintptr_t c;
		intptr_t linemargin;

		cpxmax = 0;
		cpymax = 0;
		tm = psy_ui_component_textmetric(&self->component);
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, &tm);
		for (seqeditnode = self->tracks, c = 0,
				seqnode = workspace_song(self->workspace)->sequence.tracks;
				seqnode != NULL && seqeditnode != NULL;
				psy_list_next(&seqnode), psy_list_next(&seqeditnode), ++c) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;
			psy_ui_Size limit;
			psy_ui_Size preferredtracksize;

			limit = psy_ui_component_size(&self->component);

			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			seqeditortrack_updatetrack(seqedittrack, seqtrack, c);
			seqeditortrack_onpreferredsize(seqedittrack, &limit, &preferredtracksize);
			cpxmax = psy_max(cpxmax, psy_ui_value_px(&preferredtracksize.width, &tm));
			cpymax += psy_ui_value_px(&preferredtracksize.height, &tm) +
				linemargin;
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
	if (!seqeditortracks_notifymouse(self, ev, seqeditortrack_onmousedown)) {
		seqeditortracks_build(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void seqeditortracks_onmousemove(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	seqeditortracks_notifymouse(self, ev, seqeditortrack_onmousemove);
}

void seqeditortracks_onmouseup(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	seqeditortracks_notifymouse(self, ev, seqeditortrack_onmouseup);
}

bool seqeditortracks_notifymouse(SeqEditorTracks* self, psy_ui_MouseEvent* ev,
	bool (*fp_notify)(SeqEditorTrack*, psy_ui_MouseEvent*))
{
	bool rv;
	
	rv = TRUE;
	psy_audio_Sequence* sequence = seqeditortracks_sequence(self);
	if (sequence) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		intptr_t cpy;
		intptr_t c;
		psy_ui_TextMetric tm;
		intptr_t linemargin;
		intptr_t lineheight;

		cpy = 0;
		tm = psy_ui_component_textmetric(&self->component);
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, &tm);		
		lineheight = psy_ui_value_px(&self->trackstate->lineheight, &tm);
		if (self->capture) {
			psy_ui_MouseEvent trackev;

			trackev = *ev;
			trackev.y = ev->y - (self->capture->trackindex *
				(lineheight + linemargin));
			fp_notify(self->capture, &trackev);			
		} else {
			for (seqeditnode = self->tracks, seqnode = sequence->tracks, c = 0;
					seqeditnode != NULL;
					seqnode = (seqnode) ? seqnode->next : NULL,
					psy_list_next(&seqeditnode),
					++c) {
				SeqEditorTrack* seqedittrack;
				psy_audio_SequenceTrack* seqtrack;

				seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
				if (seqnode) {
					seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
				} else {
					seqtrack = NULL;
				}
				seqeditortrack_updatetrack(seqedittrack, seqtrack, c);
				cpy += lineheight + linemargin;
				if (ev->y >= c * (lineheight + linemargin) &&
						ev->y < (c + 1) * (lineheight + linemargin)) {
					psy_ui_MouseEvent trackev;

					trackev = *ev;
					trackev.y = ev->y - (cpy - (lineheight + linemargin));
					rv = fp_notify(seqedittrack, &trackev);
					break;
				}
			}
		}
	}
	return rv;
}

void seqeditortracks_onsequenceselectionchanged(SeqEditorTracks* self,
	Workspace* sender)
{			
	psy_ui_component_invalidate(&self->component);
}

void seqeditortracks_ontimer(SeqEditorTracks* self, uintptr_t timerid)
{		
	if (psy_audio_player_playing(workspace_player(self->workspace)) &&
			seqeditortracks_playlinechanged(self)) {
		psy_ui_Rectangle r;
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);
		if (self->lastplaylinepx != -1) {
			r = psy_ui_rectangle_make(self->lastplaylinepx,
				psy_ui_component_scrolltop(&self->component), 2, size.height);
			psy_ui_component_invalidaterect(&self->component, r);
		}
		r = psy_ui_rectangle_make(
			(intptr_t)(psy_audio_player_position(workspace_player(self->workspace)) *
				self->trackstate->pxperbeat),
			psy_ui_component_scrolltop(&self->component),
			2, size.height);
		psy_ui_component_invalidaterect(&self->component, r);
	} else {
		if (self->lastplaylinepx != -1) {
			psy_ui_Rectangle r;
			psy_ui_TextMetric tm;
			psy_ui_IntSize size;

			tm = psy_ui_component_textmetric(&self->component);
			size = psy_ui_intsize_init_size(
				psy_ui_component_size(&self->component), &tm);
			r = psy_ui_rectangle_make(self->lastplaylinepx,
					psy_ui_component_scrolltop(&self->component), 2,
					size.height);
			psy_ui_component_invalidaterect(&self->component, r);
		}
		self->lastplaylinepx = -1;
	}	
}

// SeqEditor
// prototypes
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_SongFile*);
static void seqeditor_updatesong(SeqEditor*, psy_audio_Song*);
static void seqeditor_onsequencechanged(SeqEditor*, psy_audio_Sequence*
	sender);
static void seqeditor_onscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onzoomboxbeatchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_onzoomboxheightchanged(SeqEditor*, ZoomBox* sender);
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
	psy_ui_Margin topmargin;
	
	psy_ui_component_init(&self->component, parent);
	seqeditor_vtable_init(self);
	self->component.vtable = &seqeditor_vtable;
	psy_ui_component_doublebuffer(&self->component);
	seqeditortrackstate_init(&self->trackstate);
	self->workspace = workspace;
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_beat, &self->left);
	psy_ui_component_setalign(&self->zoombox_beat.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init_all(&topmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->zoombox_beat.component, &topmargin);
	psy_signal_connect(&self->zoombox_beat.signal_changed, self,
		seqeditor_onzoomboxbeatchanged);
	seqeditortracks_init(&self->trackheaders, &self->left,
		&self->trackstate, SEQEDITOR_TRACKMODE_HEADER, workspace);
	psy_ui_component_setalign(&self->trackheaders.component,
		psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_height, &self->left);
	psy_ui_component_setalign(&self->zoombox_height.component,
		psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->zoombox_height.signal_changed, self,
		seqeditor_onzoomboxheightchanged);
	seqeditorruler_init(&self->ruler, &self->component, &self->trackstate,
		workspace);
	psy_ui_component_setmargin(&self->ruler.component, &topmargin);
	psy_ui_component_setalign(seqeditorruler_base(&self->ruler),
		psy_ui_ALIGN_TOP);
	seqeditortracks_init(&self->tracks, &self->component,
		&self->trackstate, SEQEDITOR_TRACKMODE_ENTRY, workspace);
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_resize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20.0),
			psy_ui_value_makeeh(6.0)));
	psy_ui_component_preventpreferredsize(&self->component);
	seqeditor_updatesong(self, workspace->song);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_onscroll);
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	seqeditor_updatesong(self, workspace->song);
}

void seqeditor_updatesong(SeqEditor* self, psy_audio_Song* song)
{
	if (song) {
		seqeditortracks_build(&self->tracks);
		seqeditortracks_build(&self->trackheaders);
		psy_signal_connect(&song->sequence.sequencechanged, self,
			seqeditor_onsequencechanged);
		psy_ui_component_updateoverflow(&self->tracks.component);
		psy_ui_component_invalidate(&self->tracks.component);
		psy_ui_component_invalidate(&self->trackheaders.component);
	}
}

void seqeditor_onsequencechanged(SeqEditor* self, psy_audio_Sequence* sender)
{
	seqeditortracks_build(&self->tracks);
	seqeditortracks_build(&self->trackheaders);
	psy_ui_component_updateoverflow(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
	psy_ui_component_invalidate(&self->trackheaders.component);
}

void seqeditor_onscroll(SeqEditor* self, psy_ui_Component* sender)
{
	psy_ui_component_setscrollleft(&self->ruler.component,
		psy_ui_component_scrollleft(&self->tracks.component));
	psy_ui_component_setscrolltop(&self->trackheaders.component,
		psy_ui_component_scrolltop(&self->tracks.component));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onzoomboxbeatchanged(SeqEditor* self, ZoomBox* sender)
{
	self->trackstate.pxperbeat = (intptr_t)(sender->zoomrate * DEFAULT_PXPERBEAT);
	psy_ui_component_updateoverflow(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
	psy_ui_component_invalidate(&self->ruler.component);
}

void seqeditor_onzoomboxheightchanged(SeqEditor* self, ZoomBox* sender)
{
	self->trackstate.lineheight = psy_ui_mul_value_real(
		self->trackstate.defaultlineheight, zoombox_rate(sender));
	psy_ui_component_updateoverflow(&self->tracks.component);
	psy_ui_component_invalidate(&self->tracks.component);
	psy_ui_component_invalidate(&self->trackheaders.component);
}
