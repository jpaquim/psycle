/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "seqeditor.h"
/* host */
#include "cmdsgeneral.h"
#include "pianoroll.h"
#include "resources/resource.h"
#include "sequencetrackbox.h"
#include "styles.h"
/* audio */
#include <exclusivelock.h>
#include <sequencecmds.h>
#include <songio.h>
/* std */
#include <math.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define DEFAULT_PXPERBEAT 5.0

void seqeditorstate_init(SeqEditorState* self, Workspace* workspace,
	SequenceCmds* cmds, psy_ui_Edit* edit)
{
	self->workspace = workspace;
	self->cmds = cmds;
	self->edit = edit;
	self->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
	psy_signal_init(&self->signal_cursorchanged);
	self->pxperbeat = DEFAULT_PXPERBEAT;
	self->defaultlineheight = psy_ui_value_make_eh(2.0);	
	self->lineheight = self->defaultlineheight;
	self->linemargin = psy_ui_value_make_eh(0.2);	
	self->cursorposition = (psy_dsp_big_beat_t)0.0;
	self->cursoractive = FALSE;
	self->drawcursor = TRUE;
	self->drawpatternevents = TRUE;	
	self->dragstatus = FALSE;
	self->dragstart = FALSE;
	self->draglength = FALSE;
	self->dragselection = FALSE;
	self->dragstartoffset = 0.0;
	self->dragpt = psy_ui_realpoint_zero();
	self->dragseqpos = psy_audio_orderindex_zero();	
	self->dragmode = SEQEDITORDRAG_MOVE;
	self->updatecursorposition = TRUE;
	self->sequenceentry = NULL;
	self->cmd = SEQEDTCMD_NONE;
	self->cmdtrack = psy_INDEX_INVALID;
	self->cmdrow = psy_INDEX_INVALID;
	self->showpatternnames = FALSE;
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

/* SeqEditorRuler */
/* prototypes */
static void seqeditorruler_ondestroy(SeqEditorRuler*);
static void seqeditorruler_ondraw(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_drawruler(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_draweditposition(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler*,
	psy_audio_SequenceSelection* sender);
static void seqeditorruler_onpreferredsize(SeqEditorRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static psy_dsp_big_beat_t seqeditorruler_step(SeqEditorRuler*);
static void seqeditorruler_oncursorchanged(SeqEditorRuler*, SeqEditorState*);
/* vtable */
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
	self->component.vtable = &seqeditorruler_vtable;
}
/* implementation */
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
	psy_signal_disconnect(&self->workspace->sequenceselection.signal_changed,
		self, seqeditorruler_onsequenceselectionchanged);
}

void seqeditorruler_ondraw(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	seqeditorruler_drawruler(self, g);
	seqeditorruler_draweditposition(self, g);
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

	size = psy_ui_component_size_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = size.height / 2 + 2;
	textline = 0;
	duration = (size.width + psy_ui_component_scrollleftpx(&self->component)) /
		(psy_dsp_big_beat_t)self->state->pxperbeat;
	/* psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence); */
	linewidth = duration * self->state->pxperbeat;
	
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_zero(),
		psy_ui_realsize_make(size.width, baseline)),		
		psy_ui_style(STYLE_SEQEDT_RULER_TOP)->backgroundcolour);	
	psy_ui_drawsolidrectangle(g, psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, baseline),
		psy_ui_realsize_make(size.width, baseline)),
		psy_ui_style(STYLE_SEQEDT_RULER_BOTTOM)->backgroundcolour);
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
		psy_ui_textout(g, cpx, textline, txt, psy_strlen(txt));
	}
	if (self->state->drawcursor && self->state->cursoractive) {
		double cpx;
		
		psy_ui_setcolour(g, self->skin->cursor);
		cpx = self->state->cursorposition * self->state->pxperbeat;
		psy_ui_drawline(g, psy_ui_realpoint_make(cpx, baseline),
			psy_ui_realpoint_make(cpx, baseline + size.height / 2));
	}
}

void seqeditorruler_draweditposition(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	psy_ui_IconDraw icondraw;
	double position_px;		
	psy_audio_SequenceEntry* seqentry;

	seqentry = psy_audio_sequence_entry(
		seqeditorstate_sequence(self->state),
		self->state->workspace->sequenceselection.editposition);
	if (seqentry) {
		psy_ui_RealSize size;
		double baseline;

		size = psy_ui_component_size_px(&self->component);
		baseline = size.height / 2 + 2;
		position_px = floor(seqeditorstate_beattopx(self->state, seqentry->offset));		
		psy_ui_icondraw_init(&icondraw, psy_ui_ICON_MORE);
		psy_ui_icondraw_draw(&icondraw, g,
			psy_ui_realpoint_make(position_px + 2, baseline + 2),
			psy_ui_colour_make(0x00FF0000));
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
			&workspace_song(self->workspace)->sequence) + 400.0;
		linewidth = duration * self->state->pxperbeat;
		rv->width = psy_ui_value_make_px(linewidth);
	} else {
		rv->width = psy_ui_value_make_px(0);
	}
	rv->height = psy_ui_value_make_eh(2.0);
}

/* SeqEditorLine */
static void seqeditorline_onpreferredsize(SeqEditorLine*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
/* vtable */
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
			psy_ui_point_make_px(-1.0, 0.0),
			psy_ui_size_make_px(1.0, 100.0)));
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
	rv->width = psy_ui_value_make_px(1.0);
	rv->height = psy_ui_value_make_eh(10.0);
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
	parentsize = psy_ui_component_scrollsize_px(
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

/* SeqEditorPlayLine */
/* prototypes */
static void seqeditorplayline_onmousedown(SeqEditorPlayline*, psy_ui_MouseEvent*);
static void seqeditorplayline_onmousemove(SeqEditorPlayline*, psy_ui_MouseEvent*);
static void seqeditorplayline_onmouseup(SeqEditorPlayline*, psy_ui_MouseEvent*);
/* vtable */
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
/* implementation */
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
	psy_ui_mouseevent_stop_propagation(ev);
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
	psy_ui_mouseevent_stop_propagation(ev);	
}

