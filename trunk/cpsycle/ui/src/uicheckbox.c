/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uicheckbox.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void psy_ui_checkbox_ondestroy(psy_ui_CheckBox*);
static void psy_ui_checkbox_ondraw(psy_ui_CheckBox*, psy_ui_Graphics*);
static void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void psy_ui_checkbox_onmousedown(psy_ui_CheckBox*, psy_ui_MouseEvent*);
static void psy_ui_checkbox_onlanguagechanged(psy_ui_CheckBox*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_CheckBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			psy_ui_checkbox_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			psy_ui_checkbox_ondraw;
		vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			psy_ui_checkbox_onpreferredsize;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			psy_ui_checkbox_onmousedown;
		vtable.onlanguagechanged =
			(psy_ui_fp_component_onlanguagechanged)
			psy_ui_checkbox_onlanguagechanged;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_checkbox_init(psy_ui_CheckBox* self, psy_ui_Component* parent)
{  		
	psy_ui_component_init(psy_ui_checkbox_base(self), parent, NULL);
	vtable_init(self);	
	self->text = NULL;
	self->translation = NULL;
	self->multiline = 0;
	self->state = 0;
	psy_signal_init(&self->signal_clicked);	
}

void psy_ui_checkbox_init_multiline(psy_ui_CheckBox* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(psy_ui_checkbox_base(self), parent, NULL);	
	vtable_init(self);
	self->text = NULL;
	self->translation = NULL;
	self->multiline = TRUE;	
	self->state = 0;
	psy_signal_init(&self->signal_clicked);	
}

void psy_ui_checkbox_init_text(psy_ui_CheckBox* self, psy_ui_Component* parent,
	const char* text)
{
	psy_ui_checkbox_init(self, parent);
	psy_ui_checkbox_settext(self, text);
}

void psy_ui_checkbox_ondestroy(psy_ui_CheckBox* self)
{
	free(self->text);
	free(self->translation);
	psy_signal_dispose(&self->signal_clicked);
}

void psy_ui_checkbox_settext(psy_ui_CheckBox* self, const char* text)
{
	psy_strreset(&self->text, text);
	psy_strreset(&self->translation, psy_ui_translate(self->text));
	psy_ui_component_invalidate(&self->component);
}

const char* psy_ui_checkbox_text(psy_ui_CheckBox* self)
{	
	return self->text;
}

void psy_ui_checkbox_check(psy_ui_CheckBox* self)
{
	if (self->state == 0) {
		self->state = 1;
		psy_ui_component_invalidate(&self->component);
	}
}

void psy_ui_checkbox_disablecheck(psy_ui_CheckBox* self)
{
	if (self->state) {
		self->state = 0;
		psy_ui_component_invalidate(&self->component);
	}
}

int psy_ui_checkbox_checked(psy_ui_CheckBox* self)
{
	return self->state;
}

void  psy_ui_checkbox_ondraw(psy_ui_CheckBox* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	psy_ui_RealSize checksize;
	const psy_ui_TextMetric* tm;
	psy_ui_RealRectangle r;
	char* text;

	if (self->translation) {
		text = self->translation;
	} else {
		text = self->text;
	}
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_scrollsize_px(&self->component);
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
			text, psy_strlen(text));
	}
}

void psy_ui_checkbox_onpreferredsize(psy_ui_CheckBox* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
		
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_component_textsize(&self->component, self->text);
	if (self->multiline) {
		if (!self->text) {
			*rv = psy_ui_size_zero();
			return;
		}
		rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width, tm, NULL) + 10);
		rv->height = size.height;
	} else {
		size = psy_ui_component_textsize(&self->component,
			self->translation);
		rv->width = psy_ui_value_make_px(psy_ui_value_px(&size.width,
			psy_ui_component_textmetric(&self->component), NULL) + 20);
		rv->height = size.height;
	}	
}

void psy_ui_checkbox_onlanguagechanged(psy_ui_CheckBox* self)
{
	assert(self);

	psy_strreset(&self->translation, psy_ui_translate(self->text));	
	psy_ui_component_invalidate(&self->component);
}

void psy_ui_checkbox_onmousedown(psy_ui_CheckBox* self, psy_ui_MouseEvent* ev)
{	
	if (self->state == 0) {
		self->state = 1;
	} else {
		self->state = 0;
	}
	psy_ui_component_invalidate(&self->component);
	psy_signal_emit(&self->signal_clicked, &self->component, 0);
}
