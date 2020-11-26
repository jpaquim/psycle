// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"
// local
#include "patterncmds.h"
// std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

enum {
	CMD_NAVUP = 1024,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_ENTER
};

static int isblack(int key)
{
	int offset = key % 12;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	return (offset == 1 || offset == 3 || offset == 6 || offset == 8
		|| offset == 10);
}

static void definecmd(psy_Property* cmds, int cmd, uintptr_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext);

// Header
static void pianoheader_ondraw(PianoHeader*, psy_ui_Graphics*);
static void pianoheader_drawruler(PianoHeader*, psy_ui_Graphics*);
static void pianoheader_onpreferredsize(PianoHeader*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable pianoheader_vtable;
static bool pianoheader_vtable_initialized = FALSE;

static void pianoheader_vtable_init(PianoHeader* self)
{
	if (!pianoheader_vtable_initialized) {
		pianoheader_vtable = *(self->component.vtable);
		pianoheader_vtable.ondraw = (psy_ui_fp_component_ondraw)
			pianoheader_ondraw;
		pianoheader_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pianoheader_onpreferredsize;
		pianoheader_vtable_initialized = TRUE;
	}
}

void pianoheader_init(PianoHeader* self, psy_ui_Component* parent,
	GridState* gridstate)
{
	psy_ui_component_init(&self->component, parent);
	pianoheader_vtable_init(self);
	self->component.vtable = &pianoheader_vtable;
	pianoheader_setsharedgridstate(self, gridstate);	
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1)));
}

void pianoheader_setsharedgridstate(PianoHeader* self, GridState* gridstate)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		gridstate_init(&self->defaultgridstate, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianoheader_onpreferredsize(PianoHeader* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = (self->gridstate->pattern)
		? psy_ui_value_makepx(gridstate_beattopx(self->gridstate,
		  psy_audio_pattern_length(self->gridstate->pattern)))
		: psy_ui_value_makepx(0);	
}

void pianoheader_ondraw(PianoHeader* self, psy_ui_Graphics* g)
{	
	pianoheader_drawruler(self, g);	
}

void pianoheader_drawruler(PianoHeader* self, psy_ui_Graphics* g)
{	
	int baseline;	
	psy_dsp_big_beat_t viewmin;
	psy_dsp_big_beat_t viewmax;
	psy_dsp_big_beat_t c;
	psy_dsp_big_beat_t bpl;
	int pxperline;
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	viewmin = gridstate_pxtobeat(self->gridstate, g->clip.left);
	viewmin = (int)(viewmin * self->gridstate->lpb) / (psy_dsp_big_beat_t)self->gridstate->lpb;
	viewmax = gridstate_pxtobeat(self->gridstate, g->clip.right);
	bpl = 1.0 / self->gridstate->lpb;
	pxperline = gridstate_beattopx(self->gridstate, bpl);
	baseline = size.height - 1;	
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
	psy_ui_drawline(g, psy_ui_component_scrollleft(&self->component), baseline,
		size.width + psy_ui_component_scrollleft(&self->component),
		baseline);
		
	for (c = viewmin; c <= viewmax; c += bpl) {
		int line;
		psy_dsp_big_beat_t lineoffset;
		int cpx;

		line = (int)(c * self->gridstate->lpb);
		lineoffset = line * bpl;
		cpx = gridstate_beattopx(self->gridstate, lineoffset);		
		psy_ui_drawline(g, cpx, baseline, cpx, baseline - tm.tmHeight / 3);
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
	for (c = viewmin; c <= viewmax; c += bpl) {
		char txt[40];
		int cpx;
		int line;
		psy_dsp_big_beat_t lineoffset;

		line = (int)(c * self->gridstate->lpb);
		if ((line % 4) == 0) {
			lineoffset = line * bpl;
			cpx = gridstate_beattopx(self->gridstate, lineoffset);
			psy_ui_drawline(g, (int)cpx, baseline, (int)cpx, baseline - tm.tmHeight / 2);
			psy_snprintf(txt, 40, "%d", (int)(c));
			psy_ui_textout(g, (int)cpx + 3, baseline - tm.tmHeight, txt, strlen(txt));
		}
	}	
}

// KeyboardState
void keyboardstate_init(KeyboardState* self, PatternViewSkin* skin)
{	
	self->keymin = 0;
	self->keymax = 88;
	self->keyheight = psy_ui_value_makeeh(1.2);
	self->defaultkeyheight = psy_ui_value_makeeh(1.2);
	self->notemode = psy_dsp_NOTESTAB_A220;
	self->drawpianokeys = TRUE;
	self->skin = skin;
}
// prototypes
static void pianokeyboard_ondraw(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_onmousedown(PianoKeyboard*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable pianokeyboard_vtable;
static int pianokeyboard_vtable_initialized = 0;

static void pianokeyboard_vtable_init(PianoKeyboard* self)
{
	if (!pianokeyboard_vtable_initialized) {
		pianokeyboard_vtable = *(self->component.vtable);
		pianokeyboard_vtable.ondraw = (psy_ui_fp_component_ondraw)
			pianokeyboard_ondraw;
		pianokeyboard_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianokeyboard_onmousedown;
		pianokeyboard_vtable_initialized = 1;
	}
}
// implementation
void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate)
{
	psy_ui_component_init(&self->component, parent);
	pianokeyboard_vtable_init(self);
	self->component.vtable = &pianokeyboard_vtable;
	pianokeyboard_setsharedkeyboardstate(self, keyboardstate);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_makeew(8), psy_ui_value_makepx(0)));
}

void pianokeyboard_setsharedkeyboardstate(PianoKeyboard* self,
	KeyboardState* keyboardstate)
{
	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate, NULL);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianokeyboard_ondraw(PianoKeyboard* self, psy_ui_Graphics* g)
{	
	int key;
	int keyboardheight;
	int keyheight;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	keyboardheight = keyboardstate_height(self->keyboardstate, &tm);
	keyheight = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
	psy_ui_setcolour(g, patternviewskin_keyseparatorcolour(self->keyboardstate->skin));
	if (!self->keyboardstate->drawpianokeys) {
		psy_ui_settextcolour(g, patternviewskin_keywhitecolour(self->keyboardstate->skin));
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		int cpy;

		cpy = keyboardheight - (key + 1) * keyheight;
		psy_ui_drawline(g, 0, cpy, psy_ui_value_px(&size.width, &tm), cpy);
		if (self->keyboardstate->drawpianokeys) {
			if (isblack(key)) {
				psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					(int)(psy_ui_value_px(&size.width, &tm) * 0.75), cpy,
					(int)(psy_ui_value_px(&size.width, &tm) * 0.25),
					keyheight),
					patternviewskin_keywhitecolour(self->keyboardstate->skin));
				psy_ui_drawline(g,
					0, cpy + keyheight / 2,
					psy_ui_value_px(&size.width, &tm),
					cpy + keyheight / 2);
				psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					0, cpy, (int)(psy_ui_value_px(&size.width, &tm) * 0.75),
					keyheight),
					patternviewskin_keyblackcolour(self->keyboardstate->skin));
			} else {
				psy_ui_Rectangle r;

				r = psy_ui_rectangle_make(0, cpy, psy_ui_value_px(&size.width, &tm),
					keyheight);
				psy_ui_drawsolidrectangle(g, r,
					patternviewskin_keywhitecolour(self->keyboardstate->skin));
				if (key % 12 == 0 || ((key % 12) == 5)) {
					psy_ui_drawline(g,
						0, cpy + keyheight, psy_ui_value_px(&size.width, &tm),
						cpy + keyheight);
					if (key % 12 == 0) {
						psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_CLIPPED, r,
							psy_dsp_notetostr(key, self->keyboardstate->notemode),
							strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
					}
				}
			}
		} else {
			psy_ui_Rectangle r;

			r = psy_ui_rectangle_make(0, cpy, psy_ui_value_px(&size.width, &tm),
				keyheight);
			psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_CLIPPED, r,
				psy_dsp_notetostr(key, self->keyboardstate->notemode),
				strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
			psy_ui_drawline(g,
				0, cpy + keyheight, psy_ui_value_px(&size.width, &tm),
				cpy + keyheight);
		}
	}
}

