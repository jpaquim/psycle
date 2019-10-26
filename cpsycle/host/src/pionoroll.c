// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"

static void OnGridDraw(Pianogrid*, ui_component* sender, ui_graphics*);
static void DrawGridBackground(Pianogrid*, ui_graphics*);
static void DrawGrid(Pianogrid*, ui_graphics*);
static void DrawEvents(Pianogrid*, ui_graphics*);
static void DrawEvent(Pianogrid*, ui_graphics*, PatternEvent* event, int track, float offset, float length);
static void OnSize(Pianoroll*, ui_component* sender, ui_size*);
static void OnGridSize(Pianogrid*, ui_component* sender, ui_size*);
static void OnDestroy(Pianoroll*, ui_component* component);
static void OnGridDestroy(Pianogrid*, ui_component* component);
static void OnGridKeyDown(Pianogrid*, ui_component* sender, int keycode, int keydata);
static void OnMouseDown(Pianoroll*, ui_component* sender, int x, int y, int button);
static void OnMouseUp(Pianoroll*, ui_component* sender, int x, int y, int button);
static void OnMouseMove(Pianoroll*, ui_component* sender,int x, int y, int button);
static void OnMouseDoubleClick(Pianoroll*, ui_component* sender, int x, int y, int button);
static void OnKeyDown(Pianoroll*, ui_component* sender, int keycode, int keydata);
static void OnScroll(Pianogrid*, ui_component* sender, int cx, int cy);

static void OnPianoKeyboardDraw(PianoKeyboard*, ui_component* sender, ui_graphics* g);
static void InitPianoKeyboard(PianoKeyboard*, ui_component* parent);

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
	InitPianoKeyboard(&self->keyboard, &self->component);
	InitPianogrid(&self->grid, &self->component, self);	
	self->pattern = 0;
	ui_component_move(&self->component, 0, 0);	
}

void OnDestroy(Pianoroll* self, ui_component* component)
{	
}

void OnSize(Pianoroll* self, ui_component* sender, ui_size* size)
{
	int keyboardwidth;

	self->cx = size->width;
	self->cy = size->height;
	keyboardwidth = 40;
	ui_component_setposition(&self->keyboard.component, 0, 0, keyboardwidth, size->height);
	ui_component_setposition(&self->grid.component, keyboardwidth, 0, size->width - keyboardwidth, size->height);
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
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
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

void OnGridSize(Pianogrid* self, ui_component* sender, ui_size* size)
{
	self->cx = size->width;
	self->cy = size->height;
}

void OnGridDraw(Pianogrid* self, ui_component* sender, ui_graphics* g)
{	   		
	if (self->view->pattern) {
		DrawGrid(self, g);
		DrawEvents(self, g);
	}
}

void DrawGrid(Pianogrid* self, ui_graphics* g)
{
	int keymin = 0;
	int keymax = 88;
	int key;
	

	if (self->view->pattern) {
		float offset = 0;
		int cpx = 0;

		ui_setcolor(g, 0x00333333);
		for (key = keymin; key < keymax; ++key) {	
			ui_drawline(g, 0, key * self->keyheight, self->cx, key * self->keyheight);
		}
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
	ui_drawsolidrectangle(g, r, 0x00B1C8B0);	
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

// PianoKeyboard

void InitPianoKeyboard(PianoKeyboard* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_connect(&self->component.signal_draw, self, OnPianoKeyboardDraw);
	self->dy = 0;
	self->textheight = 12;
	self->keyheight = 12;
	self->component.doublebuffered = 1;
}

int isblack(int key)
{
	int offset = key % 12;

	return (offset == 1 || offset == 3 || offset == 6 || offset == 8 
		|| offset == 10);
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
}

void OnPianoKeyboardDraw(PianoKeyboard* self, ui_component* sender, ui_graphics* g)
{		
	int keymin = 0;
	int keymax = 88;
	int key;	
	int keyboardheight;
	ui_size size;
		
	keyboardheight = (keymax - keymin) * self->keyheight;
	size = ui_component_size(&self->component);
	ui_setcolor(g, 0x00333333);
	for (key = keymin; key < keymax; ++key) {	
		int cpy;

		cpy = keyboardheight - key * self->keyheight;

		ui_drawline(g, 0, cpy, size.width, cpy);
		if (isblack(key)) {
			ui_rectangle r;			
			ui_setrectangle(&r, (int)(size.width * 0.75), cpy, 
				(int)(size.width * 0.25), self->keyheight);
			ui_drawsolidrectangle(g, r, 0x00CACACA);
			ui_drawline(g, 0, cpy + self->keyheight/2, size.width, 
					cpy + self->keyheight/2);
			ui_setrectangle(&r, 0, cpy, (int)(size.width * 0.75), self->keyheight);
			ui_drawsolidrectangle(g, r, 0x00444444);						
		} else {
			ui_rectangle r;			
			ui_setrectangle(&r, 0, cpy, size.width, self->keyheight);
			ui_drawsolidrectangle(g, r, 0x00CACACA);
			if (key % 12 == 0 || ((key % 12) == 5)) {
				ui_drawline(g, 0, cpy + self->keyheight, size.width, 
					cpy + self->keyheight);
			}
			//} else {
			//	ui_drawline(g, 0, cpy, size.width, cpy);			
			//}
		}
	}
}


