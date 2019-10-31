// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"
#include <stdio.h>

#define TIMERID_PIANOROLL 640

static void pianoheader_ondraw(PianoHeader*, ui_component* sender, ui_graphics*);
void pianoheader_drawruler(PianoHeader*, ui_graphics*);


static void pianoroll_onlpbchanged(Pianoroll*, Player* sender, unsigned int lpb);
static void pianoroll_ontimer(Pianoroll*, ui_component* sender, 
	int timerid);
static void pianogrid_ondraw(Pianogrid*, ui_component* sender, ui_graphics*);
static void pianogrid_drawgrid(Pianogrid*, ui_graphics*);
static void pianogrid_drawevents(Pianogrid*, ui_graphics*);
static void pianogrid_drawevent(Pianogrid*, ui_graphics*, PatternEvent*, int track, float offset, float length);
static void pianogrid_onsize(Pianogrid*, ui_component* sender, ui_size*);
static void pianogrid_adjustscroll(Pianogrid*);
static void pianoroll_updatemetrics(Pianoroll*);
static void pianoroll_computemetrics(Pianoroll*, PianoMetrics*);
static void pianogrid_onscroll(Pianogrid*, ui_component* sender, int cx, int cy);
static void pianogrid_onkeydown(Pianogrid*, ui_component* sender, int keycode, int keydata);
static void pianogrid_onmousedown(Pianogrid*, ui_component* sender, int x, int y, int button);

static void pianoroll_ondestroy(Pianoroll*, ui_component* component);
static void pianoroll_onsize(Pianoroll*, ui_component* sender, ui_size*);
static void pianoroll_onmousedown(Pianoroll*, ui_component* sender, int x, int y, int button);
static void pianoroll_onmouseup(Pianoroll*, ui_component* sender, int x, int y, int button);
static void pianoroll_onmousemove(Pianoroll*, ui_component* sender,int x, int y, int button);
static void pianoroll_onmousedoubleclick(Pianoroll*, ui_component* sender, int x, int y, int button);
static void pianoroll_onkeydown(Pianoroll*, ui_component* sender, int keycode, int keydata);

static void pianokeyboard_ondraw(PianoKeyboard*, ui_component* sender, ui_graphics* g);

void pianoroll_init(Pianoroll* self, ui_component* parent, Workspace* workspace)
{				
	self->workspace = workspace;
	self->cx = 0;
	self->cy = 0;
	self->opcount = 0;
	self->syncpattern = 1;
	self->pattern = 0;
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	signal_connect(&self->component.signal_destroy, self, pianoroll_ondestroy);
	signal_connect(&self->component.signal_size, self, pianoroll_onsize);
	signal_connect(&self->component.signal_mousedown, self,
		pianoroll_onmousedown);
	signal_connect(&self->component.signal_mouseup, self, pianoroll_onmouseup);
	signal_connect(&self->component.signal_mousemove, self,
		pianoroll_onmousemove);
	signal_connect(&self->component.signal_mousedoubleclick, self,
		pianoroll_onmousedoubleclick);
	signal_connect(&self->component.signal_keydown, self, pianoroll_onkeydown);
	signal_connect(&self->component.signal_timer, self, pianoroll_ontimer);
	pianoheader_init(&self->header, &self->component, self);
	ui_component_init(&self->keyboardheader, &self->component);
	pianokeyboard_init(&self->keyboard, &self->component);
	pianogrid_init(&self->grid, &self->component, self);		
	signal_connect(&workspace->player.signal_lpbchanged, self, pianoroll_onlpbchanged);
	pianoroll_updatemetrics(self);
	ui_component_starttimer(&self->component, TIMERID_PIANOROLL, 100);
}

void pianoroll_ondestroy(Pianoroll* self, ui_component* component)
{	
}

void pianoroll_ontimer(Pianoroll* self, ui_component* sender, int timerid)
{
	if (timerid == TIMERID_PIANOROLL) {		
		/*if (player_playing(self->grid.player)) {
			trackerview_invalidateline(self, self->lastplayposition);
			self->lastplayposition = player_position(&self->workspace->player);			
			trackerview_invalidateline(self, self->lastplayposition);
		} else {
			if (self->lastplayposition != -1) {				
				trackerview_invalidateline(self, self->lastplayposition);
				self->lastplayposition = -1;
			}
		}*/
		if (self->pattern && self->pattern->opcount != self->opcount &&
				self->syncpattern) {
			ui_invalidate(&self->grid.component);
		}		
		self->opcount = self->pattern ? self->pattern->opcount : 0;
	}
}