void pianokeyboard_onmousedown(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 2) {
		if (self->keyboardstate->notemode != psy_dsp_NOTESTAB_GMPERCUSSION) {
			self->keyboardstate->drawpianokeys = FALSE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_GMPERCUSSION;
			psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
				psy_ui_value_makeew(21), psy_ui_value_makepx(0)));			
		} else {
			self->keyboardstate->drawpianokeys = TRUE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_A220;
			psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
				psy_ui_value_makeew(8), psy_ui_value_makepx(0)));			
		}
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_component_parent(&self->component)));
	}
	psy_ui_mouseevent_stoppropagation(ev);
}

// PianoGrid
// default state
void gridstate_init(GridState* self, PatternViewSkin* skin)
{	
	self->defaultbeatwidth = 90;
	self->pxperbeat = self->defaultbeatwidth;
	self->lpb = 4;
	self->pattern = NULL;
	self->eventcolour = psy_ui_colour_make(0x00999999);
	self->eventcurrchannelcolour = psy_ui_colour_make(0x00CACACA);
	self->eventhovercolour = psy_ui_colour_make(0x00DADADA);	
	self->skin = skin;
	self->cursorchanging = FALSE;
	psy_audio_patterncursor_init(&self->cursor);
}

// Pianogrid
// prototypes
static void pianogrid_ondestroy(Pianogrid*);
static void pianogrid_ondraw(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawgrid(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics*);
static void pianogrid_drawentries(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawentry(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternEntry*, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length,
	int hover);
static void pianogrid_drawcursor(Pianogrid*, psy_ui_Graphics*);
static psy_ui_Colour pianogrid_linecolour(Pianogrid* self, int line);
static void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static psy_dsp_big_beat_t pianogrid_pxtobeat(Pianogrid*, int x);
static psy_dsp_big_beat_t pianogrid_quantizise(Pianogrid*,
	psy_dsp_big_beat_t offset);
static psy_audio_PatternNode* pianogrid_nextnode(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track);
static void pianogrid_onpatterneditpositionchanged(Pianogrid*,
	Workspace* sender);
static int pianogrid_scrollright(Pianogrid*, psy_audio_PatternCursor cursor);
static int pianogrid_scrollleft(Pianogrid*, psy_audio_PatternCursor cursor);
static void pianogrid_prevline(Pianogrid*);
static void pianogrid_prevlines(Pianogrid*, uintptr_t lines, int wrap);
static void pianogrid_advanceline(Pianogrid*);
static void pianogrid_advancelines(Pianogrid*, uintptr_t lines, int wrap);

static psy_ui_ComponentVtable pianogrid_vtable;
static int pianogrid_vtable_initialized = 0;
// vtable
static void pianogrid_vtable_init(Pianogrid* self)
{
	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw = (psy_ui_fp_component_ondraw)pianogrid_ondraw;	
		pianogrid_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianogrid_onmousedown;
		pianogrid_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			pianogrid_onmousemove;
		pianogrid_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pianogrid_onpreferredsize;
		pianogrid_vtable_initialized = 1;
	}
}
// implementation
void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, GridState* gridstate,
	Workspace* workspace)
{
	int track;

	psy_ui_component_init(&self->component, parent);
	pianogrid_vtable_init(self);
	self->component.vtable = &pianogrid_vtable;
	psy_ui_component_doublebuffer(&self->component);
	pianogrid_setsharedgridstate(self, gridstate);
	pianogrid_setsharedkeyboardstate(self, keyboardstate);
	self->workspace = workspace;
	self->component.wheelscroll = 4;
	self->hover = 0;
	self->lastplayposition = 0.f;
	self->sequenceentryoffset = 0.f;
	psy_audio_patterncursor_init(&self->oldcursor);
	psy_table_init(&self->channel);
	for (track = 0; track < 64; ++track) {
		psy_audio_PatternEntry* channelentry;

		channelentry = psy_audio_patternentry_allocinit();
		channelentry->offset = -1.0f;
		channelentry->track = track;
		psy_table_insert(&self->channel, track, channelentry);
	}	
	psy_signal_connect(&self->component.signal_destroy, self,
		pianogrid_ondestroy);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	psy_signal_connect(&self->workspace->signal_patterneditpositionchanged, self,
		pianogrid_onpatterneditpositionchanged);
}

