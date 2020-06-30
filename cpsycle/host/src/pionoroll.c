// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static int isblack(int key)
{
	int offset = key % 12;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	return (offset == 1 || offset == 3 || offset == 6 || offset == 8
		|| offset == 10);
}

// Header
static void pianoheader_ondraw(PianoHeader*, psy_ui_Graphics*);
static void pianoheader_drawruler(PianoHeader*, psy_ui_Graphics*);
static void pianoheader_onpreferredsize(PianoHeader*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable pianoheader_vtable;
static int pianoheader_vtable_initialized = 0;

static void pianoheader_vtable_init(PianoHeader* self)
{
	if (!pianoheader_vtable_initialized) {
		pianoheader_vtable = *(self->component.vtable);
		pianoheader_vtable.ondraw = (psy_ui_fp_ondraw)
			pianoheader_ondraw;
		pianoheader_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			pianoheader_onpreferredsize;
		pianoheader_vtable_initialized = 1;
	}
}

void pianoheader_init(PianoHeader* self, psy_ui_Component* parent,
	GridState* gridstate)
{
	psy_ui_component_init(&self->component, parent);
	pianoheader_vtable_init(self);
	pianoheader_setsharedgridstate(self, gridstate);
	self->component.vtable = &pianoheader_vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1)));
}

void pianoheader_setsharedgridstate(PianoHeader* self, GridState* gridstate)
{
	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		gridstate_init(&self->defaultgridstate);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianoheader_onpreferredsize(PianoHeader* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->width = psy_ui_value_makepx(self->gridstate->visiwidth);
}

void pianoheader_ondraw(PianoHeader* self, psy_ui_Graphics* g)
{	
	pianoheader_drawruler(self, g);	
}

void pianoheader_drawruler(PianoHeader* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int baseline;	
	int c;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = psy_ui_value_px(&size.height, &tm) - 1;	
	psy_ui_setcolor(g, self->gridstate->rulerbaselinecolour);
	psy_ui_drawline(g, psy_ui_component_scrollleft(&self->component), baseline,
		psy_ui_value_px(&size.width, &tm) + psy_ui_component_scrollleft(&self->component),
		baseline);
	if (self->gridstate->visiwidth > 0) {
		int beatstart;
		
		for (c = 0; c <= self->gridstate->visisteps; ++c) {
			int cpx;

			cpx = (int)((c * self->gridstate->stepwidth) +
				psy_ui_component_scrollleft(&self->component));
			psy_ui_drawline(g, cpx, baseline, cpx, baseline - tm.tmHeight / 3);
		}
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, self->gridstate->rulermarkcolour);
		beatstart = psy_ui_component_scrollleft(&self->component) /
			self->gridstate->beatwidth;
		for (c = 0; c <= self->gridstate->visibeats; ++c) {
			char txt[40];
			int cpx;

			cpx = ((c * self->gridstate->beatwidth) +
				psy_ui_component_scrollleft(&self->component));
			psy_ui_drawline(g, (int)cpx, baseline, (int)cpx, baseline - tm.tmHeight / 2);
			psy_snprintf(txt, 40, "%d", (int)(c + beatstart));
			psy_ui_textout(g, (int)cpx + 3, baseline - tm.tmHeight, txt, strlen(txt));
		}
	}
}

// PianoKeyboard

// KeyboardState
void keyboardstate_init(KeyboardState* self)
{	
	self->keymin = 0;
	self->keymax = 88;
	self->keyheight = 12;	
	self->defaultkeyheight = 12;
	self->zoom = 1.0;
	self->keyblackcolour = 0x00444444;
	self->keywhitecolour = 0x00CACACA;
	self->keyseparatorcolour = 0x00333333;
}
// prototypes
static void pianokeyboard_ondraw(PianoKeyboard*, psy_ui_Graphics*);
// vtable
static psy_ui_ComponentVtable pianokeyboard_vtable;
static int pianokeyboard_vtable_initialized = 0;

