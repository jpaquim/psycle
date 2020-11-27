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

// KeyboardState
void keyboardstate_init(KeyboardState* self, PatternViewSkin* skin)
{
	self->keymin = 0;
	self->keymax = 88;	
	self->defaultkeyheight = psy_ui_value_makeeh(1.0);
	self->keyheight = self->defaultkeyheight;
	self->notemode = psy_dsp_NOTESTAB_A220;
	self->drawpianokeys = TRUE;
	self->skin = skin;
}

// GridState
void gridstate_init(GridState* self, PatternViewSkin* skin)
{
	self->pattern = NULL;
	self->defaultbeatwidth = 90;
	self->pxperbeat = self->defaultbeatwidth;
	self->lpb = 4;	
	self->skin = skin;
	self->cursorchanging = FALSE;
	psy_audio_patterncursor_init(&self->cursor);
}

// Header
static void pianoheader_ondraw(PianoHeader*, psy_ui_Graphics*);
static PatternSelection pianoheader_clipselection(PianoHeader*,
	psy_ui_Rectangle clip);
static void pianoheader_drawruler(PianoHeader*, psy_ui_Graphics*, PatternSelection);
void pianoheader_drawbeat(PianoHeader*, psy_ui_Graphics*, int beat, int x,
	int baseline, int tmheight);
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
	rv->width = (gridstate_pattern(self->gridstate))
		? psy_ui_value_makepx(gridstate_beattopx(self->gridstate,
		  psy_audio_pattern_length(gridstate_pattern(self->gridstate))))
		: psy_ui_value_makepx(0);	
}

void pianoheader_ondraw(PianoHeader* self, psy_ui_Graphics* g)
{	
	pianoheader_drawruler(self, g, pianoheader_clipselection(self, g->clip));
}

PatternSelection pianoheader_clipselection(PianoHeader* self,
	psy_ui_Rectangle clip)
{
	PatternSelection rv;

	rv.topleft.offset = gridstate_quantize(self->gridstate,
		gridstate_pxtobeat(self->gridstate, clip.left));
	rv.bottomright.offset = min(
		psy_audio_pattern_length(gridstate_pattern(self->gridstate)),
		gridstate_pxtobeat(self->gridstate, clip.right));	
	return rv;
}

void pianoheader_drawruler(PianoHeader* self, psy_ui_Graphics* g,
	PatternSelection clip)
{	
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	int baselinetop;
	int baseline;
	psy_dsp_big_beat_t c;	
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);	
	baseline = size.height - 1;
	baselinetop = baseline - tm.tmHeight / 3;
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
	psy_ui_drawline(g, psy_ui_component_scrollleft(&self->component), baseline,
		size.width + psy_ui_component_scrollleft(&self->component),
		baseline);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (c = clip.topleft.offset; c <= clip.bottomright.offset;
			c += gridstate_step(self->gridstate)) {
		int cpx;
		int steps;
		
		steps = gridstate_steps(self->gridstate, c);
		cpx = gridstate_beattopx(self->gridstate, c);
		psy_ui_drawline(g, cpx, baseline, cpx, baselinetop);
		if ((steps % self->gridstate->lpb) == 0) {
			pianoheader_drawbeat(self, g, (int)(c), cpx, baseline, tm.tmHeight);
		}
	}
}

void pianoheader_drawbeat(PianoHeader* self, psy_ui_Graphics* g, int beat,
	int x, int baseline, int tmheight)
{	
	char txt[40];

	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
	psy_ui_drawline(g, x, baseline, x, baseline - tmheight / 2);
	psy_snprintf(txt, 40, "%d", beat);
	psy_ui_textout(g, x + 3, baseline - tmheight, txt, strlen(txt));
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
}