void pianogrid_setsharedgridstate(Pianogrid* self, GridState* gridstate)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		gridstate_init(&self->defaultgridstate, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianogrid_setsharedkeyboardstate(Pianogrid* self, KeyboardState*
	keyboardstate)
{
	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate, NULL);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianogrid_setpattern(Pianogrid* self, psy_audio_Pattern* pattern)
{
	self->gridstate->pattern = pattern;	
	psy_ui_component_updateoverflow(&self->component);
}

void pianogrid_ondestroy(Pianogrid* self)
{
	psy_table_disposeall(&self->channel, (psy_fp_disposefunc)
		psy_audio_patternentry_dispose);
}

void pianogrid_ondraw(Pianogrid* self, psy_ui_Graphics* g)
{
	if (!gridstate_pattern(self->gridstate)) {
		return;
	}
	pianogrid_drawgrid(self, g);
	pianogrid_drawplaybar(self, g);
	pianogrid_drawentries(self, g);
	pianogrid_drawcursor(self, g);
}

void pianogrid_drawgrid(Pianogrid* self, psy_ui_Graphics* g)
{
	int keyboardheight;
	int keyheight;
	int key;
	int keymin;
	int keymax;	
	int cpy;
	psy_dsp_big_beat_t viewmin;
	psy_dsp_big_beat_t viewmax;
	psy_dsp_big_beat_t c;
	psy_dsp_big_beat_t bpl;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);	
	bpl = 1.0 / self->gridstate->lpb;
	viewmin = gridstate_pxtobeat(self->gridstate, g->clip.left);
	viewmin = (int)(viewmin * self->gridstate->lpb) / (psy_dsp_big_beat_t)self->gridstate->lpb;
	viewmax = min(psy_audio_pattern_length(self->gridstate->pattern),
		gridstate_pxtobeat(self->gridstate, g->clip.right));	
	keyheight = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
	keyboardheight = keyboardstate_height(self->keyboardstate, &tm);
	cpy = keyboardheight;
	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, self->workspace->song->properties.tracks));
	psy_ui_drawline(g,
		gridstate_beattopx(self->gridstate, viewmin), cpy,
		gridstate_beattopx(self->gridstate, viewmax), cpy);	
	keymin = max(self->keyboardstate->keymin,
		(keyboardheight - g->clip.bottom) / keyheight - 1);
	keymax = min(self->keyboardstate->keymax,
		(keyboardheight - g->clip.top + keyheight) / keyheight);
	bpl = 1.0 / self->gridstate->lpb;
	for (key = keymin; key <= keymax; ++key) {
		cpy = keyboardheight - (key)*keyheight;
		for (c = viewmin; c <= viewmax; c += bpl) {
			psy_ui_Colour linecolour;
			int line;
			psy_dsp_big_beat_t lineoffset;

			line = (int)(c * self->gridstate->lpb);
			lineoffset = line * bpl;
			linecolour = pianogrid_linecolour(self, line);
			if (isblack(key - 1)) {
				psy_ui_colour_add(&linecolour, -4, -4, -4);
			}
			psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				gridstate_beattopx(self->gridstate, lineoffset), cpy,
				gridstate_beattopx(self->gridstate, bpl - (lineoffset - c)) + 1,
				keyheight), linecolour);
		}
	}
	// draw step separators
	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, self->workspace->song->properties.tracks));	
	for (c = viewmin; c <= viewmax; c += bpl) {
		int cpx;
		
		cpx = gridstate_beattopx(self->gridstate, 
			gridstate_quantize(self->gridstate, c));
		psy_ui_drawline(g, cpx,
			psy_ui_component_scrolltop(&self->component),
			cpx, keyboardheight);
	}	
	// draw key separators at C and E
	for (key = keymin; key <= keymax; ++key) {
		cpy = keyboardheight - (key + 1) * keyheight;
		if (((key + 1) % 12) == 0 || ((key + 1) % 12) == 5) {
			psy_ui_drawline(g,
				gridstate_beattopx(self->gridstate, viewmin), cpy,
				gridstate_beattopx(self->gridstate, viewmax), cpy);
		}
	}
}

