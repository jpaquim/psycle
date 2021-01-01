// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackerlinenumbers.h"
// local
#include "patternview.h"
// std
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

static int testcursor(psy_audio_PatternCursor cursor, TrackerLineState* linestate,
	uintptr_t track, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t bpl)
{
	int currline;
	int cursorline;
	currline = trackerlinestate_beattoline(linestate, offset);
	cursorline = trackerlinestate_beattoline(linestate, cursor.offset);

	return (cursorline == currline) && track == cursor.track;	
}

static void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks);

// TrackerLineNumbers
// prototypes
static void trackerlinenumbers_ondraw(TrackerLineNumbers*, psy_ui_Graphics*);
static void trackerlinenumbers_onalign(TrackerLineNumbers*);
static void trackerlinenumbers_onpreferredsize(TrackerLineNumbers*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void trackerlinennumbers_onpatterncursorchanged(TrackerLineNumbers*,
	Workspace*);
static void trackerlinenumbers_onscroll(TrackerLineNumbers*, psy_ui_Component* sender);
// vtable
static psy_ui_ComponentVtable trackerlinenumbers_vtable;
static int trackerlinenumbers_vtable_initialized = 0;

static void trackerlinenumbers_vtable_init(TrackerLineNumbers* self)
{
	if (!trackerlinenumbers_vtable_initialized) {
		trackerlinenumbers_vtable = *(self->component.vtable);
		trackerlinenumbers_vtable.ondraw = (psy_ui_fp_component_ondraw)trackerlinenumbers_ondraw;
		trackerlinenumbers_vtable.onalign = (psy_ui_fp_component_onalign)trackerlinenumbers_onalign;
		trackerlinenumbers_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackerlinenumbers_onpreferredsize;
		trackerlinenumbers_vtable_initialized = 1;
	}
}
// implementation
void trackerlinenumbers_init(TrackerLineNumbers* self, psy_ui_Component* parent,
	TrackerLineState* linestate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumbers_vtable_init(self);
	self->component.vtable = &trackerlinenumbers_vtable;
	self->workspace = workspace;
	self->linestate = linestate;
	self->showcursor = TRUE;
	self->shownumbersinhex = TRUE;
	trackerlinenumbers_setsharedlinestate(self, linestate);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setbackgroundcolour(&self->component,
		self->linestate->skin->background);
	psy_signal_connect(&self->component.signal_scroll, self,
		trackerlinenumbers_onscroll);
	psy_signal_connect(&self->workspace->signal_patterncursorchanged, self,
		trackerlinennumbers_onpatterncursorchanged);
	psy_audio_patterncursor_init(&self->lastcursor);	
}

void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers* self, TrackerLineState*
	linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, psy_ui_Graphics* g)
{
	if (self->linestate->pattern) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		char buffer[20];
		intptr_t cpy;
		intptr_t line;
		double offset;
		double topoffset;
		double bottomoffset;
		intptr_t topline;
		intptr_t bottomline;
		bool drawbeat;
		psy_audio_PatternCursor cursor;
		char* linecountformat;		
				
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		topoffset = trackerlinestate_pxtobeat(self->linestate, g->clip.top);
		topline = trackerlinestate_beattoline(self->linestate, topoffset);
		bottomoffset = trackerlinestate_pxtobeatnotquantized(self->linestate, g->clip.bottom);
		bottomline = (int)(bottomoffset * self->linestate->lpb + 0.5);		
		offset = topoffset;
		cpy = trackerlinestate_beattopx(self->linestate, offset);
		line = topline;
		drawbeat = (patternviewconfig_showbeatoffset(psycleconfig_patview(
			workspace_conf(self->workspace))));
		cursor = workspace_patterncursor(self->workspace);
		if (patternviewconfig_linenumbersinhex(psycleconfig_patview(
				workspace_conf(self->workspace)))) {
			if (drawbeat) {
				linecountformat = "%.2X %.3f";
			} else {
				linecountformat = "%.2X";
			}
		} else {
			if (drawbeat) {
				linecountformat = "%i %.3f";
			} else {
				linecountformat = "%i";
			}
		}
		while (offset <= bottomoffset &&
			offset < self->linestate->pattern->length) {
			psy_ui_Rectangle r;
			TrackerColumnFlags columnflags;
			intptr_t ystart;
			uintptr_t c;
			uintptr_t numdigits;		
			uintptr_t maxdigits;
			uintptr_t startdigit;
			intptr_t blankspace;
			char digit[2];
						
			columnflags.playbar = psy_audio_player_playing(workspace_player(self->workspace)) && 
				trackerlinestate_testplaybar(self->linestate, offset);
			columnflags.mid = 0;
			columnflags.cursor = self->showcursor &&
				self->linestate->drawcursor && !self->linestate->cursorchanging &&
				testcursor(cursor, self->linestate, cursor.track, offset, self->linestate->bpl);
			columnflags.beat = fmod(offset, 1.0f) == 0.0f;
			columnflags.beat4 = fmod(offset, 4.0f) == 0.0f;
			columnflags.selection = 0;
			setcolumncolour(self->linestate->skin, g, columnflags, 0, 0);			
			psy_snprintf(buffer, 10, linecountformat, line, offset);			
			digit[1] = '\0';
			numdigits = strlen(buffer);
			maxdigits = ((psy_ui_value_px(&size.width, &tm) - 4) / self->linestate->flatsize);
			startdigit = maxdigits - numdigits - 1;
			for (c = 0; c < maxdigits; ++c) {
				if (c >= startdigit && c < startdigit + numdigits) {
					digit[0] = buffer[c - startdigit];
				} else {
					digit[0] = ' ';
				}
				r = psy_ui_rectangle_make(c * self->linestate->flatsize, cpy,
					self->linestate->flatsize,
					self->linestate->lineheightpx - 1);
				psy_ui_textoutrectangle(g, r.left, r.top,
					psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
					digit, strlen(digit));
			}
			r.left += self->linestate->flatsize;
			blankspace = (psy_ui_value_px(&size.width, &tm) - r.left) - 4;
			if (blankspace > 0) {
				r = psy_ui_rectangle_make(r.left, cpy, blankspace,
					self->linestate->lineheightpx - 1);
				digit[0] = ' ';
				psy_ui_textoutrectangle(g, r.left, r.top,
					psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
					digit, strlen(digit));
			}
			cpy += self->linestate->lineheightpx;
			ystart = cpy;
			offset += self->linestate->bpl;
			++line;
		}
	}
}

