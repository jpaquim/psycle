/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "inputdefiner.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* container */
#include <hashtbl.h>
/* driver */
#include "../../driver/eventdriver.h"
/* platform */
#include "../../detail/portable.h"


static bool valid_key_code(uintptr_t keycode)
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
static void inputdefinerkeynames_add_keys(InputDefinerKeyNames*);

/* implementation */
void inputdefinerkeynames_init(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_init(&self->container);
	inputdefinerkeynames_add_keys(self);
}

void inputdefinerkeynames_dispose(InputDefinerKeyNames* self)
{
	assert(self);

	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)NULL);
}

void inputdefinerkeynames_add_keys(InputDefinerKeyNames* self)
{
	uintptr_t key;

	assert(self);
	
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
static void inputdefiner_on_key_down(InputDefiner*, psy_ui_KeyboardEvent*);
static void inputdefiner_on_key_up(InputDefiner*, psy_ui_KeyboardEvent*);
static void inputdefiner_on_focus_lost(InputDefiner*);

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
		vtable.on_key_down =
			(psy_ui_fp_component_on_key_event)
			inputdefiner_on_key_down;
		vtable.onkeyup = 
			(psy_ui_fp_component_on_key_event)
			inputdefiner_on_key_up;		
		vtable.on_focuslost =
			(psy_ui_fp_component_event)
			inputdefiner_on_focus_lost;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(inputdefiner_base(self), &vtable);
}

/* implementation */
void inputdefiner_init(InputDefiner* self, psy_ui_Component* parent)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_style_type_focus(inputdefiner_base(self),
		STYLE_INPUTDEFINER_FOCUS);
	psy_ui_component_set_tab_index(inputdefiner_base(self), 0);
	psy_signal_init(&self->signal_accept);
	keynames_init();
	self->input = 0;
	self->regular_key = 0;
	self->changed = FALSE;
}

void inputdefiner_on_destroyed(InputDefiner* self)
{	
	assert(self);

	psy_signal_dispose(&self->signal_accept);	
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
	self->changed = FALSE;
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
	char text[64];

	assert(self);
			
	inputdefiner_text(self, text);
	if (psy_strlen(text) > 0) {
		psy_ui_textout(g, psy_ui_realpoint_zero(), text, psy_strlen(text));
	}
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
	self->old_input = self->input;
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_SHIFT ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_CONTROL ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_MENU) {
		if (self->regular_key == 0) {
			self->input = psy_audio_encodeinput(0, shift, ctrl, alt, up);
		} else {
			self->input = psy_audio_encodeinput(self->regular_key, shift, ctrl, alt, up);
		}
	}
	if (valid_key_code(psy_ui_keyboardevent_keycode(ev))) {
		self->regular_key = psy_ui_keyboardevent_keycode(ev);
		self->input = psy_audio_encodeinput(self->regular_key, shift, ctrl, alt, up);
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
	if (self->regular_key) {
		if (inputalt) {
			self->input = psy_audio_encodeinput(inputkeycode, inputshift, inputctrl, inputalt, inputup);
		} else {
			self->input = psy_audio_encodeinput(inputkeycode, shift, ctrl, alt, inputup);
		}
	}
	if (valid_key_code(psy_ui_keyboardevent_keycode(ev))) {
		self->regular_key = 0;
	}
	if (!valid_key_code(inputkeycode)) {
		self->input = psy_audio_encodeinput(0, shift, ctrl, alt, up);
	}
	if (self->input != self->old_input) {
		self->changed = TRUE;
	}
	psy_ui_component_invalidate(&self->component);
	psy_ui_keyboardevent_stop_propagation(ev);
}

void inputdefiner_on_focus_lost(InputDefiner* self)
{
	assert(self);

	super_vtable.on_focuslost(&self->component);
	if (self->changed) {
		self->changed = FALSE;
		psy_signal_emit(&self->signal_accept, self, 0);
	}
}
