// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

// local
#include "uiapp.h"
#include "uicheckbox.h"
#include "uiimpfactory.h"
// std
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

#ifdef PSY_USE_PLATFORM_CHECKBOX

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
	self->imp = psy_ui_impfactory_allocinit_checkboximp(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_checkbox_base(self), parent,
		&self->imp->component_imp);
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
	self->imp = psy_ui_impfactory_allocinit_checkboximp_multiline(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_checkbox_base(self), parent,
		&self->imp->component_imp);
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
	self->imp->vtable->dev_settext(self->imp, self->translation);
}

const char* psy_ui_checkbox_text(psy_ui_CheckBox* self)
{	
	return self->text;
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	if (!psy_ui_checkbox_checked(self)) {
		self->imp->vtable->dev_check(self->imp);
	}
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	if (psy_ui_checkbox_checked(self)) {
		self->imp->vtable->dev_disablecheck(self->imp);
	}
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
		
		
		if (self->multiline) {
			psy_ui_Size preferredsize;
			psy_ui_TextMetric tm;

			tm = psy_ui_component_textmetric(&self->component);			
			preferredsize = self->imp->component_imp.vtable->dev_preferredsize(&self->imp->component_imp,
				limit);
			//rv->width = limit->width;
			rv->height = preferredsize.height;
		} else {
			psy_ui_TextMetric tm;

			tm = psy_ui_component_textmetric(&self->component);				
			size = psy_ui_component_textsize(&self->component,
				self->translation);
				rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, &tm) + 20);
			rv->height = size.height;
		}
	}
}

void psy_ui_checkbox_onlanguagechanged(psy_ui_CheckBox* self)
{
	assert(self);

	psy_strreset(&self->translation, psy_ui_translate(self->text));
	self->imp->vtable->dev_settext(self->imp, self->translation);
	psy_ui_component_invalidate(&self->component);
}

#endif

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

#ifndef PSY_USE_PLATFORM_CHECKBOX

static void psy_ui_checkbox_ondestroy(psy_ui_CheckBox*, psy_ui_Component*);
static void psy_ui_checkbox_ondraw(psy_ui_CheckBox*,
	psy_ui_Graphics*);
static void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void psy_ui_checkbox_onmousedown(psy_ui_CheckBox*, psy_ui_MouseEvent*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_CheckBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)
			psy_ui_checkbox_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			psy_ui_checkbox_onmousedown;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			psy_ui_checkbox_onpreferredsize;
		vtable_initialized = 1;
	}
}

void psy_ui_checkbox_init(psy_ui_CheckBox* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->text = strdup("");
	self->translation = NULL;
	self->state = 0;
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

void psy_ui_checkbox_ondraw(psy_ui_CheckBox* self, psy_ui_Graphics* g)
{
	psy_ui_IntSize size;
	psy_ui_IntSize checksize;
	psy_ui_TextMetric tm;
	psy_ui_Rectangle r;


	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(psy_ui_component_size(&self->component),
		&tm);
	checksize = psy_ui_intsize_make(15, tm.tmHeight);
	r = psy_ui_rectangle_make(0, (size.height - checksize.height) / 2, checksize.width,
		checksize.height);
	if (self->state == 0) {
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00444444));
	} else {
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00999999));
	}
	psy_ui_textout(g, r.right + 5, (size.height - tm.tmHeight) / 2,
		self->text, strlen(self->text));
}

void psy_ui_checkbox_settext(psy_ui_CheckBox* self, const char* text)
{
	char* temp;

	temp = self->text;
	self->text = strdup(text);
	free(temp);	
}

const char* psy_ui_checkbox_text(psy_ui_CheckBox* self)
{
	return self->text;	
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	self->state = 1;
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	self->state = 0;
	psy_ui_component_invalidate(&self->component);
}

int psy_ui_checkbox_checked(psy_ui_CheckBox* self)
{
	return self->state;	
}

void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (rv) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		char text[256];

		psy_ui_checkbox_text(self, text);
		size = psy_ui_component_textsize(&self->component, text);
		rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, &tm) + 20);
		rv->height = size.height;
	}
}

void psy_ui_checkbox_onmousedown(psy_ui_CheckBox* self, psy_ui_MouseEvent* ev)
{
	if (self->state == 0) {
		self->state = 1;
	} else {
		self->state = 0;
	}
	psy_ui_component_invalidate(&self->component);
	psy_signal_emit(&self->signal_clicked, self, 0);
}

#endif
