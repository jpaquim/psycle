// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uilabel.h"
#include "uiwincompdetail.h"

static void onpreferredsize(psy_ui_Label*, psy_ui_Size* limit, psy_ui_Size* rv);
static psy_ui_TextMetric textmetric(psy_ui_Component*);

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
	psy_ui_win32_component_init(psy_ui_label_base(self), parent, TEXT("STATIC"), 
		0, 0, 100, 20,
		WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
		0);	
	vtable_init(self);
	self->component.vtable = &vtable;
	self->charnumber = 0;	
}

void psy_ui_label_settext(psy_ui_Label* self, const char* text)
{
	SetWindowText((HWND)psy_ui_win_component_details(&self->component)->hwnd,
		text);
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
			GetWindowText((HWND) psy_ui_win_component_details(&self->component)->hwnd, text, 256);
			size = psy_ui_component_textsize(psy_ui_label_base(self), text);
			rv->width = size.width + 2 +
				psy_ui_margin_width_px(&psy_ui_label_base(self)->spacing, &tm);
		} else {		
			rv->width = tm.tmAveCharWidth * self->charnumber;
		}
		rv->height = tm.tmHeight +
			psy_ui_margin_height_px(&psy_ui_label_base(self)->spacing, &tm);
	}
}

void psy_ui_label_setstyle(psy_ui_Label* self, int style)
{
	#if defined(_WIN64)
	SetWindowLongPtr((HWND)psy_ui_win_component_details(&self->component)->hwnd, GWL_STYLE, style);		
#else
	SetWindowLong((HWND)psy_ui_win_component_details(&self->component)->hwnd, GWL_STYLE, style);
#endif
}

psy_ui_Component* psy_ui_label_base(psy_ui_Label* self)
{
	return &self->component;
}