void pianoroll_onsize(Pianoroll* self, ui_component* sender, ui_size* size)
{	
	int keyboardwidth;
	int headerheight;

	self->cx = size->width;
	self->cy = size->height;
	keyboardwidth = 40;
	headerheight = 25;	
	ui_component_setposition(&self->keyboardheader, 0, 0, keyboardwidth, headerheight);
	ui_component_setposition(&self->keyboard.component, 0, headerheight, keyboardwidth, size->height - headerheight);
	ui_component_setposition(&self->header.component, keyboardwidth, 0, size->width - keyboardwidth, headerheight);
	ui_component_setposition(&self->grid.component, keyboardwidth, headerheight, size->width - keyboardwidth, size->height - headerheight);	
	pianoroll_updatemetrics(self);
}

void pianoroll_onmousedown(Pianoroll* self, ui_component* sender, int x, int y, int button)
{
	ui_component_setfocus(&self->component);	
}

void pianoroll_onmouseup(Pianoroll* self, ui_component* sender, int x, int y, int button)
{		
}

void pianoroll_onmousemove(Pianoroll* self, ui_component* sender, int x, int y, int button)
{	
}

void pianoroll_onmousedoubleclick(Pianoroll* self, ui_component* sender, int x, int y, int button)
{	
}

void pianoroll_onkeydown(Pianoroll* self, ui_component* sender, int keycode, int keydata)
{	
	ui_component_propagateevent(sender);
}

void pianogrid_init(Pianogrid* self, ui_component* parent, Pianoroll* roll)
{
	self->view = roll;	
	self->beatscrollpos = 0;
	self->dy = 0;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;	
	signal_connect(&self->component.signal_draw, self, pianogrid_ondraw);
	signal_connect(&self->component.signal_size, self, pianogrid_onsize);
	signal_connect(&self->component.signal_scroll, self, pianogrid_onscroll);
	signal_connect(&self->component.signal_keydown, self, pianogrid_onkeydown);
	signal_connect(&self->component.signal_mousedown, self,
		pianogrid_onmousedown);
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);	
	ui_component_sethorizontalscrollrange(&self->component, 0, 16);
	ui_component_setverticalscrollrange(&self->component, 0, 88);
}

void pianogrid_onsize(Pianogrid* self, ui_component* sender, ui_size* size)
{
	self->cx = size->width;
	self->cy = size->height;
	pianogrid_adjustscroll(self);	
}

void pianogrid_ondraw(Pianogrid* self, ui_component* sender, ui_graphics* g)
{	   		
	if (self->view->pattern) {
		pianogrid_drawgrid(self, g);
		pianogrid_drawevents(self, g);
	}
}

void pianogrid_drawgrid(Pianogrid* self, ui_graphics* g)
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
			beat_t cpx;
				
			for (c = 0, cpx = 0; c <= self->metrics.visisteps;
					cpx += self->metrics.stepwidth, ++c) {
				ui_drawline(g, (int) cpx, 0, (int) cpx, self->cy);
			}		
		}		
	}
}

