/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "intedit.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <songio.h>
/* std */
#include <ctype.h>
/* platform */
#include "../../detail/portable.h"

/* IntEdit */

/* prototypes */
static void intedit_on_destroyed(IntEdit*);
static void intedit_on_less(IntEdit*, psy_ui_Component* sender);
static void intedit_on_more(IntEdit*, psy_ui_Component* sender);
static void intedit_on_edit_key_down(IntEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void intedit_on_edit_key_up(IntEdit*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void intedit_on_edit_accept(IntEdit*, psy_ui_Component* sender);

/* vtable */
static psy_ui_ComponentVtable intedit_vtable;
static bool intedit_vtable_initialized = FALSE;

static void intedit_vtable_init(IntEdit* self)
{
	if (!intedit_vtable_initialized) {
		intedit_vtable = *(self->component.vtable);
		intedit_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			intedit_on_destroyed;		
		intedit_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &intedit_vtable);
}

/* implementation */
void intedit_init(IntEdit* self, psy_ui_Component* parent,
	const char* desc, int value, int minval,
	int maxval)
{	
	psy_ui_component_init(intedit_base(self), parent, NULL);
	intedit_vtable_init(self);
	psy_ui_component_set_align_expand(intedit_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_default_align(intedit_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->minval = minval;
	self->maxval = maxval;
	self->restore = value;
	psy_ui_label_init_text(&self->desc, intedit_base(self), desc);
	psy_ui_textarea_init_single_line(&self->input, intedit_base(self));	
	psy_ui_textarea_set_char_number(&self->input, 5);	
	psy_ui_textarea_enable_input_field(&self->input);	
	psy_ui_button_init_connect(&self->less, intedit_base(self),
		self, intedit_on_less);
	psy_ui_button_set_icon(&self->less, psy_ui_ICON_LESS);
	psy_ui_button_init_connect(&self->more, intedit_base(self),
		self, intedit_on_more);
	psy_ui_button_set_icon(&self->more, psy_ui_ICON_MORE);
	psy_signal_init(&self->signal_changed);
	intedit_set_value(self, value);
	psy_signal_connect(&self->input.component.signal_keydown, self,
		intedit_on_edit_key_down);	
	psy_signal_connect(&self->input.component.signal_keyup, self,
		intedit_on_edit_key_up);
	psy_signal_connect(&self->input.signal_accept, self,
		intedit_on_edit_accept);	
}

void intedit_init_connect(IntEdit* self, psy_ui_Component* parent,
	const char* desc, int value, int minval,
	int maxval, void* context, void* fp)
{
	intedit_init(self, parent, desc, value, minval, maxval);
	psy_signal_connect(&self->signal_changed, context, fp);
}

void intedit_on_destroyed(IntEdit* self)
{
	psy_signal_dispose(&self->signal_changed);
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
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

int intedit_value(IntEdit* self)
{
	return atoi(psy_ui_textarea_text(&self->input));
}

void intedit_set_value(IntEdit* self, int value)
{
	char text[128];

	if (self->maxval != 0 && self->minval != 0) {
		value = psy_min(psy_max(value, self->minval), self->maxval);
	}
	psy_snprintf(text, 128, "%d", value);
	psy_ui_textarea_set_text(&self->input, text);
	psy_signal_emit(&self->signal_changed, self, 0);
	self->restore = value;
}

void intedit_enable_edit(IntEdit* self)
{
	psy_ui_textarea_enable(&self->input);
}

void intedit_prevent_edit(IntEdit* self)
{
	psy_ui_textarea_prevent(&self->input);
}

void intedit_set_edit_char_number(IntEdit* self, int charnumber)
{
	psy_ui_textarea_set_char_number(&self->input, charnumber);
}

void intedit_set_desc_char_number(IntEdit* self, int charnumber)
{
	psy_ui_label_set_char_number(&self->desc, charnumber);
}

void intedit_on_less(IntEdit* self, psy_ui_Component* sender)
{
	intedit_set_value(self, intedit_value(self) - 1);
}

void intedit_on_more(IntEdit* self, psy_ui_Component* sender)
{
	intedit_set_value(self, intedit_value(self) + 1);
}

void intedit_on_edit_key_down(IntEdit* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	if (isalpha(psy_ui_keyboardevent_keycode(ev)) ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE) {
		intedit_set_value(self, self->restore);
		psy_ui_keyboardevent_prevent_default(ev);
		return;
	}
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
		int value;

		psy_ui_component_set_focus(&self->component);
		psy_ui_keyboardevent_prevent_default(ev);
		value = intedit_value(self);
		if (self->maxval != 0 && self->minval != 0) {
			value = psy_min(psy_max(value, self->minval), self->maxval);
		}
		intedit_set_value(self, value);
		psy_signal_emit(&self->signal_changed, self, 0);
	}
	psy_ui_keyboardevent_stop_propagation(ev);
}

void intedit_on_edit_key_up(IntEdit* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	psy_ui_keyboardevent_stop_propagation(ev);
}

void intedit_on_edit_accept(IntEdit* self , psy_ui_Component* sender)
{
	int value;
	
	value = intedit_value(self);
	intedit_set_value(self, value);
	psy_signal_emit(&self->signal_changed, self, 0);
}
