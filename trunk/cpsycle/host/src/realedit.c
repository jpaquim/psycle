// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "realedit.h"
// audio
#include <songio.h>
// std
#include <ctype.h>
// platform
#include "../../detail/portable.h"

// RealEdit
// prototypes
static void realedit_ondestroy(RealEdit*, psy_ui_Component* sender);
static void realedit_onlessclicked(RealEdit*, psy_ui_Component* sender);
static void realedit_onmoreclicked(RealEdit*, psy_ui_Component* sender);
static void realedit_oneditkeydown(RealEdit*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void realedit_oneditkeyup(RealEdit*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void realedit_oneditfocuslost(RealEdit*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);

// implementation
void realedit_init(RealEdit* self, psy_ui_Component* parent,
	const char* desc, realedit_real_t value, realedit_real_t minval, realedit_real_t maxval)
{
	psy_ui_component_init(realedit_base(self), parent, NULL);
	psy_ui_component_setalignexpand(realedit_base(self), psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(realedit_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->minval = minval;
	self->maxval = maxval;
	self->restore = value;
	psy_ui_label_init(&self->desc, realedit_base(self));
	psy_ui_label_settext(&self->desc, desc);
	psy_ui_edit_init(&self->edit, realedit_base(self));
	psy_ui_edit_setcharnumber(&self->edit, 5);	
	psy_ui_button_init_connect(&self->less, realedit_base(self), NULL,
		self, realedit_onlessclicked);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->more, realedit_base(self), NULL,
		self, realedit_onmoreclicked);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
	psy_signal_init(&self->signal_changed);
	realedit_setvalue(self, value);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		realedit_oneditkeydown);	
	psy_signal_connect(&self->edit.component.signal_keyup, self,
		realedit_oneditkeyup);
	psy_signal_connect(&self->edit.component.signal_focuslost, self,
		realedit_oneditfocuslost);
	psy_signal_connect(&self->component.signal_destroy, self,
		realedit_ondestroy);
}

void realedit_init_connect(RealEdit* self, psy_ui_Component* parent,
	const char* desc, realedit_real_t value, realedit_real_t minval, realedit_real_t maxval,
	void* context, void* fp)
{
	realedit_init(self, parent, desc, value, minval, maxval);
	psy_signal_connect(&self->signal_changed, context, fp);
}

void realedit_ondestroy(RealEdit* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_changed);
}

realedit_real_t realedit_value(RealEdit* self)
{
	return (realedit_real_t)
		atof(psy_ui_edit_text(&self->edit));
}

void realedit_setvalue(RealEdit* self, realedit_real_t value)
{
	char text[128];

	if (self->maxval != 0 && self->minval != 0) {
		value = psy_min(psy_max(value, self->minval), self->maxval);
	}
	psy_snprintf(text, 128, "%.2f", (float)value);
	psy_ui_edit_settext(&self->edit, text);
	psy_signal_emit(&self->signal_changed, self, 0);
	self->restore = value;
}

void realedit_enableedit(RealEdit* self)
{
	psy_ui_edit_enableedit(&self->edit);
}

void realedit_preventedit(RealEdit* self)
{
	psy_ui_edit_preventedit(&self->edit);
}

void realedit_seteditcharnumber(RealEdit* self, int charnumber)
{
	psy_ui_edit_setcharnumber(&self->edit, charnumber);
}

void realedit_setdesccharnumber(RealEdit* self, int charnumber)
{
	psy_ui_label_setcharnumber(&self->desc, charnumber);
}

void realedit_onlessclicked(RealEdit* self, psy_ui_Component* sender)
{
	realedit_setvalue(self, realedit_value(self) - 1);
}

void realedit_onmoreclicked(RealEdit* self, psy_ui_Component* sender)
{
	realedit_setvalue(self, realedit_value(self) + 1);
}

void realedit_oneditkeydown(RealEdit* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	if (isalpha(ev->keycode) || ev->keycode == psy_ui_KEY_ESCAPE) {
		realedit_setvalue(self, self->restore);
		psy_ui_keyevent_preventdefault(ev);
		return;
	}
	if (ev->keycode == psy_ui_KEY_RETURN) {
		realedit_real_t value;

		psy_ui_component_setfocus(&self->component);
		psy_ui_keyevent_preventdefault(ev);
		value = realedit_value(self);
		if (self->maxval != 0 && self->minval != 0) {
			value = psy_min(psy_max(value, self->minval), self->maxval);
		}
		realedit_setvalue(self, value);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	psy_ui_keyevent_stoppropagation(ev);
}

void realedit_oneditkeyup(RealEdit* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	psy_ui_keyevent_stoppropagation(ev);
}

void realedit_oneditfocuslost(RealEdit* self , psy_ui_Component* sender,
	psy_ui_KeyEvent* ev)
{
	realedit_real_t value;

	psy_ui_keyevent_preventdefault(ev);
	value = realedit_value(self);
	realedit_setvalue(self, value);
	psy_signal_emit(&self->signal_changed, self, 0);
}
