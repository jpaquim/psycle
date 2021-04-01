// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "seqeditor.h"
// host
#include "cmdsgeneral.h"
#include "sequencetrackbox.h"
#include "styles.h"
#include "pianoroll.h"
// audio
#include <exclusivelock.h>
#include <sequencecmds.h>
#include <songio.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define DEFAULT_PXPERBEAT 5.0

void seqeditorstate_init(SeqEditorState* self)
{
	psy_signal_init(&self->signal_cursorchanged);
	self->pxperbeat = DEFAULT_PXPERBEAT;
	self->defaultlineheight = psy_ui_value_makeeh(2.0);	
	self->lineheight = self->defaultlineheight;
	self->linemargin = psy_ui_value_makeeh(0.2);	
	self->cursorposition = (psy_dsp_big_beat_t)0.0;
	self->drawcursor = TRUE;
	self->drawpatternevents = TRUE;
	self->workspace = NULL;	
	self->dragstatus = FALSE;
	self->dragpt = psy_ui_realpoint_zero();
	self->dragseqpos = psy_audio_orderindex_zero();	
	self->dragmode = SEQEDITORDRAG_MOVE;
	self->updatecursorposition = TRUE;
	self->sequenceentry = NULL;
	self->cmd = SEQEDTCMD_NONE;
	self->cmdtrack = psy_INDEX_INVALID;
	self->cmdrow = psy_INDEX_INVALID;
	self->workspace = NULL;	
}

void seqeditorstate_dispose(SeqEditorState* self)
{
	psy_signal_dispose(&self->signal_cursorchanged);
}

psy_audio_Sequence* seqeditorstate_sequence(SeqEditorState* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->sequence;
	}
	return NULL;
}

