/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerlinenumbers.h"
/* local */
#include "patternview.h"
/* std */
#include <assert.h>
#include <math.h>
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"

static int testcursor(psy_audio_SequenceCursor cursor, TrackerState* state,
	psy_dsp_big_beat_t offset)
{	
	intptr_t currline;
	intptr_t cursorline;

	currline = patternviewstate_beattoline(state->pv, offset);
	cursorline = patternviewstate_beattoline(state->pv,
		psy_audio_sequencecursor_offset(&cursor));
	return (cursorline == currline);
}

/* TrackerLineNumbers */
/* prototypes */
static void trackerlinenumbers_ondraw(TrackerLineNumbers*, psy_ui_Graphics*);
static void trackerlinenumbers_onpreferredsize(TrackerLineNumbers*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void trackerlinenumbers_onsongchanged(TrackerLineNumbers*,
	Workspace* sender);
static void trackerlinennumbers_oncursorchanged(TrackerLineNumbers*,
	psy_audio_Sequence*);
static void trackerlinenumbers_onscroll(TrackerLineNumbers*, psy_ui_Component* sender);
static void trackerlinennumbers_linetext(TrackerLineNumbers*, bool hex,
	bool seqstart, bool drawbeat, int patidx, int line, float offset,
	char* rv);
static TrackerColumnFlags trackerlinennumbers_columnflags(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t seqoffset, psy_audio_SequenceCursor);
static void trackerlinennumbers_drawtext(TrackerLineNumbers*, psy_ui_Graphics*,
	const char* format, double y, double width, const char* text);
/* vtable */
static psy_ui_ComponentVtable trackerlinenumbers_vtable;
static int trackerlinenumbers_vtable_initialized = 0;

static void trackerlinenumbers_vtable_init(TrackerLineNumbers* self)
{
	if (!trackerlinenumbers_vtable_initialized) {
		trackerlinenumbers_vtable = *(self->component.vtable);
		trackerlinenumbers_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackerlinenumbers_ondraw;
		trackerlinenumbers_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			trackerlinenumbers_onpreferredsize;
		trackerlinenumbers_vtable_initialized = 1;
	}
}

/* implementation */
void trackerlinenumbers_init(TrackerLineNumbers* self, psy_ui_Component* parent,
	TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	trackerlinenumbers_vtable_init(self);
	self->component.vtable = &trackerlinenumbers_vtable;
	self->workspace = workspace;
	self->state = state;
	self->showcursor = TRUE;
	self->shownumbersinhex = TRUE;
	self->showbeat = FALSE;
	self->state = state;	
	psy_signal_connect(&self->component.signal_scroll, self,
		trackerlinenumbers_onscroll);
	if (self->workspace->song) {
		psy_signal_connect(&self->workspace->song->sequence.signal_cursorchanged, self,
			trackerlinennumbers_oncursorchanged);
	}
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerlinenumbers_onsongchanged);
	psy_audio_sequencecursor_init(&self->lastcursor);
	self->lastcursor.orderindex = psy_audio_orderindex_make(0, 0);
	psy_ui_component_setscrollstep_height(&self->component,
		state->lineheight);
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, psy_ui_Graphics* g)
{
	if (self->state->pv->pattern) {
		psy_ui_RealSize size;		
		double cpy;
		intptr_t line;
		double offset;		
		psy_audio_SequenceCursor cursor;		
		psy_audio_SequenceTrackIterator ite;
		double seqoffset;		
		double length;
		uintptr_t maxlines;
		uintptr_t patidx;
		psy_audio_BlockSelection clip;
		psy_audio_Sequence* sequence;
		
		assert(self);
		
		sequence = patternviewstate_sequence(self->state->pv);
		trackerstate_lineclip(self->state, &g->clip, &clip);		
		offset = clip.topleft.offset;
		line = patternviewstate_beattoline(self->state->pv,
			clip.topleft.offset);
		cpy = trackerstate_beattopx(self->state, offset);
		size = psy_ui_component_scrollsize_px(&self->component);				
		cursor = self->workspace->song->sequence.cursor;
		ite.pattern = self->state->pv->pattern;
		ite.patternnode = NULL;
		ite.patterns = &self->workspace->song->patterns;
		seqoffset = 0.0;		
		patidx = 0;
		length = ite.pattern->length;		
		if (!self->state->pv->singlemode && sequence) {
			psy_audio_SequenceTrackNode* tracknode;

			tracknode = psy_list_at(sequence->tracks,
				self->state->pv->cursor.orderindex.track);
			if (!tracknode) {
				tracknode = sequence->tracks;
			}
			ite = psy_audio_sequence_begin(sequence,
				tracknode ? (psy_audio_SequenceTrack*)tracknode->entry : NULL,
				offset);
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);			
			patidx = psy_audio_sequencetrackiterator_patidx(&ite);
			if (ite.pattern) {
				length = ite.pattern->length;			
			}
		} else {
			ite.sequencentrynode = NULL;			
		}
		maxlines = patternviewstate_numlines(self->state->pv);
		while (offset <= clip.bottomright.offset && line < (intptr_t)maxlines) {
			double ystart;			
			char text[64];
			psy_ui_Colour bg;
			psy_ui_Colour fore;
				
			trackerlinennumbers_linetext(self,
				self->shownumbersinhex,
				!self->state->pv->singlemode && seqoffset == offset,
				self->showbeat,
				(int)patidx, (int)line, (float)offset, text);
			trackerstate_columncolours(self->state,
				trackerlinennumbers_columnflags(self, offset, seqoffset,
				cursor), 0, &bg, &fore);
			psy_ui_setbackgroundcolour(g, bg);
			psy_ui_settextcolour(g, fore);			
			trackerlinennumbers_drawtext(self, g, text, cpy, size.width, text);					
			cpy += self->state->lineheightpx;
			ystart = cpy;
			offset += patternviewstate_bpl(self->state->pv);
			if (offset >= seqoffset + length) {
				if (ite.sequencentrynode && ite.sequencentrynode->next) {
					psy_audio_sequencetrackiterator_inc_entry(&ite);					
					seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
					patidx = psy_audio_sequencetrackiterator_patidx(&ite);
					offset = seqoffset;
					if (ite.pattern) {
						length = ite.pattern->length;
					} else {
						break;
					}
				} else {
					break;
				}
			}
			++line;
		}
	}
}