psy_ui_Colour pianogrid_linecolour(Pianogrid* self, int line)
{
	psy_ui_Colour rv;

	if ((line % 16) == 0) {		
		rv = patternviewskin_row4beatcolour(self->gridstate->skin, 0, 0);
	} else if ((line % 4) == 0) {
		rv = patternviewskin_rowbeatcolour(self->gridstate->skin, 0, 0);
	} else {
		rv = patternviewskin_rowcolour(self->gridstate->skin, 0, 0);
	}
	return rv;
}

void pianogrid_drawcursor(Pianogrid* self, psy_ui_Graphics* g)
{
	if (!self->gridstate->cursorchanging &&
			!(psy_audio_player_playing(&self->workspace->player) &&
			workspace_followingsong(self->workspace))) {
		psy_audio_PatternCursor cursor;
		int cpy;
		int key;
		psy_ui_TextMetric tm;
		int keyheight;

		tm = psy_ui_component_textmetric(&self->component);
		keyheight = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
		cursor = workspace_patterneditposition(self->workspace);
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = 0x48;
		}		
		cpy = keyboardstate_height(self->keyboardstate, &tm) - (key + 1) * keyheight;
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				gridstate_beattopx(self->gridstate, cursor.offset), cpy,
				gridstate_beattopx(self->gridstate, 1.0 / self->gridstate->lpb), keyheight),
			patternviewskin_cursorcolour(self->gridstate->skin,
				0, self->workspace->song->properties.tracks));
	}
}

void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics* g)
{
	if (psy_audio_player_playing(&self->workspace->player)) {
		psy_dsp_big_beat_t offset;

		offset = psy_audio_player_position(&self->workspace->player);
		if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset +
			self->gridstate->pattern->length) {
			int line;
			psy_dsp_big_beat_t patternoffset;
			psy_ui_TextMetric tm;

			tm = psy_ui_component_textmetric(&self->component);			
			line = (int)((offset - self->sequenceentryoffset) *
				self->gridstate->lpb);
			patternoffset = line / (psy_dsp_big_beat_t)self->gridstate->lpb;
			psy_ui_drawsolidrectangle(g,
				psy_ui_rectangle_make(
					gridstate_beattopx(self->gridstate, patternoffset), 0,
					gridstate_steppx(self->gridstate),
					keyboardstate_height(self->keyboardstate, &tm)),
				patternviewskin_playbarcolour(self->gridstate->skin,
					0, self->workspace->song->properties.tracks));
		}
	}
}

void pianogrid_drawentries(Pianogrid* self, psy_ui_Graphics* g)
{	
	
	if (self->gridstate->pattern) {
		psy_audio_PatternNode* curr;
		int track;

		curr = psy_audio_pattern_begin(self->gridstate->pattern);
		while (curr) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEntry* channelentry;

			entry = (psy_audio_PatternEntry*)(curr->entry);
			channelentry = (psy_audio_PatternEntry*)psy_table_at(&self->channel,
				entry->track);
			if (channelentry->offset != -1.0f) {
				pianogrid_drawentry(self, g,
					channelentry,					
					channelentry->offset,
					entry->offset - channelentry->offset,
					channelentry->delta > 0);
				channelentry->offset = -1.0f;
			}
			if (psy_audio_patternentry_front(entry)->note == 120) {
				channelentry->offset = -1.0f;
			} else {
				channelentry->offset = entry->offset;
				*psy_audio_patternentry_front(channelentry)
					= *psy_audio_patternentry_front(entry);
				channelentry->delta = 0;
				if (self->hover == entry) {
					channelentry->delta = 1;
				}
			}
			psy_audio_patternnode_next(&curr);
		}
		for (track = 0; track < 64; ++track) {
			psy_audio_PatternEntry* channelentry;

			channelentry = (psy_audio_PatternEntry*)psy_table_at(&self->channel, track);
			if (channelentry->offset != -1.0f) {
				pianogrid_drawentry(self, g, channelentry,					
					channelentry->offset,
					self->gridstate->pattern->length - channelentry->offset,
					channelentry->delta > 0);
				channelentry->offset = -1.f;
			}
		}
	}
}