// SeqEditorRuler
static void seqeditorruler_ondestroy(SeqEditorRuler*);
static void seqeditorruler_ondraw(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_drawruler(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler*,
	psy_audio_SequenceSelection* sender);
static void seqeditorruler_onpreferredsize(SeqEditorRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static psy_dsp_big_beat_t seqeditorruler_step(SeqEditorRuler*);
static void seqeditorruler_oncursorchanged(SeqEditorRuler*, SeqEditorState*);
// vtable
static psy_ui_ComponentVtable seqeditorruler_vtable;
static bool seqeditorruler_vtable_initialized = FALSE;

static void seqeditorruler_vtable_init(SeqEditorRuler* self)
{
	if (!seqeditorruler_vtable_initialized) {
		seqeditorruler_vtable = *(self->component.vtable);
		seqeditorruler_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditorruler_ondestroy;
		seqeditorruler_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditorruler_ondraw;
		seqeditorruler_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditorruler_onpreferredsize;
		seqeditorruler_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditorruler_init(SeqEditorRuler* self, psy_ui_Component* parent,
	SeqEditorState* state, PatternViewSkin* skin,
	Workspace* workspace)
{
	assert(self);
	assert(state);
	assert(workspace);
	assert(skin);

	psy_ui_component_init(&self->component, parent, NULL);
	seqeditorruler_vtable_init(self);
	self->component.vtable = &seqeditorruler_vtable;
	psy_ui_component_doublebuffer(&self->component);	
	self->state = state;
	self->skin = skin;
	self->workspace = workspace;	
	psy_signal_connect(&workspace->sequenceselection.signal_changed, self,
		seqeditorruler_onsequenceselectionchanged);	
	psy_signal_connect(&self->state->signal_cursorchanged, self,
		seqeditorruler_oncursorchanged);
}

void seqeditorruler_ondestroy(SeqEditorRuler* self)
{
	psy_signal_disconnect(&self->workspace->sequenceselection.signal_changed, self,
		seqeditorruler_onsequenceselectionchanged);
}

void seqeditorruler_ondraw(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	seqeditorruler_drawruler(self, g);
}

void seqeditorruler_drawruler(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	double baseline;
	double textline;
	double linewidth;
	psy_dsp_big_beat_t duration;
	psy_dsp_big_beat_t clipstart;
	psy_dsp_big_beat_t clipend;
	psy_dsp_big_beat_t currbeat;
	psy_dsp_big_beat_t step;	
	
	size = psy_ui_component_sizepx(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = size.height / 2 + 2;
	textline = 0;
	duration = (size.width + psy_ui_component_scrollleftpx(&self->component)) /
		(psy_dsp_big_beat_t)self->state->pxperbeat;
	//psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence);
	linewidth = duration * self->state->pxperbeat;
	psy_ui_setcolour(g, self->skin->row);
	psy_ui_drawline(g, psy_ui_realpoint_make(0, baseline),
		psy_ui_realpoint_make(linewidth, baseline));
	clipstart = 0;
	clipend = duration;
	step = seqeditorruler_step(self);
	for (currbeat = clipstart; currbeat <= clipend; currbeat += step) {
		double cpx;
		char txt[40];

		cpx = floor(seqeditorstate_beattopx(self->state, currbeat));
		if (((intptr_t)(currbeat / step) % 8) == 0) {
			psy_ui_settextcolour(g, self->skin->row4beat);
			psy_ui_setcolour(g, self->skin->row4beat);
		} else if (((intptr_t)(currbeat /step) % 4) == 0) {
			psy_ui_settextcolour(g, self->skin->rowbeat);
			psy_ui_setcolour(g, self->skin->rowbeat);
		} else {
			psy_ui_settextcolour(g, self->skin->row);
			psy_ui_setcolour(g, self->skin->row);
		}
		psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
			psy_ui_realpoint_make(cpx, baseline + tm->tmHeight / 2));
		if (self->state->drawcursor && self->state->cursoractive) {
			psy_dsp_big_beat_t cursor;

			cursor = floor(self->state->cursorposition / step) * step;
			if (currbeat >= cursor && currbeat <= cursor + step) {
				double start;
				double end;

				if (cursor == currbeat &&
						self->state->cursorposition - cursor < step / 2) {
					psy_ui_settextcolour(g, self->skin->row4beat);
				} else if (cursor + step == currbeat &&
					self->state->cursorposition - cursor > step / 2) {
					psy_ui_settextcolour(g, self->skin->row4beat);
				}
				psy_ui_setcolour(g, self->skin->row4beat);
				start = cursor * self->state->pxperbeat;
				end = (cursor + step) * self->state->pxperbeat;
				psy_ui_drawline(g, psy_ui_realpoint_make(start, baseline),
					psy_ui_realpoint_make(end, baseline));
			}
		}
		psy_snprintf(txt, 40, "%d", (int)(currbeat));
		psy_ui_textout(g, cpx, textline, txt, strlen(txt));
	}
	if (self->state->drawcursor && self->state->cursoractive) {
		double cpx;
		
		psy_ui_setcolour(g, self->skin->cursor);
		cpx = self->state->cursorposition * self->state->pxperbeat;
		psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
			psy_ui_realpoint_make(cpx, baseline + size.height / 2));
	}
}

psy_dsp_big_beat_t seqeditorruler_step(SeqEditorRuler* self)
{
	psy_dsp_big_beat_t rv;
	
	rv = (psy_dsp_big_beat_t)(80.0 / self->state->pxperbeat);
	if (rv > 16.0) {
		rv = (psy_dsp_big_beat_t)(floor((rv / 16) * 16.0));
	} else if (rv > 4.0) {
		rv = (psy_dsp_big_beat_t)(floor((rv / 4.0) * 4.0));
	} else if (rv > 2.0) {
		rv = (psy_dsp_big_beat_t)(floor((rv / 2.0) * 2.0));
	} else {
		rv = (psy_dsp_big_beat_t)1.0;
	}
	return rv;
}

void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler* self,
	psy_audio_SequenceSelection* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditorruler_oncursorchanged(SeqEditorRuler* self,
	SeqEditorState* state)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditorruler_onpreferredsize(SeqEditorRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (workspace_song(self->workspace)) {			
		double linewidth;
		psy_dsp_big_beat_t duration;
		
		duration = psy_audio_sequence_duration(
			&workspace_song(self->workspace)->sequence);
		linewidth = duration * self->state->pxperbeat;
		rv->width = psy_ui_value_makepx(linewidth);
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
	rv->height = psy_ui_value_makeeh(2.0);
}

// SeqEditorLine
static void seqeditorline_onpreferredsize(SeqEditorLine*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable seqeditorline_vtable;
static bool seqeditorline_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditorline_vtable_init(SeqEditorLine* self)
{
	if (!seqeditorline_vtable_initialized) {
		seqeditorline_vtable = *(self->component.vtable);
		seqeditorline_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditorline_onpreferredsize;
	}
	return &seqeditorline_vtable;
}

void seqeditorline_init(SeqEditorLine* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state)
{
	psy_ui_component_init(seqeditorline_base(self), parent, view);
	psy_ui_component_setvtable(seqeditorline_base(self),
		seqeditorline_vtable_init(self));	
	self->state = state;
	psy_ui_component_setposition(
		seqeditorline_base(self),
		psy_ui_rectangle_make(
			psy_ui_point_makepx(-1.0, 0.0),
			psy_ui_size_makepx(1.0, 100.0)));
}

SeqEditorLine* seqeditorline_alloc(void)
{
	return (SeqEditorLine*)malloc(sizeof(SeqEditorLine));
}

SeqEditorLine* seqeditorline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state)
{
	SeqEditorLine* rv;

	rv = seqeditorline_alloc();
	if (rv) {
		seqeditorline_init(rv, parent, view, state);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void seqeditorline_onpreferredsize(SeqEditorLine* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(1.0);
	rv->height = psy_ui_value_makeeh(10.0);
}

void seqeditorline_updateposition(SeqEditorLine* self,
	psy_dsp_big_beat_t position)
{
	double position_px;
	psy_ui_RealRectangle newposition;
	psy_ui_RealRectangle updateposition;
	psy_ui_RealSize parentsize;

	position_px = floor(seqeditorstate_beattopx(self->state, position));
	updateposition = psy_ui_component_position(&self->component);
	parentsize = psy_ui_component_sizepx(
		psy_ui_component_parent(&self->component));
	if (parentsize.height != psy_ui_realrectangle_height(&updateposition) ||
			position_px != updateposition.left) {		
		newposition = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(position_px, 0.0),
			psy_ui_realsize_make(1.0, parentsize.height));		
		psy_ui_component_setposition(&self->component,
			psy_ui_rectangle_make_px(&newposition));
		psy_ui_realrectangle_union(&updateposition, &newposition);
		psy_ui_component_invalidaterect(
			psy_ui_component_parent(&self->component),
			updateposition);
	}
}

// SeqEditorPlayLine
static void seqeditorplayline_onmousedown(SeqEditorPlayline*, psy_ui_MouseEvent*);
static void seqeditorplayline_onmousemove(SeqEditorPlayline*, psy_ui_MouseEvent*);
static void seqeditorplayline_onmouseup(SeqEditorPlayline*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable seqeditorplayline_vtable;
static bool seqeditorplayline_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditorplayline_vtable_init(SeqEditorPlayline* self)
{
	if (!seqeditorplayline_vtable_initialized) {
		seqeditorplayline_vtable =
			*(seqeditorline_base(&self->seqeditorline)->vtable);		
		seqeditorplayline_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditorplayline_onmousedown;
		seqeditorplayline_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditorplayline_onmousemove;
		seqeditorplayline_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditorplayline_onmouseup;
	}
	return &seqeditorplayline_vtable;
}

void seqeditorplayline_init(SeqEditorPlayline* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state)
{
	seqeditorline_init(&self->seqeditorline, parent, view, state);
	psy_ui_component_setvtable(seqeditorline_base(&self->seqeditorline),
		seqeditorplayline_vtable_init(self));
	psy_ui_component_setbackgroundcolour(
		seqeditorline_base(&self->seqeditorline),
		psy_ui_colour_make(0x00FF00000));	
	self->drag = FALSE;
	self->dragbase = 0.0;
}

SeqEditorPlayline* seqeditorplayline_alloc(void)
{
	return (SeqEditorPlayline*)malloc(sizeof(SeqEditorPlayline));
}

SeqEditorPlayline* seqeditorplayline_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state)
{
	SeqEditorPlayline* rv;

	rv = seqeditorplayline_alloc();
	if (rv) {
		seqeditorplayline_init(rv, parent, view, state);
		psy_ui_component_deallocateafterdestroyed(
			seqeditorline_base(&rv->seqeditorline));		
	}
	return rv;
}

void seqeditorplayline_onmousedown(SeqEditorPlayline* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_RealRectangle position;

	position = psy_ui_component_position(
		seqeditorline_base(&self->seqeditorline));
	self->drag = TRUE;	
	self->dragbase = position.left;
	psy_ui_component_capture(seqeditorline_base(&self->seqeditorline));
	psy_ui_component_setcursor(seqeditorline_base(&self->seqeditorline),
		psy_ui_CURSORSTYLE_COL_RESIZE);
	psy_ui_mouseevent_stoppropagation(ev);
}

void seqeditorplayline_onmousemove(SeqEditorPlayline* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_setcursor(seqeditorline_base(&self->seqeditorline),
		psy_ui_CURSORSTYLE_COL_RESIZE);
	if (self->drag) {
		psy_dsp_big_beat_t newplayposition;		
				
		newplayposition = seqeditorstate_pxtobeat(self->seqeditorline.state,
			self->dragbase + ev->pt.x);
		newplayposition = psy_max(0.0, newplayposition);
		psy_audio_exclusivelock_enter();
		psy_audio_player_setposition(&self->seqeditorline.state->workspace->player,
			newplayposition);
		psy_audio_exclusivelock_leave();
		seqeditorplayline_update(self);
		self->dragbase = psy_ui_component_position(
			seqeditorline_base(&self->seqeditorline)).left;		
	}
	psy_ui_mouseevent_stoppropagation(ev);	
}

void seqeditorplayline_onmouseup(SeqEditorPlayline* self, psy_ui_MouseEvent* ev)
{
	self->drag = FALSE;
	self->dragbase = 0.0;
	psy_ui_component_releasecapture(seqeditorline_base(&self->seqeditorline));
	psy_ui_mouseevent_stoppropagation(ev);
}

void seqeditorplayline_update(SeqEditorPlayline* self)
{		
	seqeditorline_updateposition(&self->seqeditorline,		
		psy_audio_player_position(workspace_player(
			self->seqeditorline.state->workspace)));	
}

// SeqEditorPatternEnrty
static void seqeditorpatternentry_ondraw(SeqEditorPatternEntry*,
	psy_ui_Graphics*);
static void seqeditorpatternentry_onpreferredsize(SeqEditorPatternEntry*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditorpatternentry_onmousedown(SeqEditorPatternEntry*,
	psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable seqeditorpatternentry_vtable;
static bool seqeditorpatternentry_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditorpatternentry_vtable_init(SeqEditorPatternEntry* self)
{
	if (!seqeditorpatternentry_vtable_initialized) {
		seqeditorpatternentry_vtable = *(self->component.vtable);
		seqeditorpatternentry_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditorpatternentry_ondraw;
		seqeditorpatternentry_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditorpatternentry_onpreferredsize;
		seqeditorpatternentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditorpatternentry_onmousedown;
	}
	return &seqeditorpatternentry_vtable;
}

void seqeditorpatternentry_init(SeqEditorPatternEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setvtable(&self->component,
		seqeditorpatternentry_vtable_init(self));
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQEDT_ITEM, STYLE_SEQEDT_ITEM, STYLE_SEQEDT_ITEM_SELECTED);
	self->state = state;
	self->sequenceentry = entry;
	self->seqpos = seqpos;
}

SeqEditorPatternEntry* seqeditorpatternentry_alloc(void)
{
	return (SeqEditorPatternEntry*)malloc(sizeof(SeqEditorTrack));
}

SeqEditorPatternEntry* seqeditorpatternentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	SeqEditorPatternEntry* rv;

	rv = seqeditorpatternentry_alloc();
	if (rv) {
		seqeditorpatternentry_init(rv, parent, view, entry, seqpos, state);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void seqeditorpatternentry_ondraw(SeqEditorPatternEntry* self,
	psy_ui_Graphics* g)
{
	if (self->sequenceentry) {		
		psy_audio_Pattern* pattern;

		pattern = psy_audio_sequenceentry_pattern(self->sequenceentry,
			&self->state->workspace->song->patterns);
		if (pattern) {
			psy_ui_RealSize size;
			const psy_ui_TextMetric* tm;
			psy_ui_RealRectangle clip;

			tm = psy_ui_component_textmetric(&self->component);
			size = psy_ui_component_sizepx(&self->component);
			size.width -= 4;
			clip = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
			psy_ui_textoutrectangle(g, psy_ui_realpoint_make(4, 2),
				psy_ui_ETO_CLIPPED, clip,
				psy_audio_pattern_name(pattern),
				strlen(psy_audio_pattern_name(pattern)));
		}
	}
}

void seqeditorpatternentry_onpreferredsize(SeqEditorPatternEntry* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t duration;

	duration = 0.0;
	if (self->sequenceentry) {
		psy_audio_Pattern* pattern;
		pattern = psy_audio_sequenceentry_pattern(self->sequenceentry,
			&self->state->workspace->song->patterns);
		if (pattern) {
			duration = psy_audio_pattern_length(pattern);
		}
	}
	rv->width = psy_ui_value_makepx(self->state->pxperbeat *
		duration);
	rv->height = self->state->lineheight;
}

void seqeditorpatternentry_onmousedown(SeqEditorPatternEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (self->sequenceentry) {		
		psy_audio_PatternCursor cursor;
		psy_dsp_big_beat_t position;
		psy_audio_OrderIndex seqeditpos;

		seqeditpos = workspace_sequenceeditposition(self->state->workspace);
		if (self->seqpos.track != seqeditpos.track ||
				self->seqpos.order != seqeditpos.order) {
			workspace_setsequenceeditposition(self->state->workspace,
				self->seqpos);
		}
		cursor = self->state->workspace->patterneditposition;
		position = seqeditorstate_pxtobeat(self->state, ev->pt.x);
		// quantize
		position =
			(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->state->workspace))) /
			(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->state->workspace));
		cursor.offset = position;
		workspace_setpatterncursor(self->state->workspace, cursor);
		cursor = self->state->workspace->patterneditposition;
		workspace_gotocursor(self->state->workspace, cursor);
		//self->itemdragposition = self->sequenceentry->offset;
		self->state->dragstatus = TRUE;
		self->state->dragseqpos = self->seqpos;
		self->state->sequenceentry = self->sequenceentry;
	}
}

// SeqEditorTrack
// prototypes
static void seqeditortrack_ondestroy(SeqEditorTrack*);
static void seqeditortrack_onpreferredsize(SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditortrack_onalign(SeqEditorTrack*);
static void seqeditortrack_onmousedown(SeqEditorTrack*, psy_ui_MouseEvent*);
static void seqeditortrack_onmousemove(SeqEditorTrack*, psy_ui_MouseEvent*);
static void seqeditortrack_onmouseup(SeqEditorTrack*, psy_ui_MouseEvent*);
static void seqeditortrack_onmousedoubleclick(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static void seqeditortrack_build(SeqEditorTrack* self);
static void seqeditortrack_onsequenceinsert(SeqEditorTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index);
static void seqeditortrack_onsequencetrackreposition(SeqEditorTrack*,
	psy_audio_Sequence* sender, uintptr_t trackidx);

// vtable
static psy_ui_ComponentVtable seqeditortrack_vtable;
static bool seqeditortrack_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditortrack_vtable_init(SeqEditorTrack* self)
{
	if (!seqeditortrack_vtable_initialized) {
		seqeditortrack_vtable = *(self->component.vtable);
		seqeditortrack_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditortrack_ondestroy;
		seqeditortrack_vtable.onalign =
			(psy_ui_fp_component_onalign)
			seqeditortrack_onalign;
		seqeditortrack_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditortrack_onpreferredsize;
		seqeditortrack_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditortrack_onmousedown;
		seqeditortrack_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditortrack_onmousemove;
		seqeditortrack_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditortrack_onmouseup;
		seqeditortrack_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditortrack_onmousedoubleclick;
		seqeditortrack_vtable_initialized = TRUE;
	}
	return &seqeditortrack_vtable;
}

// implementation
void seqeditortrack_init(SeqEditorTrack* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state, PatternViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	seqeditortrack_vtable_init(self);		
	self->component.vtable = &seqeditortrack_vtable;
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	self->state = state;
	self->workspace = workspace;	
	self->currtrack = NULL;
	self->trackindex = 0;
	self->drag_sequenceitem_node = NULL;
	self->dragstarting = FALSE;	
	self->skin = skin;
	self->view = view;
	self->dragline = NULL;
	if (seqeditorstate_sequence(self->state)) {
		psy_signal_connect(&seqeditorstate_sequence(self->state)->signal_insert, self,
			seqeditortrack_onsequenceinsert);
		psy_signal_connect(&seqeditorstate_sequence(self->state)->signal_trackreposition,
			self, seqeditortrack_onsequencetrackreposition);		
	}
}

void seqeditortrack_ondestroy(SeqEditorTrack* self)
{
	if (seqeditorstate_sequence(self->state)) {
		psy_signal_disconnect(&seqeditorstate_sequence(self->state)->signal_insert, self,
			seqeditortrack_onsequenceinsert);
		psy_signal_disconnect(&seqeditorstate_sequence(self->state)->signal_trackreposition,
			self, seqeditortrack_onsequencetrackreposition);		
	}
}

SeqEditorTrack* seqeditortrack_alloc(void)
{
	return (SeqEditorTrack*)malloc(sizeof(SeqEditorTrack));
}

SeqEditorTrack* seqeditortrack_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state, PatternViewSkin* skin,	
	Workspace* workspace)
{
	SeqEditorTrack* rv;

	rv = seqeditortrack_alloc();
	if (rv) {
		seqeditortrack_init(rv, parent, view, state, skin, workspace);
		psy_ui_component_deallocateafterdestroyed(seqeditortrack_base(rv));
	}
	return rv;
}

void seqeditortrack_updatetrack(SeqEditorTrack* self,
	psy_audio_SequenceTrackNode* tracknode,
	psy_audio_SequenceTrack* track,
	uintptr_t trackindex)
{
	self->currtrack = track;
	self->currtracknode = tracknode;
	self->trackindex = trackindex;	
	seqeditortrack_build(self);		
}

void seqeditortrack_build(SeqEditorTrack* self)
{
	psy_List* p;
	uintptr_t c;	

	psy_ui_component_clear(&self->component);	
	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	for (p = self->currtrack->entries, c = 0; p != NULL;
			psy_list_next(&p), ++c) {
		psy_audio_SequenceEntry* sequenceentry;
		psy_audio_Pattern* pattern;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
			psy_audio_sequenceentry_patternslot(sequenceentry));
		if (pattern) {
			SeqEditorPatternEntry* seqeditpatternentry;

			seqeditpatternentry = seqeditorpatternentry_allocinit(
				&self->component, self->view, sequenceentry,
				psy_audio_orderindex_make(self->trackindex, c),
				self->state);
			psy_ui_component_setalign(&seqeditpatternentry->component, psy_ui_ALIGN_LEFT);
		}
	}
	psy_ui_component_align(&self->component);
}

void seqeditortrack_onalign(SeqEditorTrack* self)
{		
	psy_List* p;
	psy_List* r;
	psy_List* q;
	uintptr_t c;
	psy_ui_RealSize size;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	size = psy_ui_component_sizepx(&self->component);
	q = r = psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE);
	for (p = self->currtrack->entries, c = 0; p != NULL && r != NULL;
		psy_list_next(&p), psy_list_next(&r), ++c) {
		psy_audio_SequenceEntry* sequenceentry;
		psy_audio_Pattern* pattern;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
		pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
			psy_audio_sequenceentry_patternslot(sequenceentry));
		if (pattern) {
			psy_ui_Component* component;

			component = (psy_ui_Component*)psy_list_entry(r);
			psy_ui_component_setposition(component,
				psy_ui_rectangle_make(
					psy_ui_point_makepx(
						seqeditorstate_beattopx(self->state,
							sequenceentry->offset),
						0.0),
					psy_ui_size_make(
						psy_ui_value_makepx(
							seqeditorstate_beattopx(self->state,
								psy_audio_pattern_length(pattern))),
						psy_ui_value_makepx(size.height))));
		}
	}	
}

void seqeditortrack_onpreferredsize(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&workspace_song(self->workspace)->patterns);
	}
	trackduration += 400.0;
	rv->width = psy_ui_value_makepx(self->state->pxperbeat *
		trackduration);
	rv->height = self->state->lineheight;
}