void trackerlinennumbers_linetext(TrackerLineNumbers* self, bool hex,
	bool seqstart, bool drawbeat, int patidx, int line, float offset,
	char* rv)
{
	char* format;

	if (hex) {
		if (drawbeat) {
			if (seqstart) {
				format = "%.2X %.2X %.3f";
			} else {
				format = "%X %.3f";
			}
		} else {			
			if (seqstart) {
				format = "%.2X %.2X";
			} else {
				format = "%X";
			}
		}
	} else {
		if (drawbeat) {
			if (seqstart) {
				format = "%X %i %.3f";
			} else {
				format = "%i %.3f";
			}
		} else {
			if (seqstart) {
				format = "%X %i";
			} else {
				format = "%i";
			}
		}
	}
	if (seqstart) {
		psy_snprintf(rv, 64, format, patidx, line, offset);
	} else {
		psy_snprintf(rv, 64, format, line, offset);
	}
}

TrackerColumnFlags trackerlinennumbers_columnflags(TrackerLineNumbers* self,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t seqoffset,
	psy_audio_SequenceCursor cursor)
{
	TrackerColumnFlags rv;

	rv.playbar = psy_audio_player_playing(workspace_player(self->workspace)) &&
		trackerstate_testplaybar(self->state, self->workspace->currplayposition, offset);
	rv.mid = 0;
	rv.cursor = self->showcursor &&
		self->state->drawcursor && !self->state->cursorchanging &&
		testcursor(cursor, self->state, offset);
	rv.beat = fmod(offset - seqoffset, 1.0f) == 0.0f;
	rv.beat4 = fmod(offset - seqoffset, 4.0f) == 0.0f;
	rv.selection = 0;
	return rv;
}

void trackerlinennumbers_drawtext(TrackerLineNumbers* self, psy_ui_Graphics* g,
	const char* format, double y, double width, const char* text)
{
	uintptr_t numdigits;
	uintptr_t maxdigits;
	uintptr_t startdigit;
	double blankspace;
	char digit[2];
	psy_ui_RealRectangle r;
	uintptr_t c;

	digit[1] = '\0';
	numdigits = psy_strlen(text);
	maxdigits = (uintptr_t)((width - 4) / self->state->flatsize);
	startdigit = maxdigits - numdigits - 1;
	for (c = 0; c < maxdigits; ++c) {
		if (c >= startdigit && c < startdigit + numdigits) {
			digit[0] = text[c - startdigit];
		} else {
			digit[0] = ' ';
		}
		r = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(c * self->state->flatsize, y),
			psy_ui_realsize_make(self->state->flatsize,
				self->state->lineheightpx - 1));
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
			digit, psy_strlen(digit));
	}
	r.left += self->state->flatsize;
	blankspace = (width - r.left) - 4;
	if (blankspace > 0) {
		r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(r.left, y),
			psy_ui_realsize_make(
				blankspace, self->state->lineheightpx - 1));
		digit[0] = ' ';
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
			digit, psy_strlen(digit));
	}
}