// PianoKeyboard
// prototypes
static void pianokeyboard_ondraw(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_onmousedown(PianoKeyboard*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable pianokeyboard_vtable;
static bool pianokeyboard_vtable_initialized = FALSE;

static void pianokeyboard_vtable_init(PianoKeyboard* self)
{
	if (!pianokeyboard_vtable_initialized) {
		pianokeyboard_vtable = *(self->component.vtable);
		pianokeyboard_vtable.ondraw = (psy_ui_fp_component_ondraw)
			pianokeyboard_ondraw;
		pianokeyboard_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianokeyboard_onmousedown;
		pianokeyboard_vtable_initialized = TRUE;
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
	psy_ui_component_preventalign(&self->component);
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
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	self->keyboardstate->keyboardheightpx = keyboardstate_height(self->keyboardstate, &tm);
	self->keyboardstate->keyheightpx = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
	psy_ui_setcolour(g, patternviewskin_keyseparatorcolour(self->keyboardstate->skin));
	if (!self->keyboardstate->drawpianokeys) {
		psy_ui_settextcolour(g, patternviewskin_keywhitecolour(self->keyboardstate->skin));
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		int cpy;

		cpy = keyboardstate_keypx(self->keyboardstate, key);
		psy_ui_drawline(g, 0, cpy, psy_ui_value_px(&size.width, &tm), cpy);
		if (self->keyboardstate->drawpianokeys) {
			if (isblack(key)) {
				psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					(int)(psy_ui_value_px(&size.width, &tm) * 0.75), cpy,
					(int)(psy_ui_value_px(&size.width, &tm) * 0.25),
					self->keyboardstate->keyheightpx),
					patternviewskin_keywhitecolour(self->keyboardstate->skin));
				psy_ui_drawline(g,
					0, cpy + self->keyboardstate->keyheightpx / 2,
					psy_ui_value_px(&size.width, &tm),
					cpy + self->keyboardstate->keyheightpx / 2);
				psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					0, cpy, (int)(psy_ui_value_px(&size.width, &tm) * 0.75),
					self->keyboardstate->keyheightpx),
					patternviewskin_keyblackcolour(self->keyboardstate->skin));
			} else {
				psy_ui_Rectangle r;

				r = psy_ui_rectangle_make(0, cpy, psy_ui_value_px(&size.width, &tm),
					self->keyboardstate->keyheightpx);
				psy_ui_drawsolidrectangle(g, r,
					patternviewskin_keywhitecolour(self->keyboardstate->skin));
				if (key % 12 == 0 || ((key % 12) == 5)) {
					psy_ui_drawline(g,
						0, cpy + self->keyboardstate->keyheightpx, psy_ui_value_px(&size.width, &tm),
						cpy + self->keyboardstate->keyheightpx);
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
				self->keyboardstate->keyheightpx);
			psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_CLIPPED, r,
				psy_dsp_notetostr(key, self->keyboardstate->notemode),
				strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
			psy_ui_drawline(g,
				0, cpy + self->keyboardstate->keyheightpx,
				psy_ui_value_px(&size.width, &tm),
				cpy + self->keyboardstate->keyheightpx);
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

// Pianogrid
// prototypes
static void pianogrid_ondestroy(Pianogrid*);
static void pianogrid_ondraw(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_updatekeystate(Pianogrid*);
static void pianogrid_drawgrid(Pianogrid*, psy_ui_Graphics*, PatternSelection);
static PatternSelection pianogrid_clipselection(Pianogrid*, psy_ui_Rectangle
	clip);
static void pianogrid_drawgridcells(Pianogrid*, psy_ui_Graphics*,
	PatternSelection clip);
static void pianogrid_drawstepseparators(Pianogrid*, psy_ui_Graphics*,
	PatternSelection clip);
static void pianogrid_drawkeyseparators(Pianogrid* self, psy_ui_Graphics*,
	PatternSelection clip);
static void pianogrid_drawplaybar(Pianogrid*, psy_ui_Graphics*, PatternSelection);
static void pianogrid_drawentries(Pianogrid*, psy_ui_Graphics*, PatternSelection);
static psy_audio_PatternEntry* pianogrid_channelentry_at(Pianogrid* self, uintptr_t track);
static void pianogrid_drawentry(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternEntry*, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length,
	int hover);
static void pianogrid_drawcursor(Pianogrid*, psy_ui_Graphics*, PatternSelection);
static psy_ui_Colour pianogrid_linecolour(Pianogrid* self, int line);
static void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static psy_audio_PatternNode* pianogrid_nextnode(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track);
static void pianogrid_onpatterncursorchanged(Pianogrid*, Workspace* sender);
static int pianogrid_scrollright(Pianogrid*, psy_audio_PatternCursor cursor);
static int pianogrid_scrollleft(Pianogrid*, psy_audio_PatternCursor cursor);
static void pianogrid_prevline(Pianogrid*);
static void pianogrid_prevlines(Pianogrid*, uintptr_t lines, int wrap);
static void pianogrid_advanceline(Pianogrid*);
static void pianogrid_advancelines(Pianogrid*, uintptr_t lines, int wrap);
static void pianogrid_channelentries_clear(Pianogrid*);

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
	psy_signal_connect(&self->component.signal_destroy, self,
		pianogrid_ondestroy);
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);
	psy_signal_connect(&self->workspace->signal_patterneditpositionchanged, self,
		pianogrid_onpatterncursorchanged);
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

void pianogrid_onpatternchange(Pianogrid* self, psy_audio_Pattern* pattern)
{	
	pianogrid_channelentries_clear(self);
	psy_ui_component_updateoverflow(&self->component);
}

void pianogrid_ondestroy(Pianogrid* self)
{
	psy_table_disposeall(&self->channel, (psy_fp_disposefunc)
		psy_audio_patternentry_dispose);
}

void pianogrid_ondraw(Pianogrid* self, psy_ui_Graphics* g)
{
	PatternSelection clip;

	if (!gridstate_pattern(self->gridstate)) {
		return;
	}	
	pianogrid_updatekeystate(self);
	clip = pianogrid_clipselection(self, g->clip);
	pianogrid_drawgrid(self, g, clip);
	pianogrid_drawplaybar(self, g, clip);
	pianogrid_drawentries(self, g, clip);
	pianogrid_drawcursor(self, g, clip);
}

void pianogrid_updatekeystate(Pianogrid* self)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->keyboardstate->keyheightpx = psy_ui_value_px(
		&self->keyboardstate->keyheight, &tm);
	self->keyboardstate->keyboardheightpx = keyboardstate_height(
		self->keyboardstate, &tm);
}

void pianogrid_drawgrid(Pianogrid* self, psy_ui_Graphics* g, PatternSelection clip)
{
	pianogrid_drawgridcells(self, g, clip);	
	pianogrid_drawstepseparators(self, g, clip);
	pianogrid_drawkeyseparators(self, g, clip);
}

PatternSelection pianogrid_clipselection(Pianogrid* self,
	psy_ui_Rectangle clip)
{
	PatternSelection rv;

	rv.topleft.offset = gridstate_quantize(self->gridstate,
		gridstate_pxtobeat(self->gridstate, clip.left));
	rv.bottomright.offset = min(
		psy_audio_pattern_length(gridstate_pattern(self->gridstate)),
		gridstate_pxtobeat(self->gridstate, clip.right));
	rv.bottomright.key = max(
		self->keyboardstate->keymin,
		(self->keyboardstate->keyboardheightpx - clip.bottom) /
		self->keyboardstate->keyheightpx - 1);
	rv.topleft.key = min(
		self->keyboardstate->keymax,
		(self->keyboardstate->keyboardheightpx - clip.top +
			self->keyboardstate->keyheightpx) /
			self->keyboardstate->keyheightpx);
	return rv;
}

void pianogrid_drawgridcells(Pianogrid* self, psy_ui_Graphics* g,
	PatternSelection clip)
{
	psy_dsp_big_beat_t c;
	int key;
	int cpy;
	
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {
		cpy = keyboardstate_keypx(self->keyboardstate, key);
		for (c = clip.topleft.offset; c <= clip.bottomright.offset;
				c += gridstate_step(self->gridstate)) {
			psy_ui_Colour colour;
			int steps;
			psy_dsp_big_beat_t offset_quantized;

			steps = gridstate_steps(self->gridstate, c);
			colour = pianogrid_linecolour(self, steps);
			offset_quantized = gridstate_quantize(self->gridstate, c);
			if (isblack(key)) {
				psy_ui_colour_add(&colour, -4, -4, -4);
			}
			psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				gridstate_beattopx(self->gridstate, offset_quantized), cpy,
				gridstate_beattopx(self->gridstate,
					gridstate_step(self->gridstate) -
					(offset_quantized - c)) + 1,
				self->keyboardstate->keyheightpx), colour);
		}
	}
}

