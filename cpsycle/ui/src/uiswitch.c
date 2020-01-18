// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_onmousedown) onmousedown;
		vtable.onmouseenter = (psy_ui_fp_onmouseenter) onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_onmouseleave) onmouseleave;
		vtable_initialized = 1;
	}
}

void psy_ui_switch_init(psy_ui_Switch* self, psy_ui_Component* parent)
{		
	ui_component_init(&self->component, parent);
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
	psy_ui_Rectangle r;
	int checked = 0;
	psy_ui_TextMetric tm;
	psy_ui_Size switchsize;
	psy_ui_Size size;
	psy_ui_Size cornersize;
	psy_ui_Size knobsize;
		
	switchsize = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	size.width = tm.tmAveCharWidth * 4;
	size.height = tm.tmHeight;
	knobsize.width = (int) (tm.tmAveCharWidth * 2);
	knobsize.height = (int) (tm.tmHeight * 0.7 + 0.5);
	cornersize.width = (int) (tm.tmAveCharWidth * 0.6);
	cornersize.height = (int) (tm.tmHeight * 0.6);
	r.left = 0;
	r.top = (switchsize.height - size.height) / 2;
	r.right = r.left + (int)(tm.tmAveCharWidth * 4.8);
	r.bottom = r.top + size.height;
	ui_setcolor(g, 0x00555555);
	ui_drawroundrectangle(g, r, cornersize);	
	if (!checked) {
		r.left = (int)(tm.tmAveCharWidth * 0.4);
		r.top = (switchsize.height - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		ui_drawsolidroundrectangle(g, r, cornersize, 0x00555555);
	} else {
		r.left = (int)(tm.tmAveCharWidth * 2);
		r.top = (switchsize.height - knobsize.height) / 2;
		r.right = r.left + (int)(tm.tmAveCharWidth * 2.5);
		r.bottom = r.top + knobsize.height;
		ui_drawsolidroundrectangle(g, r, cornersize, 0x00CACACA);
	}	
}

void onpreferredsize(psy_ui_Switch* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	if (rv) {		
		psy_ui_TextMetric tm;

		tm = ui_component_textmetric(&self->component);
		rv->height = (int) (1.5 * tm.tmHeight);
		rv->width = 4 * tm.tmAveCharWidth;
	} else {
		*rv = ui_component_size(&self->component);
	}
}

void onmousedown(psy_ui_Switch* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Switch* self)
{
	ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Switch* self)
{
	ui_component_invalidate(&self->component);
}