void seqeditorplayline_onmouseup(SeqEditorPlayline* self, psy_ui_MouseEvent* ev)
{
	self->drag = FALSE;
	self->dragbase = 0.0;
	psy_ui_component_releasecapture(seqeditorline_base(&self->seqeditorline));
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditorplayline_update(SeqEditorPlayline* self)
{		
	seqeditorline_updateposition(&self->seqeditorline,		
		psy_audio_player_position(workspace_player(
			self->seqeditorline.state->workspace)));	
}

/* SeqEditorPatternEnrty */
/* prototypes */
static void seqeditorpatternentry_ondestroy(SeqEditorPatternEntry*);
static void seqeditorpatternentry_ondraw(SeqEditorPatternEntry*,
	psy_ui_Graphics*);
static void seqeditorpatternentry_onpreferredsize(SeqEditorPatternEntry*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditorpatternentry_onmousedown(SeqEditorPatternEntry*,
	psy_ui_MouseEvent*);
static void seqeditorpatternentry_onmousemove(SeqEditorPatternEntry*,
	psy_ui_MouseEvent*);
static void seqeditorpatternentry_onmousedoubleclick(SeqEditorPatternEntry*,
	psy_ui_MouseEvent*);
static void seqeditorpatternentry_onsequenceselectionselect(SeqEditorPatternEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditorpatternentry_onsequenceselectiondeselect(SeqEditorPatternEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
/* vtable */
static psy_ui_ComponentVtable seqeditorpatternentry_vtable;
static bool seqeditorpatternentry_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditorpatternentry_vtable_init(SeqEditorPatternEntry* self)
{
	if (!seqeditorpatternentry_vtable_initialized) {
		seqeditorpatternentry_vtable = *(self->component.vtable);
		seqeditorpatternentry_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditorpatternentry_ondestroy;
		seqeditorpatternentry_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditorpatternentry_ondraw;
		seqeditorpatternentry_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditorpatternentry_onpreferredsize;
		seqeditorpatternentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditorpatternentry_onmousedown;
		seqeditorpatternentry_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditorpatternentry_onmousemove;
		seqeditorpatternentry_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditorpatternentry_onmousedoubleclick;
	}
	return &seqeditorpatternentry_vtable;
}
/* implementation */
void seqeditorpatternentry_init(SeqEditorPatternEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	psy_ui_component_init(&self->component, parent, view);	
	psy_ui_component_setvtable(&self->component,
		seqeditorpatternentry_vtable_init(self));
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQEDT_ITEM, STYLE_SEQEDT_ITEM_HOVER,
		STYLE_SEQEDT_ITEM_SELECTED, psy_INDEX_INVALID);	
	
	self->state = state;
	self->sequenceentry = entry;
	self->seqpos = seqpos;
	psy_signal_connect(&state->workspace->sequenceselection.signal_select, self,
		seqeditorpatternentry_onsequenceselectionselect);
	psy_signal_connect(&state->workspace->sequenceselection.signal_deselect, self,
		seqeditorpatternentry_onsequenceselectiondeselect);
	if (psy_audio_orderindex_equal(&self->seqpos,
			&self->state->workspace->sequenceselection.editposition)) {
		psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
	}
}

SeqEditorPatternEntry* seqeditorpatternentry_alloc(void)
{
	return (SeqEditorPatternEntry*)malloc(sizeof(SeqEditorPatternEntry));
}

SeqEditorPatternEntry* seqeditorpatternentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequencePatternEntry* entry, psy_audio_OrderIndex seqpos,
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

void seqeditorpatternentry_ondestroy(SeqEditorPatternEntry* self)
{
	psy_signal_disconnect(&self->state->workspace->sequenceselection.signal_select, self,
		seqeditorpatternentry_onsequenceselectionselect);
	psy_signal_disconnect(&self->state->workspace->sequenceselection.signal_deselect, self,
		seqeditorpatternentry_onsequenceselectiondeselect);
}

void seqeditorpatternentry_ondraw(SeqEditorPatternEntry* self,
	psy_ui_Graphics* g)
{
	if (self->sequenceentry) {		
		psy_audio_Pattern* pattern;

		pattern = psy_audio_sequencepatternentry_pattern(self->sequenceentry,
			&self->state->workspace->song->patterns);
		if (pattern) {
			psy_ui_RealSize size;
			const psy_ui_TextMetric* tm;
			psy_ui_RealRectangle clip;
			psy_ui_RealPoint topleft;
			char text[64];

			tm = psy_ui_component_textmetric(&self->component);
			size = psy_ui_component_scrollsize_px(&self->component);
			size.width -= 4;
			clip = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
			topleft = psy_ui_realpoint_make(4, 2);
			if (self->state->showpatternnames) {					
				psy_snprintf(text, 64, "%s", psy_audio_pattern_name(pattern));
			} else {
				psy_snprintf(text, 64, "%02X:%02X", self->seqpos.order,
					(int)psy_audio_sequencepatternentry_patternslot(self->sequenceentry));
			}
			psy_ui_textoutrectangle(g, topleft,
				psy_ui_ETO_CLIPPED, clip, text, psy_strlen(text));
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
		pattern = psy_audio_sequencepatternentry_pattern(self->sequenceentry,
			&self->state->workspace->song->patterns);
		if (pattern) {
			duration = psy_audio_pattern_length(pattern);
		}
	}
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
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
		/* quantize */
		position =
			(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->state->workspace))) /
			(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->state->workspace));
		self->state->dragstartoffset = psy_audio_sequenceentry_offset(&self->sequenceentry->entry) + position;
		cursor.offset = position;
		self->state->draglength = FALSE;
		if (position >= psy_audio_sequenceentry_length(&self->sequenceentry->entry) -
			1.0) {
			self->state->draglength = TRUE;
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		}
		if (ev->button == 1) {
			if (!self->state->draglength) {
				workspace_setpatterncursor(self->state->workspace, cursor);
				cursor = self->state->workspace->patterneditposition;
				workspace_gotocursor(self->state->workspace, cursor);
				if (workspace_currview(self->state->workspace).id != VIEW_ID_PATTERNVIEW) {
					workspace_selectview(self->state->workspace,
						VIEW_ID_PATTERNVIEW,
						workspace_currview(self->state->workspace).section,
						0);
				}
			}
			self->state->dragstatus = TRUE;
			self->state->dragstart = TRUE;
		} else {
			self->state->dragstatus = TRUE;
			self->state->cmd = SEQEDTCMD_REMOVEPATTERN;			
		}
		self->state->dragseqpos = self->seqpos;
		self->state->sequenceentry = &self->sequenceentry->entry;		
	}
}

void seqeditorpatternentry_onmousemove(SeqEditorPatternEntry* self,
	psy_ui_MouseEvent* ev)
{
	psy_dsp_big_beat_t position;
	
	position = seqeditorstate_pxtobeat(self->state, ev->pt.x);
	/* quantize */
	position =
		(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->state->workspace))) /
		(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->state->workspace));
	self->state->dragstartoffset = psy_audio_sequenceentry_offset(&self->sequenceentry->entry) + position;	
	if (position >= psy_audio_sequenceentry_length(&self->sequenceentry->entry) -
			1.0) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}

void seqeditorpatternentry_onmousedoubleclick(SeqEditorPatternEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		sequencecmds_changeplayposition(self->state->cmds);		
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditorpatternentry_onsequenceselectionselect(
	SeqEditorPatternEntry* self, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqeditorpatternentry_onsequenceselectiondeselect(
	SeqEditorPatternEntry* self, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);		
	}
}

/* SeqEditorSampleEnrty */
/* prototypes */
static void seqeditorsampleentry_ondestroy(SeqEditorSampleEntry*);
static void seqeditorsampleentry_onpreferredsize(SeqEditorSampleEntry*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditorsampleentry_onmousedown(SeqEditorSampleEntry*,
	psy_ui_MouseEvent*);
static void seqeditorsampleentry_onmousedoubleclick(SeqEditorSampleEntry*,
	psy_ui_MouseEvent*);
static void seqeditorsampleentry_onsequenceselectionselect(SeqEditorSampleEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditorsampleentry_onsequenceselectiondeselect(SeqEditorSampleEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditorsampleentry_edit(SeqEditorSampleEntry*);
static void seqeditorsampleentry_updatetext(SeqEditorSampleEntry*);
/* vtable */
static psy_ui_ComponentVtable seqeditorsampleentry_vtable;
static bool seqeditorsampleentry_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditorsampleentry_vtable_init(SeqEditorSampleEntry* self)
{
	if (!seqeditorsampleentry_vtable_initialized) {
		seqeditorsampleentry_vtable = *(self->component.vtable);
		seqeditorsampleentry_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditorsampleentry_ondestroy;		
		seqeditorsampleentry_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditorsampleentry_onpreferredsize;
		seqeditorsampleentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditorsampleentry_onmousedown;
		seqeditorsampleentry_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditorsampleentry_onmousedoubleclick;
	}
	return &seqeditorsampleentry_vtable;
}
/* implementation */
void seqeditorsampleentry_init(SeqEditorSampleEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	psy_ui_component_init(&self->component, parent, view);	
	psy_ui_component_setvtable(&self->component,
		seqeditorsampleentry_vtable_init(self));
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQEDT_SAMPLE, STYLE_SEQEDT_SAMPLE_HOVER,
		STYLE_SEQEDT_SAMPLE_SELECTED, psy_INDEX_INVALID);
	wavebox_init(&self->wavebox, &self->component, state->workspace);
	wavebox_setnowavetext(&self->wavebox, "");
	psy_ui_component_setalign(&self->wavebox.component, psy_ui_ALIGN_CLIENT);
	psy_ui_label_init(&self->label, &self->component, NULL);
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_TOP);
	self->state = state;
	self->sequenceentry = entry;
	self->seqpos = seqpos;
	self->preventedit = TRUE;
	psy_signal_connect(&state->workspace->sequenceselection.signal_select, self,
		seqeditorsampleentry_onsequenceselectionselect);
	psy_signal_connect(&state->workspace->sequenceselection.signal_deselect, self,
		seqeditorsampleentry_onsequenceselectiondeselect);
	if (psy_audio_orderindex_equal(&self->seqpos,
		&self->state->workspace->sequenceselection.editposition)) {
		psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
	}
	seqeditorsampleentry_updatesample(self);
	seqeditorsampleentry_updatetext(self);
}

SeqEditorSampleEntry* seqeditorsampleentry_alloc(void)
{
	return (SeqEditorSampleEntry*)malloc(sizeof(SeqEditorSampleEntry));
}

SeqEditorSampleEntry* seqeditorsampleentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceSampleEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	SeqEditorSampleEntry* rv;

	rv = seqeditorsampleentry_alloc();
	if (rv) {
		seqeditorsampleentry_init(rv, parent, view, entry, seqpos, state);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void seqeditorsampleentry_ondestroy(SeqEditorSampleEntry* self)
{
	psy_signal_disconnect(
		&self->state->workspace->sequenceselection.signal_select,
		self, seqeditorsampleentry_onsequenceselectionselect);
	psy_signal_disconnect(
		&self->state->workspace->sequenceselection.signal_deselect,
		self, seqeditorsampleentry_onsequenceselectiondeselect);
}

void seqeditorsampleentry_updatesample(SeqEditorSampleEntry* self)
{	
	psy_audio_Sample* sample;

	sequencecmds_update(self->state->cmds);
	if (self->state->cmds->sequence && self->state->cmds->sequence->samples) {
		sample = psy_audio_samples_at(self->state->cmds->sequence->samples,
			self->sequenceentry->sampleindex);
	} else {
		sample = NULL;
	}
	wavebox_setsample(&self->wavebox, sample, 0);
}

void seqeditorsampleentry_updatetext(SeqEditorSampleEntry* self)	
{
	if (self->sequenceentry) {
		char text[64];
		
		psy_snprintf(text, 64, "%02X:%02X:%02X", self->seqpos.order,
			(int)psy_audio_sequencesampleentry_samplesindex(self->sequenceentry).slot,
			(int)psy_audio_sequencesampleentry_samplesindex(self->sequenceentry).subslot);
		psy_ui_label_settext(&self->label, text);
	}
}

void seqeditorsampleentry_onpreferredsize(SeqEditorSampleEntry* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t duration;

	duration = 0.0;
	if (self->sequenceentry) {
		duration = psy_audio_sequenceentry_length(
			&self->sequenceentry->entry);
	}
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
		duration);
	rv->height = self->state->lineheight;
}

void seqeditorsampleentry_onmousedown(SeqEditorSampleEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (self->sequenceentry) {
		psy_dsp_big_beat_t position;
		psy_audio_OrderIndex seqeditpos;

		seqeditpos = workspace_sequenceeditposition(self->state->workspace);
		if (self->seqpos.track != seqeditpos.track ||
			self->seqpos.order != seqeditpos.order) {
			workspace_setsequenceeditposition(self->state->workspace,
				self->seqpos);
		}
		position = seqeditorstate_pxtobeat(self->state, ev->pt.x);
		/* quantize */
		position =
			(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->state->workspace))) /
			(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->state->workspace));
		self->state->dragstartoffset = psy_audio_sequenceentry_offset(&self->sequenceentry->entry) + position;
		self->state->draglength = FALSE;
		if (position >= psy_audio_sequenceentry_length(&self->sequenceentry->entry) -
			1.0) {
			self->state->draglength = TRUE;
		}
		if (ev->button == 1) {
			self->state->dragstatus = TRUE;
			self->state->dragstart = TRUE;			
			if (workspace_currview(self->state->workspace).id != VIEW_ID_SAMPLESVIEW) {
				workspace_selectview(self->state->workspace,
					VIEW_ID_SAMPLESVIEW,
					workspace_currview(self->state->workspace).section,
					0);
			}			
		} else {
			self->state->dragstatus = TRUE;
			self->state->cmd = SEQEDTCMD_REMOVEPATTERN;
		}
		self->state->dragseqpos = self->seqpos;
		self->state->sequenceentry = &self->sequenceentry->entry;
	}
}

