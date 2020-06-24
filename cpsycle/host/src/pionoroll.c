// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static KeyboardConfig defaultkeyboardconfig;
static bool defaultkeyboardconfig_initialized = FALSE;

void defaultkeyboardconfig_init()
{
	if (!defaultkeyboardconfig_initialized) {
		defaultkeyboardconfig.keymin = 0;
		defaultkeyboardconfig.keymax = 88;
		defaultkeyboardconfig.keyheight = 12;
		defaultkeyboardconfig_initialized = TRUE;
	}
}

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

// vtable
static psy_ui_ComponentVtable pianoheader_vtable;
static int pianoheader_vtable_initialized = 0;

static void pianoheader_vtable_init(PianoHeader* self)
{
	if (!pianoheader_vtable_initialized) {
		pianoheader_vtable = *(self->component.vtable);
		pianoheader_vtable.ondraw = (psy_ui_fp_ondraw)
			pianoheader_ondraw;
		pianoheader_vtable_initialized = 1;
	}
}

void pianoheader_init(PianoHeader* self, psy_ui_Component* parent,
	Pianoroll* roll)
{
	psy_ui_component_init(&self->component, parent);
	pianoheader_vtable_init(self);
	self->component.vtable = &pianoheader_vtable;
	self->view = roll;
	self->scrollpos = 0;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1)));
}

void pianoheader_ondraw(PianoHeader* self, psy_ui_Graphics* g)
{
	if (self->view->pattern) {
		pianoheader_drawruler(self, g);
	}
}

void pianoheader_drawruler(PianoHeader* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	psy_ui_TextMetric tm;
	int baseline;
	psy_dsp_big_beat_t cpx;
	int c;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = psy_ui_value_px(&size.height, &tm) - 1;
	psy_ui_setcolor(g, 0x00555555);
	psy_ui_drawline(g, 0, baseline, psy_ui_value_px(&size.width, &tm), baseline);

	for (c = 0, cpx = 0; c <= self->metrics.visisteps;
		cpx += self->metrics.stepwidth, ++c) {
		psy_ui_drawline(g, (int)cpx, baseline, (int)cpx, baseline - tm.tmHeight / 3);
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x00666666);
	for (c = 0, cpx = 0; c <= self->metrics.visibeats;
		cpx += self->metrics.beatwidth, ++c) {
		char txt[40];
		psy_ui_drawline(g, (int)cpx, baseline, (int)cpx, baseline - tm.tmHeight / 2);
		psy_snprintf(txt, 40, "%d", (int)(c - self->scrollpos));
		psy_ui_textout(g, (int)cpx + 3, baseline - tm.tmHeight, txt, strlen(txt));
	}
}

// PianoKeyboard

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

void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent,
	KeyboardConfig* keyboardconfig)
{
	psy_ui_component_init(&self->component, parent);
	pianokeyboard_vtable_init(self);
	self->component.vtable = &pianokeyboard_vtable;
	pianokeyboard_setkeyboardconfig(self, keyboardconfig);
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_setpreferredsize(&self->component, psy_ui_size_make(
		psy_ui_value_makeew(8), psy_ui_value_makepx(0)));
}