static void pianokeyboard_vtable_init(PianoKeyboard* self)
{
	if (!pianokeyboard_vtable_initialized) {
		pianokeyboard_vtable = *(self->component.vtable);
		pianokeyboard_vtable.ondraw = (psy_ui_fp_ondraw)
			pianokeyboard_ondraw;
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
		keyboardstate_init(&self->defaultkeyboardstate);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianokeyboard_ondraw(PianoKeyboard* self, psy_ui_Graphics* g)
{
	int keymin = 0;
	int keymax = 88;
	int key;
	int keyboardheight;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	keyboardheight = (keymax - keymin) * self->keyboardstate->keyheight;
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setcolor(g, self->keyboardstate->keyseparatorcolour);
	for (key = keymin; key < keymax; ++key) {
		int cpy;

		cpy = keyboardheight - (key + 1) * self->keyboardstate->keyheight;
		psy_ui_drawline(g, 0, cpy, psy_ui_value_px(&size.width, &tm), cpy);
		if (isblack(key)) {			
			psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					(int)(psy_ui_value_px(&size.width, &tm) * 0.75), cpy,
					(int)(psy_ui_value_px(&size.width, &tm) * 0.25),
					self->keyboardstate->keyheight),
				self->keyboardstate->keywhitecolour);
			psy_ui_drawline(g,
				0, cpy + self->keyboardstate->keyheight / 2,
				psy_ui_value_px(&size.width, &tm),
				cpy + self->keyboardstate->keyheight / 2);			
			psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					0, cpy, (int)(psy_ui_value_px(&size.width, &tm) * 0.75),
					self->keyboardstate->keyheight),
				self->keyboardstate->keyblackcolour);
		} else {			
			psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					0, cpy, psy_ui_value_px(&size.width, &tm),
					self->keyboardstate->keyheight),
				self->keyboardstate->keywhitecolour);
			if (key % 12 == 0 || ((key % 12) == 5)) {
				psy_ui_drawline(g,
					0, cpy + self->keyboardstate->keyheight, psy_ui_value_px(&size.width, &tm),
					cpy + self->keyboardstate->keyheight);
			}
		}
	}
}

