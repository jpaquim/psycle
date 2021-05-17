// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"


#include "uinativecheckboximp.h"
/* local */
#include "../../uicomponent.h"
#include "../../uiapp.h"
#include "../../uiimpfactory.h"
/* portable */
#include "../../detail/portable.h"

// CheckBoxImp VTable
static void dev_settext(psy_ui_native_CheckBoxImp*, const char* text);
static void dev_text(psy_ui_native_CheckBoxImp*, char* text);
static void dev_check(psy_ui_native_CheckBoxImp*);
static void dev_disablecheck(psy_ui_native_CheckBoxImp*);
static int dev_checked(psy_ui_native_CheckBoxImp*);

static psy_ui_CheckBoxImpVTable checkboximp_vtable;
static bool checkboximp_vtable_initialized = FALSE;

static void checkboximp_imp_vtable_init(psy_ui_native_CheckBoxImp* self)
{	
	if (!checkboximp_vtable_initialized) {
		checkboximp_vtable = *((psy_ui_CheckBoxImpVTable*)self->imp->extended_vtable);		
		checkboximp_vtable.dev_settext =
			(psy_ui_fp_checkboximp_dev_settext)
			dev_settext;
		checkboximp_vtable.dev_text =
			(psy_ui_fp_checkboximp_dev_text)
			dev_text;
		checkboximp_vtable.dev_check =
			(psy_ui_fp_checkboximp_dev_check)
			dev_check;
		checkboximp_vtable.dev_disablecheck =
			(psy_ui_fp_checkboximp_dev_disablecheck)
			dev_disablecheck;
		checkboximp_vtable.dev_checked =
			(psy_ui_fp_checkboximp_dev_checked)
			dev_checked;
		checkboximp_vtable_initialized = TRUE;
	}
	self->imp->extended_vtable = (void*)&checkboximp_vtable;
}

static psy_ui_Size dev_preferredsize(psy_ui_ComponentImp*,
	const psy_ui_Size* limits);
static void dev_draw(psy_ui_ComponentImp*, psy_ui_Graphics*);
static void dev_mousedown(psy_ui_ComponentImp*, psy_ui_MouseEvent*);
static void dev_dispose(psy_ui_ComponentImp*);
// vtable
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_native_CheckBoxImp* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp->vtable;
		vtable.dev_dispose =
			(psy_ui_fp_componentimp_dev_dispose)
			dev_dispose;
		vtable.dev_preferredsize =
			(psy_ui_fp_componentimp_dev_preferredsize)
			dev_preferredsize;
		vtable.dev_draw =
			(psy_ui_fp_componentimp_dev_draw)
			dev_draw;
		vtable.dev_mousedown =
			(psy_ui_fp_componentimp_dev_mouseevent)
			dev_mousedown;
		vtable_initialized = TRUE;
	}
	self->imp->vtable = &vtable;
}


void psy_ui_native_checkboximp_init(psy_ui_native_CheckBoxImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_checkboximp_extend(self->imp);	
	checkboximp_imp_vtable_init(self);
	self->imp->extended_imp = (void*)self;
	self->component = component;
	self->text = NULL;	
	self->multiline = 0;
	self->state = 0;
}

void psy_ui_native_checkboximp_init_multiline(psy_ui_native_CheckBoxImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_checkboximp_extend(self->imp);
	self->imp->extended_imp = (void*)self;	
	checkboximp_imp_vtable_init(self);	
	self->component = component;
	self->text = NULL;
	self->multiline = 1;
	self->state = 0;
}

void dev_dispose(psy_ui_ComponentImp* context)
{
	psy_ui_native_CheckBoxImp* self;	

	self = (psy_ui_native_CheckBoxImp*)context->extended_imp;
	free(self->text);
	self->text = 0;
	free(self);
}

psy_ui_native_CheckBoxImp* psy_ui_native_checkboximp_alloc(void)
{
	return (psy_ui_native_CheckBoxImp*) malloc(sizeof(psy_ui_native_CheckBoxImp));
}

psy_ui_native_CheckBoxImp* psy_ui_native_checkboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_CheckBoxImp* rv;

	rv = psy_ui_native_checkboximp_alloc();
	if (rv) {
		psy_ui_native_checkboximp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_native_CheckBoxImp* psy_ui_native_checkboximp_allocinit_multiline(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_CheckBoxImp* rv;

	rv = psy_ui_native_checkboximp_alloc();
	if (rv) {
		psy_ui_native_checkboximp_init_multiline(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_native_CheckBoxImp* self, const char* text)
{	
	psy_strreset(&self->text, text);	
	psy_ui_component_invalidate(self->component);
}

void dev_text(psy_ui_native_CheckBoxImp* self, char* text)
{	
	psy_snprintf(self->text, 256, "%s", text);
}

void dev_check(psy_ui_native_CheckBoxImp* self)
{
	if (self->state == 0) {
		self->state = 1;
		psy_ui_component_invalidate(self->component);
	}
}

void dev_disablecheck(psy_ui_native_CheckBoxImp* self)
{
	if (self->state) {
		self->state = 0;
		psy_ui_component_invalidate(self->component);
	}
}

int dev_checked(psy_ui_native_CheckBoxImp* self)
{
	return self->state;
}

void dev_draw(psy_ui_ComponentImp* context, psy_ui_Graphics* g)
{	
	psy_ui_native_CheckBoxImp* self;
	psy_ui_RealSize size;
	psy_ui_RealSize checksize;
	const psy_ui_TextMetric* tm;
	psy_ui_RealRectangle r;
		
	self = (psy_ui_native_CheckBoxImp*)context->extended_imp;
	psy_ui_component_draw(self->component, g, NULL);	
	tm = psy_ui_component_textmetric(self->component);
	size = psy_ui_component_scrollsize_px(self->component);
	checksize = psy_ui_realsize_make(15.0, tm->tmHeight);
	r = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, (size.height - checksize.height) / 2),
		checksize);
	if (self->state == 0) {
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00444444));
	} else {
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00999999));
	}
	if (psy_strlen(self->text) > 0) {
		psy_ui_textout(g, r.right + 5, (size.height - tm->tmHeight) / 2,
			self->text, strlen(self->text));
	}
}

psy_ui_Size dev_preferredsize(psy_ui_ComponentImp* context, const psy_ui_Size* limit)
{
	psy_ui_native_CheckBoxImp* self;
	psy_ui_Size rv;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;	

	self = (psy_ui_native_CheckBoxImp*)context->extended_imp;		
	if (!self->text) {
		return psy_ui_size_zero();
	}
	tm = psy_ui_component_textmetric(self->component);
	size = psy_ui_component_textsize(self->component, self->text);
	rv.width = psy_ui_value_make_px(psy_ui_value_px(&size.width, tm, NULL) + 20);
	rv.height = size.height;
	return rv;
}

void dev_mousedown(psy_ui_ComponentImp* context, psy_ui_MouseEvent* ev)
{
	psy_ui_native_CheckBoxImp* self;

	self = (psy_ui_native_CheckBoxImp*)context->extended_imp;
	if (self->state == 0) {
		self->state = 1;
	} else {
		self->state = 0;
	}
	psy_ui_component_invalidate(self->component);
	psy_signal_emit(&((psy_ui_CheckBox*)(self->component))->signal_clicked,
		self->component, 0);
}
