// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiknob.h"

#include <stdlib.h>
#include <string.h>

static void ondestroy(psy_ui_Knob*, psy_ui_Component* sender);
static void ondraw(psy_ui_Knob*, psy_ui_Graphics*);
static void onmousedown(psy_ui_Knob*, psy_ui_MouseEvent*);
static void onmouseenter(psy_ui_Knob*);
static void onmouseleave(psy_ui_Knob*);
static void onpreferredsize(psy_ui_Knob*, psy_ui_Size* limit,
	psy_ui_Size* size);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_Knob* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw) ondraw;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize) onpreferredsize;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown) onmousedown;
		vtable.onmouseenter = (psy_ui_fp_component_onmouseenter) onmouseenter;
		vtable.onmouseleave = (psy_ui_fp_component_onmouseleave) onmouseleave;
		vtable_initialized = 1;
	}
}

void psy_ui_knob_init(psy_ui_Knob* self, psy_ui_Component* parent)
{		
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->bitmap = 0;
	self->label = strdup("");
	self->desc = strdup("");
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

void psy_ui_knob_setbitmap(psy_ui_Knob* self, psy_ui_Bitmap* bitmap)
{
	self->bitmap = bitmap;
}

void psy_ui_knob_setlabel(psy_ui_Knob* self, const char* text)
{
	free(self->label);
	self->label = strdup(text);
}

void psy_ui_knob_setdesc(psy_ui_Knob* self, const char* text)
{
	free(self->desc);
	self->desc = strdup(text);
}

void ondraw(psy_ui_Knob* self, psy_ui_Graphics* g)
{
	if (self->bitmap) {
		psy_ui_drawbitmap(g, self->bitmap, 0, 0, 28, 28, 0, 0);
	}
	psy_ui_settextcolour(g, psy_ui_colour_make(0x00CACACA));
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_textout(g, 30, 0, self->label, strlen(self->label));
	psy_ui_textout(g, 30, 28 / 2, self->desc, strlen(self->desc));
}

void onpreferredsize(psy_ui_Knob* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	if (rv) {		
		rv->width = psy_ui_value_makepx(28 + 50);
		rv->height = psy_ui_value_makepx(28);
	} else {
		*rv = psy_ui_component_size(&self->component);
	}
}

void onmousedown(psy_ui_Knob* self, psy_ui_MouseEvent* ev)
{
	// psy_signal_emit(&self->signal_clicked, self, 0);
}

void onmouseenter(psy_ui_Knob* self)
{
//	self->hover = 1;
	psy_ui_component_invalidate(&self->component);
}

void onmouseleave(psy_ui_Knob* self)
{		
//	self->hover = 0;
	psy_ui_component_invalidate(&self->component);
}