void seqeditorsampleentry_onmousedoubleclick(SeqEditorSampleEntry* self,
	psy_ui_MouseEvent* ev)
{	
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditorsampleentry_onsequenceselectionselect(SeqEditorSampleEntry* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqeditorsampleentry_onsequenceselectiondeselect(SeqEditorSampleEntry* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

/* SeqEditorMarkerEnrty */
/* prototypes */
static void seqeditormarkerentry_ondestroy(SeqEditorMarkerEntry*);
static void seqeditormarkerentry_ondraw(SeqEditorMarkerEntry*,
	psy_ui_Graphics*);
static void seqeditormarkerentry_onpreferredsize(SeqEditorMarkerEntry*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static void seqeditormarkerentry_onmousedown(SeqEditorMarkerEntry*,
	psy_ui_MouseEvent*);
static void seqeditormarkerentry_onmousemove(SeqEditorMarkerEntry* self,
	psy_ui_MouseEvent*);
static void seqeditormarkerentry_onmousedoubleclick(SeqEditorMarkerEntry*,
	psy_ui_MouseEvent*);
static void seqeditormarkerentry_onsequenceselectionselect(SeqEditorMarkerEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditormarkerentry_onsequenceselectiondeselect(SeqEditorMarkerEntry*,
	psy_audio_SequenceSelection*, psy_audio_OrderIndex*);
static void seqeditormarkerentry_oneditaccept(SeqEditorMarkerEntry*, psy_ui_Edit* sender);
static void seqeditormarkerentry_oneditreject(SeqEditorMarkerEntry*, psy_ui_Edit* sender);
static void seqeditormarkerentry_edit(SeqEditorMarkerEntry*);
/* vtable */
static psy_ui_ComponentVtable seqeditormarkerentry_vtable;
static bool seqeditormarkerentry_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* seqeditormarkerentry_vtable_init(SeqEditorMarkerEntry* self)
{
	if (!seqeditormarkerentry_vtable_initialized) {
		seqeditormarkerentry_vtable = *(self->component.vtable);
		seqeditormarkerentry_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditormarkerentry_ondestroy;
		seqeditormarkerentry_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditormarkerentry_ondraw;
		seqeditormarkerentry_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			seqeditormarkerentry_onpreferredsize;
		seqeditormarkerentry_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditormarkerentry_onmousedown;
		seqeditormarkerentry_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditormarkerentry_onmousemove;
		seqeditormarkerentry_vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			seqeditormarkerentry_onmousedoubleclick;
	}
	return &seqeditormarkerentry_vtable;
}
/* implementation */
void seqeditormarkerentry_init(SeqEditorMarkerEntry* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry,
	psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setvtable(&self->component,
		seqeditormarkerentry_vtable_init(self));
	psy_ui_component_setstyletypes(&self->component,
		STYLE_SEQEDT_MARKER, STYLE_SEQEDT_MARKER_HOVER,
		STYLE_SEQEDT_MARKER_SELECTED, psy_INDEX_INVALID);

	self->state = state;
	self->sequenceentry = entry;
	self->seqpos = seqpos;
	self->preventedit = TRUE;
	psy_signal_connect(&state->workspace->sequenceselection.signal_select, self,
		seqeditormarkerentry_onsequenceselectionselect);
	psy_signal_connect(&state->workspace->sequenceselection.signal_deselect, self,
		seqeditormarkerentry_onsequenceselectiondeselect);
	if (psy_audio_orderindex_equal(&self->seqpos,
		&self->state->workspace->sequenceselection.editposition)) {
		psy_ui_component_addstylestate(&self->component, psy_ui_STYLESTATE_SELECT);
	}
	if (self->state->edit) {		
		psy_signal_connect(&self->state->edit->signal_accept, self,
			seqeditormarkerentry_oneditaccept);
		psy_signal_connect(&self->state->edit->signal_reject, self,
			seqeditormarkerentry_oneditreject);
	}
}

SeqEditorMarkerEntry* seqeditormarkerentry_alloc(void)
{
	return (SeqEditorMarkerEntry*)malloc(sizeof(SeqEditorMarkerEntry));
}

SeqEditorMarkerEntry* seqeditormarkerentry_allocinit(
	psy_ui_Component* parent, psy_ui_Component* view,
	psy_audio_SequenceMarkerEntry* entry, psy_audio_OrderIndex seqpos,
	SeqEditorState* state)
{
	SeqEditorMarkerEntry* rv;

	rv = seqeditormarkerentry_alloc();
	if (rv) {
		seqeditormarkerentry_init(rv, parent, view, entry, seqpos, state);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void seqeditormarkerentry_ondestroy(SeqEditorMarkerEntry* self)
{
	psy_signal_disconnect(&self->state->workspace->sequenceselection.signal_select, self,
		seqeditormarkerentry_onsequenceselectionselect);
	psy_signal_disconnect(&self->state->workspace->sequenceselection.signal_deselect, self,
		seqeditormarkerentry_onsequenceselectiondeselect);
	if (self->state->edit) {
		psy_signal_disconnect(&self->state->edit->signal_accept, self,
			seqeditormarkerentry_oneditaccept);
		psy_signal_disconnect(&self->state->edit->signal_reject, self,
			seqeditormarkerentry_oneditreject);
	}
}

void seqeditormarkerentry_ondraw(SeqEditorMarkerEntry* self,
	psy_ui_Graphics* g)
{
	if (self->sequenceentry && self->sequenceentry->text) {
		psy_ui_RealSize size;
		const psy_ui_TextMetric* tm;
		psy_ui_RealRectangle clip;
		psy_ui_RealPoint topleft;
		char text[64];

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_scrollsize_px(&self->component);
		size.width -= 4;
		clip = psy_ui_realrectangle_make(psy_ui_realpoint_zero(), size);
		topleft = psy_ui_realpoint_make(4, 2);
		psy_snprintf(text, 64, "%s", self->sequenceentry->text);			
		psy_ui_textoutrectangle(g, topleft,
			psy_ui_ETO_CLIPPED, clip, text, psy_strlen(text));		
	}
}

void seqeditormarkerentry_onpreferredsize(SeqEditorMarkerEntry* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t duration;

	duration = 0.0;
	if (self->sequenceentry) {
		duration = psy_audio_sequenceentry_length(
			&self->sequenceentry->entry);
	}
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
		duration);
	rv->height = self->state->lineheight;
}

void seqeditormarkerentry_onmousedown(SeqEditorMarkerEntry* self,
	psy_ui_MouseEvent* ev)
{
	if (self->sequenceentry) {		
		psy_dsp_big_beat_t position;
		psy_audio_OrderIndex seqeditpos;

		seqeditpos = workspace_sequenceeditposition(self->state->workspace);
		if (self->seqpos.track != seqeditpos.track ||
			self->seqpos.order != seqeditpos.order) {
			workspace_setsequenceeditposition(self->state->workspace,
				self->seqpos);
		}		
		position = seqeditorstate_pxtobeat(self->state, ev->pt.x);
		/* quantize */
		position =
			(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->state->workspace))) /
			(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->state->workspace));
		self->state->dragstartoffset = psy_audio_sequenceentry_offset(&self->sequenceentry->entry) + position;
		self->state->draglength = FALSE;
		if (position >= psy_audio_sequenceentry_length(&self->sequenceentry->entry) -
				1.0) {
			self->state->draglength = TRUE;
			psy_ui_component_setcursor(&self->component,
				psy_ui_CURSORSTYLE_COL_RESIZE);
		}
		if (ev->button == 1) {			
			self->state->dragstatus = TRUE;
			self->state->dragstart = TRUE;
		} else {
			self->state->dragstatus = TRUE;
			self->state->cmd = SEQEDTCMD_REMOVEPATTERN;
		}
		self->state->dragseqpos = self->seqpos;
		self->state->sequenceentry = &self->sequenceentry->entry;
	}
}

void seqeditormarkerentry_onmousemove(SeqEditorMarkerEntry* self,
	psy_ui_MouseEvent* ev)
{
	psy_dsp_big_beat_t position;

	position = seqeditorstate_pxtobeat(self->state, ev->pt.x);
	/* quantize */
	position =
		(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->state->workspace))) /
		(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->state->workspace));
	self->state->dragstartoffset = psy_audio_sequenceentry_offset(&self->sequenceentry->entry) + position;
	if (position >= psy_audio_sequenceentry_length(&self->sequenceentry->entry) -
		1.0) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}

