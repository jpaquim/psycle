/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "inputdefiner.h"
#include "styles.h"
/* ui */
#include <uiapp.h>
/* container */
#include <hashtbl.h>
/* driver */
#include "../../driver/eventdriver.h"
/* platform */
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

/* InputDefinerKeyNames */

/* prototypes */
static void inputdefinerkeynames_init_keys(InputDefinerKeyNames*);

/* implementation */
void inputdefinerkeynames_init(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_init(&self->container);
	inputdefinerkeynames_init_keys(self);
}

void inputdefinerkeynames_dispose(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)NULL);
}

void inputdefinerkeynames_init_keys(InputDefinerKeyNames* self)
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

/* static InputDefinerKeyNames definition */
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

/* InputDefiner */

/* prototypes */
static void inputdefiner_on_destroyed(InputDefiner*);
static void inputdefiner_on_draw(InputDefiner*, psy_ui_Graphics*);
static void inputdefiner_on_mouse_down(InputDefiner*, psy_ui_MouseEvent*);
static void inputdefiner_on_key_down(InputDefiner*, psy_ui_KeyboardEvent*);
static void inputdefiner_on_key_up(InputDefiner*, psy_ui_KeyboardEvent*);
static void inputdefiner_on_focus(InputDefiner*);
static void inputdefiner_on_focus_in(InputDefiner*, psy_ui_Event*);
static void inputdefiner_on_focus_lost(InputDefiner*);
static void inputdefiner_on_mouse_hook(InputDefiner*, psy_ui_App* sender,
	psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(InputDefiner* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			inputdefiner_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			inputdefiner_on_draw;
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			inputdefiner_on_mouse_down;
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			inputdefiner_on_key_down;
		vtable.onkeyup = 
			(psy_ui_fp_component_on_key_event)
			inputdefiner_on_key_up;
		vtable.on_focus =
			(psy_ui_fp_component_event)
			inputdefiner_on_focus;
		vtable.on_focusin =
			(psy_ui_fp_component_focusin)
			inputdefiner_on_focus_in;
		vtable.on_focuslost =
			(psy_ui_fp_component_event)
			inputdefiner_on_focus_lost;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void inputdefiner_init(InputDefiner* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_style_type_focus(&self->component,
		STYLE_INPUTDEFINER_FOCUS);		
	psy_signal_init(&self->signal_accept);
	keynames_init();
	self->input = 0;
	self->regularkey = 0;
	self->preventhook = TRUE;
	psy_signal_connect(&psy_ui_app()->signal_mousehook, self,
		inputdefiner_on_mouse_hook);	
}

void inputdefiner_on_destroyed(InputDefiner* self)
{	
	assert(self);

	psy_signal_dispose(&self->signal_accept);	
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		inputdefiner_on_mouse_hook);
	keynames_release();
}

InputDefiner* inputdefiner_alloc(void)
{
	return (InputDefiner*)malloc(sizeof(InputDefiner));
}

InputDefiner* inputdefiner_allocinit(psy_ui_Component* parent)
{
	InputDefiner* rv;

	rv = inputdefiner_alloc();
	if (rv) {
		inputdefiner_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

void inputdefiner_set_input(InputDefiner* self, uint32_t input)
{
	assert(self);

	self->input = input;
}

void inputdefiner_text(InputDefiner* self, char* text)
{	
	assert(self);

	inputdefiner_input_to_text(self->input, text);	
}

void inputdefiner_input_to_text(uint32_t input, char* text)
{
	uint32_t keycode;
	bool shift;
	bool ctrl;
	bool alt;
	bool up;

	text[0] = '\0';
	if (input == 0) {
		return;
	}
	psy_audio_decodeinput(input, &keycode, &shift, &ctrl, &alt, &up);
	if (alt) {
		strcat(text, "Alt + ");
	}
	if (shift) {
		strcat(text, "Shift + ");
	}
	if (ctrl) {
		strcat(text, "Ctrl + ");
	}
	if (up) {
		strcat(text, "UP + ");
	}
	keynames_init();
	strcat(text, inputdefinerkeynames_at(&keynames, keycode));	
	keynames_release();
}

void inputdefiner_on_draw(InputDefiner* self, psy_ui_Graphics* g)
{
	char text[40];

	assert(self);
	
	if (psy_ui_component_has_focus(&self->component)) {
		self = self;
	}
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	inputdefiner_text(self, text);
	if (psy_strlen(text) > 0) {
		psy_ui_textout(g, psy_ui_realpoint_zero(), text, psy_strlen(text));
	}
}

void inputdefiner_on_mouse_down(InputDefiner* self, psy_ui_MouseEvent* ev)
{
	psy_ui_mouseevent_stop_propagation(ev);
	psy_ui_component_set_focus(&self->component);
	psy_ui_component_invalidate(&self->component);	
}

void inputdefiner_on_key_down(InputDefiner* self, psy_ui_KeyboardEvent* ev)
{
	bool alt;
	bool shift;
	bool ctrl;
	bool up;

	assert(self);

	shift = psy_ui_keyboardevent_shiftkey(ev);
	ctrl = psy_ui_keyboardevent_ctrlkey(ev);
	alt = psy_ui_keyboardevent_altkey(ev);
	up = 0;
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_SHIFT ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_CONTROL ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_MENU) {
		if (self->regularkey == 0) {
			self->input = psy_audio_encodeinput(0, shift, ctrl, alt, up);
		} else {
			self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl, alt, up);
		}
	}
	if (validkeycode(psy_ui_keyboardevent_keycode(ev))) {
		self->regularkey = psy_ui_keyboardevent_keycode(ev);
		self->input = psy_audio_encodeinput(self->regularkey, shift, ctrl, alt, up);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);
}

void inputdefiner_on_key_up(InputDefiner* self, psy_ui_KeyboardEvent* ev)
{
	bool alt;
	bool shift;
	bool ctrl;
	bool up;
	uint32_t inputkeycode;
	bool inputshift;
	bool inputctrl;
	bool inputalt;
	bool inputup;

	assert(self);

	alt = psy_ui_keyboardevent_altkey(ev);
    shift = psy_ui_keyboardevent_shiftkey(ev);
    ctrl = psy_ui_keyboardevent_ctrlkey(ev);
	up = 0;
	psy_audio_decodeinput(self->input, &inputkeycode, &inputshift, &inputctrl, &inputalt, &inputup);
	if (self->regularkey) {
		if (inputalt) {
			self->input = psy_audio_encodeinput(inputkeycode, inputshift, inputctrl, inputalt, inputup);
		} else {
			self->input = psy_audio_encodeinput(inputkeycode, shift, ctrl, alt, inputup);
		}
	}
	if (validkeycode(psy_ui_keyboardevent_keycode(ev))) {
		self->regularkey = 0;
	}
	if (!validkeycode(inputkeycode)) {
		self->input = psy_audio_encodeinput(0, shift, ctrl, alt, up);
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);
}

void inputdefiner_on_focus(InputDefiner* self)
{	
	assert(self);

	super_vtable.on_focus(&self->component);
	self->preventhook = FALSE;
	psy_ui_app_startmousehook(psy_ui_app());
	psy_ui_component_invalidate(&self->component);
}

void inputdefiner_on_focus_in(InputDefiner* self, psy_ui_Event* ev)
{
	psy_ui_event_stop_propagation(ev);
}

void inputdefiner_on_focus_lost(InputDefiner* self)
{
	assert(self);

	super_vtable.on_focuslost(&self->component);
	self->preventhook = TRUE;
	psy_ui_app_stopmousehook(psy_ui_app());
	psy_signal_emit(&self->signal_accept, self, 0);
}

void inputdefiner_on_mouse_hook(InputDefiner* self, psy_ui_App* sender,
	psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_component_visible(&self->component) && !self->preventhook) {
		psy_ui_RealRectangle position;

		position = psy_ui_component_screenposition(&self->component);
		if (!psy_ui_realrectangle_intersect(&position,
				psy_ui_mouseevent_pt(ev))) {
			self->preventhook = TRUE;
			psy_signal_emit(&self->signal_accept, self, 0);
			psy_ui_app_stopmousehook(psy_ui_app());
		}
	}
}
