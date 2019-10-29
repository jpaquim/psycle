// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "inputdefiner.h"
#include "inputmap.h"
#include <stdio.h>

static void ondraw(InputDefiner*, ui_component* sender, ui_graphics*);
static void onkeydown(InputDefiner*, ui_component* sender, int keycode, int keydata);
static void onkeyup(InputDefiner*, ui_component* sender, int keycode, int keydata);

void inputdefiner_init(InputDefiner* self, ui_component* parent)
{
	self->input = 0;
	self->regularkey = 0;
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, ondraw);
	signal_connect(&self->component.signal_keydown, self, onkeydown);
	signal_connect(&self->component.signal_keyup, self, onkeyup);
}

void inputdefiner_setinput(InputDefiner* self, unsigned int input)
{
	self->input = input;
}

void inputdefiner_text(InputDefiner* self, char* text)
{
	char keystr[5];
	int keycode;
	int shift;
	int ctrl;

	decodeinput(self->input, &keycode, &shift, &ctrl);
	_snprintf(keystr, 5, "%c", keycode);
	text[0] = '\0';	
	if (shift) {
		strcat(text, "Shift + ");		
	}
	if (ctrl) {		
		strcat(text, "Ctrl + ");
	}
	if (keycode >= 0x30) {	
		strcat(text, keystr);	
	}
}

void ondraw(InputDefiner* self, ui_component* sender, ui_graphics* g)
{
	char text[40];	
	
	ui_settextcolor(g, 0x00FFFFFF);
	ui_setbackgroundmode(g, TRANSPARENT);
	inputdefiner_text(self, text);
	ui_textout(g, 0, 0, text, strlen(text));
}

void onkeydown(InputDefiner* self, ui_component* sender, int keycode, int keydata)
{
	int shift;
	int ctrl;	

	shift = GetKeyState (VK_SHIFT) < 0;
	ctrl = GetKeyState (VK_CONTROL) < 0;	

	if ((keycode == VK_SHIFT || keycode == VK_CONTROL)) {
		if (self->regularkey == 0) {
			self->input = encodeinput(0, shift, ctrl);
		} else {
			self->input = encodeinput(self->regularkey, shift, ctrl);
		}
	}
	if (keycode >= 0x30) {
		self->regularkey = keycode;	
		self->input = encodeinput(self->regularkey, shift, ctrl);
	}
	ui_invalidate(&self->component);
}

void onkeyup(InputDefiner* self, ui_component* sender, int keycode, int keydata)
{
	int shift;
	int ctrl;
	int inputkeycode;
	int inputshift;
	int inputctrl;

	shift = GetKeyState (VK_SHIFT) < 0;
	ctrl = GetKeyState (VK_CONTROL) < 0;
	decodeinput(self->input, &inputkeycode, &inputshift, &inputctrl);
	if (self->regularkey) {		
		self->input = encodeinput(inputkeycode, shift, ctrl);
	}
	if (keycode >= 0x30) {
		self->regularkey = 0;
	}
	if (inputkeycode <= 0x30) {
		self->input = encodeinput(0, shift, ctrl);
	}
	ui_invalidate(&self->component);
}

