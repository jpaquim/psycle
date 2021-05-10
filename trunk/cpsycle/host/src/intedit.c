// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "intedit.h"
// audio
#include <songio.h>
// std
#include <ctype.h>
// platform
#include "../../detail/portable.h"

// IntEdit
// prototypes
static void intedit_ondestroy(IntEdit*, psy_ui_Component* sender);
static void intedit_onlessclicked(IntEdit*, psy_ui_Component* sender);
static void intedit_onmoreclicked(IntEdit*, psy_ui_Component* sender);
static void intedit_oneditkeydown(IntEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void intedit_oneditkeyup(IntEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void intedit_oneditfocuslost(IntEdit*, psy_ui_Component* sender);

// implementation
void intedit_init(IntEdit* self, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval)
{
	psy_ui_component_init(intedit_base(self), parent, NULL);
	psy_ui_component_setalignexpand(intedit_base(self), psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(intedit_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->minval = minval;
	self->maxval = maxval;
	self->restore = value;
	psy_ui_label_init_text(&self->desc, intedit_base(self), NULL, desc);
	psy_ui_edit_init(&self->edit, intedit_base(self));
	psy_ui_edit_setcharnumber(&self->edit, 5);	
	psy_ui_button_init_connect(&self->less, intedit_base(self), NULL,
		self, intedit_onlessclicked);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->more, intedit_base(self), NULL,
		self, intedit_onmoreclicked);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
	psy_signal_init(&self->signal_changed);
	intedit_setvalue(self, value);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		intedit_oneditkeydown);	
	psy_signal_connect(&self->edit.component.signal_keyup, self,
		intedit_oneditkeyup);
	psy_signal_connect(&self->edit.component.signal_focuslost, self,
		intedit_oneditfocuslost);
	psy_signal_connect(&self->component.signal_destroy, self,
		intedit_ondestroy);
}

void intedit_init_connect(IntEdit* self, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval,
	void* context, void* fp)
{
	intedit_init(self, parent, desc, value, minval, maxval);
	psy_signal_connect(&self->signal_changed, context, fp);
}

IntEdit* intedit_alloc(void)
{
	return (IntEdit*)malloc(sizeof(IntEdit));
}

IntEdit* intedit_allocinit(psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval)
{
	IntEdit* rv;

	rv = intedit_alloc();
	if (rv) {
		intedit_init(rv, parent, desc, value, minval, maxval);
		psy_ui_component_deallocateafterdestroyed(&rv->component);
	}
	return rv;
}

void intedit_ondestroy(IntEdit* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
}

int intedit_value(IntEdit* self)
{
	return atoi(psy_ui_edit_text(&self->edit));
}

void intedit_setvalue(IntEdit* self, int value)
{
	char text[128];

	if (self->maxval != 0 && self->minval != 0) {
		value = psy_min(psy_max(value, self->minval), self->maxval);
	}
	psy_snprintf(text, 128, "%d", value);
	psy_ui_edit_settext(&self->edit, text);
	psy_signal_emit(&self->signal_changed, self, 0);
	self->restore = value;
}

void intedit_enableedit(IntEdit* self)
{
	psy_ui_edit_enableedit(&self->edit);
}

void intedit_preventedit(IntEdit* self)
{
	psy_ui_edit_preventedit(&self->edit);
}

void intedit_seteditcharnumber(IntEdit* self, int charnumber)
{
	psy_ui_edit_setcharnumber(&self->edit, charnumber);
}

void intedit_setdesccharnumber(IntEdit* self, int charnumber)
{
	psy_ui_label_setcharnumber(&self->desc, charnumber);
}

void intedit_onlessclicked(IntEdit* self, psy_ui_Component* sender)
{
	intedit_setvalue(self, intedit_value(self) - 1);
}

void intedit_onmoreclicked(IntEdit* self, psy_ui_Component* sender)
{
	intedit_setvalue(self, intedit_value(self) + 1);
}

void intedit_oneditkeydown(IntEdit* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	if (isalpha(ev->keycode) || ev->keycode == psy_ui_KEY_ESCAPE) {
		intedit_setvalue(self, self->restore);
		psy_ui_keyboardevent_prevent_default(ev);
		return;
	}
	if (ev->keycode == psy_ui_KEY_RETURN) {
		int value;

		psy_ui_component_setfocus(&self->component);
		psy_ui_keyboardevent_prevent_default(ev);
		value = intedit_value(self);
		if (self->maxval != 0 && self->minval != 0) {
			value = psy_min(psy_max(value, self->minval), self->maxval);
		}
		intedit_setvalue(self, value);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void intedit_oneditkeyup(IntEdit* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
}

void intedit_oneditfocuslost(IntEdit* self , psy_ui_Component* sender)
{
	int value;
	
	value = intedit_value(self);
	intedit_setvalue(self, value);
	psy_signal_emit(&self->signal_changed, self, 0);
}

