/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uinativeeditimp.h"
/* local */
#include "../../uicomponent.h"
#include "../../uiapp.h"
#include "../../uiimpfactory.h"
/* portable */
#include "../../detail/portable.h"

/* EditImp VTable */
static void dev_settext(psy_ui_native_EditImp*, const char* text);
static void dev_text(psy_ui_native_EditImp*, char* text);
static void dev_check(psy_ui_native_EditImp*);
static void dev_disablecheck(psy_ui_native_EditImp*);
static int dev_checked(psy_ui_native_EditImp*);

static psy_ui_EditImpVTable editimp_vtable;
static bool editimp_vtable_initialized = FALSE;

static void editimp_imp_vtable_init(psy_ui_native_EditImp* self)
{	
	if (!editimp_vtable_initialized) {
		editimp_vtable = *((psy_ui_EditImpVTable*)self->imp->extended_vtable);
		editimp_vtable.dev_settext =
			(psy_ui_fp_editimp_dev_settext)
			dev_settext;
		editimp_vtable.dev_text =
			(psy_ui_fp_editimp_dev_text)
			dev_text;		
		editimp_vtable_initialized = TRUE;
	}
	self->imp->extended_vtable = (void*)&editimp_vtable;
}

static psy_ui_Size dev_preferredsize(psy_ui_ComponentImp*,
	const psy_ui_Size* limits);
static void dev_draw(psy_ui_ComponentImp*, psy_ui_Graphics*);
static void dev_mousedown(psy_ui_ComponentImp*, psy_ui_MouseEvent*);
static void dev_dispose(psy_ui_ComponentImp*);
/* vtable */
static psy_ui_ComponentImpVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_native_EditImp* self)
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


void psy_ui_native_editimp_init(psy_ui_native_EditImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_editimp_extend(self->imp);	
	editimp_imp_vtable_init(self);
	self->imp->extended_imp = (void*)self;
	self->component = component;
	self->text = NULL;	
	self->multiline = 0;	
}

void psy_ui_native_editimp_init_multiline(psy_ui_native_EditImp* self,
	psy_ui_Component* component,
	psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_componentimp(
		psy_ui_app_impfactory(psy_ui_app()), component, parent);
	vtable_init(self);	
	psy_ui_editimp_extend(self->imp);
	self->imp->extended_imp = (void*)self;	
	editimp_imp_vtable_init(self);	
	self->component = component;
	self->text = NULL;
	self->multiline = 1;	
}

void dev_dispose(psy_ui_ComponentImp* context)
{
	psy_ui_native_EditImp* self;	

	self = (psy_ui_native_EditImp*)context->extended_imp;
	free(self->text);
	self->text = 0;
	free(self);
}

psy_ui_native_EditImp* psy_ui_native_editimp_alloc(void)
{
	return (psy_ui_native_EditImp*)malloc(sizeof(psy_ui_native_EditImp));
}

psy_ui_native_EditImp* psy_ui_native_editimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_EditImp* rv;

	rv = psy_ui_native_editimp_alloc();
	if (rv) {
		psy_ui_native_editimp_init(rv, component, parent);
	}
	return rv;
}

psy_ui_native_EditImp* psy_ui_native_editimp_allocinit_multiline(
	struct psy_ui_Component* component,
	psy_ui_Component* parent)
{
	psy_ui_native_EditImp* rv;

	rv = psy_ui_native_editimp_alloc();
	if (rv) {
		psy_ui_native_editimp_init_multiline(rv, component, parent);
	}
	return rv;
}

void dev_settext(psy_ui_native_EditImp* self, const char* text)
{	
	psy_strreset(&self->text, text);	
	psy_ui_component_invalidate(self->component);
}

void dev_text(psy_ui_native_EditImp* self, char* text)
{	
	if (self->text) {
		psy_snprintf(text, 256, "%s", self->text);
	} else {
		text = '\0';
	}
}

void dev_draw(psy_ui_ComponentImp* context, psy_ui_Graphics* g)
{	
	psy_ui_native_EditImp* self;
	psy_ui_RealSize size;	
	const psy_ui_TextMetric* tm;
		
	self = (psy_ui_native_EditImp*)context->extended_imp;
	psy_ui_component_draw(self->component, g, NULL);	
	tm = psy_ui_component_textmetric(self->component);
	size = psy_ui_component_scrollsize_px(self->component);	
	if (psy_strlen(self->text) > 0) {
		psy_ui_textout(g, 5, (size.height - tm->tmHeight) / 2,
			self->text, strlen(self->text));
	}
}

psy_ui_Size dev_preferredsize(psy_ui_ComponentImp* context, const psy_ui_Size* limit)
{
	psy_ui_native_EditImp* self;
	psy_ui_Size rv;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;	

	self = (psy_ui_native_EditImp*)context->extended_imp;		
	if (!self->text) {
		return psy_ui_size_zero();
	}
	tm = psy_ui_component_textmetric(self->component);
	size = psy_ui_component_textsize(self->component, self->text);
	rv.width = psy_ui_value_make_px(psy_ui_value_px(&size.width, tm, NULL) + 5);
	rv.height = size.height;
	return rv;
}

void dev_mousedown(psy_ui_ComponentImp* context, psy_ui_MouseEvent* ev)
{
	psy_ui_native_EditImp* self;

	self = (psy_ui_native_EditImp*)context->extended_imp;	
	psy_ui_component_invalidate(self->component);	
}
