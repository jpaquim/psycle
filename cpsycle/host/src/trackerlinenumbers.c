// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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

static char* hex_tab[16] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "A", "B", "C", "D", "E", "F"
};

static int testcursor(psy_audio_PatternCursor cursor, uintptr_t track,
	psy_dsp_big_beat_t offset, uintptr_t lpb)
{
	return cursor.track == track && psy_dsp_testrange(cursor.offset, offset, 1.0 / lpb);
}

static void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks);

// TrackerLineNumbers
// prototypes
static void trackerlinenumbers_ondraw(TrackerLineNumbers*, psy_ui_Graphics*);
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
	self->showlinenumbercursor = 1;
	self->showlinenumbersinhex = 1;
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
		int cpy = 0;
		int line;
		double offset;
		double topoffset;
		double bottomoffset;
		int topline;
		int bottomline;
		psy_audio_PatternCursor cursor;
		char* linecountformat;
				
		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		topoffset = trackerlinestate_offset(self->linestate, g->clip.top,
			&topline);
		bottomoffset = trackerlinestate_offset(self->linestate, g->clip.bottom,
			&bottomline);
		cpy = (topline) * self->linestate->lineheight;
		offset = topoffset;
		line = topline;
		cursor = workspace_patterncursor(self->workspace);
		if (workspace_showlinenumbersinhex(self->workspace)) {
			if (workspace_showbeatoffset(self->workspace)) {
				linecountformat = "%.2X %.3f";
			} else {
				linecountformat = "%.2X";
			}
		} else {
			if (workspace_showbeatoffset(self->workspace)) {
				linecountformat = "%3i %.3f";
			} else {
				linecountformat = "%3i";
			}
		}
		while (offset <= bottomoffset &&
			offset < self->linestate->pattern->length) {
			psy_ui_Rectangle r;
			TrackerColumnFlags columnflags;
			int ystart;
			int drawbeat;
			uintptr_t c;
			int flatsize;
			char digit[2];

			drawbeat = workspace_showbeatoffset(self->workspace);
			columnflags.playbar = psy_audio_player_playing(&self->workspace->player) && 
				trackerlinestate_testplaybar(self->linestate, offset);
			columnflags.mid = 0;
			columnflags.cursor = self->linestate->drawcursor && !self->linestate->cursorchanging &&
				testcursor(cursor, cursor.track, offset, self->linestate->lpb);
			columnflags.beat = fmod(offset, 1.0f) == 0.0f;
			columnflags.beat4 = fmod(offset, 4.0f) == 0.0f;
			columnflags.selection = 0;
			setcolumncolour(self->linestate->skin, g, columnflags, 0, 0);			
			psy_snprintf(buffer, 10, linecountformat, line, offset);
			flatsize = tm.tmAveCharWidth + 2;
			digit[1] = '\0';
			for (c = 0; c <= strlen(buffer); ++c) {				
				digit[0] = buffer[c];				
				r = psy_ui_rectangle_make(c * flatsize, cpy, flatsize,
					tm.tmHeight);
				psy_ui_textoutrectangle(g, r.left, r.top,
					psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
					digit, strlen(digit));
			}
			cpy += self->linestate->lineheight;
			ystart = cpy;
			offset += (1.0 / self->linestate->lpb);
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
	line = trackerlinestate_offsettoscreenline(self->linestate,
		cursor->offset);
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_rectangle_make(
			0, self->linestate->lineheight * line,
			size.width, self->linestate->lineheight));
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, psy_dsp_big_beat_t offset)
{		
	if (self->linestate->pattern) {
		if (offset >= self->linestate->sequenceentryoffset &&
				offset < self->linestate->sequenceentryoffset +
				self->linestate->pattern->length) {
			psy_ui_Size size;
			psy_ui_TextMetric tm;
			int line;

			size = psy_ui_component_size(&self->component);
			tm = psy_ui_component_textmetric(&self->component);
			line = (int)((offset - self->linestate->sequenceentryoffset) *
				self->linestate->lpb);
			psy_ui_component_invalidaterect(&self->component,
				psy_ui_rectangle_make(
					0,
					self->linestate->lineheight * line,
					psy_ui_value_px(&size.width, &tm),
					self->linestate->lineheight));
		}
	}
}

void trackerlinenumbers_onpreferredsize(TrackerLineNumbers* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = (workspace_showbeatoffset(self->workspace))
		? psy_ui_value_makeew(13)
		: psy_ui_value_makeew(6);
	rv->height = psy_ui_value_makepx(0);
}