void trackerlinenumbers_onsongchanged(TrackerLineNumbers* self,
	Workspace* sender)
{
	if (sender->song) {
		psy_signal_connect(&sender->song->sequence.signal_cursorchanged, self,
			trackerlinennumbers_oncursorchanged);
	}
}


void trackerlinennumbers_oncursorchanged(TrackerLineNumbers* self,
	psy_audio_Sequence* sender)
{
	psy_audio_SequenceCursor currcursor;

	currcursor = sender->cursor;
	if (self->lastcursor.offset != currcursor.offset) {
		if (!self->state->cursorchanging) {
			trackerlinenumbers_invalidatecursor(self, &self->lastcursor);
		}
		self->state->cursorchanging = FALSE;
		trackerlinenumbers_invalidatecursor(self, &currcursor);		
	}
	self->lastcursor = currcursor;
}

void trackerlinenumbers_onscroll(TrackerLineNumbers* self, psy_ui_Component* sender)
{	
	if (self->state->cursorchanging) {
		trackerlinenumbers_invalidatecursor(self, &self->lastcursor);
	}
}

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers* self,
	const psy_audio_SequenceCursor* cursor)
{	
	psy_ui_RealSize size;
	intptr_t line;
		
	size = psy_ui_component_scrollsize_px(&self->component);
	line = patternviewstate_beattoline(self->state->pv,
		psy_audio_sequencecursor_offset(cursor));		
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, self->state->lineheightpx * line),
			psy_ui_realsize_make(size.width, self->state->lineheightpx)));
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, intptr_t line)
{				
	psy_ui_RealSize size;
	
	size = psy_ui_component_scrollsize_px(&self->component);		
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, self->state->lineheightpx *
				(line - patternviewstate_seqstartline(self->state->pv))),
			psy_ui_realsize_make(
				size.width, self->state->lineheightpx)));	
}

void trackerlinenumbers_onpreferredsize(TrackerLineNumbers* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	static int margin = 4;
	uintptr_t numcols;
	
	numcols = 6;
	if (self->showbeat) {
		numcols += 8;
	}
	if (!self->state->pv->singlemode) {
		numcols += 2;
	}
	rv->width = psy_ui_value_make_px(self->state->flatsize * numcols +
		margin);
	rv->height = psy_ui_value_make_px(
		patternviewstate_numlines(self->state->pv) *
		trackerstate_lineheight(self->state));
}

void trackerlinenumbers_showlinenumbercursor(TrackerLineNumbers* self,
	bool showstate)
{
	self->showcursor = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers* self,
	bool showstate)
{
	self->shownumbersinhex = showstate;
	psy_ui_component_invalidate(&self->component);
}

/* LineNumbersLabel */
/* prototypes */
static void trackerlinenumberslabel_ondestroy(TrackerLineNumbersLabel*);
static void trackerlinenumberslabel_updatetext(TrackerLineNumbersLabel*);
static void trackerlinenumberslabel_onlanguagechanged(TrackerLineNumbersLabel*);
static void trackerlinenumberslabel_onmousedown(TrackerLineNumbersLabel*,
	psy_ui_MouseEvent*);
static void trackerlinenumberslabel_ondraw(TrackerLineNumbersLabel*, psy_ui_Graphics*);
static void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel*,
	psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable trackerlinenumberslabel_vtable;
static int trackerlinenumberslabel_vtable_initialized = 0;