void seqeditormarkerentry_onmousedoubleclick(SeqEditorMarkerEntry* self,
	psy_ui_MouseEvent* ev)
{
	seqeditormarkerentry_edit(self);
	psy_ui_mouseevent_stop_propagation(ev);
}

void seqeditormarkerentry_onsequenceselectionselect(SeqEditorMarkerEntry* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_addstylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqeditormarkerentry_onsequenceselectiondeselect(SeqEditorMarkerEntry* self,
	psy_audio_SequenceSelection* selection, psy_audio_OrderIndex* index)
{
	if (psy_audio_orderindex_equal(&self->seqpos, index)) {
		psy_ui_component_removestylestate(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}

void seqeditormarkerentry_edit(SeqEditorMarkerEntry* self)
{
	if (self->state->edit) {
		psy_ui_RealRectangle screenposition;
		psy_ui_RealRectangle viewscreenposition;		
		const psy_ui_TextMetric* tm;
		double centery;
		psy_ui_RealSize size;

		screenposition = psy_ui_component_screenposition(&self->component);	
		viewscreenposition = psy_ui_component_screenposition(
			psy_ui_component_parent(&self->state->edit->component));
		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_component_scrollsize_px(&self->component);
		centery = (size.height - tm->tmHeight) / 2;
		psy_ui_component_setposition(&self->state->edit->component,
			psy_ui_rectangle_make(
				psy_ui_point_make_px(
					screenposition.left - viewscreenposition.left,
					screenposition.top - viewscreenposition.top + centery),
				psy_ui_size_make_px(
					screenposition.right - screenposition.left,
					tm->tmHeight)));
		psy_ui_edit_settext(self->state->edit, self->sequenceentry->text);
		psy_ui_edit_setsel(self->state->edit, 0, -1);
		psy_ui_edit_enableinputfield(self->state->edit);
		self->preventedit = FALSE;
		psy_ui_component_show(&self->state->edit->component);
		psy_ui_component_setfocus(&self->state->edit->component);		
	}
}

void seqeditormarkerentry_oneditaccept(SeqEditorMarkerEntry* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		psy_strreset(&self->sequenceentry->text,
			psy_ui_edit_text(self->state->edit));
		psy_ui_component_hide(psy_ui_edit_base(sender));
	}
}

void seqeditormarkerentry_oneditreject(SeqEditorMarkerEntry* self, psy_ui_Edit* sender)
{
	if (!self->preventedit) {
		self->preventedit = TRUE;
		psy_ui_component_hide(psy_ui_edit_base(sender));				
	}
}


/* SeqEditorTrack */
/* prototypes */
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
static void seqeditortrack_onsequenceremove(SeqEditorTrack*,
	psy_audio_Sequence* sender, psy_audio_OrderIndex* index);
static void seqeditortrack_onsequencetrackreposition(SeqEditorTrack*,
	psy_audio_Sequence* sender, uintptr_t trackidx);
/* vtable */
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
/* implementation */
void seqeditortrack_init(SeqEditorTrack* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	SeqEditorState* state, PatternViewSkin* skin,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	seqeditortrack_vtable_init(self);
	self->component.vtable = &seqeditortrack_vtable;
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HEXPAND);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	self->state = state;
	self->workspace = workspace;	
	self->currtrack = NULL;
	self->trackindex = 0;
	self->drag_sequenceitem_node = NULL;
	self->dragstarting = FALSE;	
	self->skin = skin;
	self->view = view;
	self->dragline = NULL;
	self->entries = NULL;
	if (seqeditorstate_sequence(self->state)) {
		psy_signal_connect(
			&seqeditorstate_sequence(self->state)->signal_insert,
			self, seqeditortrack_onsequenceinsert);
		psy_signal_connect(
			&seqeditorstate_sequence(self->state)->signal_remove,
			self, seqeditortrack_onsequenceremove);
		psy_signal_connect(
			&seqeditorstate_sequence(self->state)->signal_trackreposition,
			self, seqeditortrack_onsequencetrackreposition);		
	}
}

void seqeditortrack_ondestroy(SeqEditorTrack* self)
{
	if (seqeditorstate_sequence(self->state)) {
		psy_signal_disconnect(
			&seqeditorstate_sequence(self->state)->signal_insert,
			self, seqeditortrack_onsequenceinsert);
		psy_signal_disconnect(
			&seqeditorstate_sequence(self->state)->signal_remove,
			self, seqeditortrack_onsequenceremove);
		psy_signal_disconnect(
			&seqeditorstate_sequence(self->state)->signal_trackreposition,
			self, seqeditortrack_onsequencetrackreposition);		
	}
	psy_list_free(self->entries);
	self->entries = NULL;
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
				pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
					psy_audio_sequencepatternentry_patternslot(seqpatternentry));
				if (pattern) {
					SeqEditorPatternEntry* seqeditpatternentry;

					seqeditpatternentry = seqeditorpatternentry_allocinit(
						&self->component, self->view, seqpatternentry,
						psy_audio_orderindex_make(self->trackindex, c),
						self->state);
					if (seqeditpatternentry) {
						psy_ui_component_setalign(&seqeditpatternentry->component, psy_ui_ALIGN_LEFT);
						psy_list_append(&self->entries, seqeditpatternentry);
					}
				}
				break; }
			case psy_audio_SEQUENCEENTRY_SAMPLE: {
				psy_audio_SequenceSampleEntry* seqsampleentry;
				SeqEditorSampleEntry* seqeditsampleentry;

				seqsampleentry = (psy_audio_SequenceSampleEntry*)seqentry;
				seqeditsampleentry = seqeditorsampleentry_allocinit(
					&self->component, self->view, seqsampleentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqsampleentry) {
					psy_ui_component_setalign(&seqeditsampleentry->component, psy_ui_ALIGN_LEFT);
					psy_list_append(&self->entries, seqeditsampleentry);							
				}
				seqeditsampleentry->wavebox.preventselection = TRUE;
				break; }
			case psy_audio_SEQUENCEENTRY_MARKER: {
				psy_audio_SequenceMarkerEntry* seqmarkerentry;				
				SeqEditorMarkerEntry* seqeditmarkerentry;

				seqmarkerentry = (psy_audio_SequenceMarkerEntry*)seqentry;
				seqeditmarkerentry = seqeditormarkerentry_allocinit(
					&self->component, self->view, seqmarkerentry,
					psy_audio_orderindex_make(self->trackindex, c),
					self->state);
				if (seqeditmarkerentry) {
					psy_ui_component_setalign(&seqeditmarkerentry->component, psy_ui_ALIGN_LEFT);
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

void seqeditortrack_onalign(SeqEditorTrack* self)
{		
	psy_List* p;
	psy_List* q;
	psy_ui_RealSize size;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {
		return;
	}
	size = psy_ui_component_scrollsize_px(&self->component);	
	p = self->entries;
	for (p = self->entries, q = self->currtrack->entries; p != NULL && q != NULL; psy_list_next(&p), q = q->next) {
		psy_ui_Component* component;
		psy_audio_SequenceEntry* seqentry;		

		component = (psy_ui_Component*)psy_list_entry(p);
		seqentry = (psy_audio_SequenceEntry*)psy_list_entry(q);
		if (seqentry) {			
			psy_ui_component_setposition(component,
				psy_ui_rectangle_make(
					psy_ui_point_make_px(
						seqeditorstate_beattopx(self->state, seqentry->offset),
						0.0),
					psy_ui_size_make(
						psy_ui_value_make_px(
							seqeditorstate_beattopx(self->state,
								psy_audio_sequenceentry_length(seqentry))),
						psy_ui_value_make_px(size.height))));
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
	rv->width = psy_ui_value_make_px(self->state->pxperbeat *
		trackduration);
	rv->height = self->state->lineheight;
}

void seqeditortrack_onmousedown(SeqEditorTrack* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		if (self->state->dragstatus && self->state->sequenceentry) {
			psy_ui_component_capture(&self->component);
			self->state->dragpt = ev->pt;
			if (self->state->dragmode == SEQEDITORDRAG_REORDER) {
				self->dragline = seqeditorline_allocinit(&self->component, self->view, self->state);
				psy_ui_component_setbackgroundcolour(&self->dragline->component,
					self->skin->font);
				seqeditorline_updateposition(self->dragline,
					psy_audio_sequenceentry_offset(self->state->sequenceentry));
				self->state->cmd = SEQEDTCMD_REORDER;
				self->state->cmdtrack = self->trackindex;
				self->state->cmdrow = self->state->sequenceentry->row;
			}
		} else {
			psy_audio_OrderIndex seqeditpos;

			seqeditpos = workspace_sequenceeditposition(self->state->workspace);
			if (self->trackindex != seqeditpos.track) {
				seqeditpos.track = self->trackindex;
				seqeditpos.order = psy_INDEX_INVALID;
				workspace_setsequenceeditposition(self->state->workspace,
					seqeditpos);
			}
		}
	}
}

void seqeditortrack_onmousemove(SeqEditorTrack* self, psy_ui_MouseEvent* ev)
{		
	if (self->state->dragselection) {
		return;
	}
	if (self->state->draglength) {
		self->state->dragstart = FALSE;

	}
	if (self->state->dragstart) {
		psy_dsp_big_beat_t dragposition;		
		psy_dsp_big_beat_t bpl;

		dragposition = seqeditorstate_pxtobeat(self->state, ev->pt.x);
		bpl = (1.0 / (psy_dsp_big_beat_t)
			psy_audio_player_lpb(workspace_player(self->workspace)));
		/*
		** start drag only if a difference of one line exists if only the
		** pattern cursor (done in seqeditor_patternentry) wanted to be changed
		** and prevent moving the sequencentry
		*/
		if (fabs(dragposition - self->state->dragstartoffset) >= bpl) {
			self->state->dragstart = FALSE;
		}
		return;
	}
	if (self->state->dragstatus && self->state->sequenceentry) {
		if ((self->state->draglength)) {
			psy_dsp_big_beat_t dragposition;

			dragposition = seqeditorstate_pxtobeat(self->state, ev->pt.x);
			dragposition =
				(intptr_t)(dragposition * psy_audio_player_lpb(
					workspace_player(self->workspace))) /
				(psy_dsp_big_beat_t)psy_audio_player_lpb(
					workspace_player(self->workspace));
			psy_audio_sequenceentry_setlength(self->state->sequenceentry,
				psy_max(1.0, dragposition - psy_audio_sequenceentry_offset(
					self->state->sequenceentry)));
			psy_audio_sequence_reposition_track(
				&workspace_song(self->workspace)->sequence, self->currtrack);
			
		} else if ((self->state->dragmode & SEQEDITORDRAG_MOVE) ==
				SEQEDITORDRAG_MOVE) {
			psy_dsp_big_beat_t dragposition;

			dragposition = seqeditorstate_pxtobeat(self->state, ev->pt.x);
			if (dragposition - (self->state->sequenceentry->offset -
				self->state->sequenceentry->repositionoffset) >= 0) {
				dragposition =				
					(intptr_t)(dragposition * psy_audio_player_lpb(
						workspace_player(self->workspace))) /
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
		} else if (self->dragline &&
				((self->state->dragmode & SEQEDITORDRAG_REORDER) == SEQEDITORDRAG_REORDER)) {
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
	self->state->draglength = FALSE;
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

void seqeditortrack_onsequenceremove(SeqEditorTrack* self, psy_audio_Sequence* sender,
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

/* SeqEditorTrackDesc */
/* prototypes */
static void seqeditortrackdesc_ondestroy(SeqEditorTrackDesc*);
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
static void seqeditortrackdesc_ondragstart(SeqEditorTrackDesc*,
	psy_ui_Component* sender, psy_ui_DragEvent*);
static void seqeditortrackdesc_ondragover(SeqEditorTrackDesc*,
	psy_ui_DragEvent*);
static void seqeditortrackdesc_ondragdrop(SeqEditorTrackDesc*,
	psy_ui_DragEvent*);
static void seqeditortrackdesc_onresize(SeqEditorTrackDesc*,
	psy_ui_Component* sender, double* offset);
/* vtable */
static psy_ui_ComponentVtable seqeditortrackdesc_vtable;
static bool seqeditortrackdesc_vtable_initialized = FALSE;

static void seqeditortrackdesc_vtable_init(SeqEditorTrackDesc* self)
{
	if (!seqeditortrackdesc_vtable_initialized) {
		seqeditortrackdesc_vtable = *(self->component.vtable);		
		seqeditortrackdesc_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			seqeditortrackdesc_ondestroy;		
		seqeditortrackdesc_vtable.ondragover =
			(psy_ui_fp_component_ondragover)
			seqeditortrackdesc_ondragover;
		seqeditortrackdesc_vtable.ondrop =
			(psy_ui_fp_component_ondrop)
			seqeditortrackdesc_ondragdrop;
		seqeditortrackdesc_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditortrackdesc_vtable;
}
/*  implementation */
void seqeditortrackdesc_init(SeqEditorTrackDesc* self, psy_ui_Component* parent,
	SeqEditorState* state, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);	
	seqeditortrackdesc_vtable_init(self);
	psy_signal_init(&self->signal_resize);
	self->workspace = workspace;
	self->state = state;
	/*  psy_ui_component_doublebuffer(&self->component); */
	/*  psy_ui_component_setwheelscroll(&self->component, 1); */
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL);	
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());	
	seqeditortrackdesc_build(self);	
	psy_signal_connect(&workspace->sequenceselection.signal_select, self,
		seqeditortrackdesc_onsequenceselectionselect);
	psy_signal_connect(&workspace->sequenceselection.signal_deselect, self,
		seqeditortrackdesc_onsequenceselectiondeselect);
	psy_signal_connect(&workspace->sequenceselection.signal_update, self,
		seqeditortrackdesc_onsequenceselectionupdate);
	/* psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_VSCROLL); */
}

void seqeditortrackdesc_ondestroy(SeqEditorTrackDesc* self)
{
	psy_signal_dispose(&self->signal_resize);
}

void seqeditortrackdesc_onsequenceselectionselect(SeqEditorTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_Component* track;

	track = psy_ui_component_at(&self->component, index->track);
	if (track) {
		psy_ui_Component* trackbox;
		
		trackbox = psy_ui_component_at(track, 0);
		if (trackbox) {
			psy_ui_component_addstylestate(trackbox, psy_ui_STYLESTATE_SELECT);
		}
	}
}

void seqeditortrackdesc_onsequenceselectiondeselect(SeqEditorTrackDesc* self,
	psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex* index)
{
	psy_ui_Component* track;

	track = psy_ui_component_at(&self->component, index->track);
	if (track) {
		psy_ui_Component* trackbox;

		trackbox = psy_ui_component_at(track, 0);
		if (trackbox) {
			psy_ui_component_removestylestate(trackbox, psy_ui_STYLESTATE_SELECT);
		}
	}
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
		psy_audio_OrderIndex firstsel;

		firstsel = psy_audio_sequenceselection_first(
			&self->state->cmds->workspace->sequenceselection);
		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			SequenceTrackBox* sequencetrackbox;
			psy_audio_SequenceTrack* seqtrack;
			
			seqtrack = (psy_audio_SequenceTrack*)t->entry;			
			sequencetrackbox = sequencetrackbox_allocinit(
				&self->component, &self->component,
				seqeditorstate_sequence(self->state), c, self->state->edit);
			if (sequencetrackbox) {				
				psy_ui_component_show(&sequencetrackbox->trackbox.resize);
				psy_ui_component_setminimumsize(
					sequencetrackbox_base(sequencetrackbox),
					psy_ui_size_make_em(0.0, 2.0));
				if (seqtrack->height != 0.0) {
					psy_ui_component_setpreferredheight(
						&sequencetrackbox->component,
						psy_ui_value_make_eh(seqtrack->height));
				}
				sequencetrackbox_showtrackname(sequencetrackbox);
			}
			if (sequencetrackbox) {				
				/* psy_ui_component_setstyletype_hover(
					&sequencetrackbox->trackbox.component,
					STYLE_SEQEDT_DESCITEM_HOVER); */
				psy_ui_component_setstyletype_select(
					&sequencetrackbox->trackbox.component,
					STYLE_SEQEDT_DESCITEM_SELECTED);
				psy_ui_component_setstyletype(
					&sequencetrackbox->trackbox.track.component,
					STYLE_SEQEDT_TRACK_NUMBER);				
				psy_signal_connect(
					&sequencetrackbox->trackbox.signal_close, self,
					seqeditortrackdesc_ondeltrack);
				sequencetrackbox->trackbox.track.component.draggable = TRUE;
				psy_signal_connect(
					&sequencetrackbox->trackbox.track.component.signal_dragstart,
					self, seqeditortrackdesc_ondragstart);
				if (c == firstsel.track) {					
					psy_ui_component_addstylestate(
						&sequencetrackbox->trackbox.component,
						psy_ui_STYLESTATE_SELECT);
				}
				psy_signal_connect(&sequencetrackbox->signal_resize, self,
					seqeditortrackdesc_onresize);
			}
		}
		newtrack = psy_ui_button_allocinit(&self->component, &self->component);
		if (newtrack) {
			psy_ui_button_settext(newtrack, "seqview.new-trk");
			psy_ui_component_setminimumsize(psy_ui_button_base(newtrack),
				psy_ui_size_make_em(0.0, 2.0));
			newtrack->stoppropagation = FALSE;
			psy_signal_connect(&newtrack->signal_clicked, self,
				seqeditortrackdesc_onnewtrack);
		}		
	}
	psy_ui_component_align(&self->component);
}

void seqeditortrackdesc_onresize(SeqEditorTrackDesc* self,
	psy_ui_Component* sender, double* offset)
{		
	psy_ui_RealSize size;
	double height;
	const psy_ui_TextMetric* tm;

	size = psy_ui_component_size_px(sender);
	tm = psy_ui_component_textmetric(&self->component);
	height = (size.height + *offset) / tm->tmHeight;	
	sequencecmds_update(self->state->cmds);
	psy_audio_sequence_settrackheight(self->state->cmds->sequence,
		(uintptr_t)sender->id, height);
	psy_ui_component_setpreferredheight(sender,
		psy_ui_value_make_eh(height));
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_signal_emit(&self->signal_resize, self, 2, (void*)sender->id,
		&height);
}

void seqeditortrackdesc_ondragstart(SeqEditorTrackDesc* self,
	psy_ui_Component* sender, psy_ui_DragEvent* ev)
{
	ev->dataTransfer = psy_property_allocinit_key(NULL);	
	psy_property_append_int(ev->dataTransfer,
		"seqedittrack", sender->id, 0, 0);
	psy_ui_dragevent_prevent_default(ev);
}

void seqeditortrackdesc_ondragover(SeqEditorTrackDesc* self,
	psy_ui_DragEvent* ev)
{
	psy_ui_dragevent_prevent_default(ev);
}

void seqeditortrackdesc_ondragdrop(SeqEditorTrackDesc* self,
	psy_ui_DragEvent* ev)
{
	psy_ui_dragevent_prevent_default(ev);
	if (ev->dataTransfer && self->state->workspace->song) {
		intptr_t trackid;

		trackid = psy_property_at_int(ev->dataTransfer, "seqedittrack", -1);
		if (trackid != -1) {
			uintptr_t index;

			psy_ui_component_intersect(&self->component, ev->mouse.pt, &index);			
			if (index != trackid) {
				psy_audio_sequence_swaptracks(&self->state->workspace->song->sequence,
					trackid, index);
			}
		}
	}
}

/* SeqEditorTracks */
/* prototypes */
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_ondraw(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_onmousedown(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmousemove(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmouseup(SeqEditorTracks*,
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
static psy_ui_Component* psy_ui_component_track(psy_ui_Component*,
	psy_ui_RealPoint pt, uintptr_t* index);
static void seqeditortracks_selectdrag(SeqEditorTracks*);
/* vtable */
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);
		seqeditortracks_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			seqeditortracks_ondraw;
		seqeditortracks_vtable.onalign =
			(psy_ui_fp_component_onalign)
			seqeditortracks_onalign;
		seqeditortracks_vtable.ontimer =
			(psy_ui_fp_component_ontimer)
			seqeditortracks_ontimer;
		seqeditortracks_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmouseup;
		seqeditortracks_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			seqeditortracks_onmousemove;
		seqeditortracks_vtable.onmouseenter =
			(psy_ui_fp_component_onmouseenter)
			seqeditortracks_onmouseenter;
		seqeditortracks_vtable.onmouseleave =
			(psy_ui_fp_component_onmouseleave)
			seqeditortracks_onmouseleave;		
		seqeditortracks_vtable_initialized = TRUE;
	}
	self->component.vtable = &seqeditortracks_vtable;
}
/* implementation */
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditorState* state, PatternViewSkin* skin, Workspace* workspace)
{
	self->workspace = workspace;
	self->state = state;
	self->playline = NULL;
	self->cursorline = NULL;
	self->seqeditposline = NULL;
	self->skin = skin;
	psy_audio_patternselection_init(&self->selection);
	psy_audio_patterncursor_init(&self->selectionbase);
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditortracks_vtable_init(self);	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setscrollstep(&self->component,
		psy_ui_size_make_em(8.0, 0.0));
	/* psy_ui_component_setbackgroundcolour(&self->component,
	   psy_ui_colour_make(0x00CACACA)); */
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_TOP, psy_ui_margin_zero());
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
			psy_audio_SequenceTrack* seqtrack;
			
			seqedittrack = seqeditortrack_allocinit(
				&self->component, &self->component,
				self->state, self->skin, self->workspace);
			seqtrack = (psy_audio_SequenceTrack*)t->entry;
			if (seqedittrack) {
				seqeditortrack_updatetrack(seqedittrack,
					t, (psy_audio_SequenceTrack*)t->entry, c);
				psy_ui_component_setminimumsize(&seqedittrack->component,
					psy_ui_size_make_em(0.0, 2.0));				
				if (seqtrack->height != 0.0) {
					psy_ui_component_setpreferredheight(
						&seqedittrack->component,
						psy_ui_value_make_eh(seqtrack->height));
				}
			}					
		}
		spacer = psy_ui_component_allocinit(&self->component, &self->component);
		psy_ui_component_setminimumsize(spacer,
			psy_ui_size_make_em(10.0, 2.0));		
	}
	self->playline = seqeditorplayline_allocinit(&self->component,
		&self->component, self->state);
	if (self->playline) {
		psy_ui_component_setalign(&self->playline->seqeditorline.component, psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->playline->seqeditorline.component, psy_ui_margin_zero());
		psy_ui_component_setbackgroundcolour(&self->playline->seqeditorline.component,
			self->skin->playbar);
		seqeditorplayline_update(self->playline);
	}
	self->cursorline = seqeditorline_allocinit(&self->component,
		&self->component, self->state);
	if (self->cursorline) {
		psy_ui_component_setalign(&self->cursorline->component, psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->cursorline->component, psy_ui_margin_zero());
		psy_ui_component_hide(&self->cursorline->component);
		psy_ui_component_setbackgroundcolour(&self->cursorline->component,
			self->skin->cursor);
	}
	self->seqeditposline = seqeditorline_allocinit(&self->component,
		&self->component, self->state);
	if (self->seqeditposline) {
		psy_ui_component_setalign(&self->seqeditposline->component, psy_ui_ALIGN_NONE);
		psy_ui_component_setmargin(&self->seqeditposline->component, psy_ui_margin_zero());
		psy_ui_component_setbackgroundcolour(&self->seqeditposline->component,
			self->skin->rowbeat);
		seqeditortracks_updateseqeditlineposition(self);
	}
}

void seqeditortracks_onsequenceselectionupdate(SeqEditorTracks* self,
	psy_audio_SequenceSelection* selection)
{		
	seqeditortracks_updateseqeditlineposition(self);
	psy_ui_component_invalidate(&self->component);
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

void seqeditortracks_ondraw(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	if (psy_audio_patternselection_valid(&self->selection)) {
		double left;
		double right;
		psy_ui_Component* top;
		psy_ui_Component* bottom;
		psy_ui_RealRectangle topposition;
		psy_ui_RealRectangle bottomposition;

		top = psy_ui_component_at(&self->component,
			self->selection.topleft.track);
		if (top) {
			topposition = psy_ui_component_position(top);
		}
		bottom = psy_ui_component_at(&self->component,
			self->selection.bottomright.track);
		if (bottom) {
			bottomposition = psy_ui_component_position(bottom);
		}
		if (!top || !bottom) {
			return;
		}
		left = floor(seqeditorstate_beattopx(self->state,
			self->selection.topleft.offset));
		right = floor(seqeditorstate_beattopx(self->state,
			self->selection.bottomright.offset));
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(left, topposition.top),
				psy_ui_realsize_make(right - left,
					bottomposition.bottom - topposition.top)),
			psy_ui_colour_make(0x009B7800));
	}
}