void seqeditortrack_onmousedown(SeqEditorTrack* self, psy_ui_MouseEvent* ev)
{
	if (self->state->dragstatus && self->state->sequenceentry) {
		psy_ui_component_capture(&self->component);
		self->state->dragpt = ev->pt;		
		if (self->state->dragmode == SEQEDITORDRAG_REORDER) {
			self->dragline = seqeditorline_allocinit(&self->component, self->view, self->state);
			psy_ui_component_setbackgroundcolour(&self->dragline->component,
				self->skin->font);
			seqeditorline_updateposition(self->dragline,
				self->state->sequenceentry->offset);
			self->state->cmd = SEQEDTCMD_REORDER;
			self->state->cmdtrack = self->trackindex;
			self->state->cmdrow = self->state->sequenceentry->row;
		}
	}
}

void seqeditortrack_onmousemove(SeqEditorTrack* self, psy_ui_MouseEvent* ev)
{
	if (self->state->dragstatus && self->state->sequenceentry) {
		if (self->state->dragmode == SEQEDITORDRAG_MOVE) {
			psy_dsp_big_beat_t dragposition;

			dragposition = seqeditorstate_pxtobeat(self->state, ev->pt.x);
			if (dragposition - (self->state->sequenceentry->offset -
				self->state->sequenceentry->repositionoffset) >= 0) {
				dragposition =				
					(intptr_t)(dragposition * psy_audio_player_lpb(workspace_player(self->workspace))) /
					(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));
			} else {
				dragposition = self->state->sequenceentry->offset -
					self->state->sequenceentry->repositionoffset;
			}
			self->state->sequenceentry->repositionoffset = dragposition -
				(self->state->sequenceentry->offset -
					self->state->sequenceentry->repositionoffset);
			self->state->sequenceentry->offset = dragposition;			
			psy_audio_sequence_reposition_track(
				&workspace_song(self->workspace)->sequence, self->currtrack);			
		} else if (self->dragline && self->state->dragmode == SEQEDITORDRAG_REORDER) {
			psy_dsp_big_beat_t dragposition;

			dragposition = seqeditorstate_pxtobeat(self->state, ev->pt.x);			
			if (dragposition < 0.0) {
				dragposition = 0.0;
			}			
			seqeditorline_updateposition(self->dragline, dragposition);
		}
	}
}