static void trackerlinenumberslabel_vtable_init(TrackerLineNumbersLabel* self)
{
	if (!trackerlinenumberslabel_vtable_initialized) {
		trackerlinenumberslabel_vtable = *(self->component.vtable);
		trackerlinenumberslabel_vtable.ondestroy =
			(psy_ui_fp_component_event)
			trackerlinenumberslabel_ondestroy;
		trackerlinenumberslabel_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackerlinenumberslabel_ondraw;
		trackerlinenumberslabel_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackerlinenumberslabel_onmousedown;
		trackerlinenumberslabel_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			trackerlinenumberslabel_onpreferredsize;
		trackerlinenumberslabel_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			trackerlinenumberslabel_onlanguagechanged;
		trackerlinenumberslabel_vtable_initialized = TRUE;
	}
	self->component.vtable = &trackerlinenumberslabel_vtable;
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, psy_ui_Component* view,
	TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	trackerlinenumberslabel_vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);	
	self->state = state;
	self->linestr = NULL;
	self->defaultstr = NULL;
	self->workspace = workspace;
	self->headerheight = 12.0;
	self->showdefaultline = TRUE;
	self->showbeatoffset = FALSE;	
	trackerlinenumberslabel_updatetext(self);	
}

void trackerlinenumberslabel_ondestroy(TrackerLineNumbersLabel* self)
{
	free(self->linestr);
	self->linestr = NULL;
	free(self->defaultstr);
	self->defaultstr = NULL;
}

void trackerlinenumberslabel_updatetext(TrackerLineNumbersLabel* self)
{
	psy_strreset(&self->linestr, psy_ui_translate("patternview.line"));
	psy_strreset(&self->defaultstr, psy_ui_translate("patternview.defaults"));
}

void trackerlinenumberslabel_onlanguagechanged(TrackerLineNumbersLabel* self)
{
	trackerlinenumberslabel_updatetext(self);
}

void trackerlinenumberslabel_onmousedown(TrackerLineNumbersLabel* self,
	psy_ui_MouseEvent* ev)
{
/*	self->view->header.classic = !self->view->header.classic;
	if (self->view->header.classic) {
		trackerview_setclassicheadercoords(self->view);
	} else {
		trackerview_setheadercoords(self->view);
	}*/
	//psy_ui_component_invalidate(&self->view->header.component);
}

void trackerlinenumberslabel_ondraw(TrackerLineNumbersLabel* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	psy_ui_RealRectangle r;		
	
	size = psy_ui_component_scrollsize_px(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, patternviewstate_skin(self->state->pv)->background);
	psy_ui_setbackgroundcolour(g, patternviewstate_skin(self->state->pv)->background);
	psy_ui_settextcolour(g, patternviewstate_skin(self->state->pv)->font);
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(r.left, 0), 0, r, self->linestr, psy_strlen(self->linestr));
	if (self->showdefaultline) {		
		if ((self->showbeatoffset)) {
			psy_ui_textoutrectangle(g, psy_ui_realpoint_make(r.left, self->headerheight), 0,
				r, self->defaultstr, psy_strlen(self->defaultstr));
		} else {			
			psy_ui_textoutrectangle(g, psy_ui_realpoint_make(r.left, self->headerheight), 0,
				r, "Def", psy_strlen("Def"));
		}
	}
}

void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	double height;
	
	height = self->headerheight;	
	if (self->showdefaultline) {		
		height += self->state->lineheightpx;
	}	
	rv->height = psy_ui_value_make_px(height);
	rv->width = (self->showbeatoffset)
		? psy_ui_value_make_ew(10.0)
		: psy_ui_value_make_ew(0.0);
}

/* TrackerLineNumberBar */
/* prototypes */
static void trackerlinenumberbar_onplaylinechanged(TrackerLineNumberBar*,
	Workspace* sender);
static void trackerlinenumberbar_onconfigurepatternview(TrackerLineNumberBar*,
	PatternViewConfig*, psy_Property*);
static void trackerlinenumberbar_onsongchanged(TrackerLineNumberBar*,
	Workspace* sender);
static void trackerlinenumberbar_connectsong(TrackerLineNumberBar*);
static void trackerlinenumberbar_oncursorchanged(TrackerLineNumberBar*,
	psy_audio_Sequence* sender);

/* implementation */
void trackerlinenumberbar_init(TrackerLineNumberBar* self, psy_ui_Component* parent,
	psy_ui_Component* view, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	self->workspace = workspace;
	psy_ui_component_setbackgroundmode(&self->component, psy_ui_NOBACKGROUND);	
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component, NULL,
		state, workspace);
	psy_ui_component_setalign(&self->linenumberslabel.component, psy_ui_ALIGN_TOP);
	/* scrollpane */
	psy_ui_component_init(&self->linenumberpane, &self->component, NULL);
	psy_ui_component_setalign(&self->linenumberpane, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_setbackgroundcolour(&self->component,
		patternviewstate_skin(state->pv)->background);
	/* linenumbers */
	trackerlinenumbers_init(&self->linenumbers, &self->linenumberpane, state,
		workspace);
	psy_ui_component_setalign(&self->linenumbers.component, psy_ui_ALIGN_FIXED);	
	psy_signal_connect(&workspace->signal_playlinechanged, self,
		trackerlinenumberbar_onplaylinechanged);
	psy_signal_connect(&psycleconfig_patview(
		workspace_conf(workspace))->signal_changed,
		self, trackerlinenumberbar_onconfigurepatternview);
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerlinenumberbar_onsongchanged);
	trackerlinenumberbar_connectsong(self);
}

