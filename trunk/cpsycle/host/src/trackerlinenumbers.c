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
	intptr_t currline;
	intptr_t cursorline;
	currline = trackerlinestate_beattoline(linestate, offset);
	cursorline = trackerlinestate_beattoline(linestate, cursor.offset + cursor.seqoffset);

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
static void trackerlinennumbers_linetext(TrackerLineNumbers*, bool hex,
	bool seqstart, bool drawbeat, int patidx, int line, float offset,
	char* rv);
static TrackerColumnFlags trackerlinennumbers_columnflags(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t seqoffset, psy_audio_PatternCursor);
static void trackerlinennumbers_drawtext(TrackerLineNumbers*, psy_ui_Graphics*,
	const char* format, double y, double width, const char* text);
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
	psy_ui_component_init(&self->component, parent, NULL);
	trackerlinenumbers_vtable_init(self);
	self->component.vtable = &trackerlinenumbers_vtable;
	self->workspace = workspace;
	self->linestate = linestate;
	self->showcursor = TRUE;
	self->shownumbersinhex = TRUE;
	self->showbeat = FALSE;
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

void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers* self,
	TrackerLineState* linestate)
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
		psy_ui_RealSize size;		
		double cpy;
		intptr_t line;
		double offset;		
		psy_audio_PatternCursor cursor;		
		psy_audio_SequenceTrackIterator ite;
		double seqoffset;		
		double length;
		uintptr_t maxlines;
		uintptr_t patidx;
		psy_audio_PatternSelection clip;
				
		assert(self);
		
		trackerlinestate_clip(self->linestate, &g->clip, &clip);		
		offset = clip.topleft.offset;
		line = clip.topleft.line;
		cpy = trackerlinestate_beattopx(self->linestate, offset);
		size = psy_ui_component_scrollsize_px(&self->component);				
		cursor = workspace_patterncursor(self->workspace);
		ite.pattern = self->linestate->pattern;
		ite.patternnode = NULL;
		ite.patterns = &self->workspace->song->patterns;
		seqoffset = 0.0;		
		patidx = 0;
		length = ite.pattern->length;		
		if (!self->linestate->singlemode && self->linestate->sequence) {
			psy_audio_SequenceTrackNode* tracknode;

			tracknode = psy_list_at(self->linestate->sequence->tracks,
				workspace_sequenceeditposition(self->workspace).track);
			if (!tracknode) {
				tracknode = self->linestate->sequence->tracks;
			}
			ite = psy_audio_sequence_begin(self->linestate->sequence,
				tracknode, offset);
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);			
			patidx = psy_audio_sequencetrackiterator_patidx(&ite);
			if (ite.pattern) {
				length = ite.pattern->length;			
			}
		} else {
			ite.sequencentrynode = NULL;			
		}
		maxlines = trackerlinestate_numlines(self->linestate);
		while (offset <= clip.bottomright.offset && line < (intptr_t)maxlines) {
			double ystart;			
			char text[64];
				
			trackerlinennumbers_linetext(self,
				self->shownumbersinhex,
				!self->linestate->singlemode && seqoffset == offset,
				self->showbeat,
				(int)patidx, (int)line, (float)offset, text);
			setcolumncolour(self->linestate->skin, g,
				trackerlinennumbers_columnflags(self, offset, seqoffset,
					cursor), 0, 0);
			trackerlinennumbers_drawtext(self, g, text, cpy, size.width, text);					
			cpy += self->linestate->lineheightpx;
			ystart = cpy;
			offset += self->linestate->bpl;			
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
	psy_audio_PatternCursor cursor)
{
	TrackerColumnFlags rv;

	rv.playbar = psy_audio_player_playing(workspace_player(self->workspace)) &&
		trackerlinestate_testplaybar(self->linestate, offset);
	rv.mid = 0;
	rv.cursor = self->showcursor &&
		self->linestate->drawcursor && !self->linestate->cursorchanging &&
		testcursor(cursor, self->linestate, cursor.track, offset, self->linestate->bpl);
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
	maxdigits = (uintptr_t)((width - 4) / self->linestate->flatsize);
	startdigit = maxdigits - numdigits - 1;
	for (c = 0; c < maxdigits; ++c) {
		if (c >= startdigit && c < startdigit + numdigits) {
			digit[0] = text[c - startdigit];
		} else {
			digit[0] = ' ';
		}
		r = psy_ui_realrectangle_make(
			psy_ui_realpoint_make(c * self->linestate->flatsize, y),
			psy_ui_realsize_make(self->linestate->flatsize,
				self->linestate->lineheightpx - 1));
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
			digit, psy_strlen(digit));
	}
	r.left += self->linestate->flatsize;
	blankspace = (width - r.left) - 4;
	if (blankspace > 0) {
		r = psy_ui_realrectangle_make(
				psy_ui_realpoint_make(r.left, y),
			psy_ui_realsize_make(
				blankspace, self->linestate->lineheightpx - 1));
		digit[0] = ' ';
		psy_ui_textoutrectangle(g, psy_ui_realrectangle_topleft(&r),
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r,
			digit, psy_strlen(digit));
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
	psy_ui_RealSize size;
	intptr_t line;
		
	size = psy_ui_component_scrollsize_px(&self->component);
	line = trackerlinestate_beattoline(self->linestate,
		cursor->offset + cursor->seqoffset);
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0.0, self->linestate->lineheightpx * line),
			psy_ui_realsize_make(size.width, self->linestate->lineheightpx)));
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, psy_dsp_big_beat_t offset)
{			
	if (!self->linestate->singlemode ||(trackerlinestate_pattern(self->linestate) &&
			trackerlinestate_testplayposition(self->linestate, offset))) {
		psy_ui_RealSize size;		
		intptr_t line;

		size = psy_ui_component_scrollsize_px(&self->component);		
		line = trackerlinestate_beattoline(self->linestate,
			offset - ((self->linestate->singlemode)
				? self->linestate->sequenceentryoffset
				: 0.0));
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(0.0,
					self->linestate->lineheightpx * line),
				psy_ui_realsize_make(
					size.width, self->linestate->lineheightpx)));
	}
}

