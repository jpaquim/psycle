/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicheckbox.h"
/* local */
#include "uiapp.h"
#ifdef PSY_USE_PLATFORM_CHECKBOX
#include "uiimpfactory.h"
#else
#include "imps/natives/uinativecheckboximp.h"
#endif
/* platform */
#include "../../detail/portable.h"

static void psy_ui_checkbox_ondestroy(psy_ui_CheckBox*, psy_ui_Component*);
static void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void psy_ui_checkbox_onlanguagechanged(psy_ui_CheckBox*);

static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_CheckBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			psy_ui_checkbox_onpreferredsize;
		vtable.onlanguagechanged = (psy_ui_fp_component_onlanguagechanged)
			psy_ui_checkbox_onlanguagechanged;		
		vtable_initialized = TRUE;
	}
}

void psy_ui_checkbox_init(psy_ui_CheckBox* self, psy_ui_Component* parent)
{  	
	psy_ui_ComponentImp* imp;

#ifdef PSY_USE_PLATFORM_CHECKBOX	
	imp = psy_ui_impfactory_allocinit_checkboximp(
		psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
#else
	imp = psy_ui_native_checkboximp_allocinit(&self->component, parent)->imp;
#endif
	psy_ui_component_init_imp(psy_ui_checkbox_base(self), parent, imp);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->text = NULL;
	self->translation = NULL;
	self->multiline = 0;	
	psy_signal_init(&self->signal_clicked);	
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_checkbox_ondestroy);	
}

void psy_ui_checkbox_init_multiline(psy_ui_CheckBox* self, psy_ui_Component* parent)
{	
	psy_ui_ComponentImp* imp;

#ifdef PSY_USE_PLATFORM_CHECKBOX	
	imp = psy_ui_impfactory_allocinit_checkboximp_multiline(
		psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
#else
	imp = (psy_ui_ComponentImp*)psy_ui_native_checkboximp_allocinit_multiline(&self->component, parent)->imp;
#endif	
	psy_ui_component_init_imp(psy_ui_checkbox_base(self), parent, imp);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->text = NULL;
	self->translation = NULL;
	self->multiline = TRUE;	
	psy_signal_init(&self->signal_clicked);
	psy_signal_connect(&self->component.signal_destroy, self,
		psy_ui_checkbox_ondestroy);
}

void psy_ui_checkbox_init_text(psy_ui_CheckBox* self, psy_ui_Component* parent, const char* text)
{
	psy_ui_checkbox_init(self, parent);
	psy_ui_checkbox_settext(self, text);
}

void psy_ui_checkbox_ondestroy(psy_ui_CheckBox* self, psy_ui_Component* sender)
{
	free(self->text);
	free(self->translation);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_checkbox_settext(psy_ui_CheckBox* self, const char* text)
{
	psy_strreset(&self->text, text);	
	psy_strreset(&self->translation, psy_ui_translate(text));
	psy_ui_checkboximp_vtable(self->component.imp)->dev_settext(
		self->component.imp->extended_imp, self->translation);
}

const char* psy_ui_checkbox_text(psy_ui_CheckBox* self)
{	
	return self->text;
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	if (!psy_ui_checkbox_checked(self)) {
		psy_ui_checkboximp_vtable(self->component.imp)->dev_check(
			self->component.imp->extended_imp);
	}
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	if (psy_ui_checkbox_checked(self)) {
		psy_ui_checkboximp_vtable(self->component.imp)->dev_disablecheck(
			self->component.imp->extended_imp);
	}
}

int psy_ui_checkbox_checked(psy_ui_CheckBox* self)
{
	return psy_ui_checkboximp_vtable(self->component.imp)->dev_checked(
		self->component.imp->extended_imp);
}

void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	assert(rv);
	if (rv) {
		psy_ui_Size size;		
		
		if (self->multiline) {					
			*rv = self->component.imp->vtable->dev_preferredsize(
				self->component.imp, limit);			
		} else {			
			size = psy_ui_component_textsize(&self->component,
				self->translation);
			rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width,
				psy_ui_component_textmetric(&self->component), NULL) + 20);
			rv->height = size.height;
		}
	}	
}

void psy_ui_checkbox_onlanguagechanged(psy_ui_CheckBox* self)
{
	assert(self);

	psy_strreset(&self->translation, psy_ui_translate(self->text));
	psy_ui_checkboximp_vtable(self->component.imp)->dev_settext(
		self->component.imp->extended_imp, self->translation);
	psy_ui_component_invalidate(&self->component);
}


/* psy_ui_ComponentImp vtable */
static void dev_settext(psy_ui_ComponentImp* self, const char* title) { }
static void dev_text(psy_ui_ComponentImp* self, char* text) { }
static void dev_check(psy_ui_ComponentImp* self) { }
static void dev_disablecheck(psy_ui_ComponentImp* self) { }
static int dev_checked(psy_ui_ComponentImp* self) { return 0;  }

static psy_ui_CheckBoxImpVTable checkbox_imp_vtable;
static bool checkbox_imp_vtable_initialized = FALSE;

static void checkbox_imp_vtable_init(void)
{
	if (!checkbox_imp_vtable_initialized) {
		checkbox_imp_vtable.dev_settext = dev_settext;
		checkbox_imp_vtable.dev_text = dev_text;
		checkbox_imp_vtable.dev_check = dev_check;
		checkbox_imp_vtable.dev_disablecheck = dev_disablecheck;
		checkbox_imp_vtable.dev_checked = dev_checked;
		checkbox_imp_vtable_initialized = TRUE;
	}
}

void psy_ui_checkboximp_extend(psy_ui_ComponentImp* self)
{	
	checkbox_imp_vtable_init();
	self->extended_vtable = &checkbox_imp_vtable;
}
