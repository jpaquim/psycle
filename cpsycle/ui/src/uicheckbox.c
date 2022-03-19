/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicheckbox.h"


/* prototypes */
static void psy_ui_checkbox_ondestroy(psy_ui_CheckBox*);
static void psy_ui_checkbox_onmousedown(psy_ui_CheckBox*, psy_ui_MouseEvent*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_CheckBox* self)
{
	assert(self);

	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(psy_ui_checkbox_base(self)->vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			psy_ui_checkbox_ondestroy;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_checkbox_onmousedown;
		vtable_initialized = TRUE;
	}
	psy_ui_component_setvtable(&self->component, &vtable);
}

/* implementation */
void psy_ui_checkbox_init(psy_ui_CheckBox* self, psy_ui_Component* parent)
{  	
	assert(self);

	psy_ui_component_init(psy_ui_checkbox_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_init(&self->checkmark, psy_ui_checkbox_base(self), NULL);
	psy_ui_component_setalign(&self->checkmark, psy_ui_ALIGN_LEFT);
	psy_ui_component_setstyletypes(&self->checkmark,
		psy_ui_STYLE_CHECKMARK, psy_ui_STYLE_CHECKMARK_HOVER,
		psy_ui_STYLE_CHECKMARK_SELECT, psy_INDEX_INVALID);	
	psy_ui_component_setpreferredsize(&self->checkmark,
		psy_ui_size_make_em(2.0, 1.0));
	psy_ui_label_init(&self->text, psy_ui_checkbox_base(self));
	psy_ui_component_set_margin(&self->text.component,
		psy_ui_margin_make_em(0.0, 0.0, 0.0, 1.0));
	psy_ui_label_preventwrap(&self->text);
	psy_ui_component_setalign(&self->text.component, psy_ui_ALIGN_LEFT);
	psy_ui_component_setalignexpand(&self->text.component, psy_ui_HEXPAND);	
	psy_signal_init(&self->signal_clicked);	
}

void psy_ui_checkbox_init_text(psy_ui_CheckBox* self, psy_ui_Component* parent,
	const char* text)
{
	assert(self);

	psy_ui_checkbox_init(self, parent);
	psy_ui_checkbox_settext(self, text);
}

void psy_ui_checkbox_ondestroy(psy_ui_CheckBox* self)
{	
	assert(self);

	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_checkbox_settext(psy_ui_CheckBox* self, const char* text)
{
	assert(self);

	psy_ui_label_settext(&self->text, text);	
}

const char* psy_ui_checkbox_text(psy_ui_CheckBox* self)
{	
	assert(self);

	return psy_ui_label_text(&self->text);
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	assert(self);

	if (!psy_ui_checkbox_checked(self)) {
		psy_ui_component_addstylestate(&self->checkmark,
			psy_ui_STYLESTATE_SELECT);
	}
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	assert(self);

	if (psy_ui_checkbox_checked(self)) {
		psy_ui_component_removestylestate(&self->checkmark,
			psy_ui_STYLESTATE_SELECT);
	}
}

bool psy_ui_checkbox_checked(const psy_ui_CheckBox* self)
{
	assert(self);

	return (psy_ui_componentstyle_state(&self->checkmark.style) &
		psy_ui_STYLESTATE_SELECT) == psy_ui_STYLESTATE_SELECT;
}

void psy_ui_checkbox_preventwrap(psy_ui_CheckBox* self)
{
	assert(self);

	psy_ui_label_preventwrap(&self->text);
}

void psy_ui_checkbox_enablewrap(psy_ui_CheckBox* self)
{
	assert(self);

	psy_ui_label_enablewrap(&self->text);
}

void psy_ui_checkbox_onmousedown(psy_ui_CheckBox* self, psy_ui_MouseEvent* ev)
{	
	assert(self);

	super_vtable.onmousedown(psy_ui_checkbox_base(self), ev);
	if (psy_ui_checkbox_checked(self)) {
		psy_ui_checkbox_disablecheck(self);
	} else {
		psy_ui_checkbox_check(self);
	}
	psy_signal_emit(&self->signal_clicked, &self->component, 0);
}
