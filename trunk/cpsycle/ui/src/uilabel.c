// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilabel.h"
#include "uiapp.h"
#include "uiimpfactory.h"

extern psy_ui_App app;

static void onpreferredsize(psy_ui_Label*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Label* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
		vtable_initialized = 1;
	}
}

void psy_ui_label_init(psy_ui_Label* self, psy_ui_Component* parent)
{  		
	self->imp = psy_ui_impfactory_allocinit_labelimp(psy_ui_app_impfactory(&app), &self->component, parent);
	psy_ui_component_init_imp(psy_ui_label_base(self), parent,
		&self->imp->component_imp);
	vtable_init(self);
	self->component.vtable = &vtable;	
	self->charnumber = 0;	
}

void psy_ui_label_settext(psy_ui_Label* self, const char* text)
{
	self->imp->vtable->dev_settext(self->imp, text);
}

void psy_ui_label_text(psy_ui_Label* self, char* text)
{
	self->imp->vtable->dev_text(self->imp, text);
}

void psy_ui_label_settextalignment(psy_ui_Label* self, psy_ui_Alignment alignment)
{
	self->imp->vtable->dev_settextalignment(self->imp, alignment);
}

void psy_ui_label_setcharnumber(psy_ui_Label* self, int number)
{
	self->charnumber = number;
}

void onpreferredsize(psy_ui_Label* self, psy_ui_Size* limit, psy_ui_Size* rv)
{	
	if (rv) {
		psy_ui_TextMetric tm;	
		char text[256];
				
		tm = psy_ui_component_textmetric(psy_ui_label_base(self));	
		if (self->charnumber == 0) {
			psy_ui_Size size;

			psy_ui_label_text(self, text);			
			size = psy_ui_component_textsize(psy_ui_label_base(self), text);
			rv->width = psy_ui_value_makepx(psy_ui_value_px(&size.width, &tm) + 2 +
				psy_ui_margin_width_px(&psy_ui_label_base(self)->spacing, &tm));
		} else {		
			rv->width = psy_ui_value_makepx(tm.tmAveCharWidth * self->charnumber);
		}
		rv->height = psy_ui_value_makepx(tm.tmHeight +
			psy_ui_margin_height_px(&psy_ui_label_base(self)->spacing, &tm));
	}
}

void psy_ui_label_setstyle(psy_ui_Label* self, int style)
{	
	self->imp->vtable->dev_setstyle(self->imp, style);
}

// psy_ui_LabelImp vtable
static void dev_settext(psy_ui_LabelImp* self, const char* title) { }
static void dev_text(psy_ui_LabelImp* self, char* text) { }
static void dev_setstyle(psy_ui_LabelImp* self, int style) { }
static void dev_settextalignment(psy_ui_LabelImp* self, psy_ui_Alignment alignment) { }

static psy_ui_LabelImpVTable label_imp_vtable;
static int label_imp_vtable_initialized = 0;

static void label_imp_vtable_init(void)
{
	if (!label_imp_vtable_initialized) {		
		label_imp_vtable.dev_settext = dev_settext;
		label_imp_vtable.dev_text = dev_text;
		label_imp_vtable.dev_setstyle = dev_setstyle;	
		label_imp_vtable.dev_settextalignment = dev_settextalignment;
		label_imp_vtable_initialized = 1;
	}
}

void psy_ui_labelimp_init(psy_ui_LabelImp* self)
{
	label_imp_vtable_init();
	self->vtable = &label_imp_vtable;
}