void pianokeyboard_setkeyboardconfig(PianoKeyboard* self,
	KeyboardConfig* keyboardconfig)
{
	if (self->keyboardconfig) {
		self->keyboardconfig = keyboardconfig;
	} else {
		defaultkeyboardconfig_init();
		self->keyboardconfig = &defaultkeyboardconfig;
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

	keyboardheight = (keymax - keymin) * self->keyboardconfig->keyheight;
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_setcolor(g, 0x00333333);
	for (key = keymin; key < keymax; ++key) {
		int cpy;

		cpy = keyboardheight - (key + 1) * self->keyboardconfig->keyheight;
		psy_ui_drawline(g, 0, cpy, psy_ui_value_px(&size.width, &tm), cpy);
		if (isblack(key)) {
			psy_ui_Rectangle r;
			psy_ui_setrectangle(&r, (int)(psy_ui_value_px(&size.width, &tm) * 0.75), cpy,
				(int)(psy_ui_value_px(&size.width, &tm) * 0.25), self->keyboardconfig->keyheight);
			psy_ui_drawsolidrectangle(g, r, 0x00CACACA);
			psy_ui_drawline(g, 0, cpy + self->keyboardconfig->keyheight / 2, psy_ui_value_px(&size.width, &tm),
				cpy + self->keyboardconfig->keyheight / 2);
			psy_ui_setrectangle(&r, 0, cpy, (int)(psy_ui_value_px(&size.width, &tm) * 0.75),
				self->keyboardconfig->keyheight);
			psy_ui_drawsolidrectangle(g, r, 0x00444444);
		} else {
			psy_ui_Rectangle r;

			psy_ui_setrectangle(&r, 0, cpy, psy_ui_value_px(&size.width, &tm), self->keyboardconfig->keyheight);
			psy_ui_drawsolidrectangle(g, r, 0x00CACACA);
			if (key % 12 == 0 || ((key % 12) == 5)) {
				psy_ui_drawline(g, 0, cpy + self->keyboardconfig->keyheight, psy_ui_value_px(&size.width, &tm),
					cpy + self->keyboardconfig->keyheight);
			}
		}
	}
}

// grid
static void pianogrid_ondraw(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawgrid(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics*);
static void pianogrid_drawevents(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawevent(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternEvent*, int track, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length,
	int hover);
static void pianogrid_onsize(Pianogrid*, const psy_ui_Size*);
static void pianogrid_adjustscroll(Pianogrid*);
static void pianogrid_onscroll(Pianogrid*, psy_ui_Component* sender, int stepx,
	int stepy);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_MouseEvent*);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static psy_dsp_big_beat_t pianogrid_pxtobeat(Pianogrid*, int x);
static psy_dsp_big_beat_t pianogrid_quantizise(Pianogrid*,
	psy_dsp_big_beat_t offset);
static psy_audio_PatternNode* pianogrid_nextnode(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track);
static void pianogrid_ondestroy(Pianogrid*);

static psy_ui_ComponentVtable pianogrid_vtable;
static int pianogrid_vtable_initialized = 0;

static void pianogrid_vtable_init(Pianogrid* self)
{
	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw = (psy_ui_fp_ondraw)pianogrid_ondraw;
		pianogrid_vtable.onsize = (psy_ui_fp_onsize)pianogrid_onsize;
		pianogrid_vtable.onmousedown = (psy_ui_fp_onmousedown)
			pianogrid_onmousedown;
		pianogrid_vtable.onmousemove = (psy_ui_fp_onmousemove)
			pianogrid_onmousemove;		
		pianogrid_vtable_initialized = 1;
	}
}

void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardConfig* keyboardconfig, Workspace* workspace)
{
	int track;

	psy_ui_component_init(&self->component, parent);
	pianogrid_vtable_init(self);
	self->component.vtable = &pianogrid_vtable;
	self->keyboardconfig = keyboardconfig;
	psy_ui_component_doublebuffer(&self->component);
	pianogrid_setkeyboardconfig(self, keyboardconfig);
	self->workspace = workspace;
	self->component.wheelscroll = 4;	
	self->pattern = NULL;
	self->beatscrollpos = 0;	
	self->hover = 0;
	self->lastplayposition = 0.f;
	self->sequenceentryoffset = 0.f;
	psy_table_init(&self->channel);	
	for (track = 0; track < 64; ++track) {
		psy_audio_PatternEntry* channelentry;

		channelentry = patternentry_allocinit();
		channelentry->offset = -1.0f;
		psy_table_insert(&self->channel, track, channelentry);
	}
	psy_signal_connect(&self->component.signal_scroll, self,
		pianogrid_onscroll);
	psy_signal_connect(&self->component.signal_destroy, self,
		pianogrid_ondestroy);
	psy_ui_component_showhorizontalscrollbar(&self->component);
	psy_ui_component_showverticalscrollbar(&self->component);	
	psy_ui_component_sethorizontalscrollrange(&self->component, 0, 16);
	psy_ui_component_setverticalscrollrange(&self->component, 0, 88);
}

void pianogrid_setkeyboardconfig(Pianogrid* self, KeyboardConfig*
	keyboardconfig)
{
	if (self->keyboardconfig) {
		self->keyboardconfig = keyboardconfig;
	} else {
		defaultkeyboardconfig_init();
		self->keyboardconfig = &defaultkeyboardconfig;
	}
}

void pianogrid_setpattern(Pianogrid* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
}

void pianogrid_ondestroy(Pianogrid* self)
{
	psy_table_disposeall(&self->channel, (psy_fp_disposefunc)
		patternentry_dispose);	
}