void seqeditortrack_onmouseup(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	self->state->sequenceentry = NULL;
	self->state->dragstatus = FALSE;
	if (self->dragline) {
		psy_ui_component_remove(&self->component,
			&self->dragline->component);
		self->dragline = NULL;
	}
	psy_ui_component_invalidate(&self->component);
}

void seqeditortrack_onmousedoubleclick(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	if (psy_audio_orderindex_invalid(&self->state->dragseqpos)) {
		self->state->cmd = SEQEDTCMD_INSERTPATTERN;
		self->state->cmdtrack = self->trackindex;
	}
}

void seqeditortrack_onsequenceinsert(SeqEditorTrack* self, psy_audio_Sequence* sender,
	psy_audio_OrderIndex* index)
{
	if (self->trackindex == index->track) {
		seqeditortrack_build(self);
	}
}

void seqeditortrack_onsequencetrackreposition(SeqEditorTrack* self,
	psy_audio_Sequence* sender, uintptr_t trackidx)
{
	seqeditortrack_onalign(self);
	psy_ui_component_invalidate(&self->component);
}

// SeqEditorTrackDesc
// prototypes
static void seqeditortrackdesc_onsequenceselectionselect(SeqEditorTrackDesc*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortrackdesc_onsequenceselectiondeselect(SeqEditorTrackDesc*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortrackdesc_onsequenceselectionupdate(SeqEditorTrackDesc*,
	psy_audio_SequenceSelection*);
static void seqeditortrackdesc_onnewtrack(SeqEditorTrackDesc*,
	psy_ui_Button* sender);
static void seqeditortrackdesc_ondeltrack(SeqEditorTrackDesc*,
	TrackBox* sender);
static void seqeditortrackdesc_build(SeqEditorTrackDesc*);
// implementation
void seqeditortrackdesc_init(SeqEditorTrackDesc* self, psy_ui_Component* parent,
	SeqEditorState* state, Workspace* workspace)
{
	self->workspace = workspace;
	self->state = state;	
	psy_ui_component_init(&self->component, parent, NULL);	
	// psy_ui_component_doublebuffer(&self->component);
	// psy_ui_component_setwheelscroll(&self->component, 1);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);
	psy_ui_component_setscrollmode(&self->component, psy_ui_SCROLL_COMPONENTS);	
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
	seqeditortrackdesc_build(self);	
	psy_signal_connect(&workspace->sequenceselection.signal_select, self,
		seqeditortrackdesc_onsequenceselectionselect);
	psy_signal_connect(&workspace->sequenceselection.signal_deselect, self,
		seqeditortrackdesc_onsequenceselectiondeselect);
	psy_signal_connect(&workspace->sequenceselection.signal_update, self,
		seqeditortrackdesc_onsequenceselectionupdate);		
	//psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);		
}

void seqeditortrackdesc_onsequenceselectionselect(SeqEditorTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditortrackdesc_onsequenceselectiondeselect(SeqEditorTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditortrackdesc_onsequenceselectionupdate(SeqEditorTrackDesc* self,
	psy_audio_SequenceSelection* selection)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditortrackdesc_onnewtrack(SeqEditorTrackDesc* self,
	psy_ui_Button* sender)
{
	self->state->cmd = SEQEDTCMD_NEWTRACK;
}

void seqeditortrackdesc_ondeltrack(SeqEditorTrackDesc* self,
	TrackBox* sender)
{
	self->state->cmd = SEQEDTCMD_DELTRACK;
	self->state->cmdtrack = trackbox_trackindex(sender);
}

void seqeditortrackdesc_build(SeqEditorTrackDesc* self)
{
	psy_audio_Sequence* sequence;

	psy_ui_component_clear(&self->component);
	sequence = &self->state->workspace->song->sequence;
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Button* newtrack;

		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			SequenceTrackBox* sequencetrackbox;
			
			sequencetrackbox = sequencetrackbox_allocinit(
				&self->component, &self->component,
				seqeditorstate_sequence(self->state), c);
			if (sequencetrackbox) {				
				psy_ui_component_setminimumsize(
					sequencetrackbox_base(sequencetrackbox),
					psy_ui_size_makeem(0.0, 2.0));
				sequencetrackbox_showtrackname(sequencetrackbox);
			}
			if (sequencetrackbox) {				
				psy_signal_connect(
					&sequencetrackbox->trackbox.signal_close, self,
					seqeditortrackdesc_ondeltrack);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->component, &self->component);
		if (newtrack) {
			psy_ui_button_settext(newtrack, "sequencerview.new-trk");
			psy_ui_component_setminimumsize(psy_ui_button_base(newtrack),
				psy_ui_size_makeem(0.0, 2.0));
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				seqeditortrackdesc_onnewtrack);
		}		
	}
	psy_ui_component_align(&self->component);
}

// SeqEditorTracks
// prototypes
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_onmousedown(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmousemove(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmousedoubleclick(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmouseenter(SeqEditorTracks*);
static void seqeditortracks_onmouseleave(SeqEditorTracks*);
static void seqeditortracks_build(SeqEditorTracks*);
static void seqeditortracks_onsequenceselectionselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_onsequenceselectiondeselect(SeqEditorTracks*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks*,
	psy_audio_SequenceSelection*);
static void seqeditortracks_onupdatestyles(SeqEditorTracks*);
static void seqeditortracks_onalign(SeqEditorTracks*);
static void seqeditortracks_ontimer(SeqEditorTracks*, uintptr_t timerid);
static void seqeditortracks_oncursorchanged(SeqEditorTracks*, SeqEditorState*);
static void seqeditortracks_updatecursorlineposition(SeqEditorTracks*);
static void seqeditortracks_updateseqeditlineposition(SeqEditorTracks*);
static void seqeditortracks_outputstatusposition(SeqEditorTracks*, double x);
// vtable
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);
		seqeditortracks_vtable.onalign = (psy_ui_fp_component_onalign)
			seqeditortracks_onalign;
		seqeditortracks_vtable.ontimer = (psy_ui_fp_component_ontimer)
			seqeditortracks_ontimer;
		seqeditortracks_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmousemove;
		seqeditortracks_vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)
			seqeditortracks_onmouseenter;
		seqeditortracks_vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)
			seqeditortracks_onmouseleave;		
		seqeditortracks_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditorState* state, PatternViewSkin* skin, Workspace* workspace)
{
	self->workspace = workspace;
	self->state = state;
	self->playline = NULL;
	self->cursorline = NULL;
	self->seqeditposline = NULL;
	self->skin = skin;
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditortracks_vtable_init(self);
	self->component.vtable = &seqeditortracks_vtable;
	psy_ui_component_doublebuffer(&self->component);
	//psy_ui_component_setbackgroundcolour(&self->component,
		//psy_ui_colour_make(0x00CACACA));
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	psy_ui_component_setscrollmode(&self->component, psy_ui_SCROLL_COMPONENTS);
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero()); // makeem(0.0, 0.0, 0.1, 0.0));	
	psy_signal_connect(&workspace->sequenceselection.signal_select, self,
		seqeditortracks_onsequenceselectionselect);
	psy_signal_connect(&workspace->sequenceselection.signal_deselect, self,
		seqeditortracks_onsequenceselectiondeselect);
	psy_signal_connect(&workspace->sequenceselection.signal_update, self,
		seqeditortracks_onsequenceselectionupdate);	
	seqeditortracks_build(self);
	psy_signal_connect(&self->state->signal_cursorchanged, self,
		seqeditortracks_oncursorchanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void seqeditortracks_ondestroy(SeqEditorTracks* self, psy_ui_Component* sender)
{	
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;

	self->playline = NULL;
	psy_ui_component_clear(&self->component);
	sequence = seqeditorstate_sequence(self->state);
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		psy_ui_Component* spacer;
		
		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			SeqEditorTrack* seqedittrack;
			
			seqedittrack = seqeditortrack_allocinit(
				&self->component, &self->component,
				self->state, self->skin, self->workspace);	
			if (seqedittrack) {				
				seqeditortrack_updatetrack(seqedittrack,
					t, (psy_audio_SequenceTrack*)t->entry, c);
			}
		}
		spacer = psy_ui_component_allocinit(&self->component, &self->component);
		psy_ui_component_setminimumsize(spacer,
			psy_ui_size_makeem(10.0, 2.0));
	}
	self->playline = seqeditorplayline_allocinit(&self->component,
		&self->component, self->state);
	if (self->playline) {
		psy_ui_component_setalign(&self->playline->seqeditorline.component, psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->playline->seqeditorline.component, NULL);
		psy_ui_component_setbackgroundcolour(&self->playline->seqeditorline.component,
			self->skin->playbar);
		seqeditorplayline_update(self->playline);
	}
	self->cursorline = seqeditorline_allocinit(&self->component,
		&self->component, self->state);
	if (self->cursorline) {
		psy_ui_component_setalign(&self->cursorline->component, psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->cursorline->component, NULL);
		psy_ui_component_hide(&self->cursorline->component);
		psy_ui_component_setbackgroundcolour(&self->cursorline->component,
			self->skin->cursor);
	}
	self->seqeditposline = seqeditorline_allocinit(&self->component,
		&self->component, self->state);
	if (self->seqeditposline) {
		psy_ui_component_setalign(&self->seqeditposline->component, psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->seqeditposline->component, NULL);		
		psy_ui_component_setbackgroundcolour(&self->seqeditposline->component,
			self->skin->rowbeat);
		seqeditortracks_updateseqeditlineposition(self);
	}
}