void pianogrid_drawentry(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternEntry* entry, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length,
	int hover)
{
	psy_ui_Rectangle r;
	psy_audio_PatternEvent* event;
	psy_ui_Colour colour;
	psy_audio_PatternCursor editposition;
	psy_ui_Size corner;
	psy_ui_TextMetric tm;
	int keyheight;

	tm = psy_ui_component_textmetric(&self->component);
	keyheight = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
	editposition = workspace_patterneditposition(self->workspace);
	event = psy_audio_patternentry_front(entry);
	int left = (int)((offset) * self->gridstate->pxperbeat);
	int width = (int)(length * self->gridstate->pxperbeat);
	corner = psy_ui_size_makepx(2, 2);
	psy_ui_setrectangle(&r, left,
		(self->keyboardstate->keymax - event->note - 1) * keyheight
		+ 1, width, keyheight - 2);	
	if (hover) {
		colour = self->gridstate->eventhovercolour;
	} else
	if (entry->track == editposition.track) {
		colour = self->gridstate->eventcurrchannelcolour;
	} else {
		colour = self->gridstate->eventcolour;
	}
	psy_ui_drawsolidroundrectangle(g, r, corner, colour);
}

void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_TextMetric tm;
	int keyheight;

	tm = psy_ui_component_textmetric(&self->component);
	keyheight = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
	rv->height = psy_ui_value_makepx((self->keyboardstate->keymax - self->keyboardstate->keymin) *
		keyheight);
	if (self->gridstate->pattern) {
		rv->width = psy_ui_value_makepx((int)(
			self->gridstate->pxperbeat * psy_audio_pattern_length(self->gridstate->pattern)));
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	if (self->gridstate->pattern) {
		psy_dsp_big_beat_t offset;

		psy_ui_component_setfocus(&self->component);
		offset = pianogrid_quantizise(self, pianogrid_pxtobeat(self, ev->x));
		if (ev->button == 1) {
			psy_audio_PatternEvent patternevent;
			psy_ui_TextMetric tm;
			int keyheight;

			tm = psy_ui_component_textmetric(&self->component);
			keyheight = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
			psy_audio_patternevent_clear(&patternevent);
			patternevent.note = self->keyboardstate->keymax - 1 - ev->y / keyheight;
			pianogrid_storecursor(self);
			self->gridstate->cursor.offset = offset;
			self->gridstate->cursor.key = patternevent.note;
			workspace_setpatterneditposition(self->workspace, self->gridstate->cursor);
			psy_undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(self->gridstate->pattern, 0.25,
					self->gridstate->cursor, patternevent, self->workspace)->command);
		} else if (ev->button == 2) {
			psy_audio_PatternNode* prev;

			psy_audio_PatternNode* node = psy_audio_pattern_findnode(self->gridstate->pattern, self->gridstate->cursor.track,
				offset, 1 / (psy_dsp_big_beat_t)self->gridstate->lpb, &prev);
			if (!node) {
				node = prev;
			}
			if (node) {
				psy_audio_PatternNode* next;

				next = pianogrid_nextnode(self, node, self->gridstate->cursor.track);
				if (self->hover == psy_audio_patternnode_entry(node)) {
					self->hover = 0;
				}
				psy_audio_pattern_remove(self->gridstate->pattern, node);
				if (next) {
					if (psy_audio_patternentry_front(psy_audio_patternnode_entry(next))->note
						== psy_audio_NOTECOMMANDS_RELEASE) {
						if (self->hover == psy_audio_patternnode_entry(next)) {
							self->hover = 0;
						}
						psy_audio_pattern_remove(self->gridstate->pattern, next);
					}
				}
			}
		}
	}
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	if (self->gridstate->pattern) {
		psy_dsp_big_beat_t offset;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* next = 0;
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* hover;

		offset = pianogrid_quantizise(self, pianogrid_pxtobeat(self, ev->x - self->component.scroll.x));
		node = psy_audio_pattern_findnode(self->gridstate->pattern, self->gridstate->cursor.track, offset,
			1 / (psy_dsp_big_beat_t)self->gridstate->lpb, &prev);
		hover = self->hover;
		if (!node) {
			node = prev;
		}
		if (node) {
			self->hover = psy_audio_patternnode_entry(node);
		} else {
			self->hover = 0;
		}
		if (self->hover != hover) {
			psy_ui_component_invalidate(&self->component);
		}
		if (node) {
			next = pianogrid_nextnode(self, node, self->gridstate->cursor.track);
			if (next) {
				if (psy_audio_patternnode_entry(next)->offset - offset == 0.25) {
					psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
				}
			}
		}
	}
}

psy_audio_PatternNode* pianogrid_nextnode(Pianogrid* self, psy_audio_PatternNode* node,
	uintptr_t track)
{
	psy_audio_PatternNode* rv;

	if (node) {
		rv = node->next;
		while (rv) {
			psy_audio_PatternEntry* entry;

			entry = psy_audio_patternnode_entry(rv);
			if (entry->track == track) {
				break;
			}
			rv = rv->next;
		}
	} else {
		rv = 0;
	}
	return rv;
}

psy_dsp_big_beat_t pianogrid_pxtobeat(Pianogrid* self, int px)
{
	return (px / (psy_dsp_big_beat_t)self->gridstate->pxperbeat);
}