void seqeditortracks_onmousedown(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{	
	if (self->state->dragstatus && self->state->cmd
			== SEQEDTCMD_REMOVEPATTERN) {
		sequencecmds_delentry(self->state->cmds);
		self->state->dragstatus = FALSE;
		psy_ui_mouseevent_stop_propagation(ev);
	} else if ((self->state->dragmode & SEQEDITORDRAG_REORDER)
			== SEQEDITORDRAG_REORDER) {
		if (self->cursorline) {
			psy_ui_component_hide(&self->cursorline->component);
			psy_ui_component_invalidate(&self->component);
		}
	} else {
		/* uintptr_t index;

		psy_ui_component_intersect(&self->component, ev->pt, &index);
		self->selectionbase.offset = self->state->cursorposition;
		self->selectionbase.track = index;
		psy_audio_patternselection_startdrag(&self->selection,
			self->selectionbase, self->selectionbase, 0.25);
		self->state->dragselection = TRUE;
		psy_ui_component_capture(&self->component); */
	}
}

void seqeditortracks_onmousemove(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{	
	seqeditortracks_outputstatusposition(self, ev->pt.x);
	if (self->state->dragstatus && (self->state->dragmode & SEQEDITORDRAG_MOVE) == SEQEDITORDRAG_MOVE) {
		seqeditortracks_updateseqeditlineposition(self);
	} else if (psy_audio_patternselection_valid(&self->selection)) {
		psy_audio_PatternCursor curr;
		psy_ui_Component* currtrack;
		uintptr_t index;		

		currtrack = psy_ui_component_track(&self->component, ev->pt, &index);
		curr.offset = psy_max(0.0, self->state->cursorposition);
		if (index != psy_INDEX_INVALID) {
			curr.track = index;
		} else {
			curr.track = 0;
		}
		/*if (currtrack) {
			uintptr_t row;
			psy_ui_RealPoint pt;
			psy_audio_OrderIndex currindex;

			pt.y = 0;
			psy_ui_component_intersect(currtrack, ev->pt, &row);
			currindex = psy_audio_orderindex_make(curr.track, row);						
		}*/
		psy_audio_patternselection_drag(&self->selection, self->selectionbase, curr,
			0.25);
		seqeditortracks_selectdrag(self);
		psy_ui_component_invalidate(&self->component);
	}	
}

void seqeditortracks_selectdrag(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection selection;
	psy_List* t;
	psy_List* p;
	uintptr_t trackindex;

	if (!self->workspace->song) {
		return;
	}
	sequence = &self->workspace->song->sequence;
	psy_audio_sequenceselection_init(&selection);
	psy_audio_sequenceselection_setmode(&selection,
		psy_audio_SEQUENCESELECTION_MULTI);
	if (self->selection.topleft.offset == 0) {
		self = self;
	}
	for (t = sequence->tracks, trackindex = 0; t != NULL; t = t->next, ++trackindex) {
		psy_audio_SequenceTrack* track;
		psy_List* p;
		uintptr_t row;

		track = (psy_audio_SequenceTrack*)t->entry;
		for (p = track->entries, row = 0; p != NULL; p = p->next, ++row) {
			psy_audio_SequenceEntry* seqentry;
			
			seqentry = (psy_audio_SequenceEntry*)p->entry;						
			if (seqentry->offset >= self->selection.topleft.offset &&
					psy_audio_sequenceentry_rightoffset(seqentry) < self->selection.bottomright.offset &&
					trackindex >= self->selection.topleft.track && trackindex < self->selection.bottomright.track) {
				psy_audio_sequenceselection_append(&selection, psy_audio_orderindex_make(
					trackindex, row));
			}
		}
	}
	if (psy_list_size(selection.entries) > 1) {
		psy_audio_sequenceselection_setmode(&self->workspace->sequenceselection,
			psy_audio_SEQUENCESELECTION_MULTI);
	}		
	psy_audio_sequenceselection_deselectall(&self->workspace->sequenceselection);	
	psy_audio_sequenceselection_clear(&self->workspace->sequenceselection);
	for (p = selection.entries; p != NULL; p = p->next) {
		psy_audio_sequenceselection_select(&self->workspace->sequenceselection,
			*((psy_audio_OrderIndex*)p->entry));
	}
	psy_audio_sequenceselection_dispose(&selection);		
}

psy_ui_Component* psy_ui_component_track(psy_ui_Component* self, psy_ui_RealPoint pt, uintptr_t* index)
{
	psy_ui_Component* rv = 0;
	psy_List* p;
	psy_List* q;
	uintptr_t c = 0;
	double cp;

	p = q = psy_ui_component_children(self, 0);
	while (p) {
		psy_ui_RealRectangle position;
		psy_ui_Component* component;

		component = (psy_ui_Component*)p->entry;
		if (psy_ui_component_visible(component)) {
			position = psy_ui_component_position(component);
			cp = position.bottom;
			
			if (pt.y < cp) {
				rv = (psy_ui_Component*)p->entry;
				break;
			}
		}
		++c;
		p = p->next;
	}
	psy_list_free(q);
	if (rv) {
		*index = c;
	} else {
		*index = psy_INDEX_INVALID;
	}
	return rv;
}

void seqeditortracks_onmouseup(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	psy_audio_patternselection_disable(&self->selection);	
	self->state->dragselection = FALSE;
	psy_ui_component_invalidate(&self->component);
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
	/* quantize to lpb raster */
	seqeditorstate_setcursor(self->state,
		position * psy_audio_player_lpb(workspace_player(self->workspace)) /
		(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace)));
	psy_snprintf(text, 256, "Sequence Position %.3fb",
		(float)self->state->cursorposition);
	workspace_outputstatus(self->workspace, text);
}

/* SeqEditorHeaderBar */
/* implementation */
void seqeditorheaderbar_init(SeqEditorHeaderBar* self, psy_ui_Component* parent)
{	
	psy_ui_Margin spacing;

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	zoombox_init(&self->zoombox_beat, &self->component);	
	psy_ui_margin_init_em(&spacing, 0.0, 0.0, 1.0, 0.0);
	psy_ui_component_setspacing(&self->component, spacing);	
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(40.0, 2.0));
}

