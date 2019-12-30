// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"
#include <stdio.h>
#include <portable.h>

#define TIMERID_PIANOROLL 640

static void pianoheader_ondraw(PianoHeader*, psy_ui_Component* sender,
	psy_ui_Graphics*);
void pianoheader_drawruler(PianoHeader*, psy_ui_Graphics*);

static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player* sender,
	uintptr_t lpb);
static void pianoroll_ontimer(Pianoroll*, psy_ui_Component* sender, 
	int timerid);
static void pianogrid_ondraw(Pianogrid*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static void pianogrid_drawgrid(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawevents(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawevent(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternEvent*, int track, float offset, float length);
static void pianogrid_onsize(Pianogrid*, psy_ui_Component* sender, ui_size*);
static void pianogrid_adjustscroll(Pianogrid*);
static void pianogrid_onscroll(Pianogrid*, psy_ui_Component* sender, int stepx,
	int stepy);
static void pianogrid_onkeydown(Pianogrid*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);

static void pianoroll_updatemetrics(Pianoroll*);
static void pianoroll_computemetrics(Pianoroll*, PianoMetrics*);
static int pianorolld_testplaybar(Pianoroll*, psy_dsp_big_beat_t offset);
static int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width);
static void pianoroll_invalidateline(Pianoroll*, psy_dsp_beat_t offset);
static void pianoroll_ondestroy(Pianoroll*, psy_ui_Component* component);
static void pianoroll_onsize(Pianoroll*, psy_ui_Component* sender, ui_size*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void pianoroll_onmouseup(Pianoroll*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void pianoroll_onmousemove(Pianoroll*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void pianoroll_onmousedoubleclick(Pianoroll*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void pianoroll_onkeydown(Pianoroll*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);

static void pianokeyboard_ondraw(PianoKeyboard*, psy_ui_Component* sender,
	psy_ui_Graphics*);

void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	Workspace* workspace)
{				
	self->workspace = workspace;	
	self->opcount = 0;
	self->syncpattern = 1;
	self->pattern = 0;
	self->sequenceentryoffset = 0.f;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_destroy, self,
		pianoroll_ondestroy);
	psy_signal_connect(&self->component.signal_size, self, pianoroll_onsize);
	psy_signal_connect(&self->component.signal_mousedown, self,
		pianoroll_onmousedown);
	psy_signal_connect(&self->component.signal_mouseup, self,
		pianoroll_onmouseup);
	psy_signal_connect(&self->component.signal_mousemove, self,
		pianoroll_onmousemove);
	psy_signal_connect(&self->component.signal_mousedoubleclick, self,
		pianoroll_onmousedoubleclick);
	psy_signal_connect(&self->component.signal_keydown, self,
		pianoroll_onkeydown);
	psy_signal_connect(&self->component.signal_timer, self,
		pianoroll_ontimer);
	pianoheader_init(&self->header, &self->component, self);
	ui_component_init(&self->keyboardheader, &self->component);
	pianokeyboard_init(&self->keyboard, &self->component);
	pianogrid_init(&self->grid, &self->component, self);		
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	pianoroll_updatemetrics(self);
	ui_component_starttimer(&self->component, TIMERID_PIANOROLL, 100);
}

void pianoroll_ondestroy(Pianoroll* self, psy_ui_Component* component)
{	
}

void pianoroll_ontimer(Pianoroll* self, psy_ui_Component* sender, int timerid)
{
	if (timerid == TIMERID_PIANOROLL && self->pattern) {		
		if (player_playing(&self->workspace->player)) {
			pianoroll_invalidateline(self, self->lastplayposition);
			self->lastplayposition = player_position(&self->workspace->player);			
			pianoroll_invalidateline(self, self->lastplayposition);
		} else {
			if (self->lastplayposition != -1) {				
				pianoroll_invalidateline(self, self->lastplayposition);
				self->lastplayposition = -1;
			}
		}
		if (self->pattern && self->pattern->opcount != self->opcount &&
				self->syncpattern) {
			ui_component_invalidate(&self->grid.component);
		}		
		self->opcount = self->pattern ? self->pattern->opcount : 0;
	}
}

int pianoroll_testplaybar(Pianoroll* self, psy_dsp_big_beat_t offset)
{
	return player_playing(&self->workspace->player) &&
		testrange(self->lastplayposition -
			self->sequenceentryoffset,
			offset, 1 / (psy_dsp_big_beat_t)self->grid.metrics.lpb);
}

int testrange(psy_dsp_big_beat_t position, psy_dsp_big_beat_t offset,
	psy_dsp_big_beat_t width)
{
	return position >= offset && position < offset + width;
}

void pianoroll_invalidateline(Pianoroll* self, psy_dsp_beat_t offset)
{
	int line;
	ui_rectangle r;

	if (offset >= self->sequenceentryoffset &&
		offset < self->sequenceentryoffset + self->pattern->length) {
		line = (int)((offset - self->sequenceentryoffset) *
			self->grid.metrics.lpb);
		ui_setrectangle(&r,
			(int)(self->grid.metrics.stepwidth * line +
				self->grid.beatscrollpos * self->grid.metrics.beatwidth),
			0,			
			(int)self->grid.metrics.stepwidth,
			self->grid.metrics.visikeys * self->grid.metrics.keyheight);
		ui_component_invalidaterect(&self->grid.component, &r);
	}
}

void pianoroll_onsize(Pianoroll* self, psy_ui_Component* sender, ui_size* size)
{	
	int keyboardwidth;
	int headerheight;
		
	keyboardwidth = 40;
	headerheight = 25;	
	ui_component_setposition(&self->keyboardheader, 0, 0, keyboardwidth,
		headerheight);
	ui_component_setposition(&self->keyboard.component, 0, headerheight,
		keyboardwidth, size->height - headerheight);
	ui_component_setposition(&self->header.component, keyboardwidth, 0,
		size->width - keyboardwidth, headerheight);
	ui_component_setposition(&self->grid.component, keyboardwidth,
		headerheight, size->width - keyboardwidth, size->height - headerheight);	
	pianoroll_updatemetrics(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	ui_component_setfocus(&self->grid.component);	
}

void pianoroll_onmouseup(Pianoroll* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{		
}

void pianoroll_onmousemove(Pianoroll* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
}

void pianoroll_onmousedoubleclick(Pianoroll* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
}

void pianoroll_onkeydown(Pianoroll* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* keyevent)
{	
	ui_component_propagateevent(sender);
}

void pianogrid_init(Pianogrid* self, psy_ui_Component* parent, Pianoroll* roll)
{
	self->view = roll;	
	self->beatscrollpos = 0;
	self->dy = 0;	
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	self->component.wheelscroll = 4;
	psy_signal_connect(&self->component.signal_draw, self, pianogrid_ondraw);
	psy_signal_connect(&self->component.signal_size, self, pianogrid_onsize);
	psy_signal_connect(&self->component.signal_scroll, self,
		pianogrid_onscroll);
	psy_signal_connect(&self->component.signal_keydown, self,
		pianogrid_onkeydown);
	psy_signal_connect(&self->component.signal_mousedown, self,
		pianogrid_onmousedown);
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);	
	ui_component_sethorizontalscrollrange(&self->component, 0, 16);
	ui_component_setverticalscrollrange(&self->component, 0, 88);
}

void pianogrid_onsize(Pianogrid* self, psy_ui_Component* sender, ui_size* size)
{		
	pianogrid_adjustscroll(self);	
}

void pianogrid_ondraw(Pianogrid* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{	   		
	if (self->view->pattern) {
		pianogrid_drawgrid(self, g);
		pianogrid_drawevents(self, g);
	}
}

void pianogrid_drawgrid(Pianogrid* self, psy_ui_Graphics* g)
{
	if (self->view->pattern) {
		int keymin = 0;
		int keymax = 88;
		int key;		
						
		ui_setcolor(g, 0x00333333);
		for (key = keymin; key < keymax; ++key) {
			int cpy;

			cpy =  key * self->metrics.keyheight;
			ui_drawline(g, 0, cpy, self->metrics.visiwidth, cpy);
		}		

		{												
			int c;
			psy_dsp_beat_t cpx;
			int cpy;
			
			cpy = self->metrics.visikeys * self->metrics.keyheight;				
			for (c = 0, cpx = 0; c <= self->metrics.visisteps;
					cpx += self->metrics.stepwidth, ++c) {
				ui_drawline(g, (int) cpx, 0, (int) cpx, cpy);
			}		
		}		
	}

	if (player_playing(&self->view->workspace->player)) {
		psy_dsp_big_beat_t offset;
		
		offset = self->view->lastplayposition;
		if (offset >= self->view->sequenceentryoffset &&
			offset < self->view->sequenceentryoffset +
				self->view->pattern->length) {
			int line;
			ui_rectangle r;

			line = (int)((offset - self->view->sequenceentryoffset) *
				self->metrics.lpb);			
			ui_setrectangle(&r,
				(int) (self->metrics.stepwidth * line +
					self->beatscrollpos * self->metrics.beatwidth),
				0,
				(int)self->metrics.stepwidth,
				self->metrics.visikeys * self->metrics.keyheight);
			ui_drawsolidrectangle(g, r, 0x009F7B00);
		}

	}
}

void pianogrid_drawevents(Pianogrid* self, psy_ui_Graphics* g)
{
	psy_audio_PatternEntry channel[64];
	int track;	

	for (track = 0; track < 64; ++track) {
		channel[track].offset = -1.0f;		
	}	
	if (self->view->pattern) {
		PatternNode* curr;
		curr = self->view->pattern->events;
		while (curr) {			
			psy_audio_PatternEntry* entry;
			
			entry = (psy_audio_PatternEntry*)(curr->entry);			
			if (channel[entry->track].offset != -1.0f) {
				pianogrid_drawevent(self, g, &channel[entry->track].event,
					entry->track,
					channel[entry->track].offset,
					entry->offset - channel[entry->track].offset);				
				channel[entry->track].offset = -1.0f;
			}				
			if (entry->event.note == 120) {				
				channel[entry->track].offset = -1.0f;				
			} else {
				channel[entry->track].offset = entry->offset;								
				channel[entry->track].event = entry->event;
			}
			curr = curr->next;
		}
	}	
	for (track = 0; track < 64; ++track) {
		if (channel[track].offset != -1.0f) {
			pianogrid_drawevent(self, g, &channel[track].event, track,
				channel[track].offset,
				self->view->pattern->length - channel[track].offset);			
		}
	}
}

void pianogrid_drawevent(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternEvent* event, int track, float offset, float length)
{
	ui_rectangle r;	
	int left = (int)((offset + self->beatscrollpos) * self->metrics.beatwidth);
	int width = (int)(length * self->metrics.beatwidth);
	ui_setrectangle(&r, left,
		(self->metrics.keymax - event->note - 1) * self->metrics.keyheight
		+ 1 + self->dy, width, self->metrics.keyheight - 2);		
	ui_drawsolidrectangle(g, r, 0x00B1C8B0);	
}

void pianogrid_onkeydown(Pianogrid* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{	
	ui_component_propagateevent(sender);
}

void pianogrid_onscroll(Pianogrid* self, psy_ui_Component* sender, int stepx,
	int stepy)
{	
	if (stepx != 0) {		
		self->beatscrollpos += stepx;
		self->view->header.scrollpos = self->beatscrollpos;
		pianoroll_computemetrics(self->view, &self->metrics);
		self->view->header.scrollpos = self->beatscrollpos;
		self->view->header.metrics = self->metrics;
		self->view->keyboard.metrics = self->metrics;
		ui_component_invalidate(&self->view->header.component);
		ui_component_update(&self->view->header.component);		
	}
	if (stepy != 0) {
		self->dy += (stepy * sender->scrollstepy);
		self->view->keyboard.dy = self->dy;
		ui_component_invalidate(&self->view->keyboard.component);
		ui_component_update(&self->view->keyboard.component);
	}	
}

// PianoKeyboard

void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent)
{	
	self->dy = 0;
	self->textheight = 12;	
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	psy_signal_connect(&self->component.signal_draw, self,
		pianokeyboard_ondraw);
}

int isblack(int key)
{
	int offset = key % 12;

	return (offset == 1 || offset == 3 || offset == 6 || offset == 8 
		|| offset == 10);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
}

void pianokeyboard_ondraw(PianoKeyboard* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{		
	int keymin = 0;
	int keymax = 88;
	int key;	
	int keyboardheight;
	ui_size size;
		
	keyboardheight = (keymax - keymin) * self->metrics.keyheight;
	size = ui_component_size(&self->component);
	ui_setcolor(g, 0x00333333);
	for (key = keymin; key < keymax; ++key) {	
		int cpy;

		cpy = keyboardheight - (key + 1) * self->metrics.keyheight + self->dy;
		ui_drawline(g, 0, cpy, size.width, cpy);
		if (isblack(key)) {
			ui_rectangle r;			
			ui_setrectangle(&r, (int)(size.width * 0.75), cpy, 
				(int)(size.width * 0.25), self->metrics.keyheight);
			ui_drawsolidrectangle(g, r, 0x00CACACA);
			ui_drawline(g, 0, cpy + self->metrics.keyheight/2, size.width, 
					cpy + self->metrics.keyheight/2);
			ui_setrectangle(&r, 0, cpy, (int)(size.width * 0.75), 
				self->metrics.keyheight);
			ui_drawsolidrectangle(g, r, 0x00444444);						
		} else {
			ui_rectangle r;			
			ui_setrectangle(&r, 0, cpy, size.width, self->metrics.keyheight);
			ui_drawsolidrectangle(g, r, 0x00CACACA);
			if (key % 12 == 0 || ((key % 12) == 5)) {
				ui_drawline(g, 0, cpy + self->metrics.keyheight, size.width, 
					cpy + self->metrics.keyheight);
			}			
		}
	}
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	psy_dsp_beat_t offset;	

	ui_component_setfocus(&self->component);
	offset = (ev->x  / (psy_dsp_beat_t) self->metrics.beatwidth) +
		self->beatscrollpos;
	offset = (int)(offset * (psy_dsp_beat_t)self->metrics.lpb) /
		(psy_dsp_beat_t)self->metrics.lpb;
	if (ev->button == 1) {
		psy_audio_PatternEvent event;
		PatternNode* node = 0;
		PatternNode* prev = 0;

		patternevent_clear(&event);
		event.note = self->metrics.keymax - 1 - (ev->y - self->dy) /
			self->metrics.keyheight;
		node = pattern_findnode(self->view->pattern, 0, offset,
			1 / (psy_dsp_beat_t) self->metrics.lpb, &prev);
		if (node) {				
			pattern_setevent(self->view->pattern, node, &event);		
		} else {
			pattern_insert(self->view->pattern, prev, 0, offset, &event);
		}
	} else 
	if (ev->button == 2) {
		PatternNode* prev;
		PatternNode* node = pattern_findnode(self->view->pattern, 0,
			offset, 1 / (psy_dsp_beat_t) self->metrics.lpb, &prev);
		if (node) {
			pattern_remove(self->view->pattern, node);			
		}
	}
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{	
	self->pattern = pattern;
	self->grid.dy = 0;
	self->keyboard.dy = 0;
	pianoroll_updatemetrics(self);
	pianogrid_adjustscroll(&self->grid);
	ui_component_invalidate(&self->grid.component);
	ui_component_invalidate(&self->keyboard.component);
}

// Header
void pianoheader_init(PianoHeader* self, psy_ui_Component* parent, Pianoroll* roll)
{
	self->view = roll;	
	self->scrollpos = 0;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	psy_signal_connect(&self->component.signal_draw, self, pianoheader_ondraw);		
}

void pianoheader_ondraw(PianoHeader* self, psy_ui_Component* sender,
	psy_ui_Graphics* g)
{
	if (self->view->pattern) {
		pianoheader_drawruler(self, g);
	}
}

void pianoheader_drawruler(PianoHeader* self, psy_ui_Graphics* g)
{
	ui_size size;
	ui_margin margin = { 0, 0, 5, 0 };
	int baseline;		
	psy_dsp_beat_t cpx;	
	int c;	

	size = ui_component_size(&self->component);
	baseline = size.height - margin.bottom.quantity.integer;	
	ui_setcolor(g, 0x00CACACA); 
	ui_drawline(g, 0, baseline, size.width, baseline);	
	
	for (c = 0, cpx = 0; c <= self->metrics.visisteps; 
			cpx += self->metrics.stepwidth, ++c) {		
		ui_drawline(g, (int) cpx, baseline, (int) cpx, baseline - 4);		
	}		
	ui_setbackgroundmode(g, TRANSPARENT);
	ui_settextcolor(g, 0x00CACACA);
	for (c = 0, cpx = 0; c <= self->metrics.visibeats; 
			cpx += self->metrics.beatwidth, ++c) {		
		char txt[40];
		ui_drawline(g, (int) cpx, baseline, (int) cpx, baseline - 6);
		psy_snprintf(txt, 40, "%d", (int)(c - self->scrollpos));
		ui_textout(g, (int) cpx + 3, baseline - 14, txt, strlen(txt));		
	}
}

void pianoroll_updatemetrics(Pianoroll* self)
{
	PianoMetrics metrics;

	pianoroll_computemetrics(self, &metrics);
	self->grid.metrics = metrics;
	self->header.metrics = metrics;
	self->keyboard.metrics = metrics;	
}

void pianoroll_computemetrics(Pianoroll* self, PianoMetrics* rv)
{	
	ui_size gridsize;	

	gridsize = ui_component_size(&self->grid.component);		
	rv->lpb = player_lpb(&self->workspace->player);
	rv->beatwidth = 80;
	rv->keyheight = 12;
	rv->keymin = 0;
	rv->keymax = 88;	
	rv->visibeats = self->pattern
		? min(max(0, self->pattern->length + self->grid.beatscrollpos),
			  gridsize.width / (psy_dsp_beat_t) rv->beatwidth)
		: 0;
	rv->visisteps = (int)(rv->visibeats * rv->lpb + 0.5);
	rv->visiwidth = (int)(rv->visibeats * rv->beatwidth + 0.5);
	rv->stepwidth = rv->beatwidth / (psy_dsp_beat_t) rv->lpb;
	rv->visikeys = (int)(gridsize.height / (float)rv->keyheight + 0.5);
}

void pianogrid_adjustscroll(Pianogrid* self)
{
	PianoMetrics metrics;	

	pianoroll_computemetrics(self->view, &metrics);
	self->component.scrollstepx = metrics.beatwidth;
	self->component.scrollstepy = metrics.keyheight;
	ui_component_sethorizontalscrollrange(&self->component, 0, 
		(int)((self->view->pattern ? 
		self->view->pattern->length : 0) - metrics.visibeats + 0.5));
	ui_component_setverticalscrollrange(&self->component, 0,
		metrics.keymax - metrics.keymin - metrics.visikeys);
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	pianoroll_updatemetrics(self);
	pianogrid_adjustscroll(&self->grid);
	ui_component_invalidate(&self->grid.component);
	ui_component_invalidate(&self->header.component);	
}
