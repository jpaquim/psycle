/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "realedit.h"
/* audio */
#include <songio.h>
/* std */
#include <ctype.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void realedit_on_destroyed(RealEdit*);
static void realedit_on_less(RealEdit*, psy_ui_Component* sender);
static void realedit_on_more(RealEdit*, psy_ui_Component* sender);
static void realedit_on_edit_key_down(RealEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void realedit_on_edit_key_up(RealEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void realedit_on_edit_focus_lost(RealEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(RealEdit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			realedit_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void realedit_init(RealEdit* self, psy_ui_Component* parent,
	const char* desc, realedit_real_t value,
	realedit_real_t minval, realedit_real_t maxval)
{
	psy_ui_component_init(realedit_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_set_align_expand(realedit_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_default_align(realedit_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->minval = minval;
	self->maxval = maxval;
	self->restore = value;
	psy_ui_label_init(&self->desc, realedit_base(self));
	psy_ui_label_set_text(&self->desc, desc);
	psy_ui_textarea_init_single_line(&self->edit, realedit_base(self));	
	psy_ui_textarea_set_char_number(&self->edit, 5);
	psy_ui_button_init_connect(&self->less, realedit_base(self),
		self, realedit_on_less);
	psy_ui_button_seticon(&self->less, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->more, realedit_base(self),
		self, realedit_on_more);
	psy_ui_button_seticon(&self->more, psy_ui_ICON_MORE);
	psy_signal_init(&self->signal_changed);
	realedit_set_value(self, value);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		realedit_on_edit_key_down);	
	psy_signal_connect(&self->edit.component.signal_keyup, self,
		realedit_on_edit_key_up);
	psy_signal_connect(&self->edit.component.signal_focuslost, self,
		realedit_on_edit_focus_lost);	
}

void realedit_init_connect(RealEdit* self, psy_ui_Component* parent,
	const char* desc, realedit_real_t value,
	realedit_real_t minval, realedit_real_t maxval, void* context, void* fp)
{
	realedit_init(self, parent, desc, value, minval, maxval);
	psy_signal_connect(&self->signal_changed, context, fp);
}

void realedit_on_destroyed(RealEdit* self)
{
	psy_signal_dispose(&self->signal_changed);
}

realedit_real_t realedit_value(RealEdit* self)
{
	return (realedit_real_t)
		atof(psy_ui_textarea_text(&self->edit));
}

void realedit_set_value(RealEdit* self, realedit_real_t value)
{
	char text[128];

	if (self->maxval != 0 && self->minval != 0) {
		value = psy_min(psy_max(value, self->minval), self->maxval);
	}
	psy_snprintf(text, 128, "%.2f", (float)value);
	psy_ui_textarea_set_text(&self->edit, text);
	psy_signal_emit(&self->signal_changed, self, 0);
	self->restore = value;
}

void realedit_enable(RealEdit* self)
{
	psy_ui_textarea_enableedit(&self->edit);
}

void realedit_prevent(RealEdit* self)
{
	psy_ui_textarea_preventedit(&self->edit);
}

void realedit_set_edit_char_number(RealEdit* self, int charnumber)
{
	psy_ui_textarea_set_char_number(&self->edit, charnumber);
}

void realedit_set_desc_char_number(RealEdit* self, int charnumber)
{
	psy_ui_label_set_char_number(&self->desc, charnumber);
}

void realedit_on_less(RealEdit* self, psy_ui_Component* sender)
{
	realedit_set_value(self, realedit_value(self) - 1);
}

void realedit_on_more(RealEdit* self, psy_ui_Component* sender)
{
	realedit_set_value(self, realedit_value(self) + 1);
}

void realedit_on_edit_key_down(RealEdit* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	if (isalpha(psy_ui_keyboardevent_keycode(ev)) || psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		realedit_set_value(self, self->restore);
		psy_ui_keyboardevent_prevent_default(ev);
		return;
	}
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
		realedit_real_t value;

		psy_ui_component_set_focus(&self->component);
		psy_ui_keyboardevent_prevent_default(ev);
		value = realedit_value(self);
		if (self->maxval != 0 && self->minval != 0) {
			value = psy_min(psy_max(value, self->minval), self->maxval);
		}
		realedit_set_value(self, value);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void realedit_on_edit_key_up(RealEdit* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
}

void realedit_on_edit_focus_lost(RealEdit* self , psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	realedit_real_t value;

	psy_ui_keyboardevent_prevent_default(ev);
	value = realedit_value(self);
	realedit_set_value(self, value);
	psy_signal_emit(&self->signal_changed, self, 0);
}
