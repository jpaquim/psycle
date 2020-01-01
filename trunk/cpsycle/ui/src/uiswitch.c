// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiswitch.h"
#include <string.h>

static void ondestroy(psy_ui_Switch*, psy_ui_Component* sender);
static void ondraw(psy_ui_Switch*, psy_ui_Component* sender, psy_ui_Graphics*);
static void onmousedown(psy_ui_Switch*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Switch*, psy_ui_Component* sender);
static void onmouseleave(psy_ui_Switch*, psy_ui_Component* sender);
static void preferredsize(psy_ui_Switch*, ui_size* limit, ui_size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Switch* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.preferredsize = (psy_ui_fp_preferredsize) preferredsize;
	}
}

void ui_switch_init(psy_ui_Switch* self, psy_ui_Component* parent)
{		
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&self->component.signal_draw, self, ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self, onmousedown);
	psy_signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
	psy_signal_init(&self->signal_clicked);	
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
}

void ondestroy(psy_ui_Switch* self, psy_ui_Component* sender)
{	
	psy_signal_dispose(&self->signal_clicked);	
}

void ondraw(psy_ui_Switch* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	ui_rectangle r;
	int checked = 0;
	ui_textmetric tm;
	ui_size switchsize;
	ui_size size;
	ui_size cornersize;
	ui_size knobsize;
		
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

void preferredsize(psy_ui_Switch* self, ui_size* limit, ui_size* rv)
{		
	if (rv) {		
		ui_textmetric tm;		

		tm = ui_component_textmetric(&self->component);
		rv->height = (int) (1.5 * tm.tmHeight);
		rv->width = 4 * tm.tmAveCharWidth;
	} else {
		*rv = ui_component_size(&self->component);
	}
}

void onmousedown(psy_ui_Switch* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Switch* self, psy_ui_Component* sender)
{
//	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Switch* self, psy_ui_Component* sender)
{		
//	self->hover = 0;
	ui_component_invalidate(&self->component);
}