void trackerlinenumberbar_onplaylinechanged(TrackerLineNumberBar* self,
	Workspace* sender)
{
	if (!workspace_followingsong(sender) && psy_ui_component_drawvisible(
			&self->component)) {				
		trackerlinenumbers_invalidateline(&self->linenumbers,
			sender->lastplayline);
		trackerlinenumbers_invalidateline(&self->linenumbers,
			sender->currplayline);
	}
}

void trackerlinenumberbar_onconfigurepatternview(TrackerLineNumberBar* self,
	PatternViewConfig* config, psy_Property* property)
{	
	if (patternviewconfig_linenumbers(config)) {
		psy_ui_component_show_align(&self->component);
	} else {
		psy_ui_component_hide_align(&self->component);
	}
	if (patternviewconfig_showbeatoffset(config)) {
		self->linenumbers.showbeat = TRUE;
		trackerlinenumberslabel_showbeatoffset(&self->linenumberslabel);
	} else {
		self->linenumbers.showbeat = FALSE;
		trackerlinenumberslabel_hidebeatoffset(&self->linenumberslabel);
	}	
	trackerlinenumbers_showlinenumbercursor(&self->linenumbers,
		patternviewconfig_linenumberscursor(config));
	trackerlinenumbers_showlinenumbersinhex(&self->linenumbers,
		patternviewconfig_linenumbersinhex(config));	
	if (patternviewconfig_defaultline(config)) {
		trackerlinenumberslabel_showdefaultline(
			&self->linenumberslabel);
	} else {
		trackerlinenumberslabel_hidedefaultline(
			&self->linenumberslabel);
	}
	psy_ui_component_align(trackerlinenumberbar_base(self));
}

void trackerlinenumberbar_onsongchanged(TrackerLineNumberBar* self, Workspace* sender)
{
	trackerlinenumberbar_connectsong(self);
}

void trackerlinenumberbar_connectsong(TrackerLineNumberBar* self)
{
	if (self->workspace->song) {
		psy_signal_connect(
			&self->workspace->song->sequence.signal_cursorchanged, self,
			trackerlinenumberbar_oncursorchanged);
	}
}

void trackerlinenumberbar_oncursorchanged(TrackerLineNumberBar* self,
	psy_audio_Sequence* sender)
{
	assert(self);
	
	/* lpb changed? */
	if (sender->cursor.lpb != sender->lastcursor.lpb) {
		psy_ui_component_align(&self->linenumberpane);
		psy_ui_component_invalidate(trackerlinenumbers_base(
			&self->linenumbers));
		return;
	}
	if (!psy_audio_sequencecursor_equal(&sender->cursor, &sender->lastcursor)) {
		if (psy_audio_player_playing(&self->workspace->player) &&
				workspace_followingsong(self->workspace)) {
			trackerlinenumberbar_updatefollowsong(self);
		}
	}
}

void trackerlinenumberbar_updatefollowsong(TrackerLineNumberBar* self)
{
	psy_dsp_big_beat_t lastposition;
	psy_dsp_big_beat_t currposition;
	intptr_t lastline;
	intptr_t currline;

	psy_audio_Sequence* sequence;

	sequence = patternviewstate_sequence(self->linenumbers.state->pv);
	if (!sequence) {
		return;
	}
	lastposition = psy_audio_sequencecursor_offset_abs(&sequence->lastcursor);
	currposition = psy_audio_sequencecursor_offset_abs(&sequence->cursor);
	lastline = patternviewstate_beattoline(self->linenumbers.state->pv, lastposition);
	currline = patternviewstate_beattoline(self->linenumbers.state->pv, currposition);	
	trackerlinenumbers_invalidateline(&self->linenumbers, lastline);
	trackerlinenumbers_invalidateline(&self->linenumbers, currline);
}
