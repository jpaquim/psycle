/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "uiedit.h"
#include "uiapp.h"
#ifdef PSY_USE_PLATFORM_EDIT
#include "uiimpfactory.h"
#else
#include "imps/natives/uinativeeditimp.h"
#endif
#include "../../detail/portable.h"

static void ondestroy(psy_ui_Edit*, psy_ui_Component* sender);
static void onpreferredsize(psy_ui_Edit*, psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_edit_onkeydown(psy_ui_Edit*, psy_ui_KeyboardEvent*);
static void psy_ui_edit_onfocus(psy_ui_Edit*);
static void psy_ui_edit_onfocuslost(psy_ui_Edit*);
static void psy_ui_edit_onmousehook(psy_ui_Edit*, psy_ui_App* sender,
	psy_ui_MouseEvent*);

static psy_ui_ComponentVtable vtable;
static psy_ui_ComponentVtable super_vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_Edit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		super_vtable = *(self->component.vtable);
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			onpreferredsize;
		vtable.onfocus =
			(psy_ui_fp_component_onfocus)
			psy_ui_edit_onfocus;
		vtable.onfocuslost =
			(psy_ui_fp_component_onfocuslost)
			psy_ui_edit_onfocuslost;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			psy_ui_edit_onkeydown;		
		vtable_initialized = TRUE;
	}	
	self->component.vtable = &vtable;
}

void psy_ui_edit_init(psy_ui_Edit* self, psy_ui_Component* parent)
{ 
	psy_ui_ComponentImp* imp;

#ifdef PSY_USE_PLATFORM_EDIT
	imp = psy_ui_impfactory_allocinit_editimp(
		psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
#else
	imp = psy_ui_native_editimp_allocinit(&self->component, parent, NULL)->imp;
#endif
	psy_ui_component_init_imp(psy_ui_edit_base(self), parent, NULL, imp);
	vtable_init(self);	
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_EDIT, psy_INDEX_INVALID, psy_INDEX_INVALID, psy_INDEX_INVALID);
	psy_ui_component_setstyletype_focus(&self->component, psy_ui_STYLE_EDIT_FOCUS);
	self->charnumber = 0;
	self->linenumber = 1;
	self->isinputfield = FALSE;
	self->preventedit = TRUE;
	self->component.ncpaint = TRUE;	
	imp->vtable->dev_initialized(imp);
}

void psy_ui_edit_multiline_init(psy_ui_Edit* self, psy_ui_Component* parent)
{
	psy_ui_ComponentImp* imp;

#ifdef PSY_USE_PLATFORM_EDIT
	imp = psy_ui_impfactory_allocinit_editimp_multiline(
		psy_ui_app_impfactory(psy_ui_app()), &self->component, parent);
#else
	imp = psy_ui_native_editimp_allocinit(&self->component, parent, NULL)->imp;
#endif
	psy_ui_component_init_imp(psy_ui_edit_base(self), parent, NULL, imp);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_init(&self->signal_change);
	psy_signal_init(&self->signal_accept);
	psy_signal_init(&self->signal_reject);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->charnumber = 0;
	self->linenumber = 1;
}

void ondestroy(psy_ui_Edit* self, psy_ui_Component* sender)
{
	psy_signal_dispose(&self->signal_change);
	psy_signal_dispose(&self->signal_accept);
	psy_signal_dispose(&self->signal_reject);
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_edit_onmousehook);
}

void psy_ui_edit_enableinputfield(psy_ui_Edit* self)
{
	self->isinputfield = TRUE;
	psy_signal_disconnect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_edit_onmousehook);
	psy_signal_connect(&psy_ui_app()->signal_mousehook, self,
		psy_ui_edit_onmousehook);
}

void psy_ui_edit_settext(psy_ui_Edit* self, const char* text)
{	
	psy_ui_editimp_vtable(self->component.imp)->dev_settext(
		self->component.imp->extended_imp, text);
}

 const char* psy_ui_edit_text(const psy_ui_Edit* self)
{
	static char text[256];
	
	psy_ui_editimp_vtable(self->component.imp)->dev_text(
		self->component.imp->extended_imp, text);	
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
			rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width,
				psy_ui_component_textmetric(&self->component), NULL) + 2);
			rv->height = psy_ui_value_make_px((int)(tm->tmHeight * self->linenumber));
		} else {				
			rv->width = psy_ui_value_make_px(tm->tmAveCharWidth * self->charnumber + 2);
			rv->height = psy_ui_value_make_px((int)(tm->tmHeight * self->linenumber));
		}
	}
}