psy_dsp_big_beat_t pianogrid_quantizise(Pianogrid* self, psy_dsp_big_beat_t offset)
{
	return (int)(offset * (psy_dsp_big_beat_t)self->gridstate->lpb) /
		(psy_dsp_big_beat_t)self->gridstate->lpb;
}

void pianogrid_onpatterneditpositionchanged(Pianogrid* self,
	Workspace* sender)
{
	psy_audio_PatternCursor oldcursor;

	oldcursor = self->gridstate->cursor;
	self->gridstate->cursor = workspace_patterneditposition(sender);
	if (self->gridstate->cursor.track != oldcursor.track) {
		psy_ui_component_invalidate(&self->component);
	} else if (oldcursor.offset != self->gridstate->cursor.offset && !self->gridstate->cursorchanging) {
		pianogrid_invalidateline(self, oldcursor.offset);
		pianogrid_invalidateline(self, self->gridstate->cursor.offset);
	}
	self->gridstate->cursorchanging = FALSE;
	if (psy_audio_player_playing(&sender->player) && workspace_followingsong(sender)) {
		bool scrolldown;

		scrolldown = self->lastplayposition <
			psy_audio_player_position(&self->workspace->player);
		pianogrid_invalidateline(self, self->lastplayposition);		
		self->lastplayposition = psy_audio_player_position(&self->workspace->player);
		pianogrid_invalidateline(self, self->lastplayposition);		
		if (self->lastplayposition >= self->sequenceentryoffset &&
			self->lastplayposition < self->sequenceentryoffset +
			self->gridstate->pattern->length) {
			if (scrolldown != FALSE) {
				pianogrid_scrollright(self, self->gridstate->cursor);
			} else {
				pianogrid_scrollleft(self, self->gridstate->cursor);
			}
		}
		pianogrid_storecursor(self);
	}
}

int pianogrid_scrollright(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	int line;
	int visilines;
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;	

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);

	visilines = (int)((size.width / (psy_dsp_big_beat_t)self->gridstate->pxperbeat) * self->gridstate->lpb);
	//if (self->midline) {
		//visilines /= 2;
	//} else {
	--visilines;
	//}
	line = gridstate_steps(self->gridstate, cursor.offset);
	if (visilines < line - psy_ui_component_scrollleft(&self->component) /
			gridstate_steppx(self->gridstate) + 2) {
		int dlines;

		dlines = line - psy_ui_component_scrollleft(&self->component) /
			gridstate_steppx(self->gridstate) - visilines + 16;
		self->gridstate->cursorchanging = TRUE;
		psy_ui_component_setscrollleft(&self->component,
			max(0, psy_ui_component_scrollleft(&self->component) +
			self->component.scrollstepx * dlines));
		return FALSE;
	}
	return TRUE;
}

void pianogrid_prevline(Pianogrid* self)
{
	pianogrid_prevlines(self, workspace_cursorstep(self->workspace),
		TRUE);
}

void pianogrid_prevlines(Pianogrid* self, uintptr_t lines, int wrap)
{
	if (self->gridstate->pattern) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			self->gridstate->pattern, 1.0 / self->gridstate->lpb, wrap);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_prevlines(&cursornavigator, lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterneditposition(self->workspace, self->gridstate->cursor);
		pianogrid_invalidatecursor(self);
	}
}

void pianogrid_advanceline(Pianogrid* self)
{
	pianogrid_advancelines(self, workspace_cursorstep(
		self->workspace), TRUE);
}

void pianogrid_advancelines(Pianogrid* self, uintptr_t lines, int wrap)
{
	if (self->gridstate->pattern) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			self->gridstate->pattern, 1.0/self->gridstate->lpb, wrap);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterneditposition(self->workspace, self->gridstate->cursor);
		pianogrid_invalidatecursor(self);
	}
}

int pianogrid_scrollleft(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	int line;
	int topline;
	psy_ui_Rectangle r;

	line = gridstate_steps(self->gridstate, cursor.offset);
	psy_ui_setrectangle(&r,
		gridstate_steppx(self->gridstate) * line,
		0,
		gridstate_steppx(self->gridstate),
		200);
	/*if (self->midline) {
		psy_ui_Size gridsize;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		gridsize = psy_ui_component_size(&self->component);
		topline = psy_ui_value_px(&gridsize.height, &tm) / self->linestate->lineheight / 2;
	} */
	// else {
		topline = 0;
	//}
	if (psy_ui_component_scrollleft(&self->component) + topline * gridstate_steppx(self->gridstate) > r.left) {
		int dlines = (psy_ui_component_scrollleft(&self->component) + topline * gridstate_steppx(self->gridstate) - r.left) /
			(gridstate_steppx(self->gridstate));
		self->gridstate->cursorchanging = TRUE;
		psy_ui_component_setscrollleft (&self->component,
			max(0, psy_ui_component_scrollleft(&self->component) -
			self->component.scrollstepx * dlines));
		return FALSE;
	}
	return TRUE;
}