// PianoGrid
// default state
void gridstate_init(GridState* self)
{
	self->beatwidth = 80;
	self->defaultbeatwidth = 80;
	self->zoom = 1.0;
	self->lpb = 4;
	self->eventcolour = 0x00999999;
	self->eventcurrchannelcolour = 0x00CACACA;	
	self->eventhovercolour = 0x00DADADA;
	self->grid12separatorcolour = 0x00292929;
	self->gridseparatorcolour = 0x00363636;
	self->playbarcolour = 0x009F7B00;
	self->row4beatcolour = 0x00595959;
	self->row4beat2colour = 0x00595959;
	self->rowbeatcolour = 0x00363636;
	self->rowbeat2colour = 0x00363636;
	self->rowcolour = 0x003E3E3E;
	self->row2colour = 0x003E3E3E;
	self->rulerbaselinecolour = 0x00555555;
	self->rulermarkcolour = 0x00666666;
	self->visibeats = 16.0;
	self->visisteps = 64;
	self->visiwidth = 1280;
	self->stepwidth = 20.0;
	psy_audio_patterneditposition_init(&self->cursor);
}
// prototypes
static void pianogrid_ondestroy(Pianogrid*);
static void pianogrid_ondraw(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawgrid(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics*);
static void pianogrid_drawentries(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawentry(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternEntry*, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length,
	int hover);
static void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_onscroll(Pianogrid*, psy_ui_Component* sender);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static psy_dsp_big_beat_t pianogrid_pxtobeat(Pianogrid*, int x);
static psy_dsp_big_beat_t pianogrid_quantizise(Pianogrid*,
	psy_dsp_big_beat_t offset);
static psy_audio_PatternNode* pianogrid_nextnode(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track);
static void pianogrid_onpatterneditpositionchanged(Pianogrid*,
	Workspace* sender);

static psy_ui_ComponentVtable pianogrid_vtable;
static int pianogrid_vtable_initialized = 0;
// vtable
static void pianogrid_vtable_init(Pianogrid* self)
{
	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw = (psy_ui_fp_ondraw)pianogrid_ondraw;	
		pianogrid_vtable.onmousedown = (psy_ui_fp_onmousedown)
			pianogrid_onmousedown;
		pianogrid_vtable.onmousemove = (psy_ui_fp_onmousemove)
			pianogrid_onmousemove;
		pianogrid_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
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
	self->pattern = NULL;
	self->hover = 0;
	self->lastplayposition = 0.f;
	self->sequenceentryoffset = 0.f;
	psy_table_init(&self->channel);
	for (track = 0; track < 64; ++track) {
		psy_audio_PatternEntry* channelentry;

		channelentry = patternentry_allocinit();
		channelentry->offset = -1.0f;
		channelentry->track = track;
		psy_table_insert(&self->channel, track, channelentry);
	}
	psy_signal_connect(&self->component.signal_scroll, self,
		pianogrid_onscroll);
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
		gridstate_init(&self->defaultgridstate);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianogrid_setsharedkeyboardstate(Pianogrid* self, KeyboardState*
	keyboardstate)
{
	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianogrid_setpattern(Pianogrid* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	pianogrid_updategridstate(self);
	psy_ui_component_updateoverflow(&self->component);
}

void pianogrid_ondestroy(Pianogrid* self)
{
	psy_table_disposeall(&self->channel, (psy_fp_disposefunc)
		patternentry_dispose);
}

void pianogrid_ondraw(Pianogrid* self, psy_ui_Graphics* g)
{
	if (self->pattern) {
		pianogrid_drawgrid(self, g);
		pianogrid_drawplaybar(self, g);
		pianogrid_drawentries(self, g);
	}
}

void pianogrid_drawgrid(Pianogrid* self, psy_ui_Graphics* g)
{
	int keyboardheight;
	int key;
	int keymin;
	int keymax;
	int cpx;
	int cpy;
	int beatstart;
	int c;

	psy_ui_setcolor(g, self->gridstate->gridseparatorcolour);
	keyboardheight = keyboardstate_height(self->keyboardstate);	
	cpy = keyboardheight;
	psy_ui_drawline(g,
		psy_ui_component_scrollleft(&self->component),
		cpy,
		psy_ui_component_scrollleft(&self->component) +
		self->gridstate->visiwidth,
		cpy);
	beatstart = psy_ui_component_scrollleft(&self->component) / self->gridstate->beatwidth;
	for (c = 0; c < self->gridstate->visisteps; ++c) {
		psy_ui_Color linecolor;

		cpx = (int)(c * self->gridstate->stepwidth);
		cpx += psy_ui_component_scrollleft(&self->component);
		if (((beatstart * self->gridstate->lpb + c)  % (self->gridstate->lpb * 4)) == 0) {
			linecolor = self->gridstate->row4beatcolour;
		} else
		if ((c % self->gridstate->lpb) == 0) {
			linecolor = self->gridstate->rowbeatcolour;
		} else {
			linecolor = self->gridstate->rowcolour;
		}
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
			cpx, psy_ui_component_scrolltop(&self->component),
			(int)(self->gridstate->stepwidth) + 1, keyboardheight),
			linecolor);
		psy_ui_drawline(g,
			cpx, psy_ui_component_scrolltop(&self->component),
			cpx, keyboardheight);
	}
	
	keymin = max(self->keyboardstate->keymin,
		(keyboardheight - g->clip.bottom) / self->keyboardstate->keyheight);
	keymax = min(self->keyboardstate->keymax,
		(keyboardheight - g->clip.top) / self->keyboardstate->keyheight);
	for (key = keymin; key <= keymax; ++key) {
		cpy = keyboardheight - (key + 1) * self->keyboardstate->keyheight;
		if (((key + 1) % 12) == 0) {
			psy_ui_setcolor(g, self->gridstate->grid12separatorcolour);
		} else {
			psy_ui_setcolor(g, self->gridstate->gridseparatorcolour);
		}
		psy_ui_drawline(g,
			psy_ui_component_scrollleft(&self->component),
			cpy,
			psy_ui_component_scrollleft(&self->component) +
			self->gridstate->visiwidth,
			cpy);
	}
}

void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics* g)
{
	if (psy_audio_player_playing(&self->workspace->player)) {
		psy_dsp_big_beat_t offset;

		offset = self->lastplayposition;
		if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset +
			self->pattern->length) {
			int line;			

			line = (int)((offset - self->sequenceentryoffset) *
				self->gridstate->lpb);
			psy_ui_drawsolidrectangle(g,
				psy_ui_rectangle_make(
					(int)(self->gridstate->stepwidth * line),
					0,
					(int)self->gridstate->stepwidth,
					keyboardstate_height(self->keyboardstate)),
				self->gridstate->playbarcolour);
		}
	}
}