void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection)
{
	psy_ui_component_invalidate(&self->component);
	seqeditortracks_updateseqeditlineposition(self);
}

void seqeditortracks_onsequenceselectionselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	seqeditortracks_updateseqeditlineposition(self);
	psy_ui_component_invalidate(&self->component);	
}

void seqeditortracks_onsequenceselectiondeselect(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	if (self->seqeditposline) {
		psy_audio_SequenceEntry* seqentry;
				
		seqentry = psy_audio_sequence_entry(
			seqeditorstate_sequence(self->state),
			selection->editposition);
		if (seqentry) {
			seqeditorline_updateposition(self->seqeditposline,
				seqentry->offset);
		}
	}
	psy_ui_component_invalidate(&self->component);
}

void seqeditortracks_onalign(SeqEditorTracks* self)
{	
	if (self->playline) {
		seqeditorplayline_update(self->playline);
	}
	seqeditortracks_updatecursorlineposition(self);
	seqeditortracks_updateseqeditlineposition(self);
}

void seqeditortracks_oncursorchanged(SeqEditorTracks* self, SeqEditorState* sender)
{
	seqeditortracks_updatecursorlineposition(self);
}

void seqeditortracks_updatecursorlineposition(SeqEditorTracks* self)
{
	if (self->cursorline) {
		if (self->state->cursoractive &&
				!psy_ui_component_visible(&self->cursorline->component)) {
			psy_ui_component_show(&self->cursorline->component);
		} else if (!self->state->cursoractive &&
				psy_ui_component_visible(&self->cursorline->component)) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
			return;
		}
		seqeditorline_updateposition(self->cursorline,
			self->state->cursorposition);		
	}
}

