// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "inputdefiner.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <hashtbl.h>
#include "../../driver/eventdriver.h"

#if defined DIVERSALIS__OS__MICROSOFT

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#endif

#include "../../detail/portable.h"

static void inputdefiner_ondestroy(InputDefiner*, psy_ui_Component* sender);
static void ondraw(InputDefiner*, psy_ui_Graphics*);
static void onkeydown(InputDefiner*, psy_ui_KeyEvent*);
static void onkeyup(InputDefiner*, psy_ui_KeyEvent*);

static bool validkeycode(uintptr_t keycode);

static psy_Table keynames;
static int count = 0;

static void keynames_init(void);
static void keynames_release(void);
static void keynames_dispose(void);
static void keynames_add(uintptr_t keycode, const char* name);
static const char* keynames_at(uintptr_t keycode);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(InputDefiner* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);	
		vtable.ondraw = (psy_ui_fp_ondraw) ondraw;
		vtable.onkeydown = (psy_ui_fp_onkeydown) onkeydown;
		vtable.onkeyup = (psy_ui_fp_onkeyup) onkeyup;
		vtable_initialized = 1;
	}
}

void inputdefiner_init(InputDefiner* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&self->component.signal_destroy, self,
		inputdefiner_ondestroy);
	keynames_init();
	self->input = 0;
	self->regularkey = 0;		
}

void inputdefiner_ondestroy(InputDefiner* self, psy_ui_Component* sender)
{	
	keynames_release();
}

void inputdefiner_setinput(InputDefiner* self, unsigned int input)
{
	self->input = input;
}

void inputdefiner_text(InputDefiner* self, char* text)
{	
	uintptr_t keycode;
	bool shift;
	bool ctrl;
	
	text[0] = '\0';
	psy_audio_decodeinput(self->input, &keycode, &shift, &ctrl);
	if (shift) {
		strcat(text, "Shift + ");		
	}
	if (ctrl) {		
		strcat(text, "Ctrl + ");
	}	
	strcat(text, keynames_at(keycode));
}

void ondraw(InputDefiner* self, psy_ui_Graphics* g)
{
	char text[40];	
	
	psy_ui_settextcolor(g, psy_ui_color_make(0x00FFFFFF));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	inputdefiner_text(self, text);
	psy_ui_textout(g, 0, 0, text, strlen(text));
}

void onkeydown(InputDefiner* self, psy_ui_KeyEvent* ev)
{
	int shift;
	int ctrl;	

#if defined DIVERSALIS__OS__MICROSOFT
	shift = GetKeyState (psy_ui_KEY_SHIFT) < 0;
	ctrl = GetKeyState (psy_ui_KEY_CONTROL) < 0;	
#else
    shift = ev->shift;
    ctrl = ev->ctrl;
#endif    
	if ((ev->keycode == psy_ui_KEY_SHIFT || ev->keycode == psy_ui_KEY_CONTROL)) {
		if (self->regularkey == 0) {
			self->input = psy_audio_encodeinput(0, shift, ctrl);
		} else {
			self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl);
		}
	}
	if (validkeycode(ev->keycode)) {
		self->regularkey = ev->keycode;
		self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyevent_stoppropagation(ev);
}

void onkeyup(InputDefiner* self, psy_ui_KeyEvent* ev)
{
	bool shift;
	bool ctrl;
	uintptr_t inputkeycode;
	bool inputshift;
	bool inputctrl;

#if defined DIVERSALIS__OS__MICROSOFT
	shift = GetKeyState (psy_ui_KEY_SHIFT) < 0;
	ctrl = GetKeyState (psy_ui_KEY_CONTROL) < 0;
#else
    shift = ev->shift;
    ctrl = ev->ctrl;
#endif        
	psy_audio_decodeinput(self->input, &inputkeycode, &inputshift, &inputctrl);
	if (self->regularkey) {		
		self->input = psy_audio_encodeinput(inputkeycode, shift, ctrl);
	}
	if (validkeycode(ev->keycode)) {
		self->regularkey = 0;
	}
	if (!validkeycode(inputkeycode)) {
		self->input = psy_audio_encodeinput(0, shift, ctrl);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyevent_stoppropagation(ev);
}

bool validkeycode(uintptr_t keycode)
{
	return (keycode >= 0x30 ||
		keycode == psy_ui_KEY_LEFT ||
		keycode == psy_ui_KEY_RIGHT ||
		keycode == psy_ui_KEY_UP ||
		keycode == psy_ui_KEY_DOWN ||
		keycode == psy_ui_KEY_TAB ||
		keycode == psy_ui_KEY_BACK ||
		keycode == psy_ui_KEY_DELETE ||
		keycode == psy_ui_KEY_HOME ||
		keycode == psy_ui_KEY_END ||
		keycode == psy_ui_KEY_RETURN);
}

void keynames_init(void)
{
	if (count == 0) {
		uintptr_t key;
		
		psy_table_init(&keynames);
		keynames_add(psy_ui_KEY_LEFT,"LEFT");	
		keynames_add(psy_ui_KEY_RIGHT, "RIGHT");	
		keynames_add(psy_ui_KEY_UP,"UP");	
		keynames_add(psy_ui_KEY_DOWN, "DOWN");	
		keynames_add(psy_ui_KEY_TAB, "TAB");	
		keynames_add(psy_ui_KEY_BACK, "BACK");	
		keynames_add(psy_ui_KEY_DELETE, "DELETE");	
		keynames_add(psy_ui_KEY_HOME, "HOME");	
		keynames_add(psy_ui_KEY_END, "END");
		keynames_add(psy_ui_KEY_RETURN, "RETURN");
		for (key = psy_ui_KEY_F1; key <= psy_ui_KEY_F12; ++key) {	
			char keystr[5];
			psy_snprintf(keystr, 5, "F%d", key - psy_ui_KEY_F1 + 1);
			keynames_add(key, keystr);
		}		
		for (key = 0x30 /*psy_ui_KEY_0*/; key <= 255 /*psy_ui_KEY_Z*/; ++key) {
			char keystr[5];		
			psy_snprintf(keystr, 5, "%c", key);
			if (strlen(keystr)) {
				keynames_add(key, keystr);
			}
		}
	}
	++count;
}

void keynames_release(void)
{
	--count;
	if (count == 0) {
		keynames_dispose();
	}
}

void keynames_dispose(void)
{			
	psy_table_disposeall(&keynames, (psy_fp_disposefunc)NULL);	
}

void keynames_add(uintptr_t keycode, const char* name)
{		
	if (!psy_table_exists(&keynames, keycode)) {
		psy_table_insert(&keynames, (uintptr_t) keycode, strdup(name));
	}
}

const char* keynames_at(uintptr_t keycode)
{
	return psy_table_exists(&keynames, keycode)
		? (const char*) psy_table_at(&keynames, keycode)
		: "";
}