void pianogrid_invalidateline(Pianogrid* self, psy_dsp_big_beat_t offset)
{
	if (offset >= self->sequenceentryoffset && offset < self->sequenceentryoffset +
			psy_audio_pattern_length(gridstate_pattern(self->gridstate))) {
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);		
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_rectangle_make(
				gridstate_beattopx(self->gridstate, gridstate_quantize(
					self->gridstate, offset - self->sequenceentryoffset)),
				psy_ui_component_scrolltop(&self->component),
				gridstate_steppx(self->gridstate),
				size.height));
	}
}

void pianogrid_invalidatecursor(Pianogrid* self)
{
	pianogrid_invalidateline(self, self->oldcursor.offset);
	pianogrid_invalidateline(self, self->gridstate->cursor.offset);
}

void pianogrid_storecursor(Pianogrid* self)
{
	self->oldcursor = self->gridstate->cursor;
}

// Pianoroll
// protoypes
static void pianoroll_ontimer(Pianoroll*, uintptr_t timerid);
static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player* sender,
	uintptr_t lpb);
static void pianoroll_onalign(Pianoroll*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_ongridscroll(Pianoroll*, psy_ui_Component* sender);
static void pianoroll_onzoomboxbeatwidthchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onzoomboxkeyheightchanged(Pianoroll*, ZoomBox* sender);
static bool pianoroll_handlecommand(Pianoroll*, int cmd);
static void pianoroll_oneventdriverinput(Pianoroll* self, psy_EventDriver* sender);
// vtable
static psy_ui_ComponentVtable pianoroll_vtable;
static int pianoroll_vtable_initialized = 0;

static void pianoroll_vtable_init(Pianoroll* self)
{
	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign = (psy_ui_fp_component_onalign)pianoroll_onalign;
		pianoroll_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianoroll_onmousedown;		
		pianoroll_vtable.ontimer = (psy_ui_fp_component_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = 1;
	}
}
// implenentation
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	PatternViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	pianoroll_vtable_init(self);
	self->component.vtable = &pianoroll_vtable;	
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;
	self->pattern = 0;	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	// shared states
	keyboardstate_init(&self->keyboardstate, skin);
	gridstate_init(&self->gridstate, skin);
	// left area (keyboardheader, keyboard)
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_enablealign(&self->left);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_beatwidth, &self->left);
	psy_signal_connect(&self->zoombox_beatwidth.signal_changed, self,
		pianoroll_onzoomboxbeatwidthchanged);
	psy_ui_component_setalign(&self->zoombox_beatwidth.component, psy_ui_ALIGN_TOP);
	// Keyboard
	pianokeyboard_init(&self->keyboard, &self->left, &self->keyboardstate);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_CLIENT);	
	zoombox_init(&self->zoombox_keyheight, &self->left);
	psy_signal_connect(&self->zoombox_keyheight.signal_changed, self,
		pianoroll_onzoomboxkeyheightchanged);
	psy_ui_component_setalign(&self->zoombox_keyheight.component, psy_ui_ALIGN_BOTTOM);
	// top area (beatruler)
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	pianoheader_init(&self->header, &self->top, &self->gridstate);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// client area (event grid)
	pianogrid_init(&self->grid, &self->component, &self->keyboardstate,
		&self->gridstate, self->workspace);
	psy_ui_scroller_init(&self->scroller, &self->grid.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&self->grid.component.signal_scroll, self,
		pianoroll_ongridscroll);
	psy_signal_connect(&self->workspace->player.eventdrivers.signal_input, self,
		pianoroll_oneventdriverinput);
	psy_ui_component_starttimer(&self->component, 0, 100);
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	pianogrid_setpattern(&self->grid, pattern);
	psy_ui_component_setscroll(&self->grid.component, psy_ui_intpoint_make(0, psy_ui_component_scrolltop(&self->grid.component)));
	psy_ui_component_setscroll(&self->header.component, psy_ui_intpoint_make(0, 0));
	pianoroll_updatescroll(self);	
}

void pianoroll_ontimer(Pianoroll* self, uintptr_t timerid)
{
	if (self->pattern && psy_ui_component_visible(&self->component)) {
		if (psy_audio_player_playing(&self->workspace->player)) {
			int line;
			line = (int)(psy_audio_player_position(&self->workspace->player) /
				self->gridstate.lpb);
			if (self->grid.lastplayposition / self->gridstate.lpb != line) {
				pianogrid_invalidateline(&self->grid, self->grid.lastplayposition);
				self->grid.lastplayposition =
					psy_audio_player_position(&self->workspace->player);
				pianogrid_invalidateline(&self->grid, self->grid.lastplayposition);
			}
		} else {
			if (self->grid.lastplayposition != -1) {
				pianogrid_invalidateline(&self->grid, self->grid.lastplayposition);
				self->grid.lastplayposition = -1;
			}
		}
		if (self->pattern && self->pattern->opcount != self->opcount &&
				self->syncpattern) {
			psy_ui_component_invalidate(&self->grid.component);
		}
		self->opcount = self->pattern ? self->pattern->opcount : 0;
	}
}