void seqeditortracks_updateseqeditlineposition(SeqEditorTracks* self)
{
	if (self->seqeditposline) {
		psy_audio_SequenceEntry* seqentry;

		seqentry = psy_audio_sequence_entry(
			seqeditorstate_sequence(self->state),
			self->workspace->sequenceselection.editposition);
		if (seqentry) {
			seqeditorline_updateposition(self->seqeditposline,
				seqentry->offset);
		}
	}
}

void seqeditortracks_ontimer(SeqEditorTracks* self, uintptr_t timerid)
{		
	if (psy_audio_player_playing(workspace_player(
			self->workspace))) {
		seqeditorplayline_update(self->playline);
	}
}

void seqeditortracks_onupdatestyles(SeqEditorTracks* self)
{	
}

void seqeditortracks_onmousedown(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	if (self->state->dragmode == SEQEDITORDRAG_REORDER) {
		if (self->cursorline) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void seqeditortracks_onmousemove(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->state->dragstatus && self->state->dragmode == SEQEDITORDRAG_MOVE) {
		seqeditortracks_updateseqeditlineposition(self);
	}
	seqeditortracks_outputstatusposition(self, ev->pt.x);
}

void seqeditortracks_onmouseenter(SeqEditorTracks* self)
{
	self->state->cursoractive = TRUE;	
	psy_signal_emit(&self->state->signal_cursorchanged, self->state, 0);
}

void seqeditortracks_onmouseleave(SeqEditorTracks* self)
{
	self->state->cursoractive = FALSE;
	if (self->cursorline) {
		psy_ui_component_hide(&self->cursorline->component);
		psy_ui_component_invalidate(&self->component);
	}
	psy_signal_emit(&self->state->signal_cursorchanged, self->state, 0);
}

void seqeditortracks_outputstatusposition(SeqEditorTracks* self, double x)
{
	psy_dsp_big_beat_t position;
	char text[256];

	position = seqeditorstate_pxtobeat(self->state, x);
	// quantize to lpb raster
	seqeditorstate_setcursor(self->state,
		position * psy_audio_player_lpb(workspace_player(self->workspace)) /
		(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace)));
	psy_snprintf(text, 256, "Sequence Position %.3fb",
		(float)self->state->cursorposition);
	workspace_outputstatus(self->workspace, text);
}


