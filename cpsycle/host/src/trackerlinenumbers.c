/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackerlinenumbers.h"
/* local */
#include "patternview.h"
/* platform */
#include "../../detail/portable.h"

/* TrackerLineNumbers */

/* prototypes */
static void trackerlinenumbers_ondraw(TrackerLineNumbers*, psy_ui_Graphics*);
static void trackerlinenumbers_onpreferredsize(TrackerLineNumbers*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static TrackerColumnFlags trackerlinennumbers_columnflags(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset, intptr_t line, intptr_t seqline);
static void trackerlinennumbers_drawtext(TrackerLineNumbers*, psy_ui_Graphics*,
	const char* format, double y, double width, const char* text);

/* vtable */
static psy_ui_ComponentVtable trackerlinenumbers_vtable;
static bool trackerlinenumbers_vtable_initialized = FALSE;

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
		trackerlinenumbers_vtable_initialized = TRUE;
	}
	self->component.vtable = &trackerlinenumbers_vtable;
}

/* implementation */
void trackerlinenumbers_init(TrackerLineNumbers* self,
	psy_ui_Component* parent, TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerlinenumbers_vtable_init(self);	
	self->state = state;	
	self->shownumbersinhex = TRUE;
	self->showbeat = FALSE;
	self->draw_linenumber_cursor = TRUE;
	self->prevent_cursor = FALSE;
	psy_audio_sequencecursor_init(&self->oldcursor);
	trackerlinenumbers_updateformat(self);	
	self->state = state;
	self->workspace = workspace;
	psy_ui_component_setscrollstep_height(&self->component,
		state->lineheight);
}

void trackerlinenumbers_updateformat(TrackerLineNumbers* self)
{
	static const char* format_hex_beat_seqstart = "%.2X %.2X %.3f";
	static const char* format_hex_beat = "%X %.3f";
	static const char* format_hex_seqstart = "%.2X %.2X";
	static const char* format_hex = "%X";

	static const char* format_beat_seqstart = "%X %i %.3f";
	static const char* format_beat = "%i %.3f";
	static const char* format_seqstart = "%X %i";
	static const char* format = "%i";

	if (self->shownumbersinhex) {
		if (self->showbeat) {
			self->format = format_hex_beat;
			self->format_seqstart = format_hex_beat_seqstart;
		} else {
			self->format = format_hex;
			self->format_seqstart = format_hex_seqstart;			
		}
	} else {
		if (self->showbeat) {
			self->format = format_beat;
			self->format_seqstart = format_beat_seqstart;
		} else {
			self->format = format;
			self->format_seqstart = format_seqstart;
		}
	}
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, psy_ui_Graphics* g)
{
	if (patternviewstate_pattern(self->state->pv)) {
		psy_ui_RealSize size;
		psy_ui_RealRectangle g_clip;
		double cpy;
		intptr_t line;
		intptr_t seqline;
		double offset;				
		double seqoffset;
		double length;		
		uintptr_t maxlines;
		uintptr_t patidx;
		psy_audio_BlockSelection clip;
		psy_audio_SequenceTrackIterator ite;
		
		assert(self);
				
		size = psy_ui_component_scrollsize_px(&self->component);
		g_clip = psy_ui_cliprect(g);
		trackerstate_lineclip(self->state, &g_clip, &clip);
		offset = clip.topleft.offset;		
		cpy = trackerstate_beattopx(self->state, offset);
		psy_audio_sequencetrackiterator_init(&ite);
		patternviewstate_sequencestart(self->state->pv, offset, &ite);		
		if (ite.pattern) {
			length = ite.pattern->length;
		} else {
			length = 0.0;
		}
		patidx = psy_audio_sequencetrackiterator_patidx(&ite);
		seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
		line = patternviewstate_beattoline(self->state->pv, offset);
		seqline = patternviewstate_beattoline(self->state->pv, seqoffset);
		maxlines = patternviewstate_numlines(self->state->pv);
		while (offset <= clip.bottomright.offset && line < (intptr_t)maxlines) {
			double ystart;			
			char text[64];
			psy_ui_Colour bg;
			psy_ui_Colour fore;
				
			if (!self->state->pv->singlemode && seqoffset == offset) {
				psy_snprintf(text, 64, self->format_seqstart, (int)patidx,
					(int)line, (float)offset);
			} else {
				psy_snprintf(text, 64, self->format, (int)line, (float)offset);
			}			
			trackerstate_columncolours(self->state,
				trackerlinennumbers_columnflags(self, offset, line,
					seqline), 0, &bg, &fore);
			psy_ui_setbackgroundcolour(g, bg);
			psy_ui_settextcolour(g, fore);			
			trackerlinennumbers_drawtext(self, g, text, cpy, size.width, text);					
			cpy += self->state->lineheightpx;
			ystart = cpy;
			offset += patternviewstate_bpl(self->state->pv);
			if (offset >= seqoffset + length) {
				if (ite.sequencentrynode && ite.sequencentrynode->next) {
					psy_audio_sequencetrackiterator_inc_entry(&ite);
					if (!ite.pattern) {
						break;
					}
					seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);
					seqline = patternviewstate_beattoline(self->state->pv, seqoffset);
					patidx = psy_audio_sequencetrackiterator_patidx(&ite);
					offset = seqoffset;					
					length = ite.pattern->length;					
				} else {
					break;
				}
			}
			++line;
		}
		psy_audio_sequencetrackiterator_dispose(&ite);
	}
}