void pianoroll_onalign(Pianoroll* self)
{
	pianoroll_updatescroll(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(&self->grid.component);
}

void pianoroll_updatescroll(Pianoroll* self)
{	
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->component);
	self->grid.component.scrollstepx = gridstate_steppx(&self->gridstate);
	self->grid.component.scrollstepy = psy_ui_value_px(
		&self->keyboardstate.keyheight, &tm);
	self->keyboard.component.scrollstepy = psy_ui_value_px(
		&self->keyboardstate.keyheight, &tm);
	self->header.component.scrollstepx = self->grid.component.scrollstepx;
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_invalidate(&self->keyboard.component);
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	gridstate_setlpb(&self->gridstate, lpb);	
	self->grid.component.scrollstepx = gridstate_steppx(&self->gridstate);
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_setscrollleft(&self->header.component,
		psy_ui_component_scrollleft(&self->grid.component));
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_update(&self->header.component);
	psy_ui_component_invalidate(&self->grid.component);
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleft(&self->grid.component) !=
			psy_ui_component_scrollleft(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->grid.component));		
	}
	if (psy_ui_component_scrolltop(&self->grid.component) !=
			psy_ui_component_scrolltop(&self->keyboard.component)) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(&self->grid.component));		
	}
}

void pianoroll_onzoomboxbeatwidthchanged(Pianoroll* self, ZoomBox* sender)
{	
	self->gridstate.pxperbeat = (int)(self->gridstate.defaultbeatwidth *
		zoombox_rate(sender));
	self->grid.component.scrollstepx = gridstate_steppx(&self->gridstate);
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_setscrollleft(&self->header.component,
		psy_ui_component_scrollleft(&self->grid.component));
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_update(&self->header.component);
	psy_ui_component_invalidate(&self->grid.component);
}

void pianoroll_onzoomboxkeyheightchanged(Pianoroll* self, ZoomBox* sender)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->keyboardstate.keyheight = psy_ui_mul_value_real(
		self->keyboardstate.defaultkeyheight, zoombox_rate(sender));
	self->grid.component.scrollstepy = psy_ui_value_px(
		&self->keyboardstate.keyheight, &tm);
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_setscrolltop(&self->keyboard.component,
		psy_ui_component_scrolltop(&self->grid.component));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);
	psy_ui_component_invalidate(&self->grid.component);
}

void pianoroll_oneventdriverinput(Pianoroll* self, psy_EventDriver* sender)
{
	if (psy_ui_component_hasfocus(&self->grid.component)) {
		psy_EventDriverCmd cmd;

		cmd = psy_eventdriver_getcmd(sender, "pianoroll");
		if (cmd.id != -1) {
			pianoroll_handlecommand(self, cmd.id);
		}
	}
}

void pianoroll_makecmds(psy_Property* parent)
{
	psy_Property* cmds;

	cmds = psy_property_settext(
		psy_property_append_section(parent, "pianoroll"),
		"Pianoroll");
	definecmd(cmds, CMD_NAVUP,
		psy_ui_KEY_UP, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navup", "up");
	definecmd(cmds, CMD_NAVDOWN,
		psy_ui_KEY_DOWN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navdown", "down");
	definecmd(cmds, CMD_NAVLEFT,
		psy_ui_KEY_LEFT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navleft", "left");
	definecmd(cmds, CMD_NAVRIGHT,
		psy_ui_KEY_RIGHT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navright", "right");
	definecmd(cmds, CMD_ENTER,
		psy_ui_KEY_SPACE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"enter", "enter");
}

bool pianoroll_handlecommand(Pianoroll* self, int cmd)
{
	bool handled = TRUE;
	switch (cmd) {
	case CMD_NAVUP:
		if (self->gridstate.cursor.key < self->keyboardstate.keymax - 1) {
			pianogrid_storecursor(&self->grid);
			++self->gridstate.cursor.key;
			workspace_setpatterneditposition(self->workspace,
				self->gridstate.cursor);
			pianogrid_invalidatecursor(&self->grid);
		}
		break;
	case CMD_NAVDOWN:
		if (self->gridstate.cursor.key > self->keyboardstate.keymin) {
			pianogrid_storecursor(&self->grid);
			--self->gridstate.cursor.key;
			workspace_setpatterneditposition(self->workspace,
				self->gridstate.cursor);
			pianogrid_invalidatecursor(&self->grid);
		}
		break;
	case CMD_NAVLEFT:
		pianogrid_prevline(&self->grid);		
		break;
	case CMD_NAVRIGHT:
		pianogrid_advanceline(&self->grid);		
		break;
	case CMD_ENTER: {
		psy_audio_PatternEvent patternevent;

		psy_audio_patternevent_clear(&patternevent);
		patternevent.note = self->gridstate.cursor.key;
		psy_undoredo_execute(&self->workspace->undoredo,
			&InsertCommandAlloc(self->pattern, 0.25,
				self->gridstate.cursor, patternevent, self->workspace)->command);
		break; }
	default:
		handled = FALSE;
		break;
	}
	return handled;
}

// Defines a property with shortcut defaults for the keyboard driver
// key		: cmd id used by the trackerview
// text		: "cmds.key" language dictionary key used by the translator
// shorttext: short description for the keyboard help view
// value	: encoded key shortcut (keycode/shift/ctrl)
void definecmd(psy_Property* cmds, int cmd, uintptr_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint(psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(cmds, key,
			psy_audio_encodeinput(keycode, shift, ctrl), 0, 0),
			cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}