// SeqEditor
void seqeditorbar_init(SeqEditorBar* self, psy_ui_Component* parent)
{	
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	zoombox_init(&self->zoombox_beat, &self->component);
	psy_ui_button_init_text(&self->move, &self->component, NULL,
		"seqedit.move");
	psy_ui_button_init_text(&self->reorder, &self->component, NULL,
		"seqedit.reorder");
	psy_ui_margin_init_all_em(&spacing, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setspacing(&self->component, &spacing);	
}

void seqeditorbar_setdragmode(SeqEditorBar* self, SeqEditorDragMode mode)
{
	switch (mode) {
	case SEQEDITORDRAG_MOVE:
		psy_ui_button_highlight(&self->move);
		psy_ui_button_disablehighlight(&self->reorder);
		break;
	case SEQEDITORDRAG_REORDER:
		psy_ui_button_highlight(&self->reorder);
		psy_ui_button_disablehighlight(&self->move);
		break;
	default:
		break;
	}
}	

// SeqEditor
// prototypes
static void seqeditor_ondestroy(SeqEditor*);
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_Song*);
static void seqeditor_updatesong(SeqEditor*, psy_audio_Song*);
static void seqeditor_build(SeqEditor*);
static void seqeditor_ontracksscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onheaderscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onzoomboxbeatchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_onzoomboxheightchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_updatescrollstep(SeqEditor*);
static void seqeditor_updateoverflow(SeqEditor*);
static void seqeditor_ondragmodemove(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_ondragmodereorder(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onsequenceclear(SeqEditor*, psy_audio_Sequence*);
static void seqeditor_onsequenceremove(SeqEditor*, psy_audio_Sequence*,
	psy_audio_OrderIndex*);
static void seqeditor_onsequencetrackinsert(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_onsequencetrackremove(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_onmouseup(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_onmousemove(SeqEditor*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);
		seqeditor_vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			seqeditor_ondestroy;
		seqeditor_vtable.onmouseup = (psy_ui_fp_component_onmouseevent)
			seqeditor_onmouseup;
		seqeditor_vtable.onmousemove = (psy_ui_fp_component_onmouseevent)
			seqeditor_onmousemove;
		seqeditor_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	PatternViewSkin* skin,
	Workspace* workspace)
{		
	psy_ui_Margin topmargin;	
	
	psy_ui_margin_init_all_em(&topmargin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditor_vtable_init(self);	
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQEDT, STYLE_SEQEDT, STYLE_SEQEDT);
	self->component.vtable = &seqeditor_vtable;	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	seqeditorstate_init(&self->state);
	self->state.workspace = workspace;
	self->workspace = workspace;
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	seqeditorbar_init(&self->bar, &self->left);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->bar.zoombox_beat.signal_changed, self,
		seqeditor_onzoomboxbeatchanged);
	// track description
	psy_ui_component_init(&self->trackdescpane, &self->left, NULL);
	psy_ui_component_setalign(&self->trackdescpane, psy_ui_ALIGN_CLIENT);
	seqeditortrackdesc_init(&self->trackdescriptions, &self->trackdescpane,
		&self->state, workspace);
	psy_ui_component_setalign(&self->trackdescriptions.component,
		psy_ui_ALIGN_HCLIENT);
	self->trackdescriptions.skin = skin;	
	zoombox_init(&self->zoombox_height, &self->left);
	psy_ui_component_setalign(&self->zoombox_height.component,
		psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->zoombox_height.signal_changed, self,
		seqeditor_onzoomboxheightchanged);
	// ruler
	seqeditorruler_init(&self->ruler, &self->component, &self->state,
		skin, workspace);	
	psy_ui_component_setalign(seqeditorruler_base(&self->ruler),
		psy_ui_ALIGN_TOP);	
	seqeditortracks_init(&self->tracks, &self->component,		
		&self->state, skin, workspace);
	self->tracks.skin = skin;	
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component, NULL);
	psy_ui_component_setbackgroundmode(&self->scroller.pane,
		psy_ui_SETBACKGROUND);
	psy_ui_component_setalign(&self->tracks.component,
		psy_ui_ALIGN_FIXED_RESIZE);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	seqeditorbar_setdragmode(&self->bar, self->state.dragmode);	
	// align
	psy_ui_component_resize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20.0),
			psy_ui_value_makeeh(6 * 1.4 + 2.5)));
	// use splitbar
	psy_ui_component_preventpreferredsize(&self->component);
	seqeditor_updatesong(self, workspace->song);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_ontracksscroll);
	//psy_signal_connect(&self->trackdescriptions.component.signal_scroll, self,
		//seqeditor_onheaderscroll);
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);
	psy_signal_connect(&self->bar.move.signal_clicked, self,
		seqeditor_ondragmodemove);
	psy_signal_connect(&self->bar.reorder.signal_clicked, self,
		seqeditor_ondragmodereorder);
}