void trackerlinennumbers_onpatterncursorchanged(TrackerLineNumbers* self,
	Workspace* sender)
{
	psy_audio_PatternCursor currcursor;

	currcursor = workspace_patterncursor(sender);
	if (self->lastcursor.offset != currcursor.offset) {
		if (!self->linestate->cursorchanging) {
			trackerlinenumbers_invalidatecursor(self, &self->lastcursor);
		}
		self->linestate->cursorchanging = FALSE;
		trackerlinenumbers_invalidatecursor(self, &currcursor);		
	}
	self->lastcursor = currcursor;
}

void trackerlinenumbers_onscroll(TrackerLineNumbers* self, psy_ui_Component* sender)
{	
	if (self->linestate->cursorchanging) {
		trackerlinenumbers_invalidatecursor(self, &self->lastcursor);
	}
}

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers* self,
	const psy_audio_PatternCursor* cursor)
{	
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	int line;
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	line = trackerlinestate_beattoline(self->linestate, cursor->offset);
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_rectangle_make(
			0, self->linestate->lineheightpx * line,
			size.width, self->linestate->lineheightpx));
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, psy_dsp_big_beat_t offset)
{			
	if (trackerlinestate_pattern(self->linestate) &&
			trackerlinestate_testplayposition(self->linestate, offset)) {
		psy_ui_IntSize size;		
		int line;

		size = psy_ui_component_intsize(&self->component);		
		line = trackerlinestate_beattoline(self->linestate,
			offset - self->linestate->sequenceentryoffset);
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_rectangle_make(
				0, self->linestate->lineheightpx * line,
				size.width, self->linestate->lineheightpx));
	}
}

void trackerlinenumbers_onalign(TrackerLineNumbers* self)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(trackerlinenumbers_base(self));
	self->linestate->flatsize = (int)(tm.tmAveCharWidth) + 2;
}

void trackerlinenumbers_onpreferredsize(TrackerLineNumbers* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	static int margin = 4;

	rv->width = ((patternviewconfig_showbeatoffset(psycleconfig_patview(
		workspace_conf(self->workspace)))))
			? psy_ui_value_makepx(self->linestate->flatsize * 13 + margin)
			: psy_ui_value_makepx(self->linestate->flatsize * 6 + margin);
	rv->height = psy_ui_value_makepx(0);
}

void trackerlinenumbers_showlinenumbercursor(TrackerLineNumbers* self, bool showstate)
{
	self->showcursor = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers* self, bool showstate)
{
	self->shownumbersinhex = showstate;
	psy_ui_component_invalidate(&self->component);
}

// LineNumbersLabel
// prototypes
static void trackerlinenumberslabel_ondestroy(TrackerLineNumbersLabel*, psy_ui_Component* sender);
static void trackerlinenumberslabel_updatetext(TrackerLineNumbersLabel*);
static void trackerlinenumberslabel_onlanguagechanged(TrackerLineNumbersLabel*);
static void trackerlinenumberslabel_setsharedlinestate(TrackerLineNumbersLabel*,
	TrackerLineState*);
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
		trackerlinenumberslabel_vtable.ondraw = (psy_ui_fp_component_ondraw)
			trackerlinenumberslabel_ondraw;
		trackerlinenumberslabel_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			trackerlinenumberslabel_onmousedown;
		trackerlinenumberslabel_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			trackerlinenumberslabel_onpreferredsize;
		trackerlinenumberslabel_vtable.onlanguagechanged = (psy_ui_fp_component_onlanguagechanged)
			trackerlinenumberslabel_onlanguagechanged;
	}
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerLineState* linestate,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumberslabel_vtable_init(self);
	self->component.vtable = &trackerlinenumberslabel_vtable;
	trackerlinenumberslabel_setsharedlinestate(self, linestate);
	self->linestr = NULL;
	self->defaultstr = NULL;
	self->workspace = workspace;
	self->headerheight = 12;
	self->showdefaultline = TRUE;
	self->showbeatoffset = FALSE;
	trackerlinenumberslabel_updatetext(self);	
	psy_signal_connect(&self->component.signal_destroy, self,
		trackerlinenumberslabel_ondestroy);
}