void pianogrid_onsize(Pianogrid* self, const psy_ui_Size* size)
{		
	pianogrid_adjustscroll(self);	
}

void pianogrid_ondraw(Pianogrid* self, psy_ui_Graphics* g)
{	   		
	if (self->pattern) {
		pianogrid_drawgrid(self, g);
		pianogrid_drawplaybar(self, g);
		pianogrid_drawevents(self, g);
	}
}

void pianogrid_drawgrid(Pianogrid* self, psy_ui_Graphics* g)
{
	if (self->pattern) {
		int key;
		int numkeys;
		int keymin;
		int keymax;
		int cpy;
		int keyboardheight;
						
		psy_ui_setcolor(g, 0x00333333);
		numkeys = self->keyboardconfig->keymax - self->keyboardconfig->keymin;
		keyboardheight = numkeys * self->keyboardconfig->keyheight;
		keymin = (keyboardheight - g->clip.bottom) / self->keyboardconfig->keyheight;
		keymax = (keyboardheight - g->clip.top) / self->keyboardconfig->keyheight;
		for (key = keymin; key <= keymax; ++key) {
			cpy = keyboardheight - (key + 1) * self->keyboardconfig->keyheight;
			psy_ui_drawline(g,
				psy_ui_component_scrollleft(&self->component), cpy,
				self->metrics.visiwidth, cpy);
		}
		{												
			int c;
			psy_dsp_big_beat_t cpx;
			int cpy;
			
			cpy = self->metrics.visikeys * self->keyboardconfig->keyheight;
			for (c = 0, cpx = 0; c <= self->metrics.visisteps;
					cpx += self->metrics.stepwidth, ++c) {
				psy_ui_drawline(g, 
					(int) cpx, psy_ui_component_scrolltop(&self->component),
					(int) cpx, keyboardheight);
			}		
		}		
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
			psy_ui_Rectangle r;

			line = (int)((offset - self->sequenceentryoffset) *
				self->metrics.lpb);
			psy_ui_setrectangle(&r,
				(int)(self->metrics.stepwidth * line +
					self->beatscrollpos * self->metrics.beatwidth),
				0,
				(int)self->metrics.stepwidth,
				self->metrics.visikeys * self->keyboardconfig->keyheight);
			psy_ui_drawsolidrectangle(g, r, 0x009F7B00);
		}
	}
}

void pianogrid_drawevents(Pianogrid* self, psy_ui_Graphics* g)
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
				pianogrid_drawevent(self, g,
					patternentry_front(channelentry),
					entry->track,
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
				pianogrid_drawevent(self, g, patternentry_front(channelentry),
					track,
					channelentry->offset,
					self->pattern->length - channelentry->offset,
					channelentry->delta > 0);
				channelentry->offset = -1.f;
			}
		}
	}
}

void pianogrid_drawevent(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternEvent* event, int track, psy_dsp_big_beat_t offset, psy_dsp_big_beat_t length,
	int hover)
{
	psy_ui_Rectangle r;

	int left = (int)((offset + self->beatscrollpos) * self->metrics.beatwidth);
	int width = (int)(length * self->metrics.beatwidth);
	psy_ui_setrectangle(&r, left,
		(self->keyboardconfig->keymax - event->note - 1) * self->keyboardconfig->keyheight
		+ 1, width, self->keyboardconfig->keyheight - 2);
	if (hover) {
		psy_ui_drawsolidrectangle(g, r, 0x00D1E8D0);
	} else {
		psy_ui_drawsolidrectangle(g, r, 0x00B1C8B0);
	}
}

void pianogrid_onscroll(Pianogrid* self, psy_ui_Component* sender, int stepx,
	int stepy)
{	
	if (stepx != 0) {		
		self->beatscrollpos += stepx;
		pianometrics_update(&self->metrics, self, self->pattern, self->lpb);		
	}		
}

