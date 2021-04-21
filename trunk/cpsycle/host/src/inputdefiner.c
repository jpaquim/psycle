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
// driver
#include "../../driver/eventdriver.h"
// platform
#include "../../detail/portable.h"

static bool validkeycode(uintptr_t keycode)
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

// InputDefinerKeyNames
// prototypes
static void inputdefinerkeynames_initkeys(InputDefinerKeyNames*);
// implementation
void inputdefinerkeynames_init(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_init(&self->container);
	inputdefinerkeynames_initkeys(self);
}

void inputdefinerkeynames_dispose(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_disposeall(&self->container, (psy_fp_disposefunc)NULL);
}

void inputdefinerkeynames_initkeys(InputDefinerKeyNames* self)
{
	uintptr_t key;

	assert(self);

	psy_table_init(&self->container);
	inputdefinerkeynames_add(self, psy_ui_KEY_LEFT, "LEFT");
	inputdefinerkeynames_add(self, psy_ui_KEY_RIGHT, "RIGHT");
	inputdefinerkeynames_add(self, psy_ui_KEY_UP, "UP");
	inputdefinerkeynames_add(self, psy_ui_KEY_DOWN, "DOWN");
	inputdefinerkeynames_add(self, psy_ui_KEY_PRIOR, "PGUP");
	inputdefinerkeynames_add(self, psy_ui_KEY_NEXT, "PGDOWN");
	inputdefinerkeynames_add(self, psy_ui_KEY_TAB, "TAB");
	inputdefinerkeynames_add(self, psy_ui_KEY_BACK, "BACKSPACE");
	inputdefinerkeynames_add(self, psy_ui_KEY_DELETE, "DELETE");
	inputdefinerkeynames_add(self, psy_ui_KEY_HOME, "HOME");
	inputdefinerkeynames_add(self, psy_ui_KEY_END, "END");
	inputdefinerkeynames_add(self, psy_ui_KEY_RETURN, "RETURN");
	inputdefinerkeynames_add(self, psy_ui_KEY_INSERT, "INSERT");
	for (key = psy_ui_KEY_F1; key <= psy_ui_KEY_F12; ++key) {
		char keystr[5];
		psy_snprintf(keystr, 5, "F%d", key - psy_ui_KEY_F1 + 1);
		inputdefinerkeynames_add(self, key, keystr);
	}
	for (key = 0x30 /*psy_ui_KEY_0*/; key <= 255 /*psy_ui_KEY_Z*/; ++key) {
		char keystr[5];
		psy_snprintf(keystr, 5, "%c", key);
		if (strlen(keystr)) {
			inputdefinerkeynames_add(self, key, keystr);
		}
	}
}

void inputdefinerkeynames_add(InputDefinerKeyNames* self,
	uintptr_t keycode, const char* name)
{
	assert(self);

	if (!psy_table_exists(&self->container, keycode)) {
		psy_table_insert(&self->container, (uintptr_t)keycode,
			psy_strdup(name));
	}
}

const char* inputdefinerkeynames_at(const InputDefinerKeyNames* self,
	uintptr_t keycode)
{
	assert(self);

	return (psy_table_exists(&self->container, keycode))
		? (const char*)psy_table_at_const(&self->container, keycode)
		: "";
}

// static InputDefinerKeyNames definition
static int refcount = 0;
static InputDefinerKeyNames keynames;

static void keynames_init(void);
static void keynames_release(void);

void keynames_init(void)
{	
	if (refcount == 0) {
		inputdefinerkeynames_init(&keynames);
	}
	++refcount;
}

void keynames_release(void)
{
	--refcount;
	if (refcount == 0) {
		inputdefinerkeynames_dispose(&keynames);
	}
}

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
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(InputDefiner* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
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
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setstyletype_focus(&self->component,
		STYLE_INPUTDEFINER_FOCUS);		
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
	assert(self);

	psy_signal_dispose(&self->signal_accept);	
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		inputdefiner_onmousehook);
	keynames_release();
}

void inputdefiner_setinput(InputDefiner* self, uint32_t input)
{
	assert(self);

	self->input = input;
}

void inputdefiner_text(InputDefiner* self, char* text)
{	
	assert(self);

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
	keynames_init();
	strcat(text, inputdefinerkeynames_at(&keynames, keycode));	
	keynames_release();
}

void inputdefiner_ondraw(InputDefiner* self, psy_ui_Graphics* g)
{
	char text[40];

	assert(self);
	
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	inputdefiner_text(self, text);
	if (psy_strlen(text) > 0) {
		psy_ui_textout(g, 0, 0, text, psy_strlen(text));
	}
}

void inputdefiner_onkeydown(InputDefiner* self, psy_ui_KeyEvent* ev)
{
	bool alt;
	bool shift;
	bool ctrl;

	assert(self);

	shift = ev->shift;
	ctrl = ev->ctrl;
	alt = ev->alt;
	if (ev->keycode == psy_ui_KEY_SHIFT || ev->keycode == psy_ui_KEY_CONTROL || 
			ev->keycode == psy_ui_KEY_MENU) {
		if (self->regularkey == 0) {
			self->input = psy_audio_encodeinput(0, shift, ctrl, alt);
		} else {
			self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl, alt);
		}
	}
	if (validkeycode(ev->keycode)) {
		self->regularkey = ev->keycode;
		self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl, alt);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyevent_stoppropagation(ev);
}

void inputdefiner_onkeyup(InputDefiner* self, psy_ui_KeyEvent* ev)
{
	bool alt;
	bool shift;
	bool ctrl;
	uint32_t inputkeycode;
	bool inputshift;
	bool inputctrl;
	bool inputalt;

	assert(self);

	alt = ev->alt;
    shift = ev->shift;
    ctrl = ev->ctrl;
	psy_audio_decodeinput(self->input, &inputkeycode, &inputshift, &inputctrl, &inputalt);	
	if (self->regularkey) {
		if (inputalt) {
			self->input = psy_audio_encodeinput(inputkeycode, inputshift, inputctrl, inputalt);
		} else {
			self->input = psy_audio_encodeinput(inputkeycode, shift, ctrl, alt);
		}
	}
	if (validkeycode(ev->keycode)) {
		self->regularkey = 0;
	}
	if (!validkeycode(inputkeycode)) {
		self->input = psy_audio_encodeinput(0, shift, ctrl, alt);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyevent_stoppropagation(ev);
}

void inputdefiner_onfocus(InputDefiner* self)
{	
	assert(self);

	super_vtable.onfocus(&self->component);
	self->preventhook = FALSE;
	psy_ui_app_startmousehook(psy_ui_app());
}

void inputdefiner_onfocuslost(InputDefiner* self)
{
	assert(self);

	super_vtable.onfocuslost(&self->component);
	self->preventhook = TRUE;
	psy_ui_app_stopmousehook(psy_ui_app());
	psy_signal_emit(&self->signal_accept, self, 0);
}

void inputdefiner_onmousehook(InputDefiner* self, psy_ui_App* sender,
	psy_ui_MouseEvent* ev)
{
	assert(self);

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