void pianogrid_drawentries(Pianogrid* self, psy_ui_Graphics* g)
{	
	
	if (self->pattern) {
		psy_audio_PatternNode* curr;
		int track;

		curr = psy_audio_pattern_begin(self->pattern);
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
			if (patternentry_front(entry)->note == 120) {
				channelentry->offset = -1.0f;
			} else {
				channelentry->offset = entry->offset;
				*patternentry_front(channelentry)
					= *patternentry_front(entry);
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
					self->pattern->length - channelentry->offset,
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
	psy_ui_Color colour;
	psy_audio_PatternEditPosition editposition;

	editposition = workspace_patterneditposition(self->workspace);
	event = patternentry_front(entry);
	int left = (int)((offset) * self->gridstate->beatwidth);
	int width = (int)(length * self->gridstate->beatwidth);
	psy_ui_setrectangle(&r, left,
		(self->keyboardstate->keymax - event->note - 1) * self->keyboardstate->keyheight
		+ 1, width, self->keyboardstate->keyheight - 2);	
	if (hover) {
		colour = self->gridstate->eventhovercolour;
	} else
	if (entry->track == editposition.track) {
		colour = self->gridstate->eventcurrchannelcolour;
	} else {
		colour = self->gridstate->eventcolour;
	}
	psy_ui_drawsolidrectangle(g, r, colour);	
}

void pianogrid_onscroll(Pianogrid* self, psy_ui_Component* sender)
{	
	if (psy_ui_component_scrollleft(&self->component) != 0) {
		pianogrid_updategridstate(self);
	}		
}

void pianogrid_updategridstate(Pianogrid* self)
{
	psy_ui_Size gridsize;
	psy_ui_TextMetric tm;

	gridsize = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);

	self->gridstate->beatwidth = (int)(self->gridstate->defaultbeatwidth * self->gridstate->zoom);
	self->gridstate->visibeats = (self->pattern)
		? min(max(0, self->pattern->length - psy_ui_component_scrollleft(&self->component) / self->gridstate->beatwidth),
			(psy_ui_value_px(&gridsize.width, &tm) / (psy_dsp_big_beat_t)self->gridstate->beatwidth) + 0.5)
		: 0;
	self->gridstate->visisteps = (int)(self->gridstate->visibeats * self->gridstate->lpb + 0.5);
	self->gridstate->visiwidth = (self->pattern)
		? (int)(self->gridstate->visibeats * self->gridstate->beatwidth + 0.5)
		: 0;
	self->gridstate->stepwidth = self->gridstate->beatwidth / (psy_dsp_big_beat_t)self->gridstate->lpb;
}

void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	rv->height = psy_ui_value_makepx((self->keyboardstate->keymax - self->keyboardstate->keymin) *
		self->keyboardstate->keyheight);
	if (self->pattern) {
		rv->width = psy_ui_value_makepx((int)(
			self->gridstate->beatwidth * psy_audio_pattern_length(self->pattern)));
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	if (self->pattern) {
		psy_dsp_big_beat_t offset;

		psy_ui_component_setfocus(&self->component);
		offset = pianogrid_quantizise(self, pianogrid_pxtobeat(self, ev->x));
		if (ev->button == 1) {
			psy_audio_PatternEvent event;
			psy_audio_PatternNode* node = 0;
			psy_audio_PatternNode* prev = 0;

			patternevent_clear(&event);
			event.note = self->keyboardstate->keymax - 1 - ev->y / self->keyboardstate->keyheight;
			node = psy_audio_pattern_findnode(self->pattern, self->gridstate->cursor.track, offset,
				1 / (psy_dsp_big_beat_t)self->gridstate->lpb, &prev);
			if (node) {
				psy_audio_pattern_setevent(self->pattern, node, &event);
			} else {
				psy_audio_pattern_insert(self->pattern, prev, self->gridstate->cursor.track, offset, &event);
			}
		} else if (ev->button == 2) {
			psy_audio_PatternNode* prev;

			psy_audio_PatternNode* node = psy_audio_pattern_findnode(self->pattern, self->gridstate->cursor.track,
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
				psy_audio_pattern_remove(self->pattern, node);
				if (next) {
					if (patternentry_front(psy_audio_patternnode_entry(next))->note
						== NOTECOMMANDS_RELEASE) {
						if (self->hover == psy_audio_patternnode_entry(next)) {
							self->hover = 0;
						}
						psy_audio_pattern_remove(self->pattern, next);
					}
				}
			}
		}
	}
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	if (self->pattern) {
		psy_dsp_big_beat_t offset;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* next = 0;
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* hover;

		offset = pianogrid_quantizise(self, pianogrid_pxtobeat(self, ev->x - self->component.scroll.x));
		node = psy_audio_pattern_findnode(self->pattern, self->gridstate->cursor.track, offset,
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
	return (px / (psy_dsp_big_beat_t)self->gridstate->beatwidth);
}

psy_dsp_big_beat_t pianogrid_quantizise(Pianogrid* self, psy_dsp_big_beat_t offset)
{
	return (int)(offset * (psy_dsp_big_beat_t)self->gridstate->lpb) /
		(psy_dsp_big_beat_t)self->gridstate->lpb;
}

void pianogrid_onpatterneditpositionchanged(Pianogrid* self,
	Workspace* sender)
{
	psy_audio_PatternEditPosition oldcursor;

	oldcursor = self->gridstate->cursor;
	self->gridstate->cursor = workspace_patterneditposition(sender);
	if (self->gridstate->cursor.track != oldcursor.track) {
		psy_ui_component_invalidate(&self->component);
	}
}

// Pianoroll
// protoypes
static void pianoroll_ontimer(Pianoroll*, uintptr_t timerid);
static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player* sender,
	uintptr_t lpb);
static int pianoroll_testplaybar(Pianoroll*, psy_dsp_big_beat_t offset);
static void pianoroll_invalidateline(Pianoroll*, psy_dsp_big_beat_t offset);
static void pianoroll_ondestroy(Pianoroll*, psy_ui_Component* component);
static void pianoroll_onalign(Pianoroll*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_ongridscroll(Pianoroll*, psy_ui_Component* sender);
static void pianoroll_onzoomboxbeatwidthchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onzoomboxkeyheightchanged(Pianoroll*, ZoomBox* sender);
// vtable
static psy_ui_ComponentVtable pianoroll_vtable;
static int pianoroll_vtable_initialized = 0;

static void pianoroll_vtable_init(Pianoroll* self)
{
	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign = (psy_ui_fp_onalign)pianoroll_onalign;
		pianoroll_vtable.onmousedown = (psy_ui_fp_onmousedown)
			pianoroll_onmousedown;		
		pianoroll_vtable.ontimer = (psy_ui_fp_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = 1;
	}
}
// implenentation
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	pianoroll_vtable_init(self);
	self->component.vtable = &pianoroll_vtable;	
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;
	self->pattern = 0;	
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		pianoroll_ondestroy);
	// shared states
	keyboardstate_init(&self->keyboardstate);
	gridstate_init(&self->gridstate);
	// left area (keyboardheader, keyboard)
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_enablealign(&self->left);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_beatwidth, &self->left);
	psy_signal_connect(&self->zoombox_beatwidth.signal_changed, self,
		pianoroll_onzoomboxbeatwidthchanged);
	psy_ui_component_setalign(&self->zoombox_beatwidth.component, psy_ui_ALIGN_TOP);
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
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&self->grid.component.signal_scroll, self,
		pianoroll_ongridscroll);
	psy_ui_component_starttimer(&self->component, 0, 100);
}