void pianogrid_adjustscroll(Pianogrid* self)
{
	PianoMetrics metrics;

	pianometrics_update(&metrics, self, self->pattern, self->lpb);
	self->component.scrollstepx = metrics.beatwidth;
	self->component.scrollstepy = self->keyboardconfig->keyheight;
	psy_ui_component_sethorizontalscrollrange(&self->component, 0,
		(int)((self->pattern ?
			self->pattern->length : 0) - metrics.visibeats + 0.5));
	psy_ui_component_setverticalscrollrange(&self->component, 0,
		self->keyboardconfig->keymax - self->keyboardconfig->keymin - metrics.visikeys);
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
			event.note = self->keyboardconfig->keymax - 1 - ev->y / self->keyboardconfig->keyheight;
			node = psy_audio_pattern_findnode(self->pattern, 0, offset,
				1 / (psy_dsp_big_beat_t)self->metrics.lpb, &prev);
			if (node) {
				psy_audio_pattern_setevent(self->pattern, node, &event);
			} else {
				psy_audio_pattern_insert(self->pattern, prev, 0, offset, &event);
			}
		} else if (ev->button == 2) {
			psy_audio_PatternNode* prev;

			psy_audio_PatternNode* node = psy_audio_pattern_findnode(self->pattern, 0,
				offset, 1 / (psy_dsp_big_beat_t)self->metrics.lpb, &prev);
			if (!node) {
				node = prev;
			}
			if (node) {
				psy_audio_PatternNode* next;

				next = pianogrid_nextnode(self, node, 0);
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
	psy_dsp_big_beat_t offset;
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* next = 0;
	psy_audio_PatternNode* prev;
	psy_audio_PatternEntry* hover;
		
	offset = pianogrid_quantizise(self, pianogrid_pxtobeat(self, ev->x - self->component.scroll.x));
	node = psy_audio_pattern_findnode(self->pattern, 0, offset,
			1 / (psy_dsp_big_beat_t) self->metrics.lpb, &prev);
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
		next = pianogrid_nextnode(self, node, 0);
		if (next) {
			if (psy_audio_patternnode_entry(next)->offset - offset == 0.25) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
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
	return (px  / (psy_dsp_big_beat_t) self->metrics.beatwidth) +
		self->beatscrollpos;	
}

psy_dsp_big_beat_t pianogrid_quantizise(Pianogrid* self, psy_dsp_big_beat_t offset)
{
	return (int)(offset * (psy_dsp_big_beat_t)self->metrics.lpb) /
		(psy_dsp_big_beat_t)self->metrics.lpb;
}

// pianoroll
static void pianoroll_ontimer(Pianoroll*, uintptr_t timerid);
static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player* sender,
	uintptr_t lpb);
static int pianoroll_testplaybar(Pianoroll*, psy_dsp_big_beat_t offset);
static int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width);
static void pianoroll_invalidateline(Pianoroll*, psy_dsp_big_beat_t offset);
static void pianoroll_ondestroy(Pianoroll*, psy_ui_Component* component);
static void pianoroll_onsize(Pianoroll*, psy_ui_Size*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_onmouseup(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_onmousemove(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_onmousedoubleclick(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_ongridscroll(Pianoroll*, psy_ui_Component* sender, int stepx, int stepy);

// vtable
static psy_ui_ComponentVtable pianoroll_vtable;
static int pianoroll_vtable_initialized = 0;

static void pianoroll_vtable_init(Pianoroll* self)
{
	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onsize = (psy_ui_fp_onsize)pianoroll_onsize;
		pianoroll_vtable.onmousedown = (psy_ui_fp_onmousedown)
			pianoroll_onmousedown;
		pianoroll_vtable.onmouseup = (psy_ui_fp_onmouseup)
			pianoroll_onmouseup;
		pianoroll_vtable.onmousemove = (psy_ui_fp_onmousemove)
			pianoroll_onmousemove;
		pianoroll_vtable.onmousedoubleclick = (psy_ui_fp_onmousedoubleclick)
			pianoroll_onmousedoubleclick;
		pianoroll_vtable.ontimer = (psy_ui_fp_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = 1;
	}
}

void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	pianoroll_vtable_init(self);
	self->component.vtable = &pianoroll_vtable;
	defaultkeyboardconfig_init();
	self->keyboardconfig = defaultkeyboardconfig;
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;
	self->pattern = 0;	
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		pianoroll_ondestroy);
	// left area (keyboardheader, keyboard)
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_enablealign(&self->left);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	psy_ui_component_init(&self->keyboardheader, &self->left);
	psy_ui_component_setalign(&self->keyboardheader, psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredsize(&self->keyboardheader, psy_ui_size_make(
		psy_ui_value_makeew(8), psy_ui_value_makeeh(1)));
	pianokeyboard_init(&self->keyboard, &self->left, &self->keyboardconfig);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_CLIENT);
	// top area (beatruler)
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	pianoheader_init(&self->header, &self->top, self);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	// client area (event grid)
	pianogrid_init(&self->grid, &self->component, &self->keyboardconfig, self->workspace);
	psy_ui_component_setalign(&self->grid.component, psy_ui_ALIGN_CLIENT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&self->grid.component.signal_scroll, self,
		pianoroll_ongridscroll);
	pianoroll_updatemetrics(self);
	psy_ui_component_starttimer(&self->component, 0, 100);
}

void pianoroll_ondestroy(Pianoroll* self, psy_ui_Component* component)
{
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
	pianogrid_setpattern(&self->grid, pattern);
	psy_ui_component_setscrolltop(&self->grid.component, 0);
	psy_ui_component_setscrolltop(&self->keyboard.component, 0);
	pianoroll_updatemetrics(self);
	pianogrid_adjustscroll(&self->grid);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->keyboard.component);
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
		testrange(self->grid.lastplayposition -
			self->grid.sequenceentryoffset,
			offset, 1 / (psy_dsp_big_beat_t)self->grid.metrics.lpb);
}

int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position >= offset && position < offset + width;
}