TrackerColumnFlags trackerlinennumbers_columnflags(TrackerLineNumbers* self,
	psy_dsp_big_beat_t offset, intptr_t line, intptr_t seqline)
{
	TrackerColumnFlags rv;	
	rv.playbar =
		psy_audio_player_playing(workspace_player(self->workspace)) &&
		trackerstate_testplaybar(self->state,
			self->workspace->host_sequencer_time.currplayposition, offset);
	rv.mid = 0;
	rv.cursor = self->draw_linenumber_cursor && !self->prevent_cursor &&
		(psy_audio_sequencecursor_line(&self->state->pv->cursor) == line);
	if (!self->state->pv->singlemode) {
		line -= seqline;
	}
	rv.beat = (line % self->state->pv->cursor.lpb) == 0;
	rv.beat4 = (line % (self->state->pv->cursor.lpb * 4)) == 0;
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
	self->oldcursor = *cursor;
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, intptr_t line)
{				
	psy_ui_RealSize size;
	
	size = psy_ui_component_scrollsize_px(&self->component);		
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, self->state->lineheightpx *
				(line - patternviewstate_seqstartline(self->state->pv))),
			psy_ui_realsize_make(size.width, self->state->lineheightpx)));
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
	self->draw_linenumber_cursor = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers* self,
	bool showstate)
{
	self->shownumbersinhex = showstate;
	trackerlinenumbers_updateformat(self);
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_updatecursor(TrackerLineNumbers* self)
{
	psy_audio_Sequence* sequence;

	sequence = patternviewstate_sequence(self->state->pv);
	if (sequence) {
		trackerlinenumbers_invalidateline(self,
			psy_audio_sequencecursor_line_abs(&sequence->lastcursor));
		trackerlinenumbers_invalidateline(self,
			psy_audio_sequencecursor_line_abs(&sequence->cursor));
		self->oldcursor = sequence->cursor;
	}
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

/* vtable */
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

/* implementation */
void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerState* state)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerlinenumberslabel_vtable_init(self);	
	self->state = state;
	self->linestr = NULL;
	self->defaultstr = NULL;	
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
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0, 0),
		psy_ui_realsize_make(size.width, size.height));
	// psy_ui_drawsolidrectangle(g, r, patternviewstate_skin(self->state->pv)->background);
	// psy_ui_setbackgroundcolour(g, patternviewstate_skin(self->state->pv)->background);
	// psy_ui_settextcolour(g, patternviewstate_skin(self->state->pv)->font);
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(r.left, 0), 0, r,
		self->linestr, psy_strlen(self->linestr));
	if (self->showdefaultline) {		
		if ((self->showbeatoffset)) {
			psy_ui_textoutrectangle(g,
				psy_ui_realpoint_make(r.left, self->headerheight), 0,
				r, self->defaultstr, psy_strlen(self->defaultstr));
		} else {			
			psy_ui_textoutrectangle(g,
				psy_ui_realpoint_make(r.left, self->headerheight), 0,
				r, "Def", psy_strlen("Def"));
		}
	}
}

void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{		
	rv->height = (self->showdefaultline)
		? psy_ui_value_make_px(self->headerheight + self->state->lineheightpx)
		: psy_ui_value_make_px(self->headerheight);	
	rv->width = (self->showbeatoffset)
		? psy_ui_value_make_ew(10.0)
		: psy_ui_value_make_ew(0.0);
}