void pianogrid_drawevents(Pianogrid* self, ui_graphics* g)
{
	PatternEntry channel[64];
	int track;	

	for (track = 0; track < 64; ++track) {
		channel[track].offset = -1.0f;		
	}	
	if (self->view->pattern) {
		PatternNode* curr;
		curr = self->view->pattern->events;
		while (curr) {			
			PatternEntry* entry = (PatternEntry*)(curr->entry);			
			if (channel[entry->track].offset != -1.0f) {
				pianogrid_drawevent(self, g, &channel[entry->track].event, entry->track,
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

void pianogrid_drawevent(Pianogrid* self, ui_graphics* g, PatternEvent* event, int track, float offset, float length)
{
	ui_rectangle r;
	int left = (int)((offset + self->beatscrollpos) * self->metrics.beatwidth);
	int width = (int)(length * self->metrics.beatwidth);
	ui_setrectangle(&r, left, (self->metrics.keymax - event->note - 1) * self->metrics.keyheight
		+ 1 + self->dy, width, self->metrics.keyheight - 2);
	ui_drawsolidrectangle(g, r, 0x00B1C8B0);	
}

void pianogrid_onkeydown(Pianogrid* self, ui_component* sender, int keycode, int keydata)
{	
	ui_component_propagateevent(sender);
}

void pianogrid_onscroll(Pianogrid* self, ui_component* sender, int cx, int cy)
{	
	if (self->cx != 0) {		
		self->beatscrollpos += (cx / self->metrics.beatwidth);
		self->view->header.scrollpos = self->beatscrollpos;
		pianoroll_computemetrics(self->view, &self->metrics);
		self->view->header.scrollpos = self->beatscrollpos;
		self->view->header.metrics = self->metrics;
		self->view->keyboard.metrics = self->metrics;
		ui_invalidate(&self->view->header.component);
		UpdateWindow((HWND)self->view->header.component.hwnd);
	}
	if (self->cy != 0) {
		self->dy += cy;
		self->view->keyboard.dy = self->dy;
		ui_invalidate(&self->view->keyboard.component);
		UpdateWindow((HWND)self->view->keyboard.component.hwnd);
	}	
}

// PianoKeyboard

void pianokeyboard_init(PianoKeyboard* self, ui_component* parent)
{	
	self->dy = 0;
	self->textheight = 12;	
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, pianokeyboard_ondraw);
}

int isblack(int key)
{
	int offset = key % 12;

	return (offset == 1 || offset == 3 || offset == 6 || offset == 8 
		|| offset == 10);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
}

void pianokeyboard_ondraw(PianoKeyboard* self, ui_component* sender, ui_graphics* g)
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

void pianogrid_onmousedown(Pianogrid* self, ui_component* sender, int x, int y, int button)
{
	beat_t offset;	
	
	offset = (x  / (beat_t) self->metrics.beatwidth) + self->beatscrollpos;
	offset = (int)(offset * (beat_t)self->metrics.lpb) / (beat_t)self->metrics.lpb;
	if (button == 1) {
		PatternEvent event;
		PatternNode* node = 0;
		PatternNode* prev = 0;

		patternevent_clear(&event);
		event.note = self->metrics.keymax - 1 - (y - self->dy) /
			self->metrics.keyheight;
		node = pattern_findnode(self->view->pattern, 0, offset, 0,
			1 / (beat_t) self->metrics.lpb, &prev);
		if (node) {				
			pattern_setevent(self->view->pattern, node, &event);		
		} else {
			pattern_insert(self->view->pattern, prev, 0, offset, &event);
		}
	} else 
	if (button == 2) {
		PatternNode* prev;
		PatternNode* node = pattern_findnode(self->view->pattern, 0,
			offset, 0, 1 / (beat_t) self->metrics.lpb, &prev);
		if (node) {
			pattern_remove(self->view->pattern, node);			
		}
	}
}

void pianoroll_setpattern(Pianoroll* self, Pattern* pattern)
{	
	self->pattern = pattern;
	self->grid.dy = 0;
	self->keyboard.dy = 0;
	pianoroll_updatemetrics(self);
	pianogrid_adjustscroll(&self->grid);
	ui_invalidate(&self->grid.component);
	ui_invalidate(&self->keyboard.component);
}

// Header

void pianoheader_init(PianoHeader* self, ui_component* parent, Pianoroll* roll)
{
	self->view = roll;	
	self->scrollpos = 0;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, pianoheader_ondraw);		
}

void pianoheader_ondraw(PianoHeader* self, ui_component* sender, ui_graphics* g)
{
	if (self->view->pattern) {
		pianoheader_drawruler(self, g);
	}
}

void pianoheader_drawruler(PianoHeader* self, ui_graphics* g)
{
	ui_size size;
	ui_margin margin = { 0, 0, 5, 0 };
	int baseline;		
	beat_t cpx;	
	int c;	

	size = ui_component_size(&self->component);
	baseline = size.height - margin.bottom;	
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
		_snprintf(txt, 40, "%d", (int)(c - self->scrollpos));
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
			  gridsize.width / (beat_t) rv->beatwidth)
		: 0;
	rv->visisteps = (int)(rv->visibeats * rv->lpb + 0.5);
	rv->visiwidth = (int)(rv->visibeats * rv->beatwidth + 0.5);
	rv->stepwidth = rv->beatwidth / (beat_t) rv->lpb;
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

void pianoroll_onlpbchanged(Pianoroll* self, Player* sender, unsigned int lpb)
{
	pianoroll_updatemetrics(self);
	pianogrid_adjustscroll(&self->grid);
	ui_invalidate(&self->grid.component);
	ui_invalidate(&self->header.component);	
}