/* SeqEditToolBar */
static void seqeditortoolbar_setdragmode(SeqEditToolBar*, SeqEditorDragMode);
static void seqedittoolbar_oninserttypeselchange(SeqEditToolBar*,
	psy_ui_Component* sender, int index);
static void seqeditortoolbar_ondragmodemove(SeqEditToolBar*, psy_ui_Component* sender);
static void seqeditortoolbar_ondragmodereorder(SeqEditToolBar*, psy_ui_Component* sender);

/* implenentation */
void seqedittoolbar_init(SeqEditToolBar* self, psy_ui_Component* parent,
	SeqEditorState* state, Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setstyletype(&self->component, STYLE_SEQEDT_TOOLBAR);
	margin = psy_ui_defaults_hmargin(psy_ui_defaults());
	psy_ui_margin_setleft(&margin, psy_ui_value_make_ew(1.0));
	psy_ui_margin_setbottom(&margin, psy_ui_value_make_ew(0.5));
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		margin);
	self->workspace = workspace;
	self->state = state;
	psy_ui_button_init_text(&self->move, &self->component, NULL,
		"seqedit.move");
	psy_ui_button_init_text(&self->reorder, &self->component, NULL,
		"seqedit.reorder");
	seqeditortoolbar_setdragmode(self, self->state->dragmode);
	psy_ui_label_init_text(&self->desctype, seqedittoolbar_base(self), NULL,
		"Insert");	
	psy_ui_combobox_init(&self->inserttype, seqedittoolbar_base(self), NULL);
	psy_ui_combobox_setcharnumber(&self->inserttype, 12.0);
	psy_ui_combobox_addtext(&self->inserttype, "Pattern");
	psy_ui_combobox_addtext(&self->inserttype, "Marker");
	psy_ui_combobox_addtext(&self->inserttype, "Sample");
	psy_ui_combobox_setcursel(&self->inserttype, 0);
	/* expand */
	psy_ui_button_init(&self->expand, &self->component, NULL);
	psy_ui_button_loadresource(&self->expand, IDB_EXPAND_DARK,
		psy_ui_colour_white());
	psy_ui_component_setalign(psy_ui_button_base(&self->expand),
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_setspacing(psy_ui_button_base(&self->expand),
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_signal_connect(&self->inserttype.signal_selchanged, self,
		seqedittoolbar_oninserttypeselchange);
	psy_signal_connect(&self->move.signal_clicked, self,
		seqeditortoolbar_ondragmodemove);
	psy_signal_connect(&self->reorder.signal_clicked, self,
		seqeditortoolbar_ondragmodereorder);
}

void seqeditortoolbar_ondragmodemove(SeqEditToolBar* self, psy_ui_Component* sender)
{	
	seqeditortoolbar_setdragmode(self, SEQEDITORDRAG_MOVE);
}

void seqeditortoolbar_ondragmodereorder(SeqEditToolBar* self, psy_ui_Component* sender)
{
	
	seqeditortoolbar_setdragmode(self, SEQEDITORDRAG_REORDER);
}

void seqeditortoolbar_setdragmode(SeqEditToolBar* self, SeqEditorDragMode mode)
{
	self->state->dragmode = mode;
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


void seqedittoolbar_oninserttypeselchange(SeqEditToolBar* self, psy_ui_Component* sender,
	int index)
{
	switch (index) {
	case 0:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
		break;
	case 1:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_MARKER;
		break;
	case 2:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_SAMPLE;
		break;
	default:
		self->state->inserttype = psy_audio_SEQUENCEENTRY_PATTERN;
		break;
	}
}


/* SeqEditor */
/* prototypes */
static void seqeditor_ondestroy(SeqEditor*);
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_Song*);
static void seqeditor_updatesong(SeqEditor*, psy_audio_Song*);
static void seqeditor_build(SeqEditor*);
static void seqeditor_ontracksscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onzoomboxbeatchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_onzoomboxheightchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_updatescrollstep(SeqEditor*);
static void seqeditor_updateoverflow(SeqEditor*);
static void seqeditor_onsequenceclear(SeqEditor*, psy_audio_Sequence*);
static void seqeditor_onsequencetrackinsert(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_onsequencetrackremove(SeqEditor*, psy_audio_Sequence*,
	uintptr_t trackidx);
static void seqeditor_onsequencetrackswap(SeqEditor*, psy_audio_Sequence* sender,
	uintptr_t first, uintptr_t second);
static void seqeditor_onmouseup(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_onmousemove(SeqEditor*, psy_ui_MouseEvent*);
static void seqeditor_ontoggleexpand(SeqEditor*, psy_ui_Button* sender);
static void seqeditor_ontrackresize(SeqEditor*, psy_ui_Component* sender,
	uintptr_t trackid, double* height);
/* vtable */
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
	self->component.vtable = &seqeditor_vtable;
}
/* implementation */
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	PatternViewSkin* skin,
	Workspace* workspace)
{		
	psy_ui_Margin topmargin;	
	
	psy_ui_margin_init_em(&topmargin, 0.0, 0.0, 0.5, 0.0);
	psy_ui_component_init(&self->component, parent, NULL);
	seqeditor_vtable_init(self);	
	psy_ui_component_setstyletype(&self->component, STYLE_SEQEDT);	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_ui_edit_init(&self->edit, &self->component);
	psy_ui_component_hide(&self->edit.component);
	sequencecmds_init(&self->cmds, workspace);
	seqeditorstate_init(&self->state, workspace, &self->cmds, &self->edit);	
	self->workspace = workspace;
	/* toolbar */
	seqedittoolbar_init(&self->toolbar, &self->component, &self->state,
		self->workspace);
	psy_ui_component_setalign(&self->toolbar.component, psy_ui_ALIGN_TOP);	
	/* spacer */
	psy_ui_component_init(&self->spacer, &self->component, NULL);
	psy_ui_component_setstyletype(&self->spacer, STYLE_SEQEDT_SPACER);
	psy_ui_component_setalign(&self->spacer, psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->spacer,
		psy_ui_size_make_em(0.0, 0.25));
	/* left */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);	
	psy_ui_component_setstyletype(&self->left, STYLE_SEQEDT_LEFT);
	/* SeqEditorHeaderBar */
	seqeditorheaderbar_init(&self->headerbar, &self->left);
	psy_ui_component_setalign(&self->headerbar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->headerbar.zoombox_beat.signal_changed, self,
		seqeditor_onzoomboxbeatchanged);
	/* track description */
	psy_ui_component_init(&self->trackdescpane, &self->left, NULL);
	psy_ui_component_setalign(&self->trackdescpane, psy_ui_ALIGN_CLIENT);
	seqeditortrackdesc_init(&self->trackdescriptions, &self->trackdescpane,
		&self->state, workspace);
	psy_signal_connect(&self->trackdescriptions.signal_resize, self,
		seqeditor_ontrackresize);
	psy_ui_component_setalign(&self->trackdescriptions.component,
		psy_ui_ALIGN_HCLIENT);
	self->trackdescriptions.skin = skin;	
	zoombox_init(&self->zoombox_height, &self->left);
	psy_ui_component_setalign(&self->zoombox_height.component,
		psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->zoombox_height.signal_changed, self,
		seqeditor_onzoomboxheightchanged);
	/* header */
	psy_ui_component_init_align(&self->header, &self->component, NULL,
		psy_ui_ALIGN_TOP);
	/* ruler */
	psy_ui_component_init_align(&self->rulerpane, &self->header, NULL,
		psy_ui_ALIGN_CLIENT);
	seqeditorruler_init(&self->ruler, &self->rulerpane, &self->state,
		skin, workspace);
	psy_ui_component_setalign(&self->ruler.component, psy_ui_ALIGN_FIXED_RESIZE);
	/* connect expand */
	psy_signal_connect(&self->toolbar.expand.signal_clicked, self,
		seqeditor_ontoggleexpand);
	self->expanded = FALSE;
	/* tracks */
	seqeditortracks_init(&self->tracks, &self->component,		
		&self->state, skin, workspace);
	self->tracks.skin = skin;	
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component, NULL);	
	psy_ui_component_setalign(&self->tracks.component,
		psy_ui_ALIGN_FIXED_RESIZE);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	seqeditortoolbar_setdragmode(&self->toolbar, self->state.dragmode);	
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_make_ew(20.0),
			psy_ui_value_make_ph(0.3)));	
	seqeditor_updatesong(self, workspace->song);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_ontracksscroll);	
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);	
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
		psy_signal_connect(&song->sequence.signal_trackinsert, self,
			seqeditor_onsequencetrackinsert);
		psy_signal_connect(&song->sequence.signal_trackswap, self,
			seqeditor_onsequencetrackswap);
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
	psy_ui_component_setscrollstep_height(&self->tracks.component,
		psy_ui_add_values(
			self->tracks.state->lineheight,
			self->tracks.state->linemargin,
			tm, NULL));
	psy_ui_component_setscrollstep_height(&self->tracks.component,
		psy_ui_value_make_px(floor(
			psy_ui_component_scrollstep_height_px(&self->tracks.component))));
	psy_ui_component_setscrollstep_height(&self->trackdescriptions.component,
		psy_ui_component_scrollstep_height(&self->tracks.component));
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

