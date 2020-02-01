// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicheckbox.h"
#include "uiapp.h"
#include "uiimpfactory.h"
#include <string.h>

extern psy_ui_App app;

static void psy_ui_checkbox_ondestroy(psy_ui_CheckBox*, psy_ui_Component*);
static void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_CheckBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			psy_ui_checkbox_onpreferredsize;
		vtable_initialized = 1;
	}
}

void psy_ui_checkbox_init(psy_ui_CheckBox* self, psy_ui_Component* parent)
{  	
	self->imp = psy_ui_impfactory_allocinit_checkboximp(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_checkbox_base(self), parent,
		&self->imp->component_imp);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_init(&self->signal_clicked);	
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_checkbox_ondestroy);	
}

void psy_ui_checkbox_ondestroy(psy_ui_CheckBox* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_checkbox_settext(psy_ui_CheckBox* self, const char* text)
{
	self->imp->vtable->dev_settext(self->imp, text);
}

void psy_ui_checkbox_text(psy_ui_CheckBox* self, char* text)
{
	self->imp->vtable->dev_text(self->imp, text);
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	self->imp->vtable->dev_check(self->imp);
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	self->imp->vtable->dev_disablecheck(self->imp);
}

int psy_ui_checkbox_checked(psy_ui_CheckBox* self)
{
	return self->imp->vtable->dev_checked(self->imp);
}

void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	if (rv) {
		psy_ui_Size size;	
		char text[256];

		psy_ui_checkbox_text(self, text);
		size = psy_ui_component_textsize(&self->component, text);	
		rv->width = size.width + 20;
		rv->height = size.height + 4;
	}
}

// psy_ui_CheckBoxImp vtable
static void dev_settext(psy_ui_CheckBoxImp* self, const char* title) { }
static void dev_text(psy_ui_CheckBoxImp* self, char* text) { }
static void dev_check(psy_ui_CheckBoxImp* self) { }
static void dev_disablecheck(psy_ui_CheckBoxImp* self) { }
static int dev_checked(psy_ui_CheckBoxImp* self) { return 0;  }

static psy_ui_CheckBoxImpVTable checkbox_imp_vtable;
static int checkbox_imp_vtable_initialized = 0;

static void checkbox_imp_vtable_init(void)
{
	if (!checkbox_imp_vtable_initialized) {
		checkbox_imp_vtable.dev_settext = dev_settext;
		checkbox_imp_vtable.dev_text = dev_text;
		checkbox_imp_vtable.dev_check = dev_check;
		checkbox_imp_vtable.dev_disablecheck = dev_disablecheck;
		checkbox_imp_vtable.dev_checked = dev_checked;
		checkbox_imp_vtable_initialized = 1;
	}
}

void psy_ui_checkboximp_init(psy_ui_CheckBoxImp* self)
{
	checkbox_imp_vtable_init();
	self->vtable = &checkbox_imp_vtable;
}
