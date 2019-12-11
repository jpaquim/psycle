// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "inputdefiner.h"
#include "inputmap.h"
#include <stdio.h>
#include <portable.h>

static void ondraw(InputDefiner*, ui_component* sender, ui_graphics*);
static void onkeydown(InputDefiner*, ui_component* sender, KeyEvent*);
static void onkeyup(InputDefiner*, ui_component* sender, KeyEvent*);

void inputdefiner_init(InputDefiner* self, ui_component* parent)
{
	self->input = 0;
	self->regularkey = 0;
	ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_draw, self, ondraw);
	psy_signal_connect(&self->component.signal_keydown, self, onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self, onkeyup);
}

void inputdefiner_setinput(InputDefiner* self, unsigned int input)
{
	self->input = input;
}

void inputdefiner_text(InputDefiner* self, char* text)
{
	char keystr[10];
	int keycode;
	int shift;
	int ctrl;

	decodeinput(self->input, &keycode, &shift, &ctrl);
	if (keycode == VK_RETURN) {
		strcpy(keystr, "RETURN");
	} else
	if (keycode == VK_LEFT) {
		strcpy(keystr, "LEFT");
	} else
	if (keycode == VK_RIGHT) {
		strcpy(keystr, "RIGHT");
	} else
	if (keycode == VK_UP) {
		strcpy(keystr, "UP");
	} else
	if (keycode == VK_DOWN) {
		strcpy(keystr, "DOWN");
	} else
	if (keycode == VK_TAB) {
		strcpy(keystr, "TAB");
	} else
	if (keycode == VK_BACK) {
		strcpy(keystr, "BACK");
	} else
	if (keycode == VK_DELETE) {
		strcpy(keystr, "DELETE");
	} else
	if (keycode == VK_HOME) {
		strcpy(keystr, "HOME");
	} else
	if (keycode == VK_END) {
		strcpy(keystr, "END");
	} else	
	if (keycode >= VK_F1 && keycode <= VK_F12) {
		psy_snprintf(keystr, 5, "F%d", keycode - VK_F1 + 1);
	} else {
		psy_snprintf(keystr, 5, "%c", keycode);
	}
	text[0] = '\0';	
	if (shift) {
		strcat(text, "Shift + ");		
	}
	if (ctrl) {		
		strcat(text, "Ctrl + ");
	}
	// if (keycode >= 0x30) {
		strcat(text, keystr);	
	// }
}

void ondraw(InputDefiner* self, ui_component* sender, ui_graphics* g)
{
	char text[40];	
	
	ui_settextcolor(g, 0x00FFFFFF);
	ui_setbackgroundmode(g, TRANSPARENT);
	inputdefiner_text(self, text);
	ui_textout(g, 0, 0, text, strlen(text));
}

void onkeydown(InputDefiner* self, ui_component* sender, KeyEvent* keyevent)
{
	int shift;
	int ctrl;	

	shift = GetKeyState (VK_SHIFT) < 0;
	ctrl = GetKeyState (VK_CONTROL) < 0;	

	if ((keyevent->keycode == VK_SHIFT || keyevent->keycode == VK_CONTROL)) {
		if (self->regularkey == 0) {
			self->input = encodeinput(0, shift, ctrl);
		} else {
			self->input = encodeinput(self->regularkey, shift, ctrl);
		}
	}
	if (keyevent->keycode >= 0x30) {
		self->regularkey = keyevent->keycode;	
		self->input = encodeinput(self->regularkey, shift, ctrl);
	}
	ui_component_invalidate(&self->component);
}

void onkeyup(InputDefiner* self, ui_component* sender, KeyEvent* keyevent)
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
	if (keyevent->keycode >= 0x30) {
		self->regularkey = 0;
	}
	if (inputkeycode <= 0x30) {
		self->input = encodeinput(0, shift, ctrl);
	}
	ui_component_invalidate(&self->component);
}