void pianoroll_invalidateline(Pianoroll* self, psy_dsp_big_beat_t offset)
{
	if (offset >= self->grid.sequenceentryoffset &&
		offset < self->grid.sequenceentryoffset + self->pattern->length) {
		int line;

		line = (int)((offset - self->grid.sequenceentryoffset) *
			self->grid.metrics.lpb);
		psy_ui_component_invalidaterect(&self->grid.component,
			psy_ui_rectangle_make(
				(int)(self->grid.metrics.stepwidth * line +
					self->grid.beatscrollpos * self->grid.metrics.beatwidth),
				0,
				(int)self->grid.metrics.stepwidth,
				self->grid.metrics.visikeys * self->keyboardconfig.keyheight));
	}
}

void pianoroll_onsize(Pianoroll* self, psy_ui_Size* size)
{
	pianoroll_updatemetrics(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_setfocus(&self->grid.component);
}

void pianoroll_onmouseup(Pianoroll* self, psy_ui_MouseEvent* ev)
{
}

void pianoroll_onmousemove(Pianoroll* self, psy_ui_MouseEvent* ev)
{
}

void pianoroll_onmousedoubleclick(Pianoroll* self, psy_ui_MouseEvent* ev)
{
}


void pianoroll_updatemetrics(Pianoroll* self)
{
	PianoMetrics metrics;

	pianometrics_update(&metrics, &self->grid, self->pattern, self->grid.lpb);
	self->grid.metrics = metrics;
	self->header.metrics = metrics;
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	pianoroll_updatemetrics(self);
	pianogrid_adjustscroll(&self->grid);
	psy_ui_component_invalidate(&self->grid.component);
	psy_ui_component_invalidate(&self->header.component);	
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender, int stepx, int stepy)
{
	if (stepx != 0) {
		self->header.scrollpos = self->grid.beatscrollpos;
		self->header.scrollpos = self->grid.beatscrollpos;
		self->header.metrics = self->grid.metrics;
		psy_ui_component_invalidate(&self->header.component);
		psy_ui_component_update(&self->header.component);
	}
	if (stepy != 0) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(&self->grid.component));
		psy_ui_component_invalidate(&self->keyboard.component);
		psy_ui_component_update(&self->keyboard.component);
	}
}

// Piano Metrics
void pianometrics_update(PianoMetrics* rv, Pianogrid* grid,
	psy_audio_Pattern* pattern, int lpb)
{
	psy_ui_Size gridsize;
	psy_ui_TextMetric tm;

	gridsize = psy_ui_component_size(&grid->component);
	tm = psy_ui_component_textmetric(&grid->component);
	rv->lpb = 4;
	rv->beatwidth = 80;
	rv->visibeats = pattern
		? min(max(0, pattern->length + grid->beatscrollpos),
			psy_ui_value_px(&gridsize.width, &tm) / (psy_dsp_big_beat_t)rv->beatwidth)
		: 0;
	rv->visisteps = (int)(rv->visibeats * rv->lpb + 0.5);
	rv->visiwidth = (int)(rv->visibeats * rv->beatwidth + 0.5);
	rv->stepwidth = rv->beatwidth / (psy_dsp_big_beat_t)rv->lpb;
	rv->visikeys = (int)(psy_ui_value_px(&gridsize.height, &tm) /
		(float)grid->keyboardconfig->keyheight + 0.5);
}
