// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiswitch.h"
#include <string.h>

static void ondestroy(ui_switch*, ui_component* sender);
static void ondraw(ui_switch*, ui_component* sender, ui_graphics*);
static void onmousedown(ui_switch*, ui_component* sender);
static void onmouseenter(ui_switch*, ui_component* sender);
static void onmouseleave(ui_switch*, ui_component* sender);
static void onpreferredsize(ui_switch*, ui_component* sender, ui_size* limit,
	ui_size* size);

void ui_switch_init(ui_switch* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, ondraw);
	signal_connect(&self->component.signal_mousedown, self, onmousedown);
	signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
	signal_init(&self->signal_clicked);	
	signal_connect(&self->component.signal_destroy, self, ondestroy);	
	signal_disconnectall(&self->component.signal_preferredsize);
	signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
}

void ondestroy(ui_switch* self, ui_component* sender)
{	
	signal_dispose(&self->signal_clicked);	
}

void ondraw(ui_switch* self, ui_component* sender, ui_graphics* g)
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

void onpreferredsize(ui_switch* self, ui_component* sender, ui_size* limit,
	ui_size* rv)
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

void onmousedown(ui_switch* self, ui_component* sender)
{
	signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(ui_switch* self, ui_component* sender)
{
//	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void onmouseleave(ui_switch* self, ui_component* sender)
{		
//	self->hover = 0;
	ui_component_invalidate(&self->component);
}