void seqeditor_onsequencetrackinsert(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t trackidx)
{
	seqeditor_build(self);	
}

void seqeditor_onsequencetrackswap(SeqEditor* self, psy_audio_Sequence* sender,
	uintptr_t first, uintptr_t second)
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
	psy_ui_component_setscrolltop(&self->trackdescriptions.component,
		psy_ui_component_scrolltop(&self->tracks.component));
	psy_ui_component_setscrollleft(&self->ruler.component,
		psy_ui_component_scrollleft(&self->tracks.component));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{	
	if (generalconfig_showingpatternnames(sender)) {
		self->state.showpatternnames = TRUE;
	} else {
		self->state.showpatternnames = FALSE;
	}	
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onzoomboxbeatchanged(SeqEditor* self, ZoomBox* sender)
{
	self->state.pxperbeat = sender->zoomrate * DEFAULT_PXPERBEAT;
	psy_ui_component_align(&self->tracks.component);
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
	psy_ui_component_invalidate(&self->ruler.component);
}

void seqeditor_onzoomboxheightchanged(SeqEditor* self, ZoomBox* sender)
{
	self->state.lineheight = psy_ui_mul_value_real(
		self->state.defaultlineheight, zoombox_rate(sender));
	psy_ui_component_align(&self->tracks.component);	
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
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
		sequencecmds_appendtrack(&self->cmds);			
	} else if (self->state.cmd == SEQEDTCMD_DELTRACK) {
		sequencecmds_deltrack(&self->cmds, self->state.cmdtrack);		
	} else if (self->state.cmd == SEQEDTCMD_INSERTPATTERN) {
		sequencecmds_newentry(&self->cmds, self->state.inserttype);
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

void seqeditor_ontoggleexpand(SeqEditor* self, psy_ui_Button* sender)
{
	psy_ui_RealRectangle position;
	position = psy_ui_component_position(&self->component);

	self->expanded = !self->expanded;
	if (self->expanded) {
		psy_ui_component_setpreferredsize(&self->component,
			psy_ui_size_make_px(0.0, position.bottom * 3 / 4.0 - 20));
	} else {
		psy_ui_component_setpreferredsize(&self->component,
			psy_ui_size_make_px(0.0, position.bottom * 1 / 4.0 - 20));
	}
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void seqeditor_ontrackresize(SeqEditor* self, psy_ui_Component* sender,
	uintptr_t trackid, double* height)
{
	psy_ui_Component* track;
	
	assert(height);

	track = psy_ui_component_at(&self->tracks.component, trackid);
	if (track) {		
		psy_ui_component_setpreferredheight(track,
			psy_ui_value_make_eh(*height));
		psy_ui_component_align(&self->scroller.pane);		
	}
}