/* TrackerLineNumberBar */
/* prototypes */
static void trackerlinenumberbar_onconfigurepatternview(TrackerLineNumberBar*,
	PatternViewConfig*, psy_Property*);
static void trackerlinenumberbar_onplaylinechanged(TrackerLineNumberBar*,
	Workspace* sender);
static void trackerlinenumberbar_onplaystatuschanged(TrackerLineNumberBar*,
	Workspace* sender);
static void trackerlinenumberbar_oncursorchanged(TrackerLineNumberBar*,
	psy_audio_Sequence* sender);
static void trackerlinenumberbar_onsongchanged(TrackerLineNumberBar*,
	Workspace* sender);
static void trackerlinenumberbar_connectsong(TrackerLineNumberBar*);

/* implementation */
void trackerlinenumberbar_init(TrackerLineNumberBar* self, psy_ui_Component* parent,
	TrackerState* state, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;	
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component,
		state);
	psy_ui_component_setalign(&self->linenumberslabel.component, psy_ui_ALIGN_TOP);
	/* scrollpane */
	psy_ui_component_init(&self->linenumberpane, &self->component, NULL);
	psy_ui_component_setalign(&self->linenumberpane, psy_ui_ALIGN_CLIENT);	
	// psy_ui_component_setbackgroundcolour(&self->component,
	//	patternviewstate_skin(state->pv)->background);
	/* linenumbers */
	trackerlinenumbers_init(&self->linenumbers, &self->linenumberpane, state,
		workspace);
	psy_ui_component_setalign(&self->linenumbers.component, psy_ui_ALIGN_FIXED);	
	psy_signal_connect(&workspace->signal_playlinechanged, self,
		trackerlinenumberbar_onplaylinechanged);
	psy_signal_connect(&workspace->signal_playstatuschanged, self,
		trackerlinenumberbar_onplaystatuschanged);
	psy_signal_connect(&psycleconfig_patview(
		workspace_conf(workspace))->signal_changed,
		self, trackerlinenumberbar_onconfigurepatternview);
	psy_signal_connect(&workspace->signal_songchanged, self,
		trackerlinenumberbar_onsongchanged);
	trackerlinenumberbar_connectsong(self);
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
		trackerlinenumbers_showbeat(&self->linenumbers);		
		trackerlinenumberslabel_showbeat(&self->linenumberslabel);
	} else {
		trackerlinenumbers_hidebeat(&self->linenumbers);		
		trackerlinenumberslabel_hidebeat(&self->linenumberslabel);
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

void trackerlinenumberbar_onsongchanged(TrackerLineNumberBar* self,
	Workspace* sender)
{
	trackerlinenumberbar_connectsong(self);
}

void trackerlinenumberbar_connectsong(TrackerLineNumberBar* self)
{
	if (self->workspace->song) {
		psy_signal_connect(
			&self->workspace->song->sequence.signal_cursorchanged,
			self, trackerlinenumberbar_oncursorchanged);
	}
}

void trackerlinenumberbar_onplaylinechanged(TrackerLineNumberBar* self,
	Workspace* sender)
{
	if (!workspace_followingsong(sender) && psy_ui_component_drawvisible(
			&self->component)) {
		trackerlinenumbers_invalidateline(&self->linenumbers,
			sender->host_sequencer_time.lastplayline);
		trackerlinenumbers_invalidateline(&self->linenumbers,
			sender->host_sequencer_time.currplayline);
	}
}

void trackerlinenumberbar_onplaystatuschanged(TrackerLineNumberBar* self,
	Workspace* sender)
{
	self->linenumbers.prevent_cursor = (workspace_followingsong(self->workspace) &&
		psy_audio_player_playing(workspace_player(self->workspace)));
	trackerlinenumbers_invalidateline(&self->linenumbers,
		sender->host_sequencer_time.lastplayline);
	trackerlinenumbers_invalidateline(&self->linenumbers,
		sender->host_sequencer_time.currplayline);
}

void trackerlinenumberbar_oncursorchanged(TrackerLineNumberBar* self,
	psy_audio_Sequence* sender)
{
	assert(self);
	
	/* lpb changed? */
	if (psy_audio_sequencecursor_lpb(&sender->cursor) !=
			psy_audio_sequencecursor_lpb(&sender->lastcursor)) {
		psy_ui_component_align(&self->linenumberpane);
		psy_ui_component_invalidate(trackerlinenumbers_base(
			&self->linenumbers));
		return;
	}	
	trackerlinenumbers_updatecursor(&self->linenumbers);	
}