psy_ui_Colour pianogrid_linecolour(Pianogrid* self, int line)
{
	psy_ui_Colour rv;

	if ((line % (self->gridstate->lpb * 4)) == 0) {
		rv = patternviewskin_row4beatcolour(self->gridstate->skin, 0, 0);
	} else if ((line % self->gridstate->lpb) == 0) {
		rv = patternviewskin_rowbeatcolour(self->gridstate->skin, 0, 0);
	} else {
		rv = patternviewskin_rowcolour(self->gridstate->skin, 0, 0);
	}
	return rv;
}

void pianogrid_drawstepseparators(Pianogrid* self, psy_ui_Graphics* g,
	PatternSelection clip)
{
	psy_dsp_big_beat_t c;

	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, self->workspace->song->properties.tracks));
	for (c = clip.topleft.offset; c <= clip.bottomright.offset;
			c += gridstate_step(self->gridstate)) {
		int cpx;

		cpx = gridstate_beattopx(self->gridstate,
			gridstate_quantize(self->gridstate, c));
		psy_ui_drawline(g, cpx,
			psy_ui_component_scrolltop(&self->component),
			cpx, self->keyboardstate->keyboardheightpx);
	}
}

void pianogrid_drawkeyseparators(Pianogrid* self, psy_ui_Graphics* g,
	PatternSelection clip)
{	
	int key;	

	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, self->workspace->song->properties.tracks));
	// draw key separators at C and E
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {		
		if (((key + 1) % 12) == 0 || ((key + 1) % 12) == 5) {
			int cpy;

			cpy = keyboardstate_keypx(self->keyboardstate, key);
			psy_ui_drawline(g,
				gridstate_beattopx(self->gridstate, clip.topleft.offset),
				cpy,
				gridstate_beattopx(self->gridstate, clip.bottomright.offset),
				cpy);
		}
	}
}