void trackerlinenumbers_showlinenumbercursor(TrackerLineNumbers* self, int showstate)
{
	self->showlinenumbercursor = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers* self, int showstate)
{
	self->showlinenumbersinhex = showstate;
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
	psy_ui_Component* parent, TrackerLineState* linestate, TrackerView* view,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	trackerlinenumberslabel_vtable_init(self);
	self->component.vtable = &trackerlinenumberslabel_vtable;
	trackerlinenumberslabel_setsharedlinestate(self, linestate);
	self->view = view;
	self->linestr = NULL;
	self->defaultstr = NULL;
	self->workspace = workspace;
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
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;
	psy_ui_IntSize headersize;

	// todo remove view
	tm = psy_ui_component_textmetric(&self->view->view->header.component);
	size = psy_ui_component_size(&self->component);
	headersize = psy_ui_intsize_init_size(
		psy_ui_component_preferredsize(&self->view->view->header.component, &size), &tm);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setrectangle(&r, 0, 0, psy_ui_value_px(&size.width, &tm),
		psy_ui_value_px(&size.height, &tm));
	psy_ui_drawsolidrectangle(g, r, self->linestate->skin->background);
	psy_ui_setbackgroundcolour(g, self->linestate->skin->background);
	psy_ui_settextcolour(g, self->linestate->skin->font);
	psy_ui_textoutrectangle(g, r.left, 0, 0, r, self->linestr, strlen(self->linestr));
	if (self->view->showdefaultline) {
		psy_ui_setfont(g, psy_ui_component_font(
			&self->view->view->left.linenumbers.component));
		if ((workspace_showbeatoffset(self->workspace))) {
			psy_ui_textoutrectangle(g, r.left, headersize.height, 0,
				r, self->defaultstr, strlen(self->defaultstr));
		} else {
			psy_ui_textoutrectangle(g, r.left, headersize.height, 0,
				r, "Def", strlen("Def"));
		}
	}
}

void trackerlinenumberslabel_onpreferredsize(TrackerLineNumbersLabel* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{	
	int height = 0;
	psy_ui_TextMetric tm;
	psy_ui_IntSize headersize;

	// todo remove view
	tm = psy_ui_component_textmetric(&self->view->view->header.component);
	headersize = psy_ui_intsize_init_size(
		psy_ui_component_preferredsize(&self->view->view->header.component, limit), &tm);
	height = headersize.height;
	if (self->view->showdefaultline) {		
		height += self->linestate->lineheight;
	}	
	rv->height = psy_ui_value_makepx(height);
	rv->width = (workspace_showbeatoffset(self->workspace))
		? psy_ui_value_makeew(10)
		: psy_ui_value_makeew(5);
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
	TrackerLineState* linestate, struct TrackerView* view, Workspace* workspace)
{
	psy_ui_Margin leftmargin;

	psy_ui_component_init(&self->component, parent);	
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component, linestate, view,
		workspace);
	psy_ui_component_setalign(&self->linenumberslabel.component, psy_ui_ALIGN_TOP);
	trackerlinenumbers_init(&self->linenumbers, &self->component, linestate,
		workspace);
	psy_ui_component_setalign(&self->linenumbers.component, psy_ui_ALIGN_CLIENT);
	zoombox_init(&self->zoombox, &self->component);
	psy_ui_component_setpreferredsize(&self->zoombox.component,
		psy_ui_size_make(psy_ui_value_makepx(0),
			psy_ui_value_makeeh(1.0)));
	psy_ui_component_setalign(&self->zoombox.component, psy_ui_ALIGN_BOTTOM);
	trackerlinenumberbar_computefontheight(self);
	psy_ui_margin_init_all(&leftmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeew(3));
	psy_ui_component_setmargin(&self->linenumbers.component,
		&leftmargin);
	psy_ui_component_setmargin(&self->linenumberslabel.component,
		&leftmargin);
}

void trackerlinenumberbar_computefontheight(TrackerLineNumberBar* self)
{
	psy_ui_Font* font;

	font = psy_ui_component_font(&self->component);
	if (font) {
		psy_ui_FontInfo fontinfo;

		fontinfo = psy_ui_font_fontinfo(font);
		self->zoomheightbase = fontinfo.lfHeight;
	} else {
		self->zoomheightbase = -16;
	}
}
