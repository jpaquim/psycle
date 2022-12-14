/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "newval.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void newvalview_updateedit(NewValView*);
static void onapply(NewValView*, psy_ui_Button* sender);
static void oncancel(NewValView*, psy_ui_Button* sender);
static void oneditkeydown(NewValView*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);
static void oneditkeyup(NewValView*, psy_ui_Component* sender,
	psy_ui_KeyboardEvent*);

/* implementation */
void newvalview_init(NewValView* self, psy_ui_Component* parent,
	uintptr_t mindex, uintptr_t pindex, intptr_t vval, intptr_t vmin, intptr_t vmax, char* title,
	Workspace* workspace)
{
	char buf[32];

	psy_ui_component_init(newvalview_base(self), parent, NULL);
	self->workspace = workspace;
	self->macindex = mindex;
	self->paramindex = pindex;
	self->value = vval;
	self->v_min = vmin;
	self->v_max = vmax;	
	self->doapply = FALSE;
	self->docancel = FALSE;
	/* title */
	psy_snprintf(self->dlgtitle, 256, "%s", title);
	psy_ui_label_init(&self->title, newvalview_base(self));
	psy_ui_label_prevent_translation(&self->title);
	psy_ui_label_set_text(&self->title, self->dlgtitle);	
	psy_ui_component_set_align(psy_ui_label_base(&self->title),
		psy_ui_ALIGN_TOP);
	/* init edit */
	psy_ui_component_init_align(&self->editbar, newvalview_base(self), NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_set_align_expand(&self->editbar, psy_ui_HEXPAND);
	psy_ui_component_set_default_align(&self->editbar, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_textarea_init_single_line(&self->edit, &self->editbar);	
	psy_ui_textarea_set_char_number(&self->edit, 40);
	psy_ui_component_set_align(psy_ui_textarea_base(&self->edit),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_init_text_connect(&self->apply, &self->editbar,
		"Apply", self, onapply);
	psy_ui_component_set_align(psy_ui_button_base(&self->apply),
		psy_ui_ALIGN_LEFT);
	psy_ui_button_init_text_connect(&self->cancel, &self->editbar,
		"Cancel", self, oncancel);
	psy_ui_component_set_align(psy_ui_button_base(&self->cancel),
		psy_ui_ALIGN_LEFT);	
	psy_snprintf(buf, 32, "%d", vval);
	psy_ui_textarea_set_text(&self->edit, buf);
	psy_signal_connect(&self->edit.component.signal_keydown, self,
		oneditkeydown);
	psy_signal_connect(&self->edit.component.signal_keyup, self,
		oneditkeyup);
	//m_value.SetSel(-1, -1, false);
	/* init label */
	psy_ui_label_init(&self->text, newvalview_base(self));
	psy_ui_label_prevent_translation(&self->text);
	psy_ui_component_set_align(psy_ui_label_base(&self->text),
		psy_ui_ALIGN_TOP);
	newvalview_updateedit(self);
}

void newvalview_reset(NewValView* self, uintptr_t mindex, uintptr_t pindex, intptr_t vval, intptr_t vmin,
	intptr_t vmax, char* title)
{
	char buf[32];

	self->macindex = mindex;
	self->paramindex = pindex;
	self->value = vval;
	self->v_min = vmin;
	self->v_max = vmax;
	psy_snprintf(self->dlgtitle, 256, "%s", title);
	psy_ui_label_set_text(&self->title, self->dlgtitle);
	psy_snprintf(buf, 32, "%d", vval);
	psy_ui_textarea_set_text(&self->edit, buf);
	newvalview_updateedit(self);
}
				
void newvalview_updateedit(NewValView* self)
{
	char buffer[256];

	psy_snprintf(buffer, 256, "%s", psy_ui_textarea_text(&self->edit));	
	self->value = atoi(buffer);
	if (self->value < self->v_min) {
		self->value = self->v_min;
		psy_snprintf(buffer, 256, "Below Range. Use this HEX value: twk %.2X %.2X %.4X",
			self->paramindex, self->macindex, self->value - self->v_min);
	} else if(self->value > self->v_max) {
		self->value = self->v_max;
		psy_snprintf(buffer, 256, "Above Range. Use this HEX value: twk %.2X %.2X %.4X",
			self->paramindex, self->macindex, self->value - self->v_min);
	} else {
		psy_snprintf(buffer, 256, "Use this HEX value: twk %.2X %.2X %.4X",
			self->paramindex, self->macindex, self->value - self->v_min);
	}
	psy_ui_label_set_text(&self->text, buffer);
}

void onapply(NewValView* self, psy_ui_Button* sender)
{
	newvalview_updateedit(self);
	self->doapply = TRUE;
	psy_ui_component_hide_align(newvalview_base(self));
}

void oncancel(NewValView* self, psy_ui_Button* sender)
{
	self->docancel = TRUE;
	psy_ui_component_hide_align(newvalview_base(self));
}

void oneditkeydown(NewValView* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	if (isalpha(psy_ui_keyboardevent_keycode(ev)) ||
			psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_ESCAPE ||
		psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
		psy_ui_keyboardevent_prevent_default(ev);
		return;
	}
}

void oneditkeyup(NewValView* self, psy_ui_Component* sender,
	psy_ui_KeyboardEvent* ev)
{
	newvalview_updateedit(self);
	if (psy_ui_keyboardevent_keycode(ev) == psy_ui_KEY_RETURN) {
		self->doapply = TRUE;
		psy_ui_keyboardevent_prevent_default(ev);
		psy_ui_component_hide_align(newvalview_base(self));
	}
}