void pianogrid_drawcursor(Pianogrid* self, psy_ui_Graphics* g, PatternSelection clip)
{
	if (!self->gridstate->cursorchanging &&
			!(psy_audio_player_playing(&self->workspace->player) &&
			workspace_followingsong(self->workspace))) {
		psy_audio_PatternCursor cursor;		
		int key;
						
		cursor = workspace_patterncursor(self->workspace);
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = 0x48;
		}				
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				gridstate_beattopx(self->gridstate, cursor.offset),
				keyboardstate_keypx(self->keyboardstate, key),
				gridstate_beattopx(self->gridstate, 1.0 / self->gridstate->lpb),
				self->keyboardstate->keyheightpx),
			patternviewskin_cursorcolour(self->gridstate->skin,
				0, self->workspace->song->properties.tracks));
	}
}

void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics* g, PatternSelection clip)
{
	if (psy_audio_player_playing(&self->workspace->player)) {
		psy_dsp_big_beat_t offset;

		offset = psy_audio_player_position(&self->workspace->player);
		if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset +
			gridstate_pattern(self->gridstate)->length) {
			int line;
			psy_dsp_big_beat_t patternoffset;			

			line = (int)((offset - self->sequenceentryoffset) *
				self->gridstate->lpb);
			patternoffset = line / (psy_dsp_big_beat_t)self->gridstate->lpb;
			psy_ui_drawsolidrectangle(g,
				psy_ui_rectangle_make(
					gridstate_beattopx(self->gridstate, patternoffset), 0,
					gridstate_steppx(self->gridstate),
					self->keyboardstate->keyboardheightpx),
				patternviewskin_playbarcolour(self->gridstate->skin,
					0, self->workspace->song->properties.tracks));
		}
	}
}

