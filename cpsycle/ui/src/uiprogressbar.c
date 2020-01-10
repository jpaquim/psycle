// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiprogressbar.h"
#include <string.h>

static void ondestroy(ui_progressbar*, psy_ui_Component* sender);
static void ondraw(ui_progressbar*, psy_ui_Component* sender, psy_ui_Graphics*);
static void preferredsize(ui_progressbar*, ui_size* limit, ui_size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(ui_progressbar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.preferredsize = (psy_ui_fp_preferredsize) preferredsize;
	}
}

void ui_progressbar_init(ui_progressbar* self, psy_ui_Component* parent)
{	
	self->text = _strdup("");
	self->progress = 0.f;
	ui_component_init(&self->component, parent);	
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&self->component.signal_draw, self, ondraw);
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);	
}

void ondestroy(ui_progressbar* self, psy_ui_Component* sender)
{	
	free(self->text);	
}

void ondraw(ui_progressbar* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	ui_size size;
	ui_rectangle r;
	int height;
		
	height = 10;
	size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 
		(size.height - height) / 2, 
		(int)(self->progress * size.width),
		height);
	ui_drawsolidrectangle(g, r, 0x00D1C5B6);
}

void preferredsize(ui_progressbar* self, ui_size* limit, ui_size* rv)
{		
	if (rv) {
		rv->width = 100;
		rv->height = 20;
	}
}

void ui_progressbar_setprogress(ui_progressbar* self, float progress)
{
	self->progress = progress;
	ui_component_invalidate(&self->component);
	ui_component_update(&self->component);	
}

void ui_progressbar_tick(ui_progressbar* self)
{
	self->progress += 0.01f;
	ui_component_invalidate(&self->component);
	ui_component_update(&self->component);	
}