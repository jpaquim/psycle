// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "pianoroll.h"

static void OnDraw(Pianoroll* self, ui_component* sender, ui_graphics* g);
static void DrawBackground(Pianoroll* self, ui_graphics* g);
static void OnGridDraw(Pianogrid* self, ui_component* sender, ui_graphics* g);
static void DrawGridBackground(Pianogrid* self, ui_graphics* g);
static void DrawGrid(Pianogrid* self, ui_graphics* g);
static void DrawEvents(Pianogrid* self, ui_graphics* g);
static void DrawEvent(Pianogrid* self, ui_graphics* g, PatternEvent* event, int track, float offset, float length);
static void OnSize(Pianoroll* self, ui_component* sender, int width, int height);
static void OnGridSize(Pianogrid* self, ui_component* sender, int width, int height);
static void OnDestroy(Pianoroll* self, ui_component* component);
static void OnGridDestroy(Pianogrid* self, ui_component* component);
static void OnGridKeyDown(Pianogrid* self, ui_component* sender, int keycode, int keydata);
static void OnMouseDown(Pianoroll* self, ui_component* sender, int x, int y, int button);
static void OnMouseUp(Pianoroll* self, ui_component* sender, int x, int y, int button);
static void OnMouseMove(Pianoroll* self, ui_component* sender,int x, int y, int button);
static void OnMouseDoubleClick(Pianoroll* self, ui_component* sender, int x, int y, int button);
static void OnKeyDown(Pianoroll* self, ui_component* sender, int keycode, int keydata);
static void OnScroll(Pianogrid* self, ui_component* sende, int cx, int cy);


void InitPianoroll(Pianoroll* self, ui_component* parent)
{				
	self->cx = 0;
	self->cy = 0;
	ui_component_init(&self->component, parent);		
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mouseup, self, OnMouseUp);
	signal_connect(&self->component.signal_mousemove, self, OnMouseMove);
	signal_connect(&self->component.signal_mousedoubleclick, self,OnMouseDoubleClick);
	signal_connect(&self->component.signal_keydown, self, OnKeyDown);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	InitPianogrid(&self->grid, &self->component, self);
	self->pattern = 0;
	ui_component_move(&self->component, 0, 0);	
}

void OnDestroy(Pianoroll* self, ui_component* component)
{	
}

void OnDraw(Pianoroll* self, ui_component* sender, ui_graphics* g)
{	   	
	DrawBackground(self, g);
}

void DrawBackground(Pianoroll* self, ui_graphics* g)
{
//	ui_rectangle r;
	
//	ui_setrectangle(&r, 0, 0, self->cx, self->cy);
//	ui_drawsolidrectangle(g, r, 0xFFFF0000);	
}

void OnSize(Pianoroll* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;
	ui_component_resize(&self->grid.component, width, height);
}

void OnMouseDown(Pianoroll* self, ui_component* sender, int x, int y, int button)
{
	ui_component_setfocus(&self->component);	
}

void OnMouseUp(Pianoroll* self, ui_component* sender, int x, int y, int button)
{		
}

void OnMouseMove(Pianoroll* self, ui_component* sender, int x, int y, int button)
{	
}

void OnMouseDoubleClick(Pianoroll* self, ui_component* sender, int x, int y, int button)
{	
}

void OnKeyDown(Pianoroll* self, ui_component* sender, int keycode, int keydata)
{	
	ui_component_propagateevent(sender);
}

void InitPianogrid(Pianogrid* self, ui_component* parent, Pianoroll* roll)
{
	self->view = roll;
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_destroy, self, OnGridDestroy);	
	signal_connect(&self->component.signal_size, self, OnGridSize);
	signal_connect(&self->component.signal_keydown, self, OnGridKeyDown);	
	signal_connect(&self->component.signal_draw, self, OnGridDraw);
	signal_connect(&self->component.signal_scroll, self, OnScroll);	
	self->keyheight = 12;
	self->beatwidth = 80;
	self->bpl = 0.25;
	self->dx = 0;
	self->dy = 0;
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);
	self->component.doublebuffered = 1;
	self->component.scrollstepx = self->beatwidth;
	self->component.scrollstepy = self->keyheight;
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);
	ui_component_sethorizontalscrollrange(&self->component, 0, 16);
	ui_component_setverticalscrollrange(&self->component, 0, 88);
}

void OnGridDestroy(Pianogrid* self, ui_component* component)
{	
}

void OnGridSize(Pianogrid* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;
}

void OnGridDraw(Pianogrid* self, ui_component* sender, ui_graphics* g)
{	   	
	DrawGridBackground(self, g);
	if (self->view->pattern) {
		DrawGrid(self, g);
		DrawEvents(self, g);
	}
}

void DrawGridBackground(Pianogrid* self, ui_graphics* g)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, 0, 0, self->cx, self->cy);
	ui_drawsolidrectangle(g, r, 0x009a887c);	
}

void DrawGrid(Pianogrid* self, ui_graphics* g)
{
	int keymin = 0;
	int keymax = 88;
	int key;
	for (key = keymin; key < keymax; ++key) {
		ui_rectangle r;
		ui_setrectangle(&r, 0, key * self->keyheight, self->cx, self->keyheight-1);
		ui_drawsolidrectangle(g, r, 0x00c1b5aa);
	}

	if (self->view->pattern) {
		float offset = 0;
		int cpx = 0;
		while (offset < self->view->pattern->length) {
			ui_drawline(g, cpx, 0, cpx, self->cy);
			offset += self->bpl;
			cpx += (int) (self->bpl * self->beatwidth);
		}
	}
}

void DrawEvents(Pianogrid* self, ui_graphics* g)
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
				DrawEvent(self, g, &channel[entry->track].event, entry->track,
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
			DrawEvent(self, g, &channel[track].event, track,
				channel[track].offset,
				self->view->pattern->length - channel[track].offset);			
		}
	}
}

void DrawEvent(Pianogrid* self, ui_graphics* g, PatternEvent* event, int track, float offset, float length)
{
	ui_rectangle r;
	int left = (int)(offset * self->beatwidth) + self->dx;
	int width = (int)(length * self->beatwidth);
	ui_setrectangle(&r, left, (88 - event->note) * self->keyheight + 1 + self->dy, width, self->keyheight-2);
	ui_drawsolidrectangle(g, r, 0x00d5ccc6);	
}

void OnGridKeyDown(Pianogrid* self, ui_component* sender, int keycode, int keydata)
{	
	ui_component_propagateevent(sender);
}

void OnScroll(Pianogrid* self, ui_component* sender, int cx, int cy)
{	
	self->dx += cx;
	self->dy += cy;	
}