void trackerlinenumberslabel_ondestroy(TrackerLineNumbersLabel* self, psy_ui_Component* sender)
{
	free(self->linestr);
	free(self->defaultstr);
}

void trackerlinenumberslabel_updatetext(TrackerLineNumbersLabel* self)
{
	free(self->linestr);
	free(self->defaultstr);
	self->linestr = strdup(psy_ui_translate("patternview.line"));
	self->defaultstr = strdup(psy_ui_translate("patternview.defaults"));
}

void trackerlinenumberslabel_onlanguagechanged(TrackerLineNumbersLabel* self)
{
	trackerlinenumberslabel_updatetext(self);
}

void trackerlinenumberslabel_setsharedlinestate(TrackerLineNumbersLabel* self,
	TrackerLineState* linestate)
{
	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
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
	psy_ui_IntSize size;
	psy_ui_Rectangle r;	
	
	size = psy_ui_component_intsize(&self->component);	
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->linestate->skin->background);
	psy_ui_setbackgroundcolour(g, self->linestate->skin->background);
	psy_ui_settextcolour(g, self->linestate->skin->font);
	psy_ui_textoutrectangle(g, r.left, 0, 0, r, self->linestr, strlen(self->linestr));
	if (self->showdefaultline) {
		if (self->linestate->gridfont) {
			psy_ui_setfont(g, self->linestate->gridfont);
		}
		if ((self->showbeatoffset)) {
			psy_ui_textoutrectangle(g, r.left, self->headerheight, 0,
				r, self->defaultstr, strlen(self->defaultstr));
		} else {			
			psy_ui_textoutrectangle(g, r.left, self->headerheight, 0,
				r, "Def", strlen("Def"));
		}
	}
}

void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	intptr_t height;
	
	height = self->headerheight;	
	if (self->showdefaultline) {		
		height += self->linestate->lineheightpx;
	}	
	rv->height = psy_ui_value_makepx(height);
	rv->width = (self->showbeatoffset)
		? psy_ui_value_makeew(10.0)
		: psy_ui_value_makepx(0);
}

void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks)
{
	if (flags.cursor != 0) {
		psy_ui_setbackgroundcolour(g, skin->cursor);
		psy_ui_settextcolour(g,
			patternviewskin_fontcurcolour(skin, track, numtracks));
	} else if (flags.playbar) {
		psy_ui_setbackgroundcolour(g,
			patternviewskin_playbarcolour(skin, track, numtracks));
		psy_ui_settextcolour(g,
			patternviewskin_fontplaycolour(skin, track, numtracks));
	} else if (flags.selection) {
		if (flags.beat4) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selection4beatcolour(skin, track, numtracks));
		} else if (flags.beat) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selectionbeatcolour(skin, track, numtracks));
		} else {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selectioncolour(skin, track, numtracks));
		}
		psy_ui_settextcolour(g,
			patternviewskin_fontselcolour(skin, track, numtracks));
	} else if (flags.mid) {
		psy_ui_setbackgroundcolour(g,
			patternviewskin_midlinecolour(skin, track, numtracks));
		if (flags.cursor != 0) {
			psy_ui_settextcolour(g,
				patternviewskin_fontcurcolour(skin, track, numtracks));
		} else {
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		}
	} else {
		if (flags.beat4) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_row4beatcolour(skin, track, numtracks));
			psy_ui_settextcolour(g, skin->font);
		} else if (flags.beat) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_rowbeatcolour(skin, track, numtracks));
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		} else {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_rowcolour(skin, track, numtracks));
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		}
	}
}



// TrackerLineNumberBar
// prototypes

// implementation
void trackerlinenumberbar_init(TrackerLineNumberBar* self, psy_ui_Component* parent,
	TrackerLineState* linestate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);	
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component, linestate,
		workspace);
	psy_ui_component_setalign(&self->linenumberslabel.component, psy_ui_ALIGN_TOP);
	trackerlinenumbers_init(&self->linenumbers, &self->component, linestate,
		workspace);
	psy_ui_component_setalign(&self->linenumbers.component, psy_ui_ALIGN_CLIENT);
	zoombox_init(&self->zoombox, &self->component);
	psy_ui_component_setpreferredsize(&self->zoombox.component,
		psy_ui_size_make(psy_ui_value_makepx(0), psy_ui_value_makeeh(1.0)));
	psy_ui_component_setalign(&self->zoombox.component, psy_ui_ALIGN_BOTTOM);	
}