void psy_ui_edit_setstyle(psy_ui_Edit* self, int style)
{
	psy_ui_editimp_vtable(self->component.imp)->dev_setstyle(
		self->component.imp->extended_imp, style);
}

void psy_ui_edit_enableedit(psy_ui_Edit* self)
{
	psy_ui_editimp_vtable(self->component.imp)->dev_enableedit(
		self->component.imp->extended_imp);
}

void psy_ui_edit_preventedit(psy_ui_Edit* self)
{
	psy_ui_editimp_vtable(self->component.imp)->dev_preventedit(
		self->component.imp->extended_imp);	
}

void psy_ui_edit_setsel(psy_ui_Edit* self, intptr_t cpmin, intptr_t cpmax)
{
	psy_ui_editimp_vtable(self->component.imp)->dev_setsel(
		self->component.imp->extended_imp, cpmin, cpmax);
}

/* signal_accept event handling */
void psy_ui_edit_onkeydown(psy_ui_Edit* self, psy_ui_KeyboardEvent* ev)
{
	assert(self);

	if (self->isinputfield) {
		switch (ev->keycode) {
		case psy_ui_KEY_ESCAPE:
			psy_ui_keyboardevent_prevent_default(ev);
			self->preventedit = TRUE;
			psy_ui_app_stopmousehook(psy_ui_app());
			psy_signal_emit(&self->signal_reject, self, 0);
			psy_ui_keyboardevent_prevent_default(ev);
			break;
		case psy_ui_KEY_RETURN:
			self->preventedit = TRUE;
			psy_ui_app_stopmousehook(psy_ui_app());
			psy_signal_emit(&self->signal_accept, self, 0);
			psy_ui_keyboardevent_prevent_default(ev);
			break;
		default:
			break;
		}
		psy_ui_keyboardevent_stop_propagation(ev);
	}
}

void psy_ui_edit_onfocus(psy_ui_Edit* self)
{	
	super_vtable.onfocus(&self->component);
	if (self->isinputfield) {
		self->preventedit = FALSE;
		psy_ui_app_startmousehook(psy_ui_app());
	}
}

void psy_ui_edit_onfocuslost(psy_ui_Edit* self)
{
	assert(self);

	super_vtable.onfocuslost(&self->component);
	if (self->isinputfield && !self->preventedit) {
		self->preventedit = TRUE;
		psy_ui_app_stopmousehook(psy_ui_app());
		psy_signal_emit(&self->signal_accept, self, 0);		
	}
}

void psy_ui_edit_onmousehook(psy_ui_Edit* self, psy_ui_App* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->isinputfield) {
		if (psy_ui_component_visible(&self->component) && !self->preventedit) {
			psy_ui_RealRectangle position;

			position = psy_ui_component_screenposition(&self->component);			
			if (!psy_ui_realrectangle_intersect(&position, ev->pt)) {
				self->preventedit = TRUE;
				psy_ui_app_stopmousehook(psy_ui_app());
				psy_signal_emit(&self->signal_accept, self, 0);								
			}
		}
	}
}


/* psy_ui_ComponentImp vtable */
static void dev_settext(psy_ui_ComponentImp* self, const char* title) { }
static void dev_text(psy_ui_ComponentImp* self, char* text) { }
static void dev_setstyle(psy_ui_ComponentImp* self, int style) { }
static void dev_enableedit(psy_ui_ComponentImp* self) { }
static void dev_preventedit(psy_ui_ComponentImp* self) { }
static void dev_setsel(psy_ui_ComponentImp* self, intptr_t cpmin, intptr_t cpmax) { }

static psy_ui_EditImpVTable edit_imp_vtable;
static bool edit_imp_vtable_initialized = FALSE;

static void edit_imp_vtable_init(void)
{
	if (!edit_imp_vtable_initialized) {
		edit_imp_vtable.dev_settext = dev_settext;
		edit_imp_vtable.dev_text = dev_text;
		edit_imp_vtable.dev_setstyle = dev_setstyle;
		edit_imp_vtable.dev_enableedit = dev_enableedit;
		edit_imp_vtable.dev_preventedit = dev_preventedit;
		edit_imp_vtable.dev_setsel = dev_setsel;
		edit_imp_vtable_initialized = TRUE;
	}
}

void psy_ui_editimp_extend(psy_ui_ComponentImp* self)
{		
	edit_imp_vtable_init();
	self->extended_vtable = &edit_imp_vtable;
}
