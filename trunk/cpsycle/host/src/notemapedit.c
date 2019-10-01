// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "notemapedit.h"
#include "cmdsnotes.h"
#include <string.h>
#include <stdio.h>

static void OnDraw(NoteMapEdit* self, ui_component* sender, ui_graphics* g);
static void SetColColor(ui_graphics* g, int col, int cursor);
static void DrawDigit(NoteMapEdit* self, ui_graphics* g, int digit, int col, int x, int y);
static void OnSize(NoteMapEdit* self, ui_component* sender, int width, int height);
static void OnDestroy(NoteMapEdit* self, ui_component* component);
static void OnMouseDown(NoteMapEdit* self, ui_component* sender, int x, int y, int button);
static void OnKeyDown(NoteMapEdit* self, ui_component* sender, int keycode, int keydata);
static void DefaultMapping(NoteMapEdit* self);


extern char* notes_tab_a440[256];

void InitNoteMapEdit(NoteMapEdit* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);	
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_keydown,self, OnKeyDown);

	self->cursor.col = 0;
	self->cursor.note = 0;
	self->dy = 0;
	memset(&self->map[0], 0, sizeof(self->map));
	DefaultMapping(self);
}

void DefaultMapping(NoteMapEdit* self)
{
	int note;
	for (note = 0; note < 120; ++note) {
		self->map[note].note = note;
		self->map[note].mach = 0;
	}
}

void OnDestroy(NoteMapEdit* self, ui_component* component)
{		
}

void OnDraw(NoteMapEdit* self, ui_component* sender, ui_graphics* g)
{	
	int note;
	int cpy;
	int lineheight = 20;

	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, 0x009a887c);	
	cpy = self->dy;
	ui_setbackgroundcolor(g, 0xFFAAAAA);
	for (note = 0; note < 120; ++note) {
		PatternEvent event = self->map[note];		
		int cursor = self->cursor.note == note;
		int hi = (event.mach & 0xF0) >> 4;
		int lo = event.mach & 0x0F;
		if (event.mach == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(g, 0, cursor);
		ui_settextcolor(g, 0x00000000);
		ui_setrectangle(&r, 0, cpy, size.width, size.height);		
		ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r,
			notes_tab_a440[note],
			strlen(notes_tab_a440[note]));
		ui_setrectangle(&r, 40, cpy, size.width, size.height);
		SetColColor(g, self->cursor.col == 0, cursor);
		ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r,
			notes_tab_a440[self->map[note].note],
			strlen(notes_tab_a440[self->map[note].note]));		
		SetColColor(g, self->cursor.col == 1, cursor);
		DrawDigit(self, g, hi, 0, 80, cpy);
		SetColColor(g, self->cursor.col == 2, cursor);
		DrawDigit(self, g, lo, 1, 80, cpy);		
		cpy += lineheight;
	}
}

void SetColColor(ui_graphics* g, int col, int cursor)
{
	if (cursor != 0) {		
		ui_setbackgroundcolor(g, 0x00c9beb8);
		if (col != 0) {		
		  ui_settextcolor(g, 0x00ffffff);					
		} else {
		  ui_settextcolor(g, 0x00000000);		
		}
	} else {
		ui_setbackgroundcolor(g, 0x009a887c);
		ui_settextcolor(g, 0x00000000);		
	}	
}

void DrawDigit(NoteMapEdit* self, ui_graphics* g, int digit, int col, int x, int y)
{
	char buffer[20];	
	ui_rectangle r;
	int textwidth = 10;
	int lineheight = 20;
	ui_setrectangle(&r, x + col * textwidth, y, textwidth, lineheight);
	if (digit != -1) {
		_snprintf(buffer, 2, "%X", digit);	
	} else {
		_snprintf(buffer, 2, "%s", "");	
	}
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer, strlen(buffer));	
}

void OnSize(NoteMapEdit* self, ui_component* sender, int width, int height)
{
}

void OnMouseDown(NoteMapEdit* self, ui_component* sender, int x, int y, int button)
{
}

void OnKeyDown(NoteMapEdit* self, ui_component* sender, int keycode, int keydata)
{
	ui_size size = ui_component_size(&self->component);
	int scrolllines = -self->dy / 20;
	int visiblelines = size.height / 20;
	if (keycode == VK_UP) {		
		if (self->cursor.note > 0) {
			--self->cursor.note;
			if (self->cursor.note < scrolllines) {
				self->dy += 20; 
			}
			ui_invalidate(&self->component);
		}		
	} else
	if (keycode == VK_DOWN) {		
		if (self->cursor.note < 120) {
			++self->cursor.note;
			if (self->cursor.note > visiblelines + scrolllines) {
				self->dy -= 20; 
			}
			ui_invalidate(&self->component);
		}
	} else
	if (keycode == VK_LEFT) {
		--self->cursor.col;
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_RIGHT) {
		++self->cursor.col;
		ui_invalidate(&self->component);
	} else {
		int cmd;

		cmd = inputs_cmd(self->noteinputs, keycode);
		if (cmd != -1) {		
			int base = 48;
			self->map[self->cursor.note].note = (unsigned char)(base + cmd);
			ui_invalidate(&self->component);
		}
	}
}