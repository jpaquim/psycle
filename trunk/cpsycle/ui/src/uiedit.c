// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiedit.h"
#include "uiapp.h"
#include "uiimpfactory.h"

extern psy_ui_App app;

static void ondestroy(psy_ui_Edit*, psy_ui_Component* sender);
static void onpreferredsize(psy_ui_Edit*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Edit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize) onpreferredsize;
	}	vtable_initialized = 1;
}

void psy_ui_edit_init(psy_ui_Edit* self, psy_ui_Component* parent)
{ 
	self->imp = psy_ui_impfactory_allocinit_editimp(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_edit_base(self), parent,
		&self->imp->component_imp);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_init(&self->signal_change);		
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->charnumber = 0;
	self->linenumber = 1;	
}

void psy_ui_edit_multiline_init(psy_ui_Edit* self, psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_editimp_multiline(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_edit_base(self), parent,
		&self->imp->component_imp);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_init(&self->signal_change);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->charnumber = 0;
	self->linenumber = 1;
}

void ondestroy(psy_ui_Edit* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_change);
}

void psy_ui_edit_settext(psy_ui_Edit* self, const char* text)
{
	self->imp->vtable->dev_settext(self->imp, text);
}

 const char* psy_ui_edit_text(psy_ui_Edit* self)
{
	static char text[256];

	self->imp->vtable->dev_text(self->imp, text);
	return text;
}

void psy_ui_edit_setcharnumber(psy_ui_Edit* self, int number)
{
	self->charnumber = number;
}

void psy_ui_edit_setlinenumber(psy_ui_Edit* self, int number)
{
	self->linenumber = number;
}

void onpreferredsize(psy_ui_Edit* self, psy_ui_Size* limit, psy_ui_Size* rv)
{			
	if (rv) {		
		const psy_ui_TextMetric* tm;
		
		tm = psy_ui_component_textmetric(&self->component);			
		if (self->charnumber == 0) {
			psy_ui_Size size;
			
			size = psy_ui_component_textsize(&self->component,
				psy_ui_edit_text(self));
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width,
				psy_ui_component_textmetric(&self->component)) + 2);
			rv->height = psy_ui_value_makepx((int)(tm->tmHeight * self->linenumber));
		} else {				
			rv->width = psy_ui_value_makepx(tm->tmAveCharWidth * self->charnumber + 2);
			rv->height = psy_ui_value_makepx((int)(tm->tmHeight * self->linenumber));
		}
	}
}

void psy_ui_edit_setstyle(psy_ui_Edit* self, int style)
{
	self->imp->vtable->dev_setstyle(self->imp, style);
}

void psy_ui_edit_enableedit(psy_ui_Edit* self)
{
	self->imp->vtable->dev_enableedit(self->imp);
}

void psy_ui_edit_preventedit(psy_ui_Edit* self)
{
	self->imp->vtable->dev_preventedit(self->imp);
}

// psy_ui_EditImp vtable
static void dev_settext(psy_ui_EditImp* self, const char* title) { }
static void dev_text(psy_ui_EditImp* self, char* text) { }
static void dev_setstyle(psy_ui_EditImp* self, int style) { }
static void dev_enableedit(psy_ui_EditImp* self) { }
static void dev_preventedit(psy_ui_EditImp* self) { }

static psy_ui_EditImpVTable edit_imp_vtable;
static int edit_imp_vtable_initialized = 0;

static void edit_imp_vtable_init(void)
{
	if (!edit_imp_vtable_initialized) {
		edit_imp_vtable.dev_settext = dev_settext;
		edit_imp_vtable.dev_text = dev_text;
		edit_imp_vtable.dev_setstyle = dev_setstyle;
		edit_imp_vtable.dev_enableedit = dev_enableedit;
		edit_imp_vtable.dev_preventedit = dev_preventedit;
		edit_imp_vtable_initialized = 1;
	}
}

void psy_ui_editimp_init(psy_ui_EditImp* self)
{
	edit_imp_vtable_init();
	self->vtable = &edit_imp_vtable;
}