void pianogrid_drawentries(Pianogrid* self, psy_ui_Graphics* g, PatternSelection clip)
{	
	
	if (gridstate_pattern(self->gridstate)) {		
		psy_audio_PatternNode* curr;
		psy_TableIterator it;
		
		curr = psy_audio_pattern_begin(gridstate_pattern(self->gridstate));
		while (curr) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEntry* channelentry;

			entry = (psy_audio_PatternEntry*)psy_list_entry(curr);
			channelentry = pianogrid_channelentry_at(self, entry->track);
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

		for (it = psy_table_begin(&self->channel);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_PatternEntry* channelentry;

			channelentry = (psy_audio_PatternEntry*)psy_tableiterator_value(&it);
			if (channelentry->offset != -1.0f) {
				pianogrid_drawentry(self, g, channelentry,					
					channelentry->offset,
					gridstate_pattern(self->gridstate)->length - channelentry->offset,
					channelentry->delta > 0);
				channelentry->offset = -1.f;
			}
		}
	}
}

void pianogrid_channelentries_clear(Pianogrid* self)
{
	psy_table_disposeall(&self->channel, (psy_fp_disposefunc)
		psy_audio_patternentry_dispose);
	psy_table_init(&self->channel);
}

psy_audio_PatternEntry* pianogrid_channelentry_at(Pianogrid* self, uintptr_t track)
{
	psy_audio_PatternEntry* rv;

	rv = psy_table_at(&self->channel, track);
	if (!rv) {
		rv = psy_audio_patternentry_allocinit();
		if (rv) {
			rv->offset = -1.0f;
			rv->track = track;
			psy_table_insert(&self->channel, track, rv);
		}
	}
	return rv;
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
	
	editposition = workspace_patterncursor(self->workspace);
	event = psy_audio_patternentry_front(entry);
	int left = (int)((offset) * self->gridstate->pxperbeat);
	int width = (int)(length * self->gridstate->pxperbeat);
	corner = psy_ui_size_makepx(2, 2);
	psy_ui_setrectangle(&r, left,
		(self->keyboardstate->keymax - event->note - 1) *
			self->keyboardstate->keyheightpx + 1, width,
		self->keyboardstate->keyheightpx - 2);
	if (hover) {
		colour = patternviewskin_eventhovercolour(self->gridstate->skin, 0, 0);
	} else
	if (entry->track == editposition.track) {
		colour = patternviewskin_eventcurrchannelcolour(self->gridstate->skin,
			0, 0);
	} else {
		colour = patternviewskin_eventcolour(self->gridstate->skin, 0, 0);
	}
	psy_ui_drawsolidroundrectangle(g, r, corner, colour);
}

