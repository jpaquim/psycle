// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiknob.h"
#include <string.h>

static void ondestroy(psy_ui_Knob*, psy_ui_Component* sender);
static void ondraw(psy_ui_Knob*, psy_ui_Graphics*);
static void onmousedown(psy_ui_Knob*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Knob*, psy_ui_Component* sender);
static void onmouseleave(psy_ui_Knob*, psy_ui_Component* sender);
static void preferredsize(psy_ui_Knob*, ui_size* limit, ui_size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Knob* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.draw = (psy_ui_fp_draw) ondraw;
		vtable.preferredsize = (psy_ui_fp_preferredsize) preferredsize;
	}
}

void ui_knob_init(psy_ui_Knob* self, psy_ui_Component* parent)
{		
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->bitmap = 0;
	self->label = strdup("");
	self->desc = strdup("");
	self->component.vtable = &vtable;
	self->component.vtable = &vtable;	
	psy_signal_connect(&self->component.signal_mousedown, self, onmousedown);
	psy_signal_connect(&self->component.signal_mouseenter, self, onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self, onmouseleave);
	psy_signal_init(&self->signal_tweak);	
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
}

void ondestroy(psy_ui_Knob* self, psy_ui_Component* sender)
{	
	psy_signal_dispose(&self->signal_tweak);
	free(self->label);
	free(self->desc);
}

void ui_knob_setbitmap(psy_ui_Knob* self, psy_ui_Bitmap* bitmap)
{
	self->bitmap = bitmap;
}

void ui_knob_setlabel(psy_ui_Knob* self, const char* text)
{
	free(self->label);
	self->label = strdup(text);
}

void ui_knob_setdesc(psy_ui_Knob* self, const char* text)
{
	free(self->desc);
	self->desc = strdup(text);
}

void ondraw(psy_ui_Knob* self, psy_ui_Graphics* g)
{
	if (self->bitmap) {
		ui_drawbitmap(g, self->bitmap, 0, 0, 28, 28, 0, 0);
	}
	ui_settextcolor(g, 0x00CACACA);
	ui_setbackgroundmode(g, TRANSPARENT);
	ui_textout(g, 30, 0, self->label, strlen(self->label));
	ui_textout(g, 30, 28 / 2, self->desc, strlen(self->desc));
}

void preferredsize(psy_ui_Knob* self, ui_size* limit, ui_size* rv)
{		
	if (rv) {		
		rv->width = 28 + 50;
		rv->height = 28;
	} else {
		*rv = ui_component_size(&self->component);
	}
}

void onmousedown(psy_ui_Knob* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	// psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Knob* self, psy_ui_Component* sender)
{
//	self->hover = 1;
	ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Knob* self, psy_ui_Component* sender)
{		
//	self->hover = 0;
	ui_component_invalidate(&self->component);
}


