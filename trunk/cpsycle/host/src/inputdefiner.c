// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// host
#include "inputdefiner.h"
#include "styles.h"
// ui
#include <uiapp.h>
// container
#include <hashtbl.h>
// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// driver
#include "../../driver/eventdriver.h"
// platform
#include "../../detail/os.h"

#if defined DIVERSALIS__OS__MICROSOFT

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#endif

#include "../../detail/portable.h"

static psy_Table keynames;
static int count = 0;

static bool validkeycode(uintptr_t keycode);
static void keynames_init(void);
static void keynames_release(void);
static void keynames_dispose(void);
static void keynames_add(uintptr_t keycode, const char* name);
static const char* keynames_at(uintptr_t keycode);

// InputDefiner
// prototypes
static void inputdefiner_ondestroy(InputDefiner*);
static void inputdefiner_ondraw(InputDefiner*, psy_ui_Graphics*);
static void inputdefiner_onkeydown(InputDefiner*, psy_ui_KeyEvent*);
static void inputdefiner_onkeyup(InputDefiner*, psy_ui_KeyEvent*);
static void inputdefiner_onfocus(InputDefiner*);
static void inputdefiner_onfocuslost(InputDefiner*);
static void inputdefiner_onmousehook(InputDefiner*, psy_ui_App* sender,
	psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(InputDefiner* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			inputdefiner_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			inputdefiner_ondraw;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			inputdefiner_onkeydown;
		vtable.onkeyup = 
			(psy_ui_fp_component_onkeyevent)
			inputdefiner_onkeyup;
		vtable.onfocus =
			(psy_ui_fp_component_onfocus)
			inputdefiner_onfocus;
		vtable.onfocuslost =
			(psy_ui_fp_component_onfocuslost)
			inputdefiner_onfocuslost;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
// implementation
void inputdefiner_init(InputDefiner* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setstyletypes(&self->component,
		psy_INDEX_INVALID, psy_INDEX_INVALID, STYLE_INPUTDEFINER_SELECT,
		psy_INDEX_INVALID);
	psy_signal_init(&self->signal_accept);
	keynames_init();
	self->input = 0;
	self->regularkey = 0;
	self->preventhook = TRUE;
	psy_signal_connect(&psy_ui_app()->signal_mousehook, self,
		inputdefiner_onmousehook);
}

void inputdefiner_ondestroy(InputDefiner* self)
{	
	psy_signal_dispose(&self->signal_accept);
	keynames_release();
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		inputdefiner_onmousehook);
}

void inputdefiner_setinput(InputDefiner* self, uint32_t input)
{
	self->input = input;
}

void inputdefiner_text(InputDefiner* self, char* text)
{	
	inputdefiner_inputtotext(self->input, text);	
}

void inputdefiner_inputtotext(uint32_t input, char* text)
{
	uint32_t keycode;
	bool shift;
	bool ctrl;
	bool alt;

	text[0] = '\0';
	if (input == 0) {
		return;
	}
	psy_audio_decodeinput(input, &keycode, &shift, &ctrl, &alt);
	if (alt) {
		strcat(text, "Alt + ");
	}
	if (shift) {
		strcat(text, "Shift + ");
	}
	if (ctrl) {
		strcat(text, "Ctrl + ");
	}
	strcat(text, keynames_at(keycode));
}

void inputdefiner_ondraw(InputDefiner* self, psy_ui_Graphics* g)
{
	char text[40];	
	
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	inputdefiner_text(self, text);
	if (psy_strlen(text)) {
		psy_ui_textout(g, 0, 0, text, strlen(text));
	}
}

void inputdefiner_onkeydown(InputDefiner* self, psy_ui_KeyEvent* ev)
{
	bool shift;
	bool ctrl;	

#if defined DIVERSALIS__OS__MICROSOFT
	shift = GetKeyState (psy_ui_KEY_SHIFT) < 0;
	ctrl = GetKeyState (psy_ui_KEY_CONTROL) < 0;	
#else
    shift = ev->shift;
    ctrl = ev->ctrl;
#endif    
	if ((ev->keycode == psy_ui_KEY_SHIFT || ev->keycode == psy_ui_KEY_CONTROL)) {
		if (self->regularkey == 0) {
			self->input = psy_audio_encodeinput(0, shift, ctrl, 0);
		} else {
			self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl, 0);
		}
	}
	if (validkeycode(ev->keycode)) {
		self->regularkey = ev->keycode;
		self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl, 0);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyevent_stoppropagation(ev);
}

void inputdefiner_onkeyup(InputDefiner* self, psy_ui_KeyEvent* ev)
{
	bool shift;
	bool ctrl;
	uint32_t inputkeycode;
	bool inputshift;
	bool inputctrl;
	bool inputalt;

#if defined DIVERSALIS__OS__MICROSOFT
	shift = GetKeyState (psy_ui_KEY_SHIFT) < 0;
	ctrl = GetKeyState (psy_ui_KEY_CONTROL) < 0;
#else
    shift = ev->shift;
    ctrl = ev->ctrl;
#endif        
	psy_audio_decodeinput(self->input, &inputkeycode, &inputshift, &inputctrl, &inputalt);
	if (self->regularkey) {		
		self->input = psy_audio_encodeinput(inputkeycode, shift, ctrl, 0);
	}
	if (validkeycode(ev->keycode)) {
		self->regularkey = 0;
	}
	if (!validkeycode(inputkeycode)) {
		self->input = psy_audio_encodeinput(0, shift, ctrl, 0);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyevent_stoppropagation(ev);
}

void inputdefiner_onfocus(InputDefiner* self)
{
	psy_ui_component_setstylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);
	self->preventhook = FALSE;
	psy_ui_app_startmousehook(psy_ui_app());
}

void inputdefiner_onfocuslost(InputDefiner* self)
{
	psy_ui_component_setstylestate(&self->component,
		psy_INDEX_INVALID);
	self->preventhook = TRUE;
	psy_ui_app_stopmousehook(psy_ui_app());
	psy_signal_emit(&self->signal_accept, self, 0);
}

void inputdefiner_onmousehook(InputDefiner* self, psy_ui_App* sender,
	psy_ui_MouseEvent* ev)
{
	if (psy_ui_component_visible(&self->component) && !self->preventhook) {
		psy_ui_RealRectangle position;

		position = psy_ui_component_screenposition(&self->component);
		if (!psy_ui_realrectangle_intersect(&position, ev->pt)) {
			self->preventhook = TRUE;
			psy_signal_emit(&self->signal_accept, self, 0);
			psy_ui_app_stopmousehook(psy_ui_app());
		}
	}
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
		keycode == psy_ui_KEY_RETURN ||
		keycode == psy_ui_KEY_PRIOR ||
		keycode == psy_ui_KEY_NEXT);
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
		keynames_add(psy_ui_KEY_PRIOR, "PGUP");
		keynames_add(psy_ui_KEY_NEXT, "PGDOWN");		
		keynames_add(psy_ui_KEY_TAB, "TAB");	
		keynames_add(psy_ui_KEY_BACK, "BACKSPACE");	
		keynames_add(psy_ui_KEY_DELETE, "DELETE");	
		keynames_add(psy_ui_KEY_HOME, "HOME");	
		keynames_add(psy_ui_KEY_END, "END");
		keynames_add(psy_ui_KEY_RETURN, "RETURN");
		keynames_add(psy_ui_KEY_INSERT, "INSERT");		
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