void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	pianogrid_updatekeystate(self);
	rv->height = psy_ui_value_makepx((self->keyboardstate->keymax -
		self->keyboardstate->keymin) * self->keyboardstate->keyheightpx);
	if (gridstate_pattern(self->gridstate)) {
		rv->width = psy_ui_value_makepx((int)(self->gridstate->pxperbeat *
			psy_audio_pattern_length(gridstate_pattern(self->gridstate))));
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	if (gridstate_pattern(self->gridstate)) {
		psy_dsp_big_beat_t offset;

		psy_ui_component_setfocus(&self->component);
		offset = gridstate_quantize(self->gridstate,
				gridstate_pxtobeat(self->gridstate, ev->x));
		if (ev->button == 1) {
			psy_audio_PatternEvent patternevent;						

			
			psy_audio_patternevent_clear(&patternevent);
			pianogrid_updatekeystate(self);
			patternevent.note = keyboardstate_pxtokey(self->keyboardstate, ev->y);
			pianogrid_storecursor(self);
			self->gridstate->cursor.offset = offset;
			self->gridstate->cursor.key = patternevent.note;
			workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
			psy_undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(
					gridstate_pattern(self->gridstate),
					gridstate_step(self->gridstate),
					self->gridstate->cursor, patternevent,
						self->workspace)->command);
		} else if (ev->button == 2) {
			psy_audio_PatternNode* prev;

			psy_audio_PatternNode* node = psy_audio_pattern_findnode(
				gridstate_pattern(self->gridstate),
				self->gridstate->cursor.track,
				offset, gridstate_step(self->gridstate), &prev);
			if (!node) {
				node = prev;
			}
			if (node) {
				psy_audio_PatternNode* next;

				next = pianogrid_nextnode(self, node, self->gridstate->cursor.track);
				if (self->hover == psy_audio_patternnode_entry(node)) {
					self->hover = 0;
				}
				psy_audio_pattern_remove(gridstate_pattern(self->gridstate), node);
				if (next) {
					if (psy_audio_patternentry_front(psy_audio_patternnode_entry(next))->note
							== psy_audio_NOTECOMMANDS_RELEASE) {
						if (self->hover == psy_audio_patternnode_entry(next)) {
							self->hover = 0;
						}
						psy_audio_pattern_remove(gridstate_pattern(self->gridstate), next);
					}
				}
			}
		}
	}
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	if (gridstate_pattern(self->gridstate)) {
		psy_dsp_big_beat_t offset;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* next = 0;
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* hover;

		offset = gridstate_quantize(self->gridstate,
			gridstate_pxtobeat(self->gridstate, ev->x - self->component.scroll.x));
		node = psy_audio_pattern_findnode(gridstate_pattern(self->gridstate),
			self->gridstate->cursor.track, offset,
			gridstate_step(self->gridstate), &prev);
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
				if (psy_audio_patternnode_entry(next)->offset -
						offset == gridstate_step(self->gridstate)) {
					psy_ui_component_setcursor(&self->component,
						psy_ui_CURSORSTYLE_COL_RESIZE);
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


void pianogrid_onpatterncursorchanged(Pianogrid* self,
	Workspace* sender)
{
	psy_audio_PatternCursor oldcursor;

	oldcursor = self->gridstate->cursor;
	self->gridstate->cursor = workspace_patterncursor(sender);
	if (self->gridstate->cursor.track != oldcursor.track) {
		psy_ui_component_invalidate(&self->component);
	} else if (oldcursor.offset != self->gridstate->cursor.offset &&
			!self->gridstate->cursorchanging) {
		pianogrid_invalidatecursor(self);		
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
			gridstate_pattern(self->gridstate)->length) {
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

	visilines = (int)((size.width /
		(psy_dsp_big_beat_t)self->gridstate->pxperbeat) * self->gridstate->lpb);
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
	if (gridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->cursor,
			gridstate_pattern(self->gridstate),
			gridstate_step(self->gridstate), wrap);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_prevlines(&cursornavigator,
				lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace,
			gridstate_cursor(self->gridstate));
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
	if (gridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			gridstate_pattern(self->gridstate),
			gridstate_step(self->gridstate), wrap);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
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
		gridstate_steppx(self->gridstate) * line, 0,
		gridstate_steppx(self->gridstate), 200);
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

// PianoBar
// implenentation
void pianobar_init(PianoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_label_init(&self->keys, pianobar_base(self));
	psy_ui_label_settext(&self->keys, "Keyboard");
	zoombox_init(&self->zoombox_keyheight, &self->component);
	psy_ui_combobox_init(&self->keytype, pianobar_base(self));
	psy_ui_combobox_setcharnumber(&self->keytype, 8);	
	psy_ui_combobox_addtext(&self->keytype, "Keys");	
	psy_ui_combobox_addtext(&self->keytype, "Notes");
	psy_ui_combobox_addtext(&self->keytype, "Drums");
	psy_ui_combobox_setcursel(&self->keytype, 0);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.25),
		psy_ui_value_makeew(1.0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(pianobar_base(self), psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
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
static void pianoroll_onkeytypeselchange(Pianoroll*, psy_ui_Component* sender, int sel);
// vtable
static psy_ui_ComponentVtable pianoroll_vtable;
static bool pianoroll_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoroll_vtable_init(Pianoroll* self)
{
	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign = (psy_ui_fp_component_onalign)
			pianoroll_onalign;
		pianoroll_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianoroll_onmousedown;		
		pianoroll_vtable.ontimer = (psy_ui_fp_component_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = TRUE;
	}
	return &pianoroll_vtable;
}
// implenentation
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	PatternViewSkin* skin, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, pianoroll_vtable_init(self));
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;	
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
	psy_ui_component_setalign(zoombox_base(&self->zoombox_beatwidth), psy_ui_ALIGN_TOP);
	// Keyboard
	pianokeyboard_init(&self->keyboard, &self->left, &self->keyboardstate);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_CLIENT);	
	// top area (beatruler)
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	pianoheader_init(&self->header, &self->top, &self->gridstate);
	psy_ui_component_setalign(pianoheader_base(&self->header), psy_ui_ALIGN_TOP);
	// client area (event grid)
	pianogrid_init(&self->grid, &self->component, &self->keyboardstate,
		&self->gridstate, self->workspace);
	psy_ui_scroller_init(&self->scroller, pianogrid_base(&self->grid),
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	// bar
	pianobar_init(&self->bar, &self->scroller.component, self->workspace);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->bar.zoombox_keyheight.signal_changed, self,
		pianoroll_onzoomboxkeyheightchanged);
	psy_ui_component_setalign(&self->bar.zoombox_keyheight.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&pianogrid_base(&self->grid)->signal_scroll, self,
		pianoroll_ongridscroll);
	psy_signal_connect(&self->bar.keytype.signal_selchanged, self,
		pianoroll_onkeytypeselchange);
	psy_signal_connect(&self->workspace->player.eventdrivers.signal_input, self,
		pianoroll_oneventdriverinput);
	psy_ui_component_starttimer(&self->component, 0, 100);
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	gridstate_setpattern(&self->gridstate, pattern);
	pianogrid_onpatternchange(&self->grid, pattern);
	psy_ui_component_setscroll(pianogrid_base(&self->grid),
		psy_ui_intpoint_make(0, psy_ui_component_scrolltop(
			pianogrid_base(&self->grid))));
	psy_ui_component_setscroll(pianoheader_base(&self->header),
		psy_ui_intpoint_make(0, 0));
	pianoroll_updatescroll(self);
}

void pianoroll_ontimer(Pianoroll* self, uintptr_t timerid)
{
	if (gridstate_pattern(&self->gridstate) &&
			psy_ui_component_visible(&self->component)) {
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
		} else if (self->grid.lastplayposition != -1) {
			pianogrid_invalidateline(&self->grid, self->grid.lastplayposition);
			self->grid.lastplayposition = -1;
		}		
		if (gridstate_pattern(&self->gridstate) &&
				gridstate_pattern(&self->gridstate)->opcount != self->opcount &&
				self->syncpattern) {
			psy_ui_component_invalidate(pianogrid_base(&self->grid));
		}
		self->opcount = gridstate_pattern(&self->gridstate)
			? gridstate_pattern(&self->gridstate)->opcount
			: 0;
	}
}

void pianoroll_onalign(Pianoroll* self)
{
	pianoroll_updatescroll(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(pianogrid_base(&self->grid));
}

void pianoroll_updatescroll(Pianoroll* self)
{			
	self->grid.component.scrollstepx = gridstate_steppx(&self->gridstate);
	self->grid.component.scrollstepy = self->keyboardstate.keyheightpx;
	pianogrid_updatekeystate(&self->grid);
	self->keyboard.component.scrollstepy = self->keyboardstate.keyheightpx;
	self->header.component.scrollstepx = self->grid.component.scrollstepx;
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianoheader_base(&self->header));
	psy_ui_component_invalidate(&self->keyboard.component);
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	gridstate_setlpb(&self->gridstate, lpb);	
	self->grid.component.scrollstepx = gridstate_steppx(&self->gridstate);
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoheader_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoheader_base(&self->header));
	psy_ui_component_update(pianoheader_base(&self->header));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleft(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrollleft(pianoheader_base(&self->header))) {
		psy_ui_component_setscrollleft(pianoheader_base(&self->header),
			psy_ui_component_scrollleft(pianogrid_base(&self->grid)));		
	}
	if (psy_ui_component_scrolltop(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrolltop(&self->keyboard.component)) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(pianogrid_base(&self->grid)));		
	}
}