void trackerlinenumbers_onalign(TrackerLineNumbers* self)
{
	const psy_ui_TextMetric* tm;

	tm = psy_ui_component_textmetric(trackerlinenumbers_base(self));
	self->linestate->flatsize = (double)(tm->tmAveCharWidth) + 2.0;
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
	if (!self->linestate->singlemode) {
		numcols += 2;
	}
	rv->width = psy_ui_value_make_px(self->linestate->flatsize * numcols +
		margin);
	rv->height = psy_ui_value_make_px(
		trackerlinestate_numlines(self->linestate) *
		trackerlinestate_lineheight(self->linestate));
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
		trackerlinenumberslabel_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackerlinenumberslabel_onmousedown;
		trackerlinenumberslabel_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			trackerlinenumberslabel_onpreferredsize;
		trackerlinenumberslabel_vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			trackerlinenumberslabel_onlanguagechanged;
	}
}

void trackerlinenumberslabel_init(TrackerLineNumbersLabel* self,
	psy_ui_Component* parent, TrackerLineState* linestate,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	trackerlinenumberslabel_vtable_init(self);
	self->component.vtable = &trackerlinenumberslabel_vtable;
	trackerlinenumberslabel_setsharedlinestate(self, linestate);
	self->linestr = NULL;
	self->defaultstr = NULL;
	self->workspace = workspace;
	self->headerheight = 12;
	self->showdefaultline = TRUE;
	self->showbeatoffset = FALSE;
	self->useheaderbitmap = TRUE;
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
	psy_ui_RealSize size;
	psy_ui_RealRectangle r;	
	
	size = psy_ui_component_scrollsize_px(&self->component);
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_drawsolidrectangle(g, r, self->linestate->skin->background);
	psy_ui_setbackgroundcolour(g, self->linestate->skin->background);
	psy_ui_settextcolour(g, self->linestate->skin->font);
	if (!self->useheaderbitmap && self->linestate->gridfont) {
		psy_ui_setfont(g, self->linestate->gridfont);
	}
	psy_ui_textoutrectangle(g, psy_ui_realpoint_make(r.left, 0), 0, r, self->linestr, psy_strlen(self->linestr));
	if (self->showdefaultline) {
		if (self->linestate->gridfont) {
			psy_ui_setfont(g, self->linestate->gridfont);
		}
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
		height += self->linestate->lineheightpx;
	}	
	rv->height = psy_ui_value_make_px(height);
	rv->width = (self->showbeatoffset)
		? psy_ui_value_make_ew(10.0)
		: psy_ui_value_make_ew(0.0);
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
	psy_ui_component_init(&self->component, parent, NULL);
	trackerlinenumberslabel_init(&self->linenumberslabel, &self->component, linestate,
		workspace);
	psy_ui_component_setalign(&self->linenumberslabel.component, psy_ui_ALIGN_TOP);
	psy_ui_component_init(&self->linenumberpane, &self->component, NULL);
	psy_ui_component_setalign(&self->linenumberpane, psy_ui_ALIGN_CLIENT);
	trackerlinenumbers_init(&self->linenumbers, &self->linenumberpane, linestate,
		workspace);
	psy_ui_component_setalign(&self->linenumbers.component, psy_ui_ALIGN_FIXED);
	zoombox_init(&self->zoombox, &self->component);
	psy_ui_component_setpreferredsize(&self->zoombox.component,
		psy_ui_size_make_em(0.0, 1.0));
	psy_ui_component_setalign(&self->zoombox.component, psy_ui_ALIGN_BOTTOM);	
}
