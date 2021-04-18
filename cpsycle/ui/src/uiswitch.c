// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiswitch.h"
#include <string.h>

static void ondestroy(psy_ui_Switch*, psy_ui_Component* sender);
static void ondraw(psy_ui_Switch*, psy_ui_Graphics*);
static void onmousedown(psy_ui_Switch*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Switch*);
static void onmouseleave(psy_ui_Switch*);
static void onpreferredsize(psy_ui_Switch*, psy_ui_Size* limit, psy_ui_Size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Switch* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)onmousedown;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter)onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave)onmouseleave;
		vtable_initialized = 1;
	}
}

void psy_ui_switch_init(psy_ui_Switch* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;		
	psy_signal_init(&self->signal_clicked);	
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
}

void ondestroy(psy_ui_Switch* self, psy_ui_Component* sender)
{	
	psy_signal_dispose(&self->signal_clicked);	
}

void ondraw(psy_ui_Switch* self, psy_ui_Graphics* g)
{
	/* psy_ui_RealRectangle r;
	int checked = 0;
	psy_ui_TextMetric tm;
	psy_ui_Size switchsize;
	psy_ui_Size size;
	psy_ui_Size cornersize;
	psy_ui_Size knobsize;
		
	switchsize = psy_ui_component_offsetsize(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	size.width = psy_ui_value_makeew(4);
	size.height = psy_ui_value_makeeh(1);
	knobsize.width = psy_ui_value_makeew(2);
	knobsize.height = psy_ui_value_makeeh(4); (int)(tm.tmHeight * 0.7 + 0.5);
	cornersize.width = (int) (tm.tmAveCharWidth * 0.6);
	cornersize.height = (int) (tm.tmHeight * 0.6);
	r.left = 0;
	r.top = (switchsize.height - size.height) / 2;
	r.right = r.left + (int)(tm.tmAveCharWidth * 4.8);
	r.bottom = r.top + size.height;
	psy_ui_setcolour(g, 0x00555555);
	psy_ui_drawroundrectangle(g, r, cornersize);	
	if (!checked) {
		r.left = (int)(tm.tmAveCharWidth * 0.4);
		r.top = (switchsize.height - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		psy_ui_drawsolidroundrectangle(g, r, cornersize, 0x00555555);
	} else {
		r.left = (int)(tm.tmAveCharWidth * 2);
		r.top = (switchsize.height - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		psy_ui_drawsolidroundrectangle(g, r, cornersize, 0x00CACACA);
	}*/	
}

void onpreferredsize(psy_ui_Switch* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	if (rv) {				
		rv->height = psy_ui_value_makeeh(1.5);
		rv->width = psy_ui_value_makeew(4);
	}
}

void onmousedown(psy_ui_Switch* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Switch* self)
{
	psy_ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Switch* self)
{
	psy_ui_component_invalidate(&self->component);
}