void pianoroll_onzoomboxbeatwidthchanged(Pianoroll* self, ZoomBox* sender)
{	
	self->gridstate.pxperbeat = (int)(self->gridstate.defaultbeatwidth *
		zoombox_rate(sender));
	self->grid.component.scrollstepx = gridstate_steppx(&self->gridstate);
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoheader_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoheader_base(&self->header));
	psy_ui_component_update(pianoheader_base(&self->header));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_onzoomboxkeyheightchanged(Pianoroll* self, ZoomBox* sender)
{
	self->keyboardstate.keyheight = psy_ui_mul_value_real(
		self->keyboardstate.defaultkeyheight, zoombox_rate(sender));
	pianogrid_updatekeystate(&self->grid);
	self->grid.component.scrollstepy = self->keyboardstate.keyheightpx;
	self->keyboard.component.scrollstepy = self->keyboardstate.keyheightpx;
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrolltop(&self->keyboard.component,
		psy_ui_component_scrolltop(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_onkeytypeselchange(Pianoroll* self, psy_ui_Component* sender, int sel)
{
	switch (sel) {		
		case 0:			
			self->keyboardstate.drawpianokeys = TRUE;
			self->keyboardstate.notemode = psy_dsp_NOTESTAB_A440;
			psy_ui_component_setpreferredsize(&self->keyboard.component, psy_ui_size_make(
				psy_ui_value_makeew(8), psy_ui_value_makepx(0)));
			break;		
		case 1:
			self->keyboardstate.drawpianokeys = FALSE;
			self->keyboardstate.notemode = psy_dsp_NOTESTAB_A440;
			psy_ui_component_setpreferredsize(&self->keyboard.component, psy_ui_size_make(
				psy_ui_value_makeew(8), psy_ui_value_makepx(0)));
			break;
		case 2:
			self->keyboardstate.drawpianokeys = FALSE;
			self->keyboardstate.notemode = psy_dsp_NOTESTAB_GMPERCUSSION;
			psy_ui_component_setpreferredsize(&self->keyboard.component, psy_ui_size_make(
				psy_ui_value_makeew(21), psy_ui_value_makepx(0)));
			break;
		default:
			break;
	}
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_align(&self->component);
}

// Handle EventDriver Input
void pianoroll_oneventdriverinput(Pianoroll* self, psy_EventDriver* sender)
{
	if (psy_ui_component_hasfocus(pianogrid_base(&self->grid))) {
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

	cmds = psy_property_settext(psy_property_append_section(parent,
		"pianoroll"), "Pianoroll");
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
	bool handled;
	
	handled = TRUE;
	switch (cmd) {
		case CMD_NAVUP:
			if (self->gridstate.cursor.key < self->keyboardstate.keymax - 1) {
				pianogrid_storecursor(&self->grid);
				++self->gridstate.cursor.key;
				workspace_setpatterncursor(self->workspace,
					self->gridstate.cursor);
				pianogrid_invalidatecursor(&self->grid);
			}
			break;
		case CMD_NAVDOWN:
			if (self->gridstate.cursor.key > self->keyboardstate.keymin) {
				pianogrid_storecursor(&self->grid);
				--self->gridstate.cursor.key;
				workspace_setpatterncursor(self->workspace,
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
				&InsertCommandAlloc(gridstate_pattern(&self->gridstate),
					gridstate_step(&self->gridstate),
					self->gridstate.cursor, patternevent,
					self->workspace)->command);
			pianogrid_advanceline(&self->grid);
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