void pianoroll_ondestroy(Pianoroll* self, psy_ui_Component* component)
{
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	pianogrid_setpattern(&self->grid, pattern);
	psy_ui_component_setscroll(&self->grid.component, psy_ui_intpoint_make(0, 0));
	psy_ui_component_setscroll(&self->header.component, psy_ui_intpoint_make(0, 0));
	pianoroll_updatescroll(self);	
}

void pianoroll_ontimer(Pianoroll* self, uintptr_t timerid)
{
	if (self->pattern) {
		if (psy_audio_player_playing(&self->workspace->player)) {
			pianoroll_invalidateline(self, self->grid.lastplayposition);
			self->grid.lastplayposition =
				psy_audio_player_position(&self->workspace->player);
			pianoroll_invalidateline(self, self->grid.lastplayposition);
		} else {
			if (self->grid.lastplayposition != -1) {
				pianoroll_invalidateline(self, self->grid.lastplayposition);
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

int pianoroll_testplaybar(Pianoroll* self, psy_dsp_big_beat_t offset)
{
	return psy_audio_player_playing(&self->workspace->player) &&
		psy_dsp_testrange(self->grid.lastplayposition -
			self->grid.sequenceentryoffset,
			offset, 1 / (psy_dsp_big_beat_t)self->grid.gridstate->lpb);
}

void pianoroll_invalidateline(Pianoroll* self, psy_dsp_big_beat_t offset)
{
	if (offset >= self->grid.sequenceentryoffset &&
		offset < self->grid.sequenceentryoffset + self->pattern->length) {
		int line;

		line = (int)((offset - self->grid.sequenceentryoffset) *
			self->gridstate.lpb);
		psy_ui_component_invalidaterect(&self->grid.component,
			psy_ui_rectangle_make(
				(int)(self->gridstate.stepwidth * line),
				0,
				(int)self->gridstate.stepwidth,
				keyboardstate_height(&self->keyboardstate)));
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
	self->gridstate.defaultbeatwidth = tm.tmAveCharWidth * 14;
	self->keyboardstate.defaultkeyheight = tm.tmHeight + 1;
	self->keyboardstate.keyheight =
		(int)(self->keyboardstate.defaultkeyheight *self->keyboardstate.zoom);
	pianogrid_updategridstate(&self->grid);	
	self->grid.component.scrollstepx = self->gridstate.beatwidth;
	self->grid.component.scrollstepy = self->keyboardstate.keyheight;
	self->keyboard.component.scrollstepy = self->keyboardstate.keyheight;
	self->header.component.scrollstepx = self->gridstate.beatwidth;
	psy_ui_component_updateoverflow(&self->grid.component);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_invalidate(&self->keyboard.component);
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	self->gridstate.lpb = lpb;
	pianoroll_updatescroll(self);	
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender)
{
	if (psy_ui_component_scrollleft(&self->grid.component) !=
			psy_ui_component_scrollleft(&self->header.component)) {
		psy_ui_component_setscrollleft(&self->header.component,
			psy_ui_component_scrollleft(&self->grid.component));		
	}
	if (psy_ui_component_scrolltop(&self->grid.component) !=
			psy_ui_component_scrolltop(&self->header.component)) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(&self->grid.component));		
	}
}

void pianoroll_onzoomboxbeatwidthchanged(Pianoroll* self, ZoomBox* sender)
{
	self->gridstate.zoom = sender->zoomrate;
	pianoroll_updatescroll(self);
	psy_ui_component_setscrollleft(&self->header.component,
		psy_ui_component_scrollleft(&self->grid.component));
	psy_ui_component_invalidate(&self->header.component);
	psy_ui_component_update(&self->header.component);
}

void pianoroll_onzoomboxkeyheightchanged(Pianoroll* self, ZoomBox* sender)
{
	self->keyboardstate.zoom = sender->zoomrate;
	self->keyboardstate.keyheight = (int)(self->keyboardstate.defaultkeyheight *
		self->keyboardstate.zoom);	
	pianoroll_updatescroll(self);
	psy_ui_component_setscrolltop(&self->keyboard.component,
		psy_ui_component_scrolltop(&self->grid.component));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);
}