void seqeditor_ondestroy(SeqEditor* self)
{
	seqeditorstate_dispose(&self->state);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{
	seqeditor_updatesong(self, workspace->song);
}

void seqeditor_updatesong(SeqEditor* self, psy_audio_Song* song)
{
	if (song) {						
		seqeditor_build(self);		
		psy_signal_connect(&song->sequence.signal_clear, self,
			seqeditor_onsequenceclear);
		psy_signal_connect(&song->sequence.signal_remove, self,
			seqeditor_onsequenceremove);
		psy_signal_connect(&song->sequence.signal_trackinsert, self,
			seqeditor_onsequencetrackinsert);
		psy_signal_connect(&song->sequence.signal_trackremove, self,
			seqeditor_onsequencetrackremove);		
		seqeditor_updatescrollstep(self);
		seqeditor_updateoverflow(self);		
	}
}

void seqeditor_updatescrollstep(SeqEditor* self)
{	
	const psy_ui_TextMetric* tm;
	
	tm = psy_ui_component_textmetric(&self->tracks.component);
	self->tracks.component.scrollstepy = psy_ui_add_values(
		self->tracks.state->lineheight,
		self->tracks.state->linemargin,
		tm);
	self->tracks.component.scrollstepy = psy_ui_value_makepx(
		(floor)(psy_ui_value_px(&self->tracks.component.scrollstepy, tm)));	
	self->trackdescriptions.component.scrollstepy =
		self->tracks.component.scrollstepy;
}

void seqeditor_updateoverflow(SeqEditor* self)
{
	psy_ui_component_updateoverflow(seqeditortracks_base(&self->tracks));
	psy_ui_component_updateoverflow(&self->trackdescriptions.component);
	psy_ui_component_invalidate(seqeditortracks_base(&self->tracks));
	psy_ui_component_invalidate(&self->trackdescriptions.component);
}

void seqeditor_onsequenceclear(SeqEditor* self, psy_audio_Sequence* sender)
{
	psy_ui_component_move(&self->trackdescriptions.component,
		psy_ui_point_zero());
	psy_ui_component_move(&self->tracks.component,
		psy_ui_point_zero());
	seqeditor_build(self);
}

void seqeditor_onsequenceinsert(SeqEditor* self, psy_audio_Sequence* sender,
	psy_audio_OrderIndex* index)
{	
	seqeditor_build(self);		
}

void seqeditor_onsequenceremove(SeqEditor* self, psy_audio_Sequence* sender,
	psy_audio_OrderIndex* index)
{
	seqeditor_build(self);	
}

void seqeditor_onsequencetrackinsert(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_onsequencetrackremove(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_build(SeqEditor* self)
{	
	seqeditortracks_build(&self->tracks);
	seqeditortrackdesc_build(&self->trackdescriptions);
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_align(&self->tracks.component);
	psy_ui_component_align(&self->trackdescpane);
	psy_ui_component_invalidate(&self->component);
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
}

void seqeditor_ontracksscroll(SeqEditor* self, psy_ui_Component* sender)
{
	psy_ui_RealRectangle tracksposition;
	psy_ui_RealRectangle trackdescposition;

	tracksposition = psy_ui_component_position(&self->tracks.component);
	trackdescposition = psy_ui_component_position(&self->trackdescriptions.component);
	psy_ui_component_move(&self->trackdescriptions.component,
		psy_ui_point_makepx(trackdescposition.left, tracksposition.top));
}

void seqeditor_onheaderscroll(SeqEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrolltop(&self->tracks.component,
		psy_ui_value_makepx(
			-psy_ui_component_position(&self->trackdescriptions.component).top));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{	
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onzoomboxbeatchanged(SeqEditor* self, ZoomBox* sender)
{
	self->state.pxperbeat = sender->zoomrate * DEFAULT_PXPERBEAT;
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
	psy_ui_component_invalidate(&self->ruler.component);
}

void seqeditor_onzoomboxheightchanged(SeqEditor* self, ZoomBox* sender)
{
	self->state.lineheight = psy_ui_mul_value_real(
		self->state.defaultlineheight, zoombox_rate(sender));
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
}

void seqeditor_ondragmodemove(SeqEditor* self, psy_ui_Component* sender)
{
	self->state.dragmode = SEQEDITORDRAG_MOVE;
	seqeditorbar_setdragmode(&self->bar, self->state.dragmode);
}

void seqeditor_ondragmodereorder(SeqEditor* self, psy_ui_Component* sender)
{
	self->state.dragmode = SEQEDITORDRAG_REORDER;
	seqeditorbar_setdragmode(&self->bar, self->state.dragmode);
}

void seqeditor_onmouseup(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.cmd == SEQEDTCMD_REORDER) {
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_reorder(
			seqeditorstate_sequence(&self->state),			
			psy_audio_orderindex_make(
				self->state.cmdtrack,
				self->state.cmdrow),
			self->state.cursorposition);
		psy_audio_exclusivelock_leave();
		psy_ui_component_invalidate(&self->tracks.component);
	} else if (self->state.cmd == SEQEDTCMD_NEWTRACK) {
		psy_audio_exclusivelock_enter();		
		psy_audio_sequence_appendtrack(&self->workspace->song->sequence,
			psy_audio_sequencetrack_allocinit());
		workspace_setsequenceeditposition(self->workspace,
			psy_audio_orderindex_make(
				psy_audio_sequence_width(&self->workspace->song->sequence) - 1, 0));
		psy_audio_exclusivelock_leave();
	} else if (self->state.cmd == SEQEDTCMD_DELTRACK) {
		psy_audio_SequencePosition position;

		position = psy_audio_sequence_at(&self->workspace->song->sequence,
			self->state.cmdtrack, 0);
		psy_audio_exclusivelock_enter();		
		psy_audio_sequence_removetrack(&self->workspace->song->sequence,
			position.tracknode);
		psy_audio_exclusivelock_leave();
	} else if (self->state.cmd == SEQEDTCMD_INSERTPATTERN) {
		psy_audio_Pattern* newpattern;
		uintptr_t patidx;

		newpattern = psy_audio_pattern_allocinit();
		// change length to default lines
		psy_audio_pattern_setlength(newpattern,
			(psy_dsp_big_beat_t)
			(psy_audio_pattern_defaultlines() /
				(psy_audio_player_lpb(workspace_player(self->workspace)))));
		patidx = psy_audio_patterns_append(&self->workspace->song->patterns, newpattern);
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceinsertcommand_alloc(
				&self->workspace->song->sequence,
				&self->workspace->sequenceselection,
				psy_audio_orderindex_make(self->state.cmdtrack,
					psy_INDEX_INVALID),
				patidx)->command);
	}
	self->state.cmd = SEQEDTCMD_NONE;
	self->state.sequenceentry = NULL;
	self->state.dragseqpos = psy_audio_orderindex_zero();
}

void seqeditor_onmousemove(SeqEditor* self, psy_ui_MouseEvent* ev)
{
	if (self->state.updatecursorposition) {
		psy_ui_component_invalidate(&self->ruler.component);
		self->state.updatecursorposition = FALSE;
	}
}
